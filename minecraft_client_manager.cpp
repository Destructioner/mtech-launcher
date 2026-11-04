#include "minecraft_client_manager.h"
#include <quazip.h>
#include <quazipfile.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QMutexLocker>
#include <functional>

downloader::MinecraftDownloader::MinecraftDownloader(): count_files(0), thread_control(4) {
    http_requests::http_client http_get;
    QByteArray json_data_raw = http_get.get("https://piston-meta.mojang.com/mc/game/version_manifest_v2.json");

    if (json_data_raw.size() != 0) {
        url_package = get_url_packages(json_data_raw);
    }

    else {
        emit error_network();
    }
}

bool downloader::MinecraftDownloader::extractZip(const QString &zipPath, const QString &destDir) {
    QuaZip zip(zipPath);
    if (!zip.open(QuaZip::mdUnzip)) {
        return false;
    }

    QDir dir;
    if (!dir.exists(destDir)) {
        if (!dir.mkpath(destDir)) {
            return false;
        }
    }

    bool success = true;

    for (bool more = zip.goToFirstFile(); more; more = zip.goToNextFile()) {
        QuaZipFileInfo64 info;
        if (!zip.getCurrentFileInfo(&info)) {
            success = false;
            continue;
        }

        if (info.name.endsWith('/')) {
            continue;
        }

        QuaZipFile file(&zip);
        if (!file.open(QIODevice::ReadOnly)) {
            success = false;
            continue;
        }

        QString fileName = QFileInfo(info.name).fileName();
        if (fileName.isEmpty()) {
            file.close();
            continue;
        }

        QString fullPath = destDir + "/" + fileName;

        QFile out(fullPath);
        if (!out.open(QIODevice::WriteOnly)) {
            file.close();
            success = false;
            continue;
        }

        out.write(file.readAll());
        out.close();
        file.close();
    }

    zip.close();
    return success;
}


int downloader::MinecraftDownloader::get_num_files_downloaded() {
    return count_files;
}


QString downloader::MinecraftDownloader::get_url_packages(QByteArray &json_raw) {
    QJsonDocument json_data_format = QJsonDocument::fromJson(json_raw, nullptr);
    const QJsonArray versions_minecraft = json_data_format.object().value("versions").toArray();


    for (const QJsonValue &version: versions_minecraft) {
        QJsonObject id_version = version.toObject();

        if (id_version.value("id").toString() == "26.2") {
            return id_version.value("url").toString();
        }
    }

    return static_cast<QString>("0");
}

void downloader::MinecraftDownloader::download_file_asset(QSemaphore &thread_count, QString hash_file) {
    QString url_file = "https://resources.download.minecraft.net/";
    url_file.append(hash_file.left(2) + "/" + hash_file);

    QString path_hash_file = ".\\minecraft\\assets\\objects\\" + hash_file.left(2) + "\\" + hash_file;

    http_requests::http_client file_download;

    while (! file_download.get_file(url_file, path_hash_file));
    thread_count.release();
}

void downloader::MinecraftDownloader::download_lib(QSemaphore &thread_count, QString url_lib, QString path_file_lib) {
    http_requests::http_client http_get_lib;

    while (! http_get_lib.get_file(url_lib, ".\\minecraft\\libraries\\" + path_file_lib));
    thread_count.release();
}

void downloader::MinecraftDownloader::download_assets() {
    http_requests::http_client http_get;

    json_data_files = http_get.get(url_package);

    json_data_format = QJsonDocument::fromJson(json_data_files, nullptr);
    const QJsonObject assets_object = json_data_format.object();

    QString url_assets = assets_object.value("assetIndex").toObject().value("url").toString();
    QByteArray json_assets_raw = http_get.get(url_assets);

    QJsonDocument json_data_assets = QJsonDocument::fromJson(json_assets_raw, nullptr);
    const QJsonObject json_array_files = json_data_assets.object().value("objects").toObject();

    QStringList hash_list;

    for (auto key_asset = json_array_files.begin(); key_asset != json_array_files.end(); ++key_asset) {
        hash_list.append(key_asset.value().toObject().value("hash").toString());
    }


    //
    // начало скачивания assets
    //

    for (const QString hash: hash_list) {
        thread_control.acquire();

        static_cast<void>(QtConcurrent::run(&downloader::MinecraftDownloader::download_file_asset, this, std::ref(thread_control), hash));
        QMutexLocker count_lock(&data_mutex);
        ++count_files;

        emit new_file_num(count_files);
    }

    QDir minecraft_client;
    minecraft_client.mkpath(".\\minecraft\\assets\\indexes");

    QFile file_write(".\\minecraft\\assets\\indexes\\32.json");

    if (file_write.open(QIODevice::WriteOnly)) {
        file_write.write(json_assets_raw);
        file_write.close();
    }

    QMutexLocker count_lock(&data_mutex);

}

