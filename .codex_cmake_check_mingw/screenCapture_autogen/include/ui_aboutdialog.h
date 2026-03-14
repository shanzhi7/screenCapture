/********************************************************************************
** Form generated from reading UI file 'aboutdialog.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ABOUTDIALOG_H
#define UI_ABOUTDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_AboutDialog
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
    QFrame *infoSection;
    QVBoxLayout *infoSectionLayout;
    QLabel *infoSectionTitle;
    QLabel *contentLabel;
    QFrame *feedbackSection;
    QVBoxLayout *feedbackSectionLayout;
    QLabel *feedbackTitleLabel;
    QLabel *feedbackBodyLabel;
    QHBoxLayout *footerLayout;
    QPushButton *copyInfoButton;
    QSpacerItem *footerSpacer;
    QPushButton *closeButton;

    void setupUi(QDialog *AboutDialog)
    {
        if (AboutDialog->objectName().isEmpty())
            AboutDialog->setObjectName("AboutDialog");
        AboutDialog->resize(620, 430);
        rootLayout = new QVBoxLayout(AboutDialog);
        rootLayout->setSpacing(0);
        rootLayout->setObjectName("rootLayout");
        rootLayout->setContentsMargins(22, 22, 22, 22);
        dialogCard = new QFrame(AboutDialog);
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

        infoSection = new QFrame(dialogCard);
        infoSection->setObjectName("infoSection");
        infoSection->setFrameShape(QFrame::NoFrame);
        infoSectionLayout = new QVBoxLayout(infoSection);
        infoSectionLayout->setSpacing(10);
        infoSectionLayout->setObjectName("infoSectionLayout");
        infoSectionLayout->setContentsMargins(18, 18, 18, 18);
        infoSectionTitle = new QLabel(infoSection);
        infoSectionTitle->setObjectName("infoSectionTitle");

        infoSectionLayout->addWidget(infoSectionTitle);

        contentLabel = new QLabel(infoSection);
        contentLabel->setObjectName("contentLabel");
        contentLabel->setWordWrap(true);

        infoSectionLayout->addWidget(contentLabel);


        cardLayout->addWidget(infoSection);

        feedbackSection = new QFrame(dialogCard);
        feedbackSection->setObjectName("feedbackSection");
        feedbackSection->setFrameShape(QFrame::NoFrame);
        feedbackSectionLayout = new QVBoxLayout(feedbackSection);
        feedbackSectionLayout->setSpacing(8);
        feedbackSectionLayout->setObjectName("feedbackSectionLayout");
        feedbackSectionLayout->setContentsMargins(18, 18, 18, 18);
        feedbackTitleLabel = new QLabel(feedbackSection);
        feedbackTitleLabel->setObjectName("feedbackTitleLabel");

        feedbackSectionLayout->addWidget(feedbackTitleLabel);

        feedbackBodyLabel = new QLabel(feedbackSection);
        feedbackBodyLabel->setObjectName("feedbackBodyLabel");
        feedbackBodyLabel->setWordWrap(true);

        feedbackSectionLayout->addWidget(feedbackBodyLabel);


        cardLayout->addWidget(feedbackSection);

        footerLayout = new QHBoxLayout();
        footerLayout->setSpacing(10);
        footerLayout->setObjectName("footerLayout");
        copyInfoButton = new QPushButton(dialogCard);
        copyInfoButton->setObjectName("copyInfoButton");
        copyInfoButton->setMinimumSize(QSize(132, 46));

        footerLayout->addWidget(copyInfoButton);

        footerSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        footerLayout->addItem(footerSpacer);

        closeButton = new QPushButton(dialogCard);
        closeButton->setObjectName("closeButton");
        closeButton->setMinimumSize(QSize(108, 46));

        footerLayout->addWidget(closeButton);


        cardLayout->addLayout(footerLayout);


        rootLayout->addWidget(dialogCard);


        retranslateUi(AboutDialog);

        QMetaObject::connectSlotsByName(AboutDialog);
    } // setupUi

    void retranslateUi(QDialog *AboutDialog)
    {
        AboutDialog->setWindowTitle(QCoreApplication::translate("AboutDialog", "\345\205\263\344\272\216\350\275\273\345\275\261\346\210\252\345\233\276", nullptr));
        heroBadgeLabel->setText(QCoreApplication::translate("AboutDialog", "\350\275\273\345\275\261\346\210\252\345\233\276", nullptr));
        titleLabel->setText(QCoreApplication::translate("AboutDialog", "\345\205\263\344\272\216\350\275\273\345\275\261\346\210\252\345\233\276", nullptr));
        subTitleLabel->setText(QCoreApplication::translate("AboutDialog", "\344\270\200\346\254\276\350\275\273\345\277\253\351\241\272\346\211\213\347\232\204\346\210\252\345\233\276\345\267\245\345\205\267\357\274\214\351\200\202\345\220\210\346\227\245\345\270\270\345\277\253\351\200\237\346\210\252\345\233\276\344\270\216\344\277\235\345\255\230\343\200\202", nullptr));
        infoSectionTitle->setText(QCoreApplication::translate("AboutDialog", "\345\272\224\347\224\250\344\277\241\346\201\257", nullptr));
        contentLabel->setText(QString());
        feedbackTitleLabel->setText(QCoreApplication::translate("AboutDialog", "\345\217\215\351\246\210\345\273\272\350\256\256", nullptr));
        feedbackBodyLabel->setText(QCoreApplication::translate("AboutDialog", "\345\246\202\346\236\234\351\201\207\345\210\260\351\227\256\351\242\230\346\210\226\346\203\263\346\217\220\345\207\272\345\273\272\350\256\256\357\274\214\345\217\257\344\273\245\345\205\210\345\244\215\345\210\266\345\272\224\347\224\250\344\277\241\346\201\257\357\274\214\345\206\215\351\231\204\344\270\212\346\210\252\345\233\276\346\210\226\347\216\260\350\261\241\346\217\217\350\277\260\350\277\233\350\241\214\345\217\215\351\246\210\343\200\202", nullptr));
        copyInfoButton->setText(QCoreApplication::translate("AboutDialog", "\345\244\215\345\210\266\344\277\241\346\201\257", nullptr));
        closeButton->setText(QCoreApplication::translate("AboutDialog", "\345\205\263\351\227\255", nullptr));
    } // retranslateUi

};

namespace Ui {
    class AboutDialog: public Ui_AboutDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ABOUTDIALOG_H
