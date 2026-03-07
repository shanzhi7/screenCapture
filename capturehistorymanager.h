/***********************************************************************************
*
* @file         capturehistorymanager.h
* @brief        截图历史管理器：负责历史图片落盘、索引持久化与读取。
*
* @author       shanzhi
* @date         2026/03/07
* @history
***********************************************************************************/

#ifndef CAPTUREHISTORYMANAGER_H
#define CAPTUREHISTORYMANAGER_H

#include <QDateTime>
#include <QList>
#include <QPixmap>
#include <QString>

/**
 * @brief 截图历史管理器。
 *
 * 管理应用历史目录、索引文件和历史条目。
 */
class CaptureHistoryManager
{
public:
    /**
     * @brief 历史条目。
     */
    struct Entry
    {
        QString id;           ///< 条目唯一标识
        QString title;        ///< 条目标题
        QString filePath;     ///< 图片绝对路径
        QDateTime createdAt;  ///< 创建时间
    };

public:
    explicit CaptureHistoryManager();

    /**
     * @brief 初始化历史目录与索引。
     * @return 成功返回 true，否则 false。
     */
    bool initialize();

    /**
     * @brief 读取当前历史列表（按时间倒序）。
     * @return 历史条目集合。
     */
    QList<Entry> entries() const;

    /**
     * @brief 新增一条截图历史。
     * @param pixmap 要保存的截图。
     * @param title 历史标题。
     * @return 成功返回 true，否则 false。
     */
    bool addCapture(const QPixmap &pixmap, const QString &title);

private:
    bool loadIndex();
    bool saveIndex();
    void trimOverflow();

private:
    QString m_historyDirPath;
    QString m_indexFilePath;
    QList<Entry> m_entries;
    int m_maxEntries = 16;
};

#endif // CAPTUREHISTORYMANAGER_H
