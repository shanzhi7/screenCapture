#include "globalhotkeymanager.h"

#include <QKeyCombination>
#include <QWidget>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

GlobalHotkeyManager::GlobalHotkeyManager(QWidget *ownerWindow, int hotkeyId, QObject *parent)
    : QObject(parent)
    , m_ownerWindow(ownerWindow)
    , m_hotkeyId(hotkeyId)
{
}

GlobalHotkeyManager::~GlobalHotkeyManager()
{
    clear();
}

bool GlobalHotkeyManager::applyHotkey(const QKeySequence &sequence)
{
    QString validationError;
    if (!validateSequence(sequence, &validationError))
    {
        m_lastError = validationError;
        return false;
    }

#ifdef Q_OS_WIN
    const QKeySequence previousHotkey = m_currentHotkey;
    const bool hadPreviousHotkey = !previousHotkey.isEmpty();

    unsigned int modifiers = 0;
    unsigned int virtualKey = 0;
    if (!translateToNative(sequence, &modifiers, &virtualKey))
    {
        m_lastError = QStringLiteral("不支持该快捷键主键，请使用字母、数字、方向键或功能键。");
        return false;
    }

    if (m_registered)
    {
        unregisterNative();
    }

    QString registerError;
    if (!registerNative(modifiers, virtualKey, &registerError))
    {
        m_lastError = registerError;

        // 新热键注册失败时，自动回滚到上一次可用热键。
        if (hadPreviousHotkey)
        {
            unsigned int rollbackModifiers = 0;
            unsigned int rollbackVirtualKey = 0;
            if (translateToNative(previousHotkey, &rollbackModifiers, &rollbackVirtualKey))
            {
                QString rollbackError;
                registerNative(rollbackModifiers, rollbackVirtualKey, &rollbackError);
            }
        }

        return false;
    }
#endif

    m_currentHotkey = sequence;
    m_lastError.clear();
    return true;
}

void GlobalHotkeyManager::clear()
{
#ifdef Q_OS_WIN
    unregisterNative();
#endif

    m_currentHotkey = QKeySequence();
    m_lastError.clear();
}

QKeySequence GlobalHotkeyManager::currentHotkey() const
{
    return m_currentHotkey;
}

QString GlobalHotkeyManager::lastError() const
{
    return m_lastError;
}

#ifdef Q_OS_WIN
bool GlobalHotkeyManager::handleNativeEvent(MSG *msg, qintptr *result)
{
    if (msg == nullptr || msg->message != WM_HOTKEY)
    {
        return false;
    }

    if (static_cast<int>(msg->wParam) != m_hotkeyId)
    {
        return false;
    }

    if (result != nullptr)
    {
        *result = 0;
    }

    emit activated();
    return true;
}

bool GlobalHotkeyManager::translateToNative(const QKeySequence &sequence,
                                            unsigned int *modifiers,
                                            unsigned int *virtualKey) const
{
    if (modifiers == nullptr || virtualKey == nullptr)
    {
        return false;
    }

    const QKeyCombination combination = sequence[0];
    const Qt::KeyboardModifiers qtModifiers = combination.keyboardModifiers();
    const Qt::Key qtKey = combination.key();

    unsigned int nativeModifiers = 0;
    if (qtModifiers.testFlag(Qt::ControlModifier))
    {
        nativeModifiers |= MOD_CONTROL;
    }
    if (qtModifiers.testFlag(Qt::AltModifier))
    {
        nativeModifiers |= MOD_ALT;
    }
    if (qtModifiers.testFlag(Qt::ShiftModifier))
    {
        nativeModifiers |= MOD_SHIFT;
    }
    if (qtModifiers.testFlag(Qt::MetaModifier))
    {
        nativeModifiers |= MOD_WIN;
    }

    *modifiers = nativeModifiers;

    auto assignVirtualKey = [virtualKey](unsigned int vk)
    {
        *virtualKey = vk;
        return true;
    };

    if (qtKey >= Qt::Key_A && qtKey <= Qt::Key_Z)
    {
        return assignVirtualKey(static_cast<unsigned int>('A' + (qtKey - Qt::Key_A)));
    }

    if (qtKey >= Qt::Key_0 && qtKey <= Qt::Key_9)
    {
        // Qt6 无 Key_NumPad0~9，需通过 KeypadModifier 区分数字主键区与小键盘。
        if (qtModifiers.testFlag(Qt::KeypadModifier))
        {
            return assignVirtualKey(static_cast<unsigned int>(VK_NUMPAD0 + (qtKey - Qt::Key_0)));
        }

        return assignVirtualKey(static_cast<unsigned int>('0' + (qtKey - Qt::Key_0)));
    }

    if (qtKey >= Qt::Key_F1 && qtKey <= Qt::Key_F24)
    {
        return assignVirtualKey(static_cast<unsigned int>(VK_F1 + (qtKey - Qt::Key_F1)));
    }

    switch (qtKey)
    {
        case Qt::Key_Space:
            return assignVirtualKey(VK_SPACE);

        case Qt::Key_Tab:
            return assignVirtualKey(VK_TAB);

        case Qt::Key_Escape:
            return assignVirtualKey(VK_ESCAPE);

        case Qt::Key_Return:
        case Qt::Key_Enter:
            return assignVirtualKey(VK_RETURN);

        case Qt::Key_Insert:
            return assignVirtualKey(VK_INSERT);

        case Qt::Key_Delete:
            return assignVirtualKey(VK_DELETE);

        case Qt::Key_Home:
            return assignVirtualKey(VK_HOME);

        case Qt::Key_End:
            return assignVirtualKey(VK_END);

        case Qt::Key_PageUp:
            return assignVirtualKey(VK_PRIOR);

        case Qt::Key_PageDown:
            return assignVirtualKey(VK_NEXT);

        case Qt::Key_Left:
            return assignVirtualKey(VK_LEFT);

        case Qt::Key_Right:
            return assignVirtualKey(VK_RIGHT);

        case Qt::Key_Up:
            return assignVirtualKey(VK_UP);

        case Qt::Key_Down:
            return assignVirtualKey(VK_DOWN);

        case Qt::Key_Minus:
            return assignVirtualKey(VK_OEM_MINUS);

        case Qt::Key_Plus:
        case Qt::Key_Equal:
            return assignVirtualKey(VK_OEM_PLUS);

        case Qt::Key_Comma:
            return assignVirtualKey(VK_OEM_COMMA);

        case Qt::Key_Period:
            return assignVirtualKey(VK_OEM_PERIOD);

        case Qt::Key_Slash:
            return assignVirtualKey(VK_OEM_2);

        case Qt::Key_Backslash:
            return assignVirtualKey(VK_OEM_5);

        case Qt::Key_BracketLeft:
            return assignVirtualKey(VK_OEM_4);

        case Qt::Key_BracketRight:
            return assignVirtualKey(VK_OEM_6);

        case Qt::Key_Semicolon:
            return assignVirtualKey(VK_OEM_1);

        case Qt::Key_Apostrophe:
            return assignVirtualKey(VK_OEM_7);

        case Qt::Key_QuoteLeft:
            return assignVirtualKey(VK_OEM_3);

        default:
            return false;
    }
}

