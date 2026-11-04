#ifndef AUTH_MINECRAFT_SERVER_H
#define AUTH_MINECRAFT_SERVER_H

#include <QObject>
#include <QSslSocket>
#include <QCryptographicHash>
#include <QWidget>
#include <QMessageBox>
#include <QString>
#include "hwid_pc.h"

#define _REGISTRATION_SUCCESS_ "800f0921"
#define _AUTH_SUCCESS_ "800f0926"
#define _NICKNAME_BUSY_ "800f0922"
#define _FLAG_MULTIACCOUNTING_ "800f0923"
#define _FLAG_NOT_ACCOUNT_ "800f0924"
#define _FLAG_ERROR_PASSWORD_ "800f0925"

#define _ADDR_SERVER_ "127.0.0.1"


namespace authenfication {
    class auth_minecraft_server: public QObject {
        Q_OBJECT

        private:
            struct auth_credentials {
                QString login_nickname;
                QString password_account;
            };

            auth_credentials user_data;
            QString error_code;

            HWID::hwid_client hwid_user;

        signals:
            void auth_failed();
            void auth_success();

            void register_failed();
            void register_success();

        public:

            auth_minecraft_server();
            auth_minecraft_server(QString login, QString password);
            void set_form(QString login, QString password);
            auth_credentials get_form();
            void auth_server();
            void register_server();
        };
}

#endif // AUTH_MINECRAFT_SERVER_H
