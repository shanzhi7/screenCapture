/***********************************************************************************
*
* @file         mainwindow.h
* @brief        Main window for capture workflow, UI interaction and task orchestration.
*
* @author       shanzhi
* @date         2026/03/06
* @history
***********************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPixmap>
#include <QRect>

#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class QGridLayout;
class QResizeEvent;
class QTimer;
class SelectionOverlay;
class ShowTip;
class LongCaptureStitcher;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void startCapture();
    void saveCurrentImage();
    void onSelectionFinished(const QRect &rect);
    void onSelectionCanceled();
    void onOverlaySaveRequested(const QRect &rect);

    void onModeFullClicked();
    void onModeRegionClicked();
    void onPlaceholderAction();

    // Long capture flow integrated in region mode overlay toolbar.
    void onOverlayLongCaptureToggled(bool enabled, const QRect &rect);
    void onOverlayLongCaptureWheel(const QRect &rect, int delta);
    void onOverlayLongCaptureSaveRequested(const QRect &rect);
    void onOverlayLongCaptureConfirmRequested(const QRect &rect);

private:
    enum class CaptureMode
    {
        Full,
        Region
    };

    void setupRecentItems();
    void addRecentItem(QGridLayout *layout,
                       int row,
                       int col,
                       const QString &title,
                       const QString &timeText,
                       const QString &colorA,
                       const QString &colorB);
    void updateModeSegmentVisuals();
    QPixmap captureRegion(const QRect &rect) const;

    void resetLongCaptureState();
    void appendLongCaptureFrame();
    void finishLongCapture(bool saveToFile, bool copyToClipboard);
    void injectWheelToBackground(int delta);

    void updatePreview(const QPixmap &pixmap);
    void showTip(const QString &text);

private:
    Ui::MainWindow *ui;
    SelectionOverlay *m_overlay = nullptr;
    ShowTip *m_tip = nullptr;
    QPixmap m_currentPixmap;
    CaptureMode m_captureMode = CaptureMode::Region;

    bool m_longCaptureActive = false;
    bool m_longCaptureWheelBusy = false;
    int m_longCaptureVisualHeight = 0;
    QRect m_longCaptureRect;
    QTimer *m_longCaptureDelayTimer = nullptr;
    std::unique_ptr<LongCaptureStitcher> m_longCaptureStitcher;
};

#endif // MAINWINDOW_H