bool GlobalHotkeyManager::registerNative(unsigned int modifiers, unsigned int virtualKey, QString *error)
{
    if (m_ownerWindow == nullptr)
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("快捷键注册失败：窗口对象为空。");
        }
        return false;
    }

    const HWND hwnd = reinterpret_cast<HWND>(m_ownerWindow->winId());
    if (hwnd == nullptr)
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("快捷键注册失败：窗口句柄无效。");
        }
        return false;
    }

    const BOOL ok = RegisterHotKey(hwnd,
                                   m_hotkeyId,
                                   modifiers | MOD_NOREPEAT,
                                   virtualKey);
    if (ok == 0)
    {
        if (error != nullptr)
        {
            const unsigned long code = GetLastError();
            *error = QStringLiteral("快捷键注册失败：") + winErrorText(code);
        }
        return false;
    }

    m_registered = true;
    return true;
}

void GlobalHotkeyManager::unregisterNative()
{
    if (!m_registered || m_ownerWindow == nullptr)
    {
        return;
    }

    const HWND hwnd = reinterpret_cast<HWND>(m_ownerWindow->winId());
    if (hwnd != nullptr)
    {
        UnregisterHotKey(hwnd, m_hotkeyId);
    }

    m_registered = false;
}

QString GlobalHotkeyManager::winErrorText(unsigned long errorCode) const
{
    LPWSTR buffer = nullptr;
    const DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
    const DWORD length = FormatMessageW(flags,
                                        nullptr,
                                        errorCode,
                                        0,
                                        reinterpret_cast<LPWSTR>(&buffer),
                                        0,
                                        nullptr);
    if (length == 0 || buffer == nullptr)
    {
        return QStringLiteral("系统错误码 %1").arg(errorCode);
    }

    QString text = QString::fromWCharArray(buffer).trimmed();
    LocalFree(buffer);

    if (text.isEmpty())
    {
        return QStringLiteral("系统错误码 %1").arg(errorCode);
    }

    return text;
}
#endif

bool GlobalHotkeyManager::validateSequence(const QKeySequence &sequence, QString *error) const
{
    if (sequence.isEmpty())
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("快捷键不能为空。");
        }
        return false;
    }

    if (sequence.count() > 1)
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("仅支持单步快捷键组合。");
        }
        return false;
    }

    const QKeyCombination combination = sequence[0];
    const Qt::KeyboardModifiers modifiers = combination.keyboardModifiers();
    const Qt::KeyboardModifiers meaningfulModifiers = modifiers
                                                      & (Qt::ControlModifier
                                                         | Qt::AltModifier
                                                         | Qt::ShiftModifier
                                                         | Qt::MetaModifier);
    const Qt::Key key = combination.key();

    // 强制要求至少一个“有效修饰键”（不把 KeypadModifier 计入）。
    if (meaningfulModifiers == Qt::NoModifier)
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("快捷键至少包含一个修饰键（Ctrl/Alt/Shift/Win）。");
        }
        return false;
    }

    if (key == Qt::Key_unknown
        || key == Qt::Key_Control
        || key == Qt::Key_Alt
        || key == Qt::Key_Shift
        || key == Qt::Key_Meta)
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("请选择有效的主键。");
        }
        return false;
    }

    if (error != nullptr)
    {
        error->clear();
    }

    return true;
}

