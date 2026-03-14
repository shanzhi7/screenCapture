#include "scrollanchorprovider.h"

#include <QDateTime>
#include <QLoggingCategory>
#include <QtMath>

#ifdef Q_OS_WIN
#include <windows.h>
#include <oleacc.h>
#include <uiautomationclient.h>
#if defined(__IUIAutomationElement_FWD_DEFINED__) && defined(__IUIAutomationScrollPattern_FWD_DEFINED__) && defined(__IUIAutomationRangeValuePattern_FWD_DEFINED__)
#define SCREENCAPTURE_HAS_UIA_INTERFACES 1
#else
#define SCREENCAPTURE_HAS_UIA_INTERFACES 0
#endif
#else
#define SCREENCAPTURE_HAS_UIA_INTERFACES 0
#endif

Q_LOGGING_CATEGORY(lcAnchorLog, "lc.anchor")

namespace
{
QString anchorKindName(ScrollAnchorKind kind)
{
    switch (kind)
    {
    case ScrollAnchorKind::Win32ScrollInfo:
        return QStringLiteral("Win32ScrollInfo");
    case ScrollAnchorKind::UiaScrollPattern:
        return QStringLiteral("UiaScrollPattern");
    case ScrollAnchorKind::UiaRangeValue:
        return QStringLiteral("UiaRangeValue");
    case ScrollAnchorKind::VisualScrollbar:
        return QStringLiteral("VisualScrollbar");
    case ScrollAnchorKind::None:
    default:
        return QStringLiteral("None");
    }
}

ScrollAnchorSnapshot invalidSnapshot(ScrollAnchorKind kind = ScrollAnchorKind::None)
{
    ScrollAnchorSnapshot snapshot;
    snapshot.kind = kind;
    snapshot.timestampMs = QDateTime::currentMSecsSinceEpoch();
    return snapshot;
}

double safeViewportRatio(const ScrollAnchorSnapshot &snapshot)
{
    if (snapshot.viewportRatio > 0.0)
    {
        return snapshot.viewportRatio;
    }

    if (snapshot.pageSize > 0.0 && snapshot.maximum > snapshot.minimum)
    {
        const double range = snapshot.maximum - snapshot.minimum;
        if (snapshot.pageSize <= 1.0 && range <= 1.0)
        {
            return snapshot.pageSize;
        }

        return snapshot.pageSize / qMax(1.0, range + snapshot.pageSize);
    }

    if (snapshot.thumbHeightRatio > 0.0)
    {
        return snapshot.thumbHeightRatio;
    }

    return 0.0;
}

#if SCREENCAPTURE_HAS_UIA_INTERFACES
class ScopedComInitialization
{
public:
    ScopedComInitialization()
    {
        const HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        if (SUCCEEDED(hr))
        {
            m_shouldUninitialize = true;
            m_ready = true;
            return;
        }

        if (hr == RPC_E_CHANGED_MODE)
        {
            m_ready = true;
        }
    }

    ~ScopedComInitialization()
    {
        if (m_shouldUninitialize)
        {
            CoUninitialize();
        }
    }

