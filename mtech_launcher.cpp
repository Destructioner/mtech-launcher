#include "mtech_launcher.h"
#include "./ui_mtech_launcher.h"


launch_mtech::launch_mtech(QWidget *parent): QMainWindow(parent), ui_launcher(new Ui::launcher_s5lab), thread_control{QThread::idealThreadCount()} {
    ui_launcher->setupUi(this);
}

launch_mtech::~launch_mtech() {
    delete ui_launcher;
}

void launch_mtech::on_register_2_clicked() {
    ui_reg_form.show();
}

void launch_mtech::on_pushButton_clicked() {
    QString auth_file_path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "\\account_data.auth";
    QFile::remove(auth_file_path);
}

qint64 launch_mtech::available_ram() {
    MEMORYSTATUSEX memory_data;

    memory_data.dwLength = sizeof(memory_data);

    GlobalMemoryStatusEx(&memory_data);
    return memory_data.ullTotalPhys;
}

bool launch_mtech::check_files_fdownload() {
    QDir minecraft_dir;

    if (! minecraft_dir.exists(".\\minecraft")) {
        return false;
    }


    if (! minecraft_dir.exists(".\\minecraft\\versions")) {
        return false;
    }

    else if (! minecraft_dir.exists(".\\minecraft\\libraries")) {
        return false;
    }

    else if (! minecraft_dir.exists(".\\minecraft\\assets")) {
        return false;
    }

    else if (! minecraft_dir.exists(".\\minecraft\\mods")) {
        return false;
    }

    return true;
}

void launch_mtech::auth_client(QString *nick, authenfication::auth_minecraft_server *auth_launch) {
    QString login_nickname;
    QString password_account;

    QString roamingPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir check_dir(roamingPath);

    if (check_dir.exists()) {
        QFile file_user_data(static_cast<QString>(roamingPath + "\\account_data.auth"));

        if (file_user_data.open(QIODevice::ReadOnly)) {
            if (file_user_data.size() != 0) {
                QTextStream data_user_file(&file_user_data);

                login_nickname = data_user_file.readLine();
                password_account = data_user_file.readLine();

                *nick = login_nickname;

                file_user_data.close();
            }

        }


        else {
            file_user_data.close();
        }
    }

    auth_launch->set_form(login_nickname, password_account);

    auth_launch->auth_server();

}





void launch_mtech::on_play_minecraft_clicked() {

    if (! check_files_fdownload()) {
        QProgressBar *game_state = new QProgressBar(this);

        game_state->setRange(0, 5154);
        game_state->setValue(0);
        game_state->setGeometry(10, 410, 780, 30);
        game_state->setTextVisible(false);
        game_state->show();

        QTimer *timer_update = new QTimer;

        downloader::MinecraftDownloader *down_game = new downloader::MinecraftDownloader;

        timer_update->start(1000);

        QObject::connect(down_game, &downloader::MinecraftDownloader::new_file_num, [down_game, game_state] (int file_num) {
            game_state->setValue(file_num);
        });

        QObject::connect(down_game, &downloader::MinecraftDownloader::success_downloaded, this, [this, timer_update, down_game, game_state] () {
            QMessageBox::information(this, "Успешно", "Загрузка завершена");
            timer_update->stop();

            down_game->deleteLater();
            game_state->deleteLater();
            timer_update->deleteLater();
        }, Qt::QueuedConnection);

        QObject::connect(down_game, &downloader::MinecraftDownloader::error_network, this, [this, timer_update, down_game, game_state] () {
            QMessageBox::warning(this, "Ошибка", "Ошибка сети");
            timer_update->stop();

            down_game->deleteLater();
            game_state->deleteLater();
            timer_update->deleteLater();
        }, Qt::QueuedConnection);


        static_cast<void>(QtConcurrent::run(QThreadPool::globalInstance(), [=]() {
            down_game->download_assets();
            down_game->download_client();
            down_game->download_fabric();
            down_game->download_mods_google_disk();
        }));
    }

    else {
        authenfication::auth_minecraft_server *auth_launcher = new authenfication::auth_minecraft_server;
        QString *nickname = new QString;

        QObject::connect(auth_launcher, &authenfication::auth_minecraft_server::auth_success, this, [this, auth_launcher, nickname] () {
            QString minecraftDir = QCoreApplication::applicationDirPath() + "/minecraft";
            QStringList libraries_jars;
            QDirIterator dir_lib(minecraftDir + "/libraries", {"*.jar"}, QDir::Files, QDirIterator::Subdirectories);

            while (dir_lib.hasNext()) {
                libraries_jars << dir_lib.next();
            }

            QString classpath = libraries_jars.join(";");
            QFile classpath_file("classpath.txt");

            if (classpath_file.open(QIODevice::WriteOnly)) {
                classpath_file.write(classpath.toUtf8());
            }

            classpath_file.close();

            qint64 ram = ((available_ram() / 1073741824) / 2) + 1;

            QString free_ram = QString::number(ram) + "G";
            game_client.start("java", {
                                          "-Xmx" + free_ram,
                                          "-Xms3G",
                                          "-Djava.library.path=" + minecraftDir + "/natives",
                                          "-Dfabric.gameJarPath=" + minecraftDir + "/versions/26.2/26.2.jar",
                                          "-Dorg.lwjgl.librarypath=" + minecraftDir + "/natives",
                                          "--enable-native-access=ALL-UNNAMED",
                                          "-cp", "@classpath.txt",
                                          "net.fabricmc.loader.launch.knot.KnotClient",
                                          "--launchTarget", "minecraft",     // ОБЯЗАТЕЛЬНО
                                          "--username", *nickname,
                                          "--version", "fabric-loader-0.19.3-26.2",
                                          "--gameDir", minecraftDir,
                                          "--assetsDir", minecraftDir + "/assets",
                                          "--assetIndex", "32",
                                          "--quickPlayMultiplayer", "127.0.0.1:25565"
                                      });

            delete nickname;
            delete auth_launcher;
        }, Qt::QueuedConnection);

        QObject::connect(auth_launcher, &authenfication::auth_minecraft_server::auth_failed, this, [this, nickname, auth_launcher] () {
            QMessageBox::critical(this, "Ошибка входа", "Неверный логин или пароль");

            delete nickname;
            delete auth_launcher;
        }, Qt::QueuedConnection);


        auth_client(nickname, auth_launcher);
    }
}

