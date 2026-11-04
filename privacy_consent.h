#ifndef PRIVACY_CONSENT_H
#define PRIVACY_CONSENT_H

#include <QDialog>

namespace Ui {
    class privacy_consent;
}

class privacy_consent : public QDialog {
    Q_OBJECT

    public:
        explicit privacy_consent(QWidget *parent = nullptr);
        ~privacy_consent() override;

    private:
        Ui::privacy_consent *ui_eula;
};

#endif // PRIVACY_CONSENT_H
