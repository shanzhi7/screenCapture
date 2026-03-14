/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QHBoxLayout *rootLayout;
    QFrame *leftSidebar;
    QVBoxLayout *sidebarLayout;
    QToolButton *sideCaptureButton;
    QToolButton *sideGalleryButton;
    QToolButton *sideConfigButton;
    QSpacerItem *sidebarSpacer;
    QToolButton *sideBottomButton;
    QVBoxLayout *contentLayout;
    QHBoxLayout *topBarLayout;
    QLabel *mainTitleLabel;
    QSpacerItem *topSpacer;
    QFrame *topActionGroup;
    QHBoxLayout *topActionLayout;
    QToolButton *btnPrev;
    QToolButton *btnShotIcon;
    QToolButton *btnNext;
    QPushButton *btnNewTask;
    QToolButton *btnHistory;
    QPushButton *btnTopSettings;
    QFrame *quickCard;
    QVBoxLayout *quickCardLayout;
    QLabel *quickTitleLabel;
    QFrame *modeSegment;
    QHBoxLayout *modeLayout;
    QToolButton *modeFullButton;
    QToolButton *modeRegionButton;
    QToolButton *modeScrollButton;
    QPushButton *startCaptureButton;
    QHBoxLayout *bodyLayout;
    QFrame *recentCard;
    QVBoxLayout *recentLayout;
    QHBoxLayout *recentHeaderLayout;
    QLabel *recentTitleLabel;
    QSpacerItem *recentHeaderSpacer;
    QToolButton *btnMoreRecent;
    QScrollArea *recentScrollArea;
    QWidget *recentContentWidget;
    QGridLayout *recentGridLayout;
    QVBoxLayout *rightPanelLayout;
    QFrame *formatCard;
    QVBoxLayout *formatLayout;
    QHBoxLayout *formatHeaderLayout;
    QLabel *formatTitleLabel;
    QSpacerItem *formatHeaderSpacer;
    QToolButton *btnMoreFormat;
    QFrame *formatPreviewFrame;
    QVBoxLayout *formatPreviewLayout;
    QLabel *formatPreviewLabel;
    QFrame *commonCard;
    QVBoxLayout *commonLayout;
    QLabel *commonTitleLabel;
    QHBoxLayout *commonActionLayout;
    QPushButton *btnHotkeySetting;
    QPushButton *btnFormatSetting;
    QPushButton *btnAutoSave;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1320, 860);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        rootLayout = new QHBoxLayout(centralwidget);
        rootLayout->setSpacing(16);
        rootLayout->setObjectName("rootLayout");
        rootLayout->setContentsMargins(20, 20, 20, 20);
        leftSidebar = new QFrame(centralwidget);
        leftSidebar->setObjectName("leftSidebar");
        leftSidebar->setMinimumSize(QSize(76, 0));
        leftSidebar->setMaximumSize(QSize(76, 16777215));
        leftSidebar->setFrameShape(QFrame::NoFrame);
        sidebarLayout = new QVBoxLayout(leftSidebar);
        sidebarLayout->setSpacing(12);
        sidebarLayout->setObjectName("sidebarLayout");
        sidebarLayout->setContentsMargins(10, 14, 10, 14);
        sideCaptureButton = new QToolButton(leftSidebar);
        sideCaptureButton->setObjectName("sideCaptureButton");

        sidebarLayout->addWidget(sideCaptureButton);

        sideGalleryButton = new QToolButton(leftSidebar);
        sideGalleryButton->setObjectName("sideGalleryButton");

        sidebarLayout->addWidget(sideGalleryButton);

        sideConfigButton = new QToolButton(leftSidebar);
        sideConfigButton->setObjectName("sideConfigButton");

        sidebarLayout->addWidget(sideConfigButton);

        sidebarSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        sidebarLayout->addItem(sidebarSpacer);

        sideBottomButton = new QToolButton(leftSidebar);
        sideBottomButton->setObjectName("sideBottomButton");

        sidebarLayout->addWidget(sideBottomButton);


        rootLayout->addWidget(leftSidebar);

        contentLayout = new QVBoxLayout();
        contentLayout->setSpacing(14);
        contentLayout->setObjectName("contentLayout");
        topBarLayout = new QHBoxLayout();
        topBarLayout->setSpacing(10);
        topBarLayout->setObjectName("topBarLayout");
        mainTitleLabel = new QLabel(centralwidget);
        mainTitleLabel->setObjectName("mainTitleLabel");

        topBarLayout->addWidget(mainTitleLabel);

        topSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        topBarLayout->addItem(topSpacer);

        topActionGroup = new QFrame(centralwidget);
        topActionGroup->setObjectName("topActionGroup");
        topActionGroup->setFrameShape(QFrame::NoFrame);
        topActionLayout = new QHBoxLayout(topActionGroup);
        topActionLayout->setSpacing(8);
        topActionLayout->setObjectName("topActionLayout");
        topActionLayout->setContentsMargins(8, 6, 8, 6);
        btnPrev = new QToolButton(topActionGroup);
        btnPrev->setObjectName("btnPrev");

        topActionLayout->addWidget(btnPrev);

        btnShotIcon = new QToolButton(topActionGroup);
        btnShotIcon->setObjectName("btnShotIcon");

        topActionLayout->addWidget(btnShotIcon);

        btnNext = new QToolButton(topActionGroup);
        btnNext->setObjectName("btnNext");

        topActionLayout->addWidget(btnNext);


        topBarLayout->addWidget(topActionGroup);

        btnNewTask = new QPushButton(centralwidget);
        btnNewTask->setObjectName("btnNewTask");

        topBarLayout->addWidget(btnNewTask);

        btnHistory = new QToolButton(centralwidget);
        btnHistory->setObjectName("btnHistory");

        topBarLayout->addWidget(btnHistory);

        btnTopSettings = new QPushButton(centralwidget);
        btnTopSettings->setObjectName("btnTopSettings");

        topBarLayout->addWidget(btnTopSettings);


        contentLayout->addLayout(topBarLayout);

        quickCard = new QFrame(centralwidget);
        quickCard->setObjectName("quickCard");
        quickCard->setFrameShape(QFrame::NoFrame);
        quickCardLayout = new QVBoxLayout(quickCard);
        quickCardLayout->setSpacing(10);
        quickCardLayout->setObjectName("quickCardLayout");
        quickCardLayout->setContentsMargins(18, 16, 18, 16);
        quickTitleLabel = new QLabel(quickCard);
        quickTitleLabel->setObjectName("quickTitleLabel");

        quickCardLayout->addWidget(quickTitleLabel);

        modeSegment = new QFrame(quickCard);
        modeSegment->setObjectName("modeSegment");
        modeSegment->setFrameShape(QFrame::NoFrame);
        modeLayout = new QHBoxLayout(modeSegment);
        modeLayout->setSpacing(6);
        modeLayout->setObjectName("modeLayout");
        modeLayout->setContentsMargins(4, 4, 4, 4);
        modeFullButton = new QToolButton(modeSegment);
        modeFullButton->setObjectName("modeFullButton");
        modeFullButton->setCheckable(true);
        modeFullButton->setAutoExclusive(true);

        modeLayout->addWidget(modeFullButton);

        modeRegionButton = new QToolButton(modeSegment);
        modeRegionButton->setObjectName("modeRegionButton");
        modeRegionButton->setCheckable(true);
        modeRegionButton->setAutoExclusive(true);
        modeRegionButton->setChecked(true);

        modeLayout->addWidget(modeRegionButton);

        modeScrollButton = new QToolButton(modeSegment);
        modeScrollButton->setObjectName("modeScrollButton");
        modeScrollButton->setCheckable(true);
        modeScrollButton->setAutoExclusive(true);

        modeLayout->addWidget(modeScrollButton);


        quickCardLayout->addWidget(modeSegment);

        startCaptureButton = new QPushButton(quickCard);
        startCaptureButton->setObjectName("startCaptureButton");

        quickCardLayout->addWidget(startCaptureButton);


        contentLayout->addWidget(quickCard);

        bodyLayout = new QHBoxLayout();
        bodyLayout->setSpacing(14);
        bodyLayout->setObjectName("bodyLayout");
        recentCard = new QFrame(centralwidget);
        recentCard->setObjectName("recentCard");
        recentCard->setFrameShape(QFrame::NoFrame);
        recentLayout = new QVBoxLayout(recentCard);
        recentLayout->setObjectName("recentLayout");
        recentLayout->setContentsMargins(18, 16, 18, 16);
        recentHeaderLayout = new QHBoxLayout();
        recentHeaderLayout->setObjectName("recentHeaderLayout");
        recentTitleLabel = new QLabel(recentCard);
        recentTitleLabel->setObjectName("recentTitleLabel");

        recentHeaderLayout->addWidget(recentTitleLabel);

        recentHeaderSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        recentHeaderLayout->addItem(recentHeaderSpacer);

        btnMoreRecent = new QToolButton(recentCard);
        btnMoreRecent->setObjectName("btnMoreRecent");

        recentHeaderLayout->addWidget(btnMoreRecent);


        recentLayout->addLayout(recentHeaderLayout);

        recentScrollArea = new QScrollArea(recentCard);
        recentScrollArea->setObjectName("recentScrollArea");
        recentScrollArea->setWidgetResizable(true);
        recentScrollArea->setFrameShape(QFrame::NoFrame);
        recentContentWidget = new QWidget();
        recentContentWidget->setObjectName("recentContentWidget");
        recentGridLayout = new QGridLayout(recentContentWidget);
        recentGridLayout->setSpacing(12);
        recentGridLayout->setObjectName("recentGridLayout");
        recentGridLayout->setContentsMargins(0, 0, 0, 0);
        recentScrollArea->setWidget(recentContentWidget);

        recentLayout->addWidget(recentScrollArea);


        bodyLayout->addWidget(recentCard);

        rightPanelLayout = new QVBoxLayout();
        rightPanelLayout->setSpacing(14);
        rightPanelLayout->setObjectName("rightPanelLayout");
        formatCard = new QFrame(centralwidget);
        formatCard->setObjectName("formatCard");
        formatCard->setMinimumSize(QSize(300, 230));
        formatCard->setFrameShape(QFrame::NoFrame);
        formatLayout = new QVBoxLayout(formatCard);
        formatLayout->setObjectName("formatLayout");
        formatLayout->setContentsMargins(16, 14, 16, 14);
        formatHeaderLayout = new QHBoxLayout();
        formatHeaderLayout->setObjectName("formatHeaderLayout");
        formatTitleLabel = new QLabel(formatCard);
        formatTitleLabel->setObjectName("formatTitleLabel");

        formatHeaderLayout->addWidget(formatTitleLabel);

        formatHeaderSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        formatHeaderLayout->addItem(formatHeaderSpacer);

        btnMoreFormat = new QToolButton(formatCard);
        btnMoreFormat->setObjectName("btnMoreFormat");

        formatHeaderLayout->addWidget(btnMoreFormat);


        formatLayout->addLayout(formatHeaderLayout);

        formatPreviewFrame = new QFrame(formatCard);
        formatPreviewFrame->setObjectName("formatPreviewFrame");
        formatPreviewFrame->setFrameShape(QFrame::NoFrame);
        formatPreviewLayout = new QVBoxLayout(formatPreviewFrame);
        formatPreviewLayout->setObjectName("formatPreviewLayout");
        formatPreviewLayout->setContentsMargins(12, 12, 12, 12);
        formatPreviewLabel = new QLabel(formatPreviewFrame);
        formatPreviewLabel->setObjectName("formatPreviewLabel");
        formatPreviewLabel->setMinimumSize(QSize(0, 130));
        formatPreviewLabel->setAlignment(Qt::AlignCenter);

        formatPreviewLayout->addWidget(formatPreviewLabel);


        formatLayout->addWidget(formatPreviewFrame);


        rightPanelLayout->addWidget(formatCard);

        commonCard = new QFrame(centralwidget);
        commonCard->setObjectName("commonCard");
        commonCard->setFrameShape(QFrame::NoFrame);
        commonLayout = new QVBoxLayout(commonCard);
        commonLayout->setObjectName("commonLayout");
        commonLayout->setContentsMargins(16, 14, 16, 14);
        commonTitleLabel = new QLabel(commonCard);
        commonTitleLabel->setObjectName("commonTitleLabel");

        commonLayout->addWidget(commonTitleLabel);

        commonActionLayout = new QHBoxLayout();
        commonActionLayout->setSpacing(10);
        commonActionLayout->setObjectName("commonActionLayout");
        btnHotkeySetting = new QPushButton(commonCard);
        btnHotkeySetting->setObjectName("btnHotkeySetting");

        commonActionLayout->addWidget(btnHotkeySetting);

        btnFormatSetting = new QPushButton(commonCard);
        btnFormatSetting->setObjectName("btnFormatSetting");

        commonActionLayout->addWidget(btnFormatSetting);

        btnAutoSave = new QPushButton(commonCard);
        btnAutoSave->setObjectName("btnAutoSave");
        btnAutoSave->setCheckable(true);
        btnAutoSave->setChecked(true);

        commonActionLayout->addWidget(btnAutoSave);


        commonLayout->addLayout(commonActionLayout);


        rightPanelLayout->addWidget(commonCard);


        bodyLayout->addLayout(rightPanelLayout);


        contentLayout->addLayout(bodyLayout);


        rootLayout->addLayout(contentLayout);

        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "\350\275\273\345\275\261\346\210\252\345\233\276", nullptr));
        sideCaptureButton->setText(QCoreApplication::translate("MainWindow", "\360\237\223\267", nullptr));
        sideGalleryButton->setText(QCoreApplication::translate("MainWindow", "\360\237\223\201", nullptr));
        sideConfigButton->setText(QCoreApplication::translate("MainWindow", "\350\256\276\347\275\256", nullptr));
        sideBottomButton->setText(QCoreApplication::translate("MainWindow", "\345\205\263\344\272\216", nullptr));
        mainTitleLabel->setText(QCoreApplication::translate("MainWindow", "\346\210\252\345\233\276\345\267\245\345\205\267", nullptr));
        btnPrev->setText(QCoreApplication::translate("MainWindow", "<", nullptr));
        btnShotIcon->setText(QCoreApplication::translate("MainWindow", "\346\210\252\345\233\276", nullptr));
        btnNext->setText(QCoreApplication::translate("MainWindow", ">", nullptr));
        btnNewTask->setText(QCoreApplication::translate("MainWindow", "\357\274\213 \346\226\260\345\273\272", nullptr));
        btnHistory->setText(QCoreApplication::translate("MainWindow", "\360\237\227\202", nullptr));
        btnTopSettings->setText(QCoreApplication::translate("MainWindow", "\350\256\276\347\275\256", nullptr));
        quickTitleLabel->setText(QCoreApplication::translate("MainWindow", "\345\277\253\351\200\237\346\210\252\345\233\276", nullptr));
        modeFullButton->setText(QCoreApplication::translate("MainWindow", "\345\205\250\345\261\217", nullptr));
        modeRegionButton->setText(QCoreApplication::translate("MainWindow", "\345\214\272\345\237\237", nullptr));
        modeScrollButton->setText(QCoreApplication::translate("MainWindow", "\346\273\232\345\212\250", nullptr));
        startCaptureButton->setText(QCoreApplication::translate("MainWindow", "\345\274\200\345\247\213\346\210\252\345\233\276", nullptr));
        recentTitleLabel->setText(QCoreApplication::translate("MainWindow", "\346\234\200\350\277\221\346\210\252\345\233\276", nullptr));
        btnMoreRecent->setText(QCoreApplication::translate("MainWindow", "\346\233\264\345\244\232 >", nullptr));
        formatTitleLabel->setText(QCoreApplication::translate("MainWindow", "\346\240\274\345\274\217", nullptr));
        btnMoreFormat->setText(QCoreApplication::translate("MainWindow", "\346\233\264\345\244\232 >", nullptr));
        formatPreviewLabel->setText(QCoreApplication::translate("MainWindow", "\346\240\274\345\274\217\351\242\204\350\247\210\345\214\272\345\237\237", nullptr));
        commonTitleLabel->setText(QCoreApplication::translate("MainWindow", "\345\270\270\347\224\250\350\256\276\347\275\256", nullptr));
        btnHotkeySetting->setText(QCoreApplication::translate("MainWindow", "\347\203\255\351\224\256 Ctrl+Shift+A", nullptr));
        btnFormatSetting->setText(QCoreApplication::translate("MainWindow", "\346\240\274\345\274\217 PNG", nullptr));
        btnAutoSave->setText(QCoreApplication::translate("MainWindow", "\350\207\252\345\212\250\344\277\235\345\255\230 \345\274\200\345\220\257", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
