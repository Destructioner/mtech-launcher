#include "privacy_consent.h"
#include "ui_privacy_consent.h"

privacy_consent::privacy_consent(QWidget *parent): QDialog(parent), ui_eula(new Ui::privacy_consent) {
    ui_eula->setupUi(this);
}

privacy_consent::~privacy_consent() {
    delete ui_eula;
}
