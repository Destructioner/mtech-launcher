#ifndef REGISTER_USER_H
#define REGISTER_USER_H

#include <memory>
#include <QWidget>
#include <QtNetwork/QNetworkAccessManager>
#include <QMessageBox>
#include <QDir>
#include <QSysInfo>
#include <QtNetwork/QSslSocket>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QtConcurrent/QtConcurrent>
#include <QStandardPaths>
#include "auth_minecraft_server.h"
#include "hwid_pc.h"

#define _REGISTRATION_SUCCESS_ "800f0921"
#define _AUTH_SUCCESS_ "800f0926"
#define _NICKNAME_BUSY_ "800f0922"
#define _FLAG_MULTIACCOUNTING_ "800f0923"
#define _FLAG_NOT_ACCOUNT_ "800f0924"
#define _FLAG_ERROR_PASSWORD_ "800f0925"

#define _ADDR_SERVER_ "127.0.0.1"

namespace Ui {
    class register_user;
}

class register_user : public QWidget {
    Q_OBJECT

    public:
        explicit register_user(QWidget *parent = nullptr);
        ~register_user() override;

    private slots:
        void on_cancel_reg_clicked();
        void on_is_ready_clicked();

    private:
        struct register_credentials {
            QString login_nickname;
            QString password_account;
        };

        struct auth_credentials {
            QString login_nickname;
            QString password_account;
        };

        HWID::hwid_client data_pc;



        void send_data_credentials(register_credentials data_user);
        bool check_forb_symbols(QString &account_form);
        void save_user_data(QString account_login, QString account_password);

        auth_credentials auth_data;
        Ui::register_user *ui_register;
};

#endif // REGISTER_USER_H