void downloader::MinecraftDownloader::download_client() {
    QMutexLocker client_mutex(&data_mutex);

    http_requests::http_client http_get;
    const QJsonObject json_data_libraries_object = json_data_format.object();
    const QString client_url = json_data_libraries_object.value("downloads").toObject().value("client").toObject().value("url").toString();

    while (! http_get.get_file(client_url, ".\\minecraft\\versions\\26.2\\26.2.jar"));
    ++count_files;
    client_mutex.unlock();


    const QJsonArray libraries_json_list = json_data_libraries_object.value("libraries").toArray();

    for (const QJsonValue obj_library: libraries_json_list) {

        const QString path_lib = obj_library.toObject().value("downloads").toObject().value("artifact").toObject().value("path").toString();
        const QString url_lib = obj_library.toObject().value("downloads").toObject().value("artifact").toObject().value("url").toString();

        if (! path_lib.contains("linux")) {
            if (! path_lib.contains("macos")) {
                thread_control.acquire();
                static_cast<void>(QtConcurrent::run(&downloader::MinecraftDownloader::download_lib, this, std::ref(thread_control), url_lib, path_lib));

                QMutexLocker mutex_new_file(&data_mutex);
                ++count_files;

                emit new_file_num(count_files);
            }
        }
    }

    QMutexLocker lock_flag(&data_mutex);

    QFile file_manifest(".\\minecraft\\versions\\26.2\\26.2.json");

    if (file_manifest.open(QIODevice::WriteOnly)) {
        file_manifest.write(json_data_files);
        file_manifest.close();
    }
}

void downloader::MinecraftDownloader::download_fabric() {
    http_requests::http_client http_get;
    QByteArray json_version_fabric = http_get.get("https://meta.fabricmc.net/v2/versions/installer");

    const QJsonDocument json_version_data = QJsonDocument::fromJson(json_version_fabric, nullptr);
    const QJsonArray json_array_data = json_version_data.array();

    QString url_fabric_installer;

    for (const QJsonValue key_fabric_version: json_array_data) {
        if (key_fabric_version.toObject().value("stable").toBool(false)) {
            url_fabric_installer = key_fabric_version.toObject().value("url").toString();
            break;
        }
    }


    while (! http_get.get_file(url_fabric_installer, "fabric-installer.jar"));

    QString java_program = "java";
    QStringList args_install;

    args_install << "-jar" << "fabric-installer.jar"<< "client" << "-dir" << "minecraft" << "-noprofile";

    QProcess::execute(java_program, args_install);

    QMutexLocker mutex_num(&data_mutex);
    count_files += 5;


    emit success_downloaded();
}

void downloader::MinecraftDownloader::download_mods_google_disk() {
    http_requests::http_client http_get;
    QString html_google_download = QString::fromUtf8(http_get.get("https://drive.usercontent.google.com/download?id=1-Fip9bN7F73SixDt4fJQxkGls2YZLG3k"));

    qsizetype index_uuid = html_google_download.indexOf(R"(name="uuid" value=")") + 19;
    QString uuid_button = html_google_download.mid(index_uuid, html_google_download.indexOf(R"(")", index_uuid));

    qsizetype index_at = html_google_download.indexOf(R"(<input type="hidden" name="at" value=")") + 38;
    QString at_button = html_google_download.mid(index_at, html_google_download.indexOf(R"(")", index_at));

    while (! http_get.get_file("https://drive.usercontent.google.com/download?id=1-Fip9bN7F73SixDt4fJQxkGls2YZLG3k&uuid=" + uuid_button + "&at=" + at_button + "&confirm=t", "mods.zip"));

    extractZip("mods.zip", ".\\minecraft\\mods");

    QFile::remove("mods.zip");

    QMutexLocker mutex_mods(&data_mutex);
    count_files += 5;

    emit new_file_num(count_files);
}