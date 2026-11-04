#ifndef MINECRAFT_CLIENT_MANAGER_H
#define MINECRAFT_CLIENT_MANAGER_H

#include <QObject>
#include <QMutex>
#include <quazip.h>
#include <quazipfile.h>
#include <QtConcurrent/QtConcurrent>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <functional>
#include "http_requests.h"

namespace downloader {
    class MinecraftDownloader : public QObject {
        Q_OBJECT
        public:
            MinecraftDownloader();
            int get_num_files_downloaded();
            void download_assets();
            void download_client();
            void download_fabric();
            void download_mods_google_disk();

        signals:
            void new_file_num(int count_file);
            void error_network();
            void success_downloaded();


        private:
            bool extractZip(const QString &zipPath, const QString &destDir);
            QString get_url_packages(QByteArray &json_raw);
            void download_file_asset(QSemaphore &thread_count, QString hash_file);
            void download_lib(QSemaphore &thread_count, QString url_lib, QString path_file_lib);

            QByteArray json_data_files;
            QJsonDocument json_data_format;
            int count_files;
            int flag_threads;

            QMutex data_mutex;
            QSemaphore thread_control;
            QFutureSynchronizer<void> sync_threads;
            QFutureSynchronizer<void> sync_main_threads;
            QFuture<void> thread_client;
            QFuture<void> thread_assets;
            QFuture<void> thread_mods;
            QString url_package;
    };
}


#endif
