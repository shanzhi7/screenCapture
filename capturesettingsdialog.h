/***********************************************************************************
*
* @file         capturesettingsdialog.h
* @brief        鎴浘璁剧疆瀵硅瘽妗嗭細鐢ㄤ簬缂栬緫鎴浘鐑敭涓庤嚜鍔ㄤ繚瀛樼洰褰曘€?
*
* @author       shanzhi
* @date         2026/03/07
* @history
***********************************************************************************/

#ifndef CAPTURESETTINGSDIALOG_H
#define CAPTURESETTINGSDIALOG_H

#include <QDialog>
#include <QKeySequence>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui
{
class CaptureSettingsDialog;
}
QT_END_NAMESPACE

class QShowEvent;
class CaptureSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CaptureSettingsDialog(QWidget *parent = nullptr);
    ~CaptureSettingsDialog() override;

    // 璁剧疆褰撳墠鎴浘蹇嵎閿€?
    void setCurrentHotkey(const QKeySequence &sequence);

    // 璁剧疆鑷姩淇濆瓨鐩綍锛堢┖瀛楃涓茶〃绀轰娇鐢ㄩ粯璁ょ洰褰曪級銆?
    void setAutoSaveDirectory(const QString &directory);

    // 鑾峰彇鐢ㄦ埛鍦ㄥ璇濇涓€夋嫨鐨勫揩鎹烽敭銆?
    QKeySequence selectedHotkey() const;

    // 鑾峰彇鐢ㄦ埛鍦ㄥ璇濇涓€夋嫨鐨勮嚜鍔ㄤ繚瀛樼洰褰曘€?
    QString selectedAutoSaveDirectory() const;

    // 璁剧疆鏄惁鍚敤寮€鏈鸿嚜鍚姩銆?
    void setLaunchAtStartupEnabled(bool enabled);

    // 璁剧疆褰撳墠骞冲彴鏄惁鏀寔寮€鏈鸿嚜鍚姩銆?
    void setLaunchAtStartupSupported(bool supported);

    // 鑾峰彇鐢ㄦ埛鏄惁鍕鹃€夊紑鏈鸿嚜鍚姩銆?
    bool selectedLaunchAtStartupEnabled() const;

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    // 閫夋嫨鑷姩淇濆瓨鐩綍銆?
    void onBrowseDirectoryClicked();

    // 娓呯┖鐩綍閰嶇疆锛屽洖閫€榛樿鐩綍绛栫暐銆?
    void onClearDirectoryClicked();

    // 鎭㈠榛樿蹇嵎閿€?
    void onResetHotkeyClicked();

private:
    Ui::CaptureSettingsDialog *ui;
};

#endif // CAPTURESETTINGSDIALOG_H
