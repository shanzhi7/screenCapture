/********************************************************************************
** Form generated from reading UI file 'showtip.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SHOWTIP_H
#define UI_SHOWTIP_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ShowTip
{
public:
    QHBoxLayout *horizontalLayout;
    QLabel *tipLabel;

    void setupUi(QWidget *ShowTip)
    {
        if (ShowTip->objectName().isEmpty())
            ShowTip->setObjectName("ShowTip");
        ShowTip->resize(260, 44);
        horizontalLayout = new QHBoxLayout(ShowTip);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setContentsMargins(14, 10, 14, 10);
        tipLabel = new QLabel(ShowTip);
        tipLabel->setObjectName("tipLabel");
        tipLabel->setAlignment(Qt::AlignCenter);

        horizontalLayout->addWidget(tipLabel);


        retranslateUi(ShowTip);

        QMetaObject::connectSlotsByName(ShowTip);
    } // setupUi

    void retranslateUi(QWidget *ShowTip)
    {
        ShowTip->setWindowTitle(QCoreApplication::translate("ShowTip", "\346\217\220\347\244\272", nullptr));
        tipLabel->setText(QCoreApplication::translate("ShowTip", "\346\217\220\347\244\272\346\266\210\346\201\257", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ShowTip: public Ui_ShowTip {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SHOWTIP_H
