#ifndef HTTP_REQUESTS_H
#define HTTP_REQUESTS_H

#include <QTimer>
#include <QObject>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QSslSocket>
#include <QEventLoop>
#include <QCoreApplication>

namespace http_requests {
    class http_client {
        public:
            QByteArray get(QString url) {
                QNetworkAccessManager http_manager;
                QNetworkRequest request_url{QUrl(url)};
                QNetworkReply *request_get = http_manager.get(request_url);

                QTimer timer_server_response;
                timer_server_response.setSingleShot(true);

                QEventLoop loop_sync;

                QObject::connect(request_get, QNetworkReply::finished, &loop_sync, &QEventLoop::quit);

                QObject::connect(&timer_server_response, &QTimer::timeout, [&] () {
                    request_get->abort();
                });

                timer_server_response.start(5000);
                loop_sync.exec();


                if (request_get->isFinished()) {
                    if (request_get->error() == QNetworkReply::NoError) {
                        if (request_get->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200) {
                            request_get->deleteLater();
                            QCoreApplication::processEvents();

                            return QByteArray();
                        }

                        timer_server_response.stop();

                        QByteArray data = request_get->readAll();

                        request_get->deleteLater();
                        QCoreApplication::processEvents();
                        return data;
                    }

                    request_get->deleteLater();
                }

                QCoreApplication::processEvents();
                return QByteArray();
            }

            bool get_file(QString url, QString file_path) {
                QString file_path_dirs = QFileInfo(file_path).absolutePath();

                QDir file_dir(file_path_dirs);
                file_dir.mkpath(file_path_dirs);

                QFile file_hand(file_path);

                if (! file_hand.open(QIODevice::WriteOnly)) {
                    return false;
                }

                QNetworkAccessManager http_manager;
                QNetworkRequest request_url{QUrl(url)};
                QNetworkReply *request_get = http_manager.get(request_url);

                QTimer timer_server_response;
                timer_server_response.setSingleShot(true);

                bool success_status = false;
                bool connect_aborted = false;
                bool finished_s = false;
                QEventLoop loop_sync;

                QObject::connect(&timer_server_response, &QTimer::timeout, [&] () {
                    request_get->abort();
                    connect_aborted = true;

                    success_status = false;
                });

                QObject::connect(request_get, QNetworkReply::readyRead, [&] () {
                    if (! connect_aborted && ! finished_s && request_get->error() == QNetworkReply::NoError && request_get->isOpen()) {
                        timer_server_response.stop();
                        QByteArray binary_data = request_get->readAll();

                        if (! binary_data.isEmpty()) {
                            file_hand.write(binary_data);
                            success_status = true;
                        }

                        else {
                            success_status = false;
                        }

                        timer_server_response.start(5000);
                    }

                    else {
                        success_status = false;
                    }
                });

                QObject::connect(request_get, QNetworkReply::finished, [&] () {
                    finished_s = true;
                    if (request_get->error() == QNetworkReply::NoError) {
                        file_hand.close();
                        request_get->deleteLater();

                        loop_sync.quit();

                        QCoreApplication::processEvents();
                        success_status = true;
                    }

                    else {
                        file_hand.close();
                        request_get->deleteLater();

                        loop_sync.quit();
                        QCoreApplication::processEvents();
                        success_status = false;
                    }
                });

                timer_server_response.start(5000);

                loop_sync.exec();

                QCoreApplication::processEvents();

                return success_status;
            }
    };
}

#endif // HTTP_REQUESTS_H