    bool isReady() const
    {
        return m_ready;
    }

private:
    bool m_ready = false;
    bool m_shouldUninitialize = false;
};

template <typename T>
void releaseCom(T *&pointer)
{
    if (pointer != nullptr)
    {
        pointer->Release();
        pointer = nullptr;
    }
}

bool variantBoolValue(IUIAutomationElement *element, PROPERTYID propertyId)
{
    if (element == nullptr)
    {
        return false;
    }

    VARIANT value;
    VariantInit(&value);
    const HRESULT hr = element->GetCurrentPropertyValue(propertyId, &value);
    const bool result = SUCCEEDED(hr) && value.vt == VT_BOOL && value.boolVal == VARIANT_TRUE;
    VariantClear(&value);
    return result;
}

int variantIntValue(IUIAutomationElement *element, PROPERTYID propertyId, int fallbackValue)
{
    if (element == nullptr)
    {
        return fallbackValue;
    }

    VARIANT value;
    VariantInit(&value);
    const HRESULT hr = element->GetCurrentPropertyValue(propertyId, &value);
    int result = fallbackValue;
    if (SUCCEEDED(hr))
    {
        if (value.vt == VT_I4 || value.vt == VT_INT)
        {
            result = value.intVal;
        }
        else if (value.vt == VT_UI4)
        {
            result = static_cast<int>(value.uintVal);
        }
    }
    VariantClear(&value);
    return result;
}
#endif

#ifdef Q_OS_WIN
HWND windowHandleOf(quintptr windowId)
{
    return reinterpret_cast<HWND>(windowId);
}

bool isValidWindowHandle(HWND window)
{
    return window != nullptr && IsWindow(window) && IsWindowVisible(window);
}

bool isScrollbarControlWindow(HWND window)
{
    if (!isValidWindowHandle(window))
    {
        return false;
    }

    wchar_t className[64] = {0};
    const int length = GetClassNameW(window, className, static_cast<int>(std::size(className)));
    if (length <= 0)
    {
        return false;
    }

    return QString::fromWCharArray(className, length).compare(QStringLiteral("ScrollBar"), Qt::CaseInsensitive) == 0;
}

bool populateSnapshotFromScrollInfo(HWND window,
                                    int scrollBar,
                                    ScrollAnchorKind kind,
                                    ScrollAnchorSnapshot *snapshot)
{
    if (snapshot == nullptr || !isValidWindowHandle(window))
    {
        return false;
    }

    SCROLLINFO info;
    ZeroMemory(&info, sizeof(info));
    info.cbSize = sizeof(info);
    info.fMask = SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS;
    if (!GetScrollInfo(window, scrollBar, &info))
    {
        return false;
    }

    const double range = qMax(0, info.nMax - info.nMin);
    const double page = static_cast<double>(info.nPage);
    if (range <= 0.0 && page <= 0.0)
    {
        return false;
    }

    snapshot->valid = true;
    snapshot->kind = kind;
    snapshot->sourceWindow = reinterpret_cast<quintptr>(window);
    snapshot->minimum = info.nMin;
    snapshot->maximum = info.nMax;
    snapshot->pageSize = page;
    snapshot->position = info.nPos;
    snapshot->viewportRatio = (page > 0.0) ? qBound(0.0, page / qMax(1.0, range + page), 1.0) : 0.0;
    snapshot->atTop = info.nPos <= info.nMin;
    snapshot->atBottom = (page > 0.0)
                         ? (info.nPos + static_cast<int>(page) >= info.nMax)
                         : (info.nPos >= info.nMax);
    snapshot->confidence = 0.98;
    snapshot->timestampMs = QDateTime::currentMSecsSinceEpoch();
    return true;
}
#endif

#if SCREENCAPTURE_HAS_UIA_INTERFACES
IUIAutomation *createAutomation()
{
    IUIAutomation *automation = nullptr;
    const HRESULT hr = CoCreateInstance(CLSID_CUIAutomation,
                                        nullptr,
                                        CLSCTX_INPROC_SERVER,
                                        IID_IUIAutomation,
                                        reinterpret_cast<void **>(&automation));
    if (FAILED(hr))
    {
        return nullptr;
    }

    return automation;
}

IUIAutomationElement *findElementWithPattern(IUIAutomation *automation,
                                             const ScrollTargetContext &context,
                                             PROPERTYID availabilityPropertyId) 
{
    if (automation == nullptr)
    {
        return nullptr;
    }

    for (quintptr candidateId : context.candidateWindows)
    {
        HWND window = windowHandleOf(candidateId);
        if (!isValidWindowHandle(window))
        {
            continue;
        }

        IUIAutomationElement *root = nullptr;
        if (FAILED(automation->ElementFromHandle(window, &root)) || root == nullptr)
        {
            continue;
        }

        if (variantBoolValue(root, availabilityPropertyId))
        {
            return root;
        }

        VARIANT desiredValue;
        VariantInit(&desiredValue);
        desiredValue.vt = VT_BOOL;
        desiredValue.boolVal = VARIANT_TRUE;

        IUIAutomationCondition *condition = nullptr;
        IUIAutomationElement *match = nullptr;
        const HRESULT conditionHr = automation->CreatePropertyCondition(availabilityPropertyId,
                                                                        desiredValue,
                                                                        &condition);
        if (SUCCEEDED(conditionHr) && condition != nullptr)
        {
            root->FindFirst(TreeScope_SubTree, condition, &match);
        }
        VariantClear(&desiredValue);
        releaseCom(condition);
        releaseCom(root);
        if (match != nullptr)
        {
            return match;
        }
    }

    return nullptr;
}

IUIAutomationElement *findVerticalScrollbarRangeElement(IUIAutomation *automation,
                                                        const ScrollTargetContext &context)
{
    if (automation == nullptr)
    {
        return nullptr;
    }

    VARIANT scrollBarTypeValue;
    VariantInit(&scrollBarTypeValue);
    scrollBarTypeValue.vt = VT_I4;
    scrollBarTypeValue.lVal = UIA_ScrollBarControlTypeId;

    VARIANT rangeAvailableValue;
    VariantInit(&rangeAvailableValue);
    rangeAvailableValue.vt = VT_BOOL;
    rangeAvailableValue.boolVal = VARIANT_TRUE;

    IUIAutomationCondition *scrollBarCondition = nullptr;
    IUIAutomationCondition *rangeCondition = nullptr;
    IUIAutomationCondition *combinedCondition = nullptr;

    if (FAILED(automation->CreatePropertyCondition(UIA_ControlTypePropertyId,
                                                   scrollBarTypeValue,
                                                   &scrollBarCondition))
        || FAILED(automation->CreatePropertyCondition(UIA_IsRangeValuePatternAvailablePropertyId,
                                                      rangeAvailableValue,
                                                      &rangeCondition))
        || FAILED(automation->CreateAndCondition(scrollBarCondition,
                                                 rangeCondition,
                                                 &combinedCondition)))
    {
        releaseCom(scrollBarCondition);
        releaseCom(rangeCondition);
        releaseCom(combinedCondition);
        VariantClear(&scrollBarTypeValue);
        VariantClear(&rangeAvailableValue);
        return nullptr;
    }

    VariantClear(&scrollBarTypeValue);
    VariantClear(&rangeAvailableValue);

    for (quintptr candidateId : context.candidateWindows)
    {
        HWND window = windowHandleOf(candidateId);
        if (!isValidWindowHandle(window))
        {
            continue;
        }

        IUIAutomationElement *root = nullptr;
        if (FAILED(automation->ElementFromHandle(window, &root)) || root == nullptr)
        {
            continue;
        }

        IUIAutomationElement *match = nullptr;
        root->FindFirst(TreeScope_SubTree, combinedCondition, &match);
        releaseCom(root);
        if (match == nullptr)
        {
            continue;
        }

        const int orientation = variantIntValue(match, UIA_OrientationPropertyId, OrientationType_None);
        if (orientation == OrientationType_Vertical || orientation == OrientationType_None)
        {
            releaseCom(scrollBarCondition);
            releaseCom(rangeCondition);
            releaseCom(combinedCondition);
            return match;
        }

        releaseCom(match);
    }

    releaseCom(scrollBarCondition);
    releaseCom(rangeCondition);
    releaseCom(combinedCondition);
    return nullptr;
}

ScrollAnchorSnapshot captureScrollPatternSnapshot(const ScrollTargetContext &context)
{
    ScopedComInitialization com;
    if (!com.isReady())
    {
        return invalidSnapshot(ScrollAnchorKind::UiaScrollPattern);
    }

    IUIAutomation *automation = createAutomation();
    if (automation == nullptr)
    {
        return invalidSnapshot(ScrollAnchorKind::UiaScrollPattern);
    }

    IUIAutomationElement *element = findElementWithPattern(automation,
                                                           context,
                                                           UIA_IsScrollPatternAvailablePropertyId);
    if (element == nullptr)
    {
        releaseCom(automation);
        return invalidSnapshot(ScrollAnchorKind::UiaScrollPattern);
    }

    IUIAutomationScrollPattern *pattern = nullptr;
    const HRESULT patternHr = element->GetCurrentPatternAs(UIA_ScrollPatternId,
                                                           IID_IUIAutomationScrollPattern,
                                                           reinterpret_cast<void **>(&pattern));
    if (FAILED(patternHr) || pattern == nullptr)
    {
        releaseCom(element);
        releaseCom(automation);
        return invalidSnapshot(ScrollAnchorKind::UiaScrollPattern);
    }

    BOOL verticallyScrollable = FALSE;
    double verticalPercent = -1.0;
    double verticalViewSize = 0.0;
    pattern->get_CurrentVerticallyScrollable(&verticallyScrollable);
    pattern->get_CurrentVerticalScrollPercent(&verticalPercent);
    pattern->get_CurrentVerticalViewSize(&verticalViewSize);

    ScrollAnchorSnapshot snapshot = invalidSnapshot(ScrollAnchorKind::UiaScrollPattern);
    if (verticallyScrollable && verticalPercent >= 0.0)
    {
        snapshot.valid = true;
        snapshot.sourceWindow = context.targetWindow;
        snapshot.minimum = 0.0;
        snapshot.maximum = 1.0;
        snapshot.pageSize = qBound(0.0, verticalViewSize / 100.0, 1.0);
        snapshot.viewportRatio = snapshot.pageSize;
        snapshot.position = qBound(0.0, verticalPercent / 100.0, 1.0);
        snapshot.atTop = snapshot.position <= 0.005;
        snapshot.atBottom = snapshot.position >= 0.995;
        snapshot.confidence = 0.88;
        snapshot.timestampMs = QDateTime::currentMSecsSinceEpoch();
    }

    releaseCom(pattern);
    releaseCom(element);
    releaseCom(automation);
    return snapshot;
}

ScrollAnchorSnapshot captureRangeValueSnapshot(const ScrollTargetContext &context)
{
    ScopedComInitialization com;
    if (!com.isReady())
    {
        return invalidSnapshot(ScrollAnchorKind::UiaRangeValue);
    }

    IUIAutomation *automation = createAutomation();
    if (automation == nullptr)
    {
        return invalidSnapshot(ScrollAnchorKind::UiaRangeValue);
    }

    IUIAutomationElement *element = findVerticalScrollbarRangeElement(automation, context);
    if (element == nullptr)
    {
        releaseCom(automation);
        return invalidSnapshot(ScrollAnchorKind::UiaRangeValue);
    }

    IUIAutomationRangeValuePattern *pattern = nullptr;
    const HRESULT patternHr = element->GetCurrentPatternAs(UIA_RangeValuePatternId,
                                                           IID_IUIAutomationRangeValuePattern,
                                                           reinterpret_cast<void **>(&pattern));
    if (FAILED(patternHr) || pattern == nullptr)
    {
        releaseCom(element);
        releaseCom(automation);
        return invalidSnapshot(ScrollAnchorKind::UiaRangeValue);
    }

    double currentValue = 0.0;
    double minimum = 0.0;
    double maximum = 0.0;
    double largeChange = 0.0;
    pattern->get_CurrentValue(&currentValue);
    pattern->get_CurrentMinimum(&minimum);
    pattern->get_CurrentMaximum(&maximum);
    pattern->get_CurrentLargeChange(&largeChange);

    ScrollAnchorSnapshot snapshot = invalidSnapshot(ScrollAnchorKind::UiaRangeValue);
    if (maximum > minimum)
    {
        snapshot.valid = true;
        snapshot.sourceWindow = context.targetWindow;
        snapshot.minimum = minimum;
        snapshot.maximum = maximum;
        snapshot.pageSize = qMax(0.0, largeChange);
        snapshot.viewportRatio = safeViewportRatio(snapshot);
        snapshot.position = currentValue;
        snapshot.atTop = currentValue <= minimum + 0.5;
        snapshot.atBottom = currentValue >= maximum - 0.5;
        snapshot.confidence = (largeChange > 0.0) ? 0.76 : 0.68;
        snapshot.timestampMs = QDateTime::currentMSecsSinceEpoch();
    }

    releaseCom(pattern);
    releaseCom(element);
    releaseCom(automation);
    return snapshot;
}
#endif

class Win32ScrollInfoAnchorProvider : public ScrollAnchorProvider
{
public:
    ScrollAnchorKind kind() const override
    {
        return ScrollAnchorKind::Win32ScrollInfo;
    }

