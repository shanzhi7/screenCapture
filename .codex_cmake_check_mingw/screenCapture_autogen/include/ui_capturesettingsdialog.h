/********************************************************************************
** Form generated from reading UI file 'capturesettingsdialog.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CAPTURESETTINGSDIALOG_H
#define UI_CAPTURESETTINGSDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QKeySequenceEdit>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_CaptureSettingsDialog
{
public:
    QVBoxLayout *rootLayout;
    QFrame *dialogCard;
    QVBoxLayout *cardLayout;
    QFrame *heroFrame;
    QVBoxLayout *heroLayout;
    QLabel *heroBadgeLabel;
    QLabel *titleLabel;
    QLabel *subTitleLabel;
    QFrame *hotkeySection;
    QVBoxLayout *hotkeySectionLayout;
    QLabel *hotkeySectionTitle;
    QLabel *hotkeySectionDesc;
    QHBoxLayout *hotkeyLayout;
    QLabel *hotkeyLabel;
    QKeySequenceEdit *hotkeyEdit;
    QPushButton *resetHotkeyButton;
    QFrame *directorySection;
    QVBoxLayout *directorySectionLayout;
    QLabel *directorySectionTitle;
    QLabel *directorySectionDesc;
    QHBoxLayout *dirLayout;
    QLabel *dirLabel;
    QLineEdit *autoSaveDirEdit;
    QPushButton *browseDirButton;
    QPushButton *clearDirButton;
    QLabel *directoryTipLabel;
    QFrame *startupSection;
    QVBoxLayout *startupSectionLayout;
    QLabel *startupSectionTitle;
    QLabel *startupSectionDesc;
    QCheckBox *launchAtStartupCheckBox;
    QLabel *startupTipLabel;
    QFrame *hintCard;
    QHBoxLayout *hintLayout;
    QLabel *hintLabel;
    QHBoxLayout *footerLayout;
    QSpacerItem *footerSpacer;
    QPushButton *cancelButton;
    QPushButton *saveButton;

    void setupUi(QDialog *CaptureSettingsDialog)
    {
        if (CaptureSettingsDialog->objectName().isEmpty())
            CaptureSettingsDialog->setObjectName("CaptureSettingsDialog");
        CaptureSettingsDialog->resize(720, 570);
        rootLayout = new QVBoxLayout(CaptureSettingsDialog);
        rootLayout->setSpacing(0);
        rootLayout->setObjectName("rootLayout");
        rootLayout->setContentsMargins(22, 22, 22, 22);
        dialogCard = new QFrame(CaptureSettingsDialog);
        dialogCard->setObjectName("dialogCard");
        dialogCard->setFrameShape(QFrame::NoFrame);
        cardLayout = new QVBoxLayout(dialogCard);
        cardLayout->setSpacing(16);
        cardLayout->setObjectName("cardLayout");
        cardLayout->setContentsMargins(24, 24, 24, 24);
        heroFrame = new QFrame(dialogCard);
        heroFrame->setObjectName("heroFrame");
        heroFrame->setFrameShape(QFrame::NoFrame);
        heroLayout = new QVBoxLayout(heroFrame);
        heroLayout->setSpacing(8);
        heroLayout->setObjectName("heroLayout");
        heroLayout->setContentsMargins(20, 18, 20, 18);
        heroBadgeLabel = new QLabel(heroFrame);
        heroBadgeLabel->setObjectName("heroBadgeLabel");

        heroLayout->addWidget(heroBadgeLabel);

        titleLabel = new QLabel(heroFrame);
        titleLabel->setObjectName("titleLabel");

        heroLayout->addWidget(titleLabel);

        subTitleLabel = new QLabel(heroFrame);
        subTitleLabel->setObjectName("subTitleLabel");
        subTitleLabel->setWordWrap(true);

        heroLayout->addWidget(subTitleLabel);


        cardLayout->addWidget(heroFrame);

        hotkeySection = new QFrame(dialogCard);
        hotkeySection->setObjectName("hotkeySection");
        hotkeySection->setFrameShape(QFrame::NoFrame);
        hotkeySectionLayout = new QVBoxLayout(hotkeySection);
        hotkeySectionLayout->setSpacing(12);
        hotkeySectionLayout->setObjectName("hotkeySectionLayout");
        hotkeySectionLayout->setContentsMargins(18, 18, 18, 18);
        hotkeySectionTitle = new QLabel(hotkeySection);
        hotkeySectionTitle->setObjectName("hotkeySectionTitle");

        hotkeySectionLayout->addWidget(hotkeySectionTitle);

        hotkeySectionDesc = new QLabel(hotkeySection);
        hotkeySectionDesc->setObjectName("hotkeySectionDesc");
        hotkeySectionDesc->setWordWrap(true);

        hotkeySectionLayout->addWidget(hotkeySectionDesc);

        hotkeyLayout = new QHBoxLayout();
        hotkeyLayout->setSpacing(10);
        hotkeyLayout->setObjectName("hotkeyLayout");
        hotkeyLabel = new QLabel(hotkeySection);
        hotkeyLabel->setObjectName("hotkeyLabel");
        hotkeyLabel->setMinimumSize(QSize(96, 0));

        hotkeyLayout->addWidget(hotkeyLabel);

        hotkeyEdit = new QKeySequenceEdit(hotkeySection);
        hotkeyEdit->setObjectName("hotkeyEdit");
        hotkeyEdit->setMinimumSize(QSize(0, 46));

        hotkeyLayout->addWidget(hotkeyEdit);

        resetHotkeyButton = new QPushButton(hotkeySection);
        resetHotkeyButton->setObjectName("resetHotkeyButton");
        resetHotkeyButton->setMinimumSize(QSize(112, 46));

        hotkeyLayout->addWidget(resetHotkeyButton);


        hotkeySectionLayout->addLayout(hotkeyLayout);


        cardLayout->addWidget(hotkeySection);

        directorySection = new QFrame(dialogCard);
        directorySection->setObjectName("directorySection");
        directorySection->setFrameShape(QFrame::NoFrame);
        directorySectionLayout = new QVBoxLayout(directorySection);
        directorySectionLayout->setSpacing(12);
        directorySectionLayout->setObjectName("directorySectionLayout");
        directorySectionLayout->setContentsMargins(18, 18, 18, 18);
        directorySectionTitle = new QLabel(directorySection);
        directorySectionTitle->setObjectName("directorySectionTitle");

        directorySectionLayout->addWidget(directorySectionTitle);

        directorySectionDesc = new QLabel(directorySection);
        directorySectionDesc->setObjectName("directorySectionDesc");
        directorySectionDesc->setWordWrap(true);

        directorySectionLayout->addWidget(directorySectionDesc);

        dirLayout = new QHBoxLayout();
        dirLayout->setSpacing(10);
        dirLayout->setObjectName("dirLayout");
        dirLabel = new QLabel(directorySection);
        dirLabel->setObjectName("dirLabel");
        dirLabel->setMinimumSize(QSize(96, 0));

        dirLayout->addWidget(dirLabel);

        autoSaveDirEdit = new QLineEdit(directorySection);
        autoSaveDirEdit->setObjectName("autoSaveDirEdit");
        autoSaveDirEdit->setMinimumSize(QSize(0, 46));
        autoSaveDirEdit->setReadOnly(true);

        dirLayout->addWidget(autoSaveDirEdit);

        browseDirButton = new QPushButton(directorySection);
        browseDirButton->setObjectName("browseDirButton");
        browseDirButton->setMinimumSize(QSize(90, 46));

        dirLayout->addWidget(browseDirButton);

        clearDirButton = new QPushButton(directorySection);
        clearDirButton->setObjectName("clearDirButton");
        clearDirButton->setMinimumSize(QSize(82, 46));

        dirLayout->addWidget(clearDirButton);


        directorySectionLayout->addLayout(dirLayout);

        directoryTipLabel = new QLabel(directorySection);
        directoryTipLabel->setObjectName("directoryTipLabel");
        directoryTipLabel->setWordWrap(true);

        directorySectionLayout->addWidget(directoryTipLabel);


        cardLayout->addWidget(directorySection);

        startupSection = new QFrame(dialogCard);
        startupSection->setObjectName("startupSection");
        startupSection->setFrameShape(QFrame::NoFrame);
        startupSectionLayout = new QVBoxLayout(startupSection);
        startupSectionLayout->setSpacing(12);
        startupSectionLayout->setObjectName("startupSectionLayout");
        startupSectionLayout->setContentsMargins(18, 18, 18, 18);
        startupSectionTitle = new QLabel(startupSection);
        startupSectionTitle->setObjectName("startupSectionTitle");

        startupSectionLayout->addWidget(startupSectionTitle);

        startupSectionDesc = new QLabel(startupSection);
        startupSectionDesc->setObjectName("startupSectionDesc");
        startupSectionDesc->setWordWrap(true);

        startupSectionLayout->addWidget(startupSectionDesc);

        launchAtStartupCheckBox = new QCheckBox(startupSection);
        launchAtStartupCheckBox->setObjectName("launchAtStartupCheckBox");

        startupSectionLayout->addWidget(launchAtStartupCheckBox);

        startupTipLabel = new QLabel(startupSection);
        startupTipLabel->setObjectName("startupTipLabel");
        startupTipLabel->setWordWrap(true);

        startupSectionLayout->addWidget(startupTipLabel);


        cardLayout->addWidget(startupSection);

        hintCard = new QFrame(dialogCard);
        hintCard->setObjectName("hintCard");
        hintCard->setFrameShape(QFrame::NoFrame);
        hintLayout = new QHBoxLayout(hintCard);
        hintLayout->setSpacing(0);
        hintLayout->setObjectName("hintLayout");
        hintLayout->setContentsMargins(16, 14, 16, 14);
        hintLabel = new QLabel(hintCard);
        hintLabel->setObjectName("hintLabel");
        hintLabel->setWordWrap(true);

        hintLayout->addWidget(hintLabel);


        cardLayout->addWidget(hintCard);

        footerLayout = new QHBoxLayout();
        footerLayout->setSpacing(10);
        footerLayout->setObjectName("footerLayout");
        footerSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        footerLayout->addItem(footerSpacer);

        cancelButton = new QPushButton(dialogCard);
        cancelButton->setObjectName("cancelButton");
        cancelButton->setMinimumSize(QSize(108, 46));

        footerLayout->addWidget(cancelButton);

        saveButton = new QPushButton(dialogCard);
        saveButton->setObjectName("saveButton");
        saveButton->setMinimumSize(QSize(132, 46));

        footerLayout->addWidget(saveButton);


        cardLayout->addLayout(footerLayout);


        rootLayout->addWidget(dialogCard);


        retranslateUi(CaptureSettingsDialog);

        QMetaObject::connectSlotsByName(CaptureSettingsDialog);
    } // setupUi

    void retranslateUi(QDialog *CaptureSettingsDialog)
    {
        CaptureSettingsDialog->setWindowTitle(QCoreApplication::translate("CaptureSettingsDialog", "\346\210\252\345\233\276\350\256\276\347\275\256", nullptr));
        heroBadgeLabel->setText(QCoreApplication::translate("CaptureSettingsDialog", "\350\275\273\345\275\261\346\210\252\345\233\276\350\256\276\347\275\256", nullptr));
        titleLabel->setText(QCoreApplication::translate("CaptureSettingsDialog", "\346\210\252\345\233\276\350\256\276\347\275\256", nullptr));
        subTitleLabel->setText(QCoreApplication::translate("CaptureSettingsDialog", "\346\214\211\344\275\240\347\232\204\344\275\277\347\224\250\344\271\240\346\203\257\350\256\276\347\275\256\345\277\253\346\215\267\351\224\256\345\222\214\344\277\235\345\255\230\344\275\215\347\275\256\343\200\202", nullptr));
        hotkeySectionTitle->setText(QCoreApplication::translate("CaptureSettingsDialog", "\345\205\250\345\261\200\345\277\253\346\215\267\351\224\256", nullptr));
        hotkeySectionDesc->setText(QCoreApplication::translate("CaptureSettingsDialog", "\350\256\276\347\275\256\345\220\216\345\217\257\345\277\253\351\200\237\345\274\200\345\247\213\346\210\252\345\233\276\343\200\202\345\273\272\350\256\256\344\275\277\347\224\250\347\273\204\345\220\210\351\224\256\357\274\214\345\207\217\345\260\221\350\257\257\350\247\246\343\200\202", nullptr));
        hotkeyLabel->setText(QCoreApplication::translate("CaptureSettingsDialog", "\345\275\223\345\211\215\345\277\253\346\215\267\351\224\256", nullptr));
        resetHotkeyButton->setText(QCoreApplication::translate("CaptureSettingsDialog", "\346\201\242\345\244\215\351\273\230\350\256\244", nullptr));
        directorySectionTitle->setText(QCoreApplication::translate("CaptureSettingsDialog", "\350\207\252\345\212\250\344\277\235\345\255\230\347\233\256\345\275\225", nullptr));
        directorySectionDesc->setText(QCoreApplication::translate("CaptureSettingsDialog", "\351\200\211\346\213\251\346\210\252\345\233\276\344\277\235\345\255\230\344\275\215\347\275\256\343\200\202\347\225\231\347\251\272\346\227\266\344\274\232\350\207\252\345\212\250\344\277\235\345\255\230\345\210\260\351\273\230\350\256\244\345\233\276\347\211\207\346\226\207\344\273\266\345\244\271\344\270\255\347\232\204\342\200\234\350\275\273\345\275\261\346\210\252\345\233\276\342\200\235\343\200\202", nullptr));
        dirLabel->setText(QCoreApplication::translate("CaptureSettingsDialog", "\345\275\223\345\211\215\347\233\256\345\275\225", nullptr));
        autoSaveDirEdit->setPlaceholderText(QCoreApplication::translate("CaptureSettingsDialog", "\347\225\231\347\251\272\345\210\231\344\275\277\347\224\250\351\273\230\350\256\244\347\233\256\345\275\225\357\274\232\345\233\276\347\211\207/\350\275\273\345\275\261\346\210\252\345\233\276", nullptr));
        browseDirButton->setText(QCoreApplication::translate("CaptureSettingsDialog", "\346\265\217\350\247\210", nullptr));
        clearDirButton->setText(QCoreApplication::translate("CaptureSettingsDialog", "\346\270\205\347\251\272", nullptr));
        directoryTipLabel->setText(QCoreApplication::translate("CaptureSettingsDialog", "\345\246\202\346\236\234\346\203\263\346\201\242\345\244\215\351\273\230\350\256\244\344\277\235\345\255\230\344\275\215\347\275\256\357\274\214\347\233\264\346\216\245\346\270\205\347\251\272\345\215\263\345\217\257\343\200\202", nullptr));
        startupSectionTitle->setText(QCoreApplication::translate("CaptureSettingsDialog", "\345\274\200\346\234\272\350\207\252\345\220\257\345\212\250", nullptr));
        startupSectionDesc->setText(QCoreApplication::translate("CaptureSettingsDialog", "\351\273\230\350\256\244\345\205\263\351\227\255\343\200\202\345\274\200\345\220\257\345\220\216\357\274\214\344\274\232\345\234\250\345\275\223\345\211\215\347\224\250\346\210\267\347\231\273\345\275\225 Windows \346\227\266\350\207\252\345\212\250\345\220\257\345\212\250\350\275\273\345\275\261\346\210\252\345\233\276\343\200\202", nullptr));
        launchAtStartupCheckBox->setText(QCoreApplication::translate("CaptureSettingsDialog", "\347\231\273\345\275\225\347\263\273\347\273\237\345\220\216\350\207\252\345\212\250\345\220\257\345\212\250\350\275\273\345\275\261\346\210\252\345\233\276", nullptr));
        startupTipLabel->setText(QCoreApplication::translate("CaptureSettingsDialog", "\344\273\205\345\275\261\345\223\215\345\275\223\345\211\215\347\224\250\346\210\267\357\274\214\344\270\215\344\277\256\346\224\271\347\263\273\347\273\237\347\272\247\345\220\257\345\212\250\351\241\271\343\200\202", nullptr));
        hintLabel->setText(QCoreApplication::translate("CaptureSettingsDialog", "\345\273\272\350\256\256\345\277\253\346\215\267\351\224\256\345\214\205\345\220\253 Ctrl / Alt / Shift / Win \344\270\255\347\232\204\350\207\263\345\260\221\344\270\200\344\270\252\346\214\211\351\224\256\343\200\202", nullptr));
        cancelButton->setText(QCoreApplication::translate("CaptureSettingsDialog", "\345\217\226\346\266\210", nullptr));
        saveButton->setText(QCoreApplication::translate("CaptureSettingsDialog", "\344\277\235\345\255\230\350\256\276\347\275\256", nullptr));
    } // retranslateUi

};

namespace Ui {
    class CaptureSettingsDialog: public Ui_CaptureSettingsDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CAPTURESETTINGSDIALOG_H
