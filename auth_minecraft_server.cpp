#include "auth_minecraft_server.h"

authenfication::auth_minecraft_server::auth_minecraft_server() {
    error_code = "\0";
}

authenfication::auth_minecraft_server::auth_minecraft_server(QString login, QString password) {
    user_data.login_nickname = login;
    user_data.password_account = password;
}

void authenfication::auth_minecraft_server::set_form(QString login, QString password) {
    user_data.login_nickname = login;
    user_data.password_account = password;
}

authenfication::auth_minecraft_server::auth_credentials authenfication::auth_minecraft_server::get_form() {
    return user_data;
}

void authenfication::auth_minecraft_server::auth_server() {
    QByteArray nickname = user_data.login_nickname.toUtf8();
    QByteArray password_acc = user_data.password_account.toUtf8();

    quint8 length_login = static_cast<quint8>(nickname.length());
    quint8 length_password = static_cast<quint8>(password_acc.length());

    QByteArray mac_addr = hwid_user.get_mac_addr();
    QByteArray motherboard_uuid = hwid_user.get_motherboard_number();
    QByteArray adapter_name = hwid_user.get_description_adapter();

    QByteArray hash_hwid = QCryptographicHash::hash(mac_addr + motherboard_uuid + adapter_name, QCryptographicHash::Sha256);
    quint8 length_hash = static_cast<quint8>(hash_hwid.length());

    QByteArray data_register;

    data_register += 0x51;

    data_register += length_hash;
    data_register += hash_hwid;

    data_register += length_login;
    data_register += nickname;

    data_register += length_password;
    data_register += password_acc;

    QSslSocket *socket_reg = new QSslSocket(this);

    QObject::connect(socket_reg, &QSslSocket::sslErrors, this, [socket_reg](const QList<QSslError> &errors) {
        socket_reg->ignoreSslErrors(); // игнорируем все ошибки
    });

    QObject::connect(socket_reg, &QSslSocket::encrypted, this, [data_register, socket_reg]() {
        socket_reg->write(data_register);
        socket_reg->flush();
        qDebug() << "sended: " << data_register;
    });

    QObject::connect(socket_reg, &QSslSocket::readyRead, this, [this, socket_reg] () {
        QByteArray code = socket_reg->readAll();
        QByteArray code_register = code.toHex();

        QMessageBox window_for_error;

        if (code_register == _NICKNAME_BUSY_) {
            this->error_code = QString::fromUtf8(code);
            window_for_error.setStyleSheet(
                "QMessageBox { background-color: #2b2b2b; }"
                "QLabel { color: #008000;}"
                "QPushButton { background-color: #3c3c3c; height: 40px; width: 90px; color: white; border: 1px solid #555; }"
                "QPushButton:hover { background-color: #555; }"
            );

            window_for_error.setText("Ник занят! Введите другой ник");
            window_for_error.setWindowTitle("Авторизация");
            window_for_error.exec();

            emit auth_failed();
            return;
        }

        if (code_register == _FLAG_MULTIACCOUNTING_) {
            this->error_code = QString::fromUtf8(code);
            window_for_error.setStyleSheet(
                "QMessageBox { background-color: #2b2b2b; }"
                "QLabel { color: #008000;}"
                "QPushButton { background-color: #3c3c3c; height: 40px; width: 90px; color: white; border: 1px solid #555; }"
                "QPushButton:hover { background-color: #555; }"
            );

            window_for_error.setText("Попытка мультиаккаунтинга!");
            window_for_error.setWindowTitle("Авторизация");
            window_for_error.exec();

            emit auth_failed();
            return;
        }

        if (code_register == _FLAG_NOT_ACCOUNT_) {
            this->error_code = QString::fromUtf8(code);
            window_for_error.setStyleSheet(
                "QMessageBox { background-color: #2b2b2b; }"
                "QLabel { color: #008000;}"
                "QPushButton { background-color: #3c3c3c; height: 40px; width: 90px; color: white; border: 1px solid #555; }"
                "QPushButton:hover { background-color: #555; }"
            );

            window_for_error.setText("Аккаунт отсутствует!");
            window_for_error.setWindowTitle("Авторизация");
            window_for_error.exec();

            emit auth_failed();
            return;
        }

        if (code_register == _FLAG_ERROR_PASSWORD_) {
            this->error_code = QString::fromUtf8(code);
            window_for_error.setStyleSheet(
                "QMessageBox { background-color: #2b2b2b; }"
                "QLabel { color: #008000;}"
                "QPushButton { background-color: #3c3c3c; height: 40px; width: 90px; color: white; border: 1px solid #555; }"
                "QPushButton:hover { background-color: #555; }"
            );

            window_for_error.setText("Неправильный пароль!");
            window_for_error.setWindowTitle("Авторизация");
            window_for_error.exec();


            emit auth_failed();
            return;
        }

        if (code_register == _AUTH_SUCCESS_) {
            emit auth_success();
            return;
        }

        window_for_error.setStyleSheet(
            "QMessageBox { background-color: #2b2b2b; }"
            "QLabel { color: #008000;}"
            "QPushButton { background-color: #3c3c3c; height: 40px; width: 90px; color: white; border: 1px solid #555; }"
            "QPushButton:hover { background-color: #555; }"
        );

        window_for_error.setText("Время ожидания ответа истекло");
        window_for_error.setWindowTitle("Авторизация");
        window_for_error.exec();

        emit auth_failed();

    });

    socket_reg->connectToHostEncrypted(static_cast<QString>(_ADDR_SERVER_), 25570);
}

