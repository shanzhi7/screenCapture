/***********************************************************************************
*
* @file         globalhotkeymanager.h
* @brief        全局热键管理器：负责注册、更新、冲突回滚与激活信号分发。
*
* @author       shanzhi
* @date         2026/03/07
* @history
***********************************************************************************/

#ifndef GLOBALHOTKEYMANAGER_H
#define GLOBALHOTKEYMANAGER_H

#include <QKeySequence>
#include <QObject>
#include <QString>

class QWidget;

#ifdef Q_OS_WIN
struct tagMSG;
using MSG = tagMSG;
#endif

class GlobalHotkeyManager : public QObject
{
    Q_OBJECT

public:
    explicit GlobalHotkeyManager(QWidget *ownerWindow, int hotkeyId, QObject *parent = nullptr);
    ~GlobalHotkeyManager() override;

    // 应用并注册新的全局热键，失败时会自动回滚到旧热键。
    bool applyHotkey(const QKeySequence &sequence);

    // 注销当前全局热键。
    void clear();

    // 当前生效热键。
    QKeySequence currentHotkey() const;

    // 最近一次失败原因。
    QString lastError() const;

#ifdef Q_OS_WIN
    // 在 nativeEvent 中转发 Windows 消息，命中时发出 activated。
    bool handleNativeEvent(MSG *msg, qintptr *result);
#endif

signals:
    void activated();

private:
#ifdef Q_OS_WIN
    // 把 QKeySequence 转换为 RegisterHotKey 所需参数。
    bool translateToNative(const QKeySequence &sequence, unsigned int *modifiers, unsigned int *virtualKey) const;

    // 注册指定原生热键参数。
    bool registerNative(unsigned int modifiers, unsigned int virtualKey, QString *error);

    // 注销已注册热键。
    void unregisterNative();

    // 将系统错误码转为可读文本。
    QString winErrorText(unsigned long errorCode) const;
#endif

    // 判断组合键是否合法（至少 1 个修饰键，且仅单步组合）。
    bool validateSequence(const QKeySequence &sequence, QString *error) const;

private:
    QWidget *m_ownerWindow = nullptr;
    int m_hotkeyId = 0;
    QKeySequence m_currentHotkey;
    QString m_lastError;

#ifdef Q_OS_WIN
    bool m_registered = false;
#endif
};

#endif // GLOBALHOTKEYMANAGER_H
