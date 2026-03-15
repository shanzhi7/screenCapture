/***********************************************************************************
*
* @file         autostartmanager.h
* @brief        开机自启动管理：负责当前用户启动项读写。
*
* @author       shanzhi
* @date         2026/03/15
* @history
***********************************************************************************/

#ifndef AUTOSTARTMANAGER_H
#define AUTOSTARTMANAGER_H

#include <QString>

class AutoStartManager
{
public:
    static bool isSupported();
    static bool isEnabled();
    static bool setEnabled(bool enabled, QString *errorMessage = nullptr);
};

#endif // AUTOSTARTMANAGER_H