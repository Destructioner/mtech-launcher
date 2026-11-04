#ifndef MTECH_LAUNCHER_H
#define MTECH_LAUNCHER_H

#include <QMainWindow>
#include <QCryptographicHash>
#include <QProcess>
#include <QSemaphore>
#include <QFutureSynchronizer>
#include <QTimer>
#include <QProgressBar>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QMessageBox>
#include <QDir>
#include <quazip.h>
#include <quazipfile.h>
#include <QSysInfo>
#include <QtNetwork/QSslSocket>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QtConcurrent/QtConcurrent>
#include <QStandardPaths>
#include <QSharedPointer>
#include <functional>
#include <QDebug>
#include "hwid_pc.h"
#include "http_requests.h"
#include "register_user.h"
#include "minecraft_client_manager.h"
#include "auth_minecraft_server.h"
#include <Windows.h>
QT_BEGIN_NAMESPACE
namespace Ui {
    class launcher_s5lab;
}
QT_END_NAMESPACE



#define _ADDR_SERVER_ "127.0.0.1"




class launch_mtech : public QMainWindow {
    Q_OBJECT

    public:
        explicit launch_mtech(QWidget *parent = nullptr);
        ~launch_mtech() override;

    private slots:
        void on_register_2_clicked();
        void on_play_minecraft_clicked();

        void on_pushButton_clicked();

    private:
        QSemaphore thread_control;
        QFutureSynchronizer<void> sync_threads;

        QProcess game_client;
        QString name_file_assets;

        HWID::hwid_client data_pc;

        qint64 available_ram();
        bool extractZip(const QString &zipPath, const QString &destDir);
        void download_mods_google_disk();
        bool check_files_fdownload();

        void auth_client(QString *nick, authenfication::auth_minecraft_server *auth_launch);

        Ui::launcher_s5lab *ui_launcher;
        register_user ui_reg_form;
};
#endif // MTECH_LAUNCHER_H
