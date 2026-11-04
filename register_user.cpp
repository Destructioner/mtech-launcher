#include "register_user.h"
#include "ui_register_user.h"

register_user::register_user(QWidget *parent): QWidget(parent), ui_register(new Ui::register_user) {
    ui_register->setupUi(this);
}

register_user::~register_user() {
    delete ui_register;
}

void register_user::on_cancel_reg_clicked() {
    close();
}


bool register_user::check_forb_symbols(QString &account_form) {
    QString symbols_valid = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_";

    for (const QChar &symb_form: account_form) {
        if (! symbols_valid.contains(symb_form)) {
            return false;
        }
    }

    return true;
}


void register_user::save_user_data(QString account_login, QString account_password) {
    QString roamingPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir check_dir(roamingPath);

    if (check_dir.exists()) {
        QFile file_user_data(static_cast<QString>(roamingPath + "\\account_data.auth"));

        if (file_user_data.open(QIODevice::WriteOnly)) {
            file_user_data.write(static_cast<QByteArray>(account_login.toUtf8() + '\n'));
            file_user_data.write(static_cast<QByteArray>(account_password.toUtf8()));
            file_user_data.flush();

            file_user_data.close();
        }
    }

    else {
        check_dir.mkdir(roamingPath);

        save_user_data(account_login, account_password);
    }
}




void register_user::send_data_credentials(register_credentials data_user) {
    authenfication::auth_minecraft_server *auth_server = new authenfication::auth_minecraft_server(data_user.login_nickname, data_user.password_account);
    qDebug() << "register try";

    QObject::connect(auth_server, &authenfication::auth_minecraft_server::auth_success, this, [this, auth_server, data_user] () {
        QMessageBox::information(this, "Авторизация", "Авторизация успешна");
        static_cast<void>(QtConcurrent::run(&register_user::save_user_data, this, data_user.login_nickname, data_user.password_account));

        delete auth_server;
    }, Qt::QueuedConnection);

    QObject::connect(auth_server, &authenfication::auth_minecraft_server::auth_failed, this, [this, auth_server] () {
        QMessageBox::critical(this, "Ошибка входа", "Неверный логин или пароль");

        delete auth_server;
    }, Qt::QueuedConnection);

    QObject::connect(auth_server, &authenfication::auth_minecraft_server::register_success, this, [this, auth_server, data_user] () {
        QMessageBox::information(this, "Регистрация", "Регистрация успешна");
        static_cast<void>(QtConcurrent::run(&register_user::save_user_data, this, data_user.login_nickname, data_user.password_account));

        delete auth_server;
    }, Qt::QueuedConnection);

    QObject::connect(auth_server, &authenfication::auth_minecraft_server::register_failed, this, [this, auth_server, data_user] () {
        QMessageBox::information(this, "Регистрация", "Ошибка регистрации");

        delete auth_server;
    }, Qt::QueuedConnection);


    qDebug() << auth_server->get_form().login_nickname << "  " << auth_server->get_form().password_account;
    auth_server->register_server();
}



void register_user::on_is_ready_clicked() {
    QMessageBox window_information;

    window_information.setStyleSheet(
        "QMessageBox { background-color: #2b2b2b; }"
        "QLabel { color: #ff0000;}"
        "QPushButton { background-color: #3c3c3c; height: 40px; width: 90px; color: white; border: 1px solid #555; }"
        "QPushButton:hover { background-color: #555; }"
        );

    if (ui_register->input_nickname->text().isEmpty()) {
        window_information.setText("Заполните все поля");
        window_information.setWindowTitle("Ошибка ввода");

        window_information.exec();

        return;
    }

    if (ui_register->input_password->text().isEmpty()) {
        window_information.setText("Заполните все поля");
        window_information.setWindowTitle("Ошибка ввода");

        window_information.exec();


        return;
    }

    QString nickname_user = ui_register->input_nickname->text().toLower();
    QString password_user = ui_register->input_password->text().toLower();

    if (nickname_user.length() >= 16) {
        window_information.setText("Длина ника не должна быть более 16 символов!");
        window_information.setWindowTitle("Ошибка ввода");
        window_information.exec();

        return;
    }

    if (password_user.length() >= 16) {
        window_information.setText("Длина пароля не должна быть более 16 символов!");
        window_information.setWindowTitle("Ошибка ввода");
        window_information.exec();
        return;
    }

    if (check_forb_symbols(nickname_user)) {
        if (check_forb_symbols(password_user)) {

            register_credentials register_data = {nickname_user, password_user};

            send_data_credentials(register_data);
        }

        else {
            window_information.setText("Запрещенные символы пароля! Допускается только английские символы A-z, цифры и нижнее подчеркивание");
            window_information.setWindowTitle("Ошибка ввода");
            window_information.exec();
        }
    }

    else {
        window_information.setText("Запрещенные символы ника! Допускается только английские символы A-z, цифры и нижнее подчеркивание");
        window_information.setWindowTitle("Ошибка ввода");
        window_information.exec();
    }
}

