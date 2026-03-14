/********************************************************************************
** Form generated from reading UI file 'closedialog.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CLOSEDIALOG_H
#define UI_CLOSEDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_CloseDialog
{
public:
    QVBoxLayout *rootLayout;
    QFrame *dialogCard;
    QVBoxLayout *cardLayout;
    QLabel *titleLabel;
    QLabel *messageLabel;
    QHBoxLayout *buttonLayout;
    QPushButton *hideToTrayButton;
    QPushButton *exitAppButton;
    QPushButton *cancelButton;

    void setupUi(QDialog *CloseDialog)
    {
        if (CloseDialog->objectName().isEmpty())
            CloseDialog->setObjectName("CloseDialog");
        CloseDialog->resize(460, 220);
        CloseDialog->setModal(true);
        rootLayout = new QVBoxLayout(CloseDialog);
        rootLayout->setSpacing(16);
        rootLayout->setObjectName("rootLayout");
        rootLayout->setContentsMargins(20, 20, 20, 20);
        dialogCard = new QFrame(CloseDialog);
        dialogCard->setObjectName("dialogCard");
        dialogCard->setFrameShape(QFrame::NoFrame);
        cardLayout = new QVBoxLayout(dialogCard);
        cardLayout->setSpacing(12);
        cardLayout->setObjectName("cardLayout");
        cardLayout->setContentsMargins(22, 22, 22, 22);
        titleLabel = new QLabel(dialogCard);
        titleLabel->setObjectName("titleLabel");

        cardLayout->addWidget(titleLabel);

        messageLabel = new QLabel(dialogCard);
        messageLabel->setObjectName("messageLabel");
        messageLabel->setWordWrap(true);

        cardLayout->addWidget(messageLabel);

        buttonLayout = new QHBoxLayout();
        buttonLayout->setSpacing(10);
        buttonLayout->setObjectName("buttonLayout");
        hideToTrayButton = new QPushButton(dialogCard);
        hideToTrayButton->setObjectName("hideToTrayButton");

        buttonLayout->addWidget(hideToTrayButton);

        exitAppButton = new QPushButton(dialogCard);
        exitAppButton->setObjectName("exitAppButton");

        buttonLayout->addWidget(exitAppButton);

        cancelButton = new QPushButton(dialogCard);
        cancelButton->setObjectName("cancelButton");

        buttonLayout->addWidget(cancelButton);


        cardLayout->addLayout(buttonLayout);


        rootLayout->addWidget(dialogCard);


        retranslateUi(CloseDialog);

        QMetaObject::connectSlotsByName(CloseDialog);
    } // setupUi

    void retranslateUi(QDialog *CloseDialog)
    {
        CloseDialog->setWindowTitle(QCoreApplication::translate("CloseDialog", "\345\205\263\351\227\255\345\272\224\347\224\250", nullptr));
        titleLabel->setText(QCoreApplication::translate("CloseDialog", "\345\205\263\351\227\255\350\275\273\345\275\261\346\210\252\345\233\276", nullptr));
        messageLabel->setText(QCoreApplication::translate("CloseDialog", "\345\272\224\347\224\250\344\273\215\345\234\250\345\220\216\345\217\260\350\277\220\350\241\214\357\274\214\351\200\211\346\213\251\344\270\200\344\270\252\345\205\263\351\227\255\350\241\214\344\270\272\357\274\232", nullptr));
        hideToTrayButton->setText(QCoreApplication::translate("CloseDialog", "\351\232\220\350\227\217\345\210\260\346\211\230\347\233\230", nullptr));
        exitAppButton->setText(QCoreApplication::translate("CloseDialog", "\351\200\200\345\207\272\345\272\224\347\224\250", nullptr));
        cancelButton->setText(QCoreApplication::translate("CloseDialog", "\345\217\226\346\266\210", nullptr));
    } // retranslateUi

};

namespace Ui {
    class CloseDialog: public Ui_CloseDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CLOSEDIALOG_H
