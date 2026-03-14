/***********************************************************************************
*
* @file         scrollanchorprovider.h
* @brief        长截图滚动锚点探测器：优先读取滚动语义，再约束拼接位移。
*
* @author       shanzhi
* @date         2026/03/12
* @history
***********************************************************************************/

#ifndef SCROLLANCHORPROVIDER_H
#define SCROLLANCHORPROVIDER_H

#include "longcapturetypes.h"

#include <QImage>
#include <QRect>

#include <memory>
#include <vector>

class ScrollAnchorProvider
{
public:
    virtual ~ScrollAnchorProvider() = default;

    virtual ScrollAnchorKind kind() const = 0;
    virtual ScrollAnchorSnapshot capture(const ScrollTargetContext &context,
                                         const QRect &captureRect,
                                         const QImage &frameHint) const = 0;
};

class ScrollAnchorResolver
{
public:
    ScrollAnchorResolver();
    ~ScrollAnchorResolver();

    ScrollAnchorSnapshot captureBestSnapshot(const ScrollTargetContext &context,
                                             const QRect &captureRect,
                                             const QImage &frameHint,
                                             ScrollAnchorKind preferredKind = ScrollAnchorKind::None) const;

private:
    std::vector<std::unique_ptr<ScrollAnchorProvider>> m_providers;
};

#endif // SCROLLANCHORPROVIDER_H