    ScrollAnchorSnapshot capture(const ScrollTargetContext &context,
                                 const QRect &captureRect,
                                 const QImage &frameHint) const override
    {
        Q_UNUSED(captureRect)
        Q_UNUSED(frameHint)
#ifndef Q_OS_WIN
        return invalidSnapshot(kind());
#else
        if (!context.valid)
        {
            return invalidSnapshot(kind());
        }

        ScrollAnchorSnapshot snapshot = invalidSnapshot(kind());
        for (quintptr candidateId : context.candidateWindows)
        {
            const HWND window = windowHandleOf(candidateId);
            if (!isValidWindowHandle(window))
            {
                continue;
            }

            if (isScrollbarControlWindow(window) && populateSnapshotFromScrollInfo(window, SB_CTL, kind(), &snapshot))
            {
                return snapshot;
            }

            if (populateSnapshotFromScrollInfo(window, SB_VERT, kind(), &snapshot))
            {
                return snapshot;
            }
        }

        return invalidSnapshot(kind());
#endif
    }
};

class UiaScrollPatternAnchorProvider : public ScrollAnchorProvider
{
public:
    ScrollAnchorKind kind() const override
    {
        return ScrollAnchorKind::UiaScrollPattern;
    }

    ScrollAnchorSnapshot capture(const ScrollTargetContext &context,
                                 const QRect &captureRect,
                                 const QImage &frameHint) const override
    {
        Q_UNUSED(captureRect)
        Q_UNUSED(frameHint)
#if SCREENCAPTURE_HAS_UIA_INTERFACES
        return captureScrollPatternSnapshot(context);
#else
        qCDebug(lcAnchorLog) << "uia scroll pattern unavailable in current toolchain, fallback to other anchors";
        return invalidSnapshot(kind());
#endif
    }
};

class UiaRangeValueAnchorProvider : public ScrollAnchorProvider
{
public:
    ScrollAnchorKind kind() const override
    {
        return ScrollAnchorKind::UiaRangeValue;
    }

