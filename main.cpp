#include "register_user.h"
#include "mtech_launcher.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[]) {
    QApplication launcher_core(argc, argv);

    launch_mtech launcher_ui;
    launcher_ui.show();

    return QApplication::exec();
}
