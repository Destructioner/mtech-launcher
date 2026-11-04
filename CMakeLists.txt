cmake_minimum_required(VERSION 3.16)

project(mtech_launcher VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================
# Поиск Qt
# ============================================================
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools Network Concurrent)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools Network Concurrent)

# ============================================================
# 1. Все исходники (включая .cpp, .h, .ui)
# ============================================================
set(PROJECT_SOURCES
    main.cpp
    mtech_launcher.cpp
    mtech_launcher.h
    mtech_launcher.ui
    register_user.ui
    register_user.cpp
    register_user.h
    http_requests.h
    hwid_pc.h
    minecraft_client_manager.h
)

if(WIN32)
    list(APPEND PROJECT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/app_ico.rc)
endif()

# ============================================================
# 2. Явная генерация moc для minecraft_client_manager.h
#    (гарантированно, независимо от AUTOMOC)
# ============================================================
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt6_wrap_cpp(moc_SOURCES minecraft_client_manager.h)
else()
    qt5_wrap_cpp(moc_SOURCES minecraft_client_manager.h)
endif()
list(APPEND PROJECT_SOURCES ${moc_SOURCES})   # добавляем сгенерированный файл

# ============================================================
# 3. Создание исполняемого файла
# ============================================================
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(mtech_launcher
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}                # все исходники + moc_*.cpp
        background_launcher.qrc
        minecraft_client_manager.cpp
        auth_minecraft_server.h auth_minecraft_server.cpp           # ресурсы
    )
else()
    if(ANDROID)
        add_library(mtech_launcher SHARED ${PROJECT_SOURCES})
    else()
        add_executable(mtech_launcher ${PROJECT_SOURCES})
    endif()
endif()

# ============================================================
# 4. Линковка с Qt и системными библиотеками
# ============================================================
target_link_libraries(mtech_launcher PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
	Qt${QT_VERSION_MAJOR}::Concurrent
    iphlpapi
    ws2_32
)

# ============================================================
# 5. Настройки для macOS / iOS (если нужно)
# ============================================================
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.mtech_launcher)
endif()
set_target_properties(mtech_launcher PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# ============================================================
# 6. Установка
# ============================================================
include(GNUInstallDirs)
install(TARGETS mtech_launcher
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Финализация для Qt6
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(mtech_launcher)
endif()

# ============================================================
# 7. QuaZip – пути, диагностика, линковка, копирование DLL
# ============================================================
set(QUAZIP_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include/quazip)
set(QUAZIP_LIB_DIR ${CMAKE_SOURCE_DIR}/lib)
set(QUAZIP_DLL_DIR ${CMAKE_SOURCE_DIR}/bin)

# Диагностика: проверяем, что заголовок quazip.h существует
if(EXISTS ${QUAZIP_INCLUDE_DIR}/quazip.h)
    message(STATUS "QuaZip header found at ${QUAZIP_INCLUDE_DIR}/quazip.h")
else()
    message(FATAL_ERROR "QuaZip header NOT found at ${QUAZIP_INCLUDE_DIR}/quazip.h\n"
                        "Please place QuaZip headers in 'include/quazip' or adjust QUAZIP_INCLUDE_DIR.")
endif()

# Подключаем заголовки
target_include_directories(mtech_launcher PRIVATE ${QUAZIP_INCLUDE_DIR})

# Указываем путь к библиотеке
target_link_directories(mtech_launcher PRIVATE ${QUAZIP_LIB_DIR})

# Линкуем библиотеку (имя без префикса lib и суффикса .a)
target_link_libraries(mtech_launcher PRIVATE quazip1-qt6)

# Копируем DLL в папку с исполняемым файлом после сборки
add_custom_command(TARGET mtech_launcher POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${QUAZIP_DLL_DIR}/libquazip1-qt6.dll
        $<TARGET_FILE_DIR:mtech_launcher>
    COMMENT "Copying QuaZip DLL to output directory"
)