    ScrollAnchorSnapshot capture(const ScrollTargetContext &context,
                                 const QRect &captureRect,
                                 const QImage &frameHint) const override
    {
        Q_UNUSED(captureRect)
        Q_UNUSED(frameHint)
#if SCREENCAPTURE_HAS_UIA_INTERFACES
        return captureRangeValueSnapshot(context);
#else
        qCDebug(lcAnchorLog) << "uia range value unavailable in current toolchain, fallback to other anchors";
        return invalidSnapshot(kind());
#endif
    }
};

class VisualScrollbarAnchorProvider : public ScrollAnchorProvider
{
public:
    ScrollAnchorKind kind() const override
    {
        return ScrollAnchorKind::VisualScrollbar;
    }

    ScrollAnchorSnapshot capture(const ScrollTargetContext &context,
                                 const QRect &captureRect,
                                 const QImage &frameHint) const override
    {
        Q_UNUSED(context)
        Q_UNUSED(captureRect)

        if (frameHint.isNull())
        {
            return invalidSnapshot(kind());
        }

        QImage frame = frameHint.convertToFormat(QImage::Format_ARGB32_Premultiplied);
        if (frame.isNull() || frame.width() < 32 || frame.height() < 120)
        {
            return invalidSnapshot(kind());
        }

        const int width = frame.width();
        const int height = frame.height();
        const int bandWidth = qBound(8, width / 18, 20);
        const int bandStart = width - bandWidth;
        const int innerStart = qMax(0, bandStart - bandWidth);
        if (bandStart <= innerStart)
        {
            return invalidSnapshot(kind());
        }

        QVector<double> rowSignal(height, 0.0);
        double meanSignal = 0.0;
        for (int y = 0; y < height; ++y)
        {
            const QRgb *line = reinterpret_cast<const QRgb *>(frame.constScanLine(y));
            double bandMean = 0.0;
            double innerMean = 0.0;
            for (int x = bandStart; x < width; ++x)
            {
                bandMean += qGray(line[x]);
            }
            for (int x = innerStart; x < bandStart; ++x)
            {
                innerMean += qGray(line[x]);
            }
            bandMean /= qMax(1, width - bandStart);
            innerMean /= qMax(1, bandStart - innerStart);
            rowSignal[y] = bandMean - innerMean;
            meanSignal += rowSignal[y];
        }
        meanSignal /= qMax(1, height);

        QVector<double> smoothSignal(height, 0.0);
        double variance = 0.0;
        for (int y = 0; y < height; ++y)
        {
            double sum = 0.0;
            int count = 0;
            for (int offset = -2; offset <= 2; ++offset)
            {
                const int sampleY = y + offset;
                if (sampleY < 0 || sampleY >= height)
                {
                    continue;
                }
                sum += rowSignal.at(sampleY);
                ++count;
            }
            smoothSignal[y] = sum / qMax(1, count);
            const double centered = smoothSignal[y] - meanSignal;
            variance += centered * centered;
        }
        const double stddev = qSqrt(variance / qMax(1, height));
        const double threshold = qMax(5.5, stddev * 0.85);
        const int minThumbHeight = qMax(14, height / 30);
        const int maxThumbHeight = qMin(height - 4, height * 4 / 5);

        int bestStart = -1;
        int bestLength = 0;
        double bestContrast = 0.0;
        for (int start = 0; start < height;)
        {
            if (qAbs(smoothSignal.at(start) - meanSignal) < threshold)
            {
                ++start;
                continue;
            }

            int end = start + 1;
            int allowedGap = 2;
            while (end < height)
            {
                if (qAbs(smoothSignal.at(end) - meanSignal) >= threshold)
                {
                    ++end;
                    continue;
                }

                if (allowedGap <= 0)
                {
                    break;
                }

                --allowedGap;
                ++end;
            }

            const int length = end - start;
            if (length >= minThumbHeight && length <= maxThumbHeight)
            {
                double localContrast = 0.0;
                for (int y = start; y < end; ++y)
                {
                    localContrast += qAbs(smoothSignal.at(y) - meanSignal);
                }
                localContrast /= qMax(1, length);

                if (length > bestLength || (length == bestLength && localContrast > bestContrast))
                {
                    bestStart = start;
                    bestLength = length;
                    bestContrast = localContrast;
                }
            }

            start = end;
        }

        if (bestStart < 0 || bestLength <= 0)
        {
            return invalidSnapshot(kind());
        }

        const double thumbHeightRatio = qBound(0.0, static_cast<double>(bestLength) / static_cast<double>(height), 1.0);
        if (thumbHeightRatio < 0.04 || thumbHeightRatio >= 0.92)
        {
            return invalidSnapshot(kind());
        }

        const int usableTrack = qMax(1, height - bestLength);
        const double normalizedPosition = qBound(0.0,
                                                 static_cast<double>(bestStart) / static_cast<double>(usableTrack),
                                                 1.0);
        const double confidence = qBound(0.35,
                                         bestContrast / 28.0 + thumbHeightRatio * 0.22,
                                         0.64);
        if (confidence < 0.38)
        {
            return invalidSnapshot(kind());
        }

        ScrollAnchorSnapshot snapshot = invalidSnapshot(kind());
        snapshot.valid = true;
        snapshot.position = normalizedPosition;
        snapshot.minimum = 0.0;
        snapshot.maximum = 1.0;
        snapshot.pageSize = thumbHeightRatio;
        snapshot.viewportRatio = thumbHeightRatio;
        snapshot.thumbTopRatio = static_cast<double>(bestStart) / static_cast<double>(height);
        snapshot.thumbHeightRatio = thumbHeightRatio;
        snapshot.atTop = normalizedPosition <= 0.02;
        snapshot.atBottom = normalizedPosition >= 0.98;
        snapshot.confidence = confidence;
        snapshot.timestampMs = QDateTime::currentMSecsSinceEpoch();
        return snapshot;
    }
};
}

