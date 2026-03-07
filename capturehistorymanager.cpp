#include "capturehistorymanager.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>
#include <QStandardPaths>

CaptureHistoryManager::CaptureHistoryManager()
{
}

bool CaptureHistoryManager::initialize()
{
    const QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (appDataPath.isEmpty())
    {
        return false;
    }

    m_historyDirPath = appDataPath + QStringLiteral("/history");
    m_indexFilePath = m_historyDirPath + QStringLiteral("/index.json");

    QDir dir;
    if (!dir.mkpath(m_historyDirPath))
    {
        return false;
    }

    return loadIndex();
}

QList<CaptureHistoryManager::Entry> CaptureHistoryManager::entries() const
{
    return m_entries;
}

bool CaptureHistoryManager::addCapture(const QPixmap &pixmap, const QString &title)
{
    if (pixmap.isNull() || m_historyDirPath.isEmpty())
    {
        return false;
    }

    const QDateTime now = QDateTime::currentDateTime();
    const QString id = now.toString(QStringLiteral("yyyyMMdd_HHmmss_zzz"))
                       + QStringLiteral("_")
                       + QString::number(QRandomGenerator::global()->generate64(), 16);

    const QString fileName = id + QStringLiteral(".png");
    const QString filePath = QDir(m_historyDirPath).filePath(fileName);

    // 统一以 PNG 保存历史截图，避免格式差异影响历史缩略图加载。
    if (!pixmap.save(filePath, "PNG"))
    {
        return false;
    }

    Entry entry;
    entry.id = id;
    entry.title = title;
    entry.filePath = filePath;
    entry.createdAt = now;

    m_entries.prepend(entry);
    trimOverflow();

    return saveIndex();
}

bool CaptureHistoryManager::loadIndex()
{
    m_entries.clear();

    QFile file(m_indexFilePath);
    if (!file.exists())
    {
        return saveIndex();
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }

    const QByteArray raw = file.readAll();
    file.close();

    const QJsonDocument doc = QJsonDocument::fromJson(raw);
    if (!doc.isObject())
    {
        return false;
    }

    const QJsonArray items = doc.object().value(QStringLiteral("items")).toArray();
    for (const QJsonValue &value : items)
    {
        if (!value.isObject())
        {
            continue;
        }

        const QJsonObject obj = value.toObject();
        Entry entry;
        entry.id = obj.value(QStringLiteral("id")).toString();
        entry.title = obj.value(QStringLiteral("title")).toString();
        entry.filePath = obj.value(QStringLiteral("file_path")).toString();
        entry.createdAt = QDateTime::fromString(obj.value(QStringLiteral("created_at")).toString(), Qt::ISODate);

        if (entry.filePath.isEmpty() || !QFile::exists(entry.filePath))
        {
            continue;
        }

        if (!entry.createdAt.isValid())
        {
            entry.createdAt = QFileInfo(entry.filePath).lastModified();
        }

        m_entries.append(entry);
    }

    trimOverflow();
    return saveIndex();
}

bool CaptureHistoryManager::saveIndex()
{
    QJsonArray items;
    for (const Entry &entry : m_entries)
    {
        QJsonObject obj;
        obj.insert(QStringLiteral("id"), entry.id);
        obj.insert(QStringLiteral("title"), entry.title);
        obj.insert(QStringLiteral("file_path"), entry.filePath);
        obj.insert(QStringLiteral("created_at"), entry.createdAt.toString(Qt::ISODate));
        items.append(obj);
    }

    QJsonObject root;
    root.insert(QStringLiteral("items"), items);

    QFile file(m_indexFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
        return false;
    }

    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

void CaptureHistoryManager::trimOverflow()
{
    while (m_entries.size() > m_maxEntries)
    {
        const Entry removed = m_entries.takeLast();
        if (!removed.filePath.isEmpty() && QFile::exists(removed.filePath))
        {
            QFile::remove(removed.filePath);
        }
    }
}
