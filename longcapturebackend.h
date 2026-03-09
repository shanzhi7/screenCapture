/***********************************************************************************
*
* @file         longcapturebackend.h
* @brief        长截图抓帧后端抽象。
*
* @author       shanzhi
* @date         2026/03/09
* @history
***********************************************************************************/

#ifndef LONGCAPTUREBACKEND_H
#define LONGCAPTUREBACKEND_H

#include "longcapturetypes.h"

#include <QDateTime>
#include <QImage>
#include <QRect>
#include <QString>

class LongCaptureBackend
{
public:
    virtual ~LongCaptureBackend() = default;

    virtual QString backendName() const = 0;
    virtual bool isAvailable() const
    {
        return true;
    }

    virtual bool beginObservation(const QRect &globalRect)
    {
        m_observationRect = globalRect;
        return globalRect.width() > 1 && globalRect.height() > 1;
    }

    virtual CaptureFrame tryGetLatestFrame()
    {
        if (!m_observationRect.isValid())
        {
            return CaptureFrame();
        }

        return buildFrame(capture(m_observationRect));
    }

    virtual void endObservation()
    {
        m_observationRect = QRect();
    }

    virtual QImage capture(const QRect &globalRect) = 0;

    static QImage normalizeCapturedImage(const QImage &image)
    {
        if (image.isNull())
        {
            return QImage();
        }

        QImage normalized;
        switch (image.format())
        {
        case QImage::Format_ARGB32:
        case QImage::Format_RGB32:
            normalized = image.copy();
            break;
        default:
            normalized = image.convertToFormat(QImage::Format_ARGB32);
            break;
        }

        if (normalized.isNull())
        {
            return normalized;
        }

        for (int y = 0; y < normalized.height(); ++y)
        {
            QRgb *line = reinterpret_cast<QRgb *>(normalized.scanLine(y));
            for (int x = 0; x < normalized.width(); ++x)
            {
                line[x] |= 0xff000000u;
            }
        }

        return normalized;
    }

protected:
    CaptureFrame buildFrame(const QImage &image)
    {
        CaptureFrame frame;
        frame.image = normalizeCapturedImage(image);
        if (!frame.image.isNull())
        {
            frame.timestampMs = QDateTime::currentMSecsSinceEpoch();
            frame.serial = ++m_frameSerial;
        }

        return frame;
    }

    QRect observationRect() const
    {
        return m_observationRect;
    }

private:
    QRect m_observationRect;
    quint64 m_frameSerial = 0;
};

#endif // LONGCAPTUREBACKEND_H