void authenfication::auth_minecraft_server::register_server() {
    QByteArray nickname = user_data.login_nickname.toUtf8();
    QByteArray password_acc = user_data.password_account.toUtf8();

    quint8 length_login = static_cast<quint8>(nickname.length());
    quint8 length_password = static_cast<quint8>(password_acc.length());

    QByteArray mac_addr = hwid_user.get_mac_addr();
    QByteArray motherboard_uuid = hwid_user.get_motherboard_number();
    QByteArray adapter_name = hwid_user.get_description_adapter();

    QByteArray hash_hwid = QCryptographicHash::hash(mac_addr + motherboard_uuid + adapter_name, QCryptographicHash::Sha256);
    quint8 length_hash = static_cast<quint8>(hash_hwid.length());

    QByteArray data_register;

    data_register += 0x50;

    data_register += length_hash;
    data_register += hash_hwid;

    data_register += length_login;
    data_register += nickname;

    data_register += length_password;
    data_register += password_acc;

    QSslSocket *socket_reg = new QSslSocket;
    qDebug() << "socket created: " << socket_reg;
    QObject::connect(socket_reg, &QSslSocket::sslErrors, this, [socket_reg](const QList<QSslError> &errors) {
        socket_reg->ignoreSslErrors(); // игнорируем все ошибки
        qDebug() << errors[0];
    });

    QObject::connect(socket_reg, &QSslSocket::encryptedBytesWritten, this,
                     [](qint64 bytes) {
                         qDebug() << "Фактически записано в TCP:" << bytes;
                     });

    QObject::connect(socket_reg, &QSslSocket::encrypted, this, [data_register, socket_reg]() {
        socket_reg->write(data_register);
        socket_reg->flush();
        qDebug() << "sended: " << data_register;
    });

    QObject::connect(socket_reg, &QSslSocket::readyRead, this, [this, socket_reg] () {
        QByteArray code = socket_reg->readAll();
        QByteArray code_register = code.toHex();

        QMessageBox window_for_error;

        if (code_register == _NICKNAME_BUSY_) {
            this->error_code = QString::fromUtf8(code);
            window_for_error.setStyleSheet(
                "QMessageBox { background-color: #2b2b2b; }"
                "QLabel { color: #008000;}"
                "QPushButton { background-color: #3c3c3c; height: 40px; width: 90px; color: white; border: 1px solid #555; }"
                "QPushButton:hover { background-color: #555; }"
                );

            window_for_error.setText("Ник занят! Введите другой ник");
            window_for_error.setWindowTitle("Авторизация");
            window_for_error.exec();

            emit register_failed();
            return;
        }

        if (code_register == _FLAG_MULTIACCOUNTING_) {
            this->error_code = QString::fromUtf8(code);
            window_for_error.setStyleSheet(
                "QMessageBox { background-color: #2b2b2b; }"
                "QLabel { color: #008000;}"
                "QPushButton { background-color: #3c3c3c; height: 40px; width: 90px; color: white; border: 1px solid #555; }"
                "QPushButton:hover { background-color: #555; }"
                );

            window_for_error.setText("Попытка мультиаккаунтинга!");
            window_for_error.setWindowTitle("Авторизация");
            window_for_error.exec();

            emit register_failed();
            return;
        }

        if (code_register == _FLAG_NOT_ACCOUNT_) {
            this->error_code = QString::fromUtf8(code);
            window_for_error.setStyleSheet(
                "QMessageBox { background-color: #2b2b2b; }"
                "QLabel { color: #008000;}"
                "QPushButton { background-color: #3c3c3c; height: 40px; width: 90px; color: white; border: 1px solid #555; }"
                "QPushButton:hover { background-color: #555; }"
                );

            window_for_error.setText("Аккаунт отсутствует!");
            window_for_error.setWindowTitle("Авторизация");
            window_for_error.exec();

            emit register_failed();
            return;
        }

        if (code_register == _FLAG_ERROR_PASSWORD_) {
            this->error_code = QString::fromUtf8(code);
            window_for_error.setStyleSheet(
                "QMessageBox { background-color: #2b2b2b; }"
                "QLabel { color: #008000;}"
                "QPushButton { background-color: #3c3c3c; height: 40px; width: 90px; color: white; border: 1px solid #555; }"
                "QPushButton:hover { background-color: #555; }"
                );

            window_for_error.setText("Неправильный пароль!");
            window_for_error.setWindowTitle("Авторизация");
            window_for_error.exec();


            emit auth_failed();
            return;
        }

        if (code_register == _AUTH_SUCCESS_) {
            emit auth_success();
            return;
        }

        if (code_register == _REGISTRATION_SUCCESS_) {
            emit register_success();
            return;
        }

        window_for_error.setStyleSheet(
            "QMessageBox { background-color: #2b2b2b; }"
            "QLabel { color: #008000;}"
            "QPushButton { background-color: #3c3c3c; height: 40px; width: 90px; color: white; border: 1px solid #555; }"
            "QPushButton:hover { background-color: #555; }"
            );

        window_for_error.setText("Время ожидания ответа истекло");
        window_for_error.setWindowTitle("Авторизация");
        window_for_error.exec();

        emit register_failed();

    });

    socket_reg->connectToHostEncrypted(static_cast<QString>(_ADDR_SERVER_), 25570);
}