ScrollAnchorResolver::ScrollAnchorResolver()
{
    m_providers.emplace_back(std::make_unique<Win32ScrollInfoAnchorProvider>());
    m_providers.emplace_back(std::make_unique<UiaScrollPatternAnchorProvider>());
    m_providers.emplace_back(std::make_unique<UiaRangeValueAnchorProvider>());
    m_providers.emplace_back(std::make_unique<VisualScrollbarAnchorProvider>());
}

ScrollAnchorResolver::~ScrollAnchorResolver() = default;

ScrollAnchorSnapshot ScrollAnchorResolver::captureBestSnapshot(const ScrollTargetContext &context,
                                                               const QRect &captureRect,
                                                               const QImage &frameHint,
                                                               ScrollAnchorKind preferredKind) const
{
    auto tryCapture = [&](const ScrollAnchorProvider *provider) -> ScrollAnchorSnapshot
    {
        if (provider == nullptr)
        {
            return invalidSnapshot();
        }

        const ScrollAnchorSnapshot snapshot = provider->capture(context, captureRect, frameHint);
        if (snapshot.valid)
        {
            qCInfo(lcAnchorLog).noquote() << QStringLiteral("provider=%1 pos=%2 page=%3 viewport=%4 conf=%5 bottom=%6")
                                                 .arg(anchorKindName(snapshot.kind))
                                                 .arg(snapshot.position, 0, 'f', 4)
                                                 .arg(snapshot.pageSize, 0, 'f', 4)
                                                 .arg(safeViewportRatio(snapshot), 0, 'f', 4)
                                                 .arg(snapshot.confidence, 0, 'f', 3)
                                                 .arg(snapshot.atBottom ? QStringLiteral("true") : QStringLiteral("false"));
        }
        return snapshot;
    };

    if (preferredKind != ScrollAnchorKind::None)
    {
        for (const std::unique_ptr<ScrollAnchorProvider> &provider : m_providers)
        {
            if (provider->kind() != preferredKind)
            {
                continue;
            }

            const ScrollAnchorSnapshot preferredSnapshot = tryCapture(provider.get());
            if (preferredSnapshot.valid)
            {
                return preferredSnapshot;
            }
        }
    }

    for (const std::unique_ptr<ScrollAnchorProvider> &provider : m_providers)
    {
        if (preferredKind != ScrollAnchorKind::None && provider->kind() == preferredKind)
        {
            continue;
        }

        const ScrollAnchorSnapshot snapshot = tryCapture(provider.get());
        if (snapshot.valid)
        {
            return snapshot;
        }
    }

    qCInfo(lcAnchorLog) << "provider=None";
    return invalidSnapshot();
}