#include "dxgiduplicationbackend.h"

#ifdef Q_OS_WIN
#include <d3d11.h>
#include <dxgi1_2.h>
#include <windows.h>

#include <cstring>
#endif

namespace
{
#ifdef Q_OS_WIN
template<typename T>
void safeRelease(T **value)
{
    if (value != nullptr && *value != nullptr)
    {
        (*value)->Release();
        *value = nullptr;
    }
}
#endif
}

DxgiDuplicationBackend::DxgiDuplicationBackend()
{
}

DxgiDuplicationBackend::~DxgiDuplicationBackend()
{
    endObservation();
#ifdef Q_OS_WIN
    resetDuplicator();
#endif
}

QString DxgiDuplicationBackend::backendName() const
{
    return QStringLiteral("DXGI");
}

bool DxgiDuplicationBackend::isAvailable() const
{
#ifdef Q_OS_WIN
    return m_runtimeAvailable;
#else
    return false;
#endif
}

bool DxgiDuplicationBackend::beginObservation(const QRect &globalRect)
{
    if (!LongCaptureBackend::beginObservation(globalRect))
    {
        return false;
    }

#ifdef Q_OS_WIN
    if (!ensureDuplicatorForRect(globalRect))
    {
        LongCaptureBackend::endObservation();
        return false;
    }

    return true;
#else
    return false;
#endif
}

CaptureFrame DxgiDuplicationBackend::tryGetLatestFrame()
{
#ifdef Q_OS_WIN
    CaptureFrame frame;
    if (!observationRect().isValid() || !ensureDuplicatorForRect(observationRect()))
    {
        return frame;
    }

    acquireFrame(&frame, 0);
    return frame;
#else
    return CaptureFrame();
#endif
}

void DxgiDuplicationBackend::endObservation()
{
    LongCaptureBackend::endObservation();
}

QImage DxgiDuplicationBackend::capture(const QRect &globalRect)
{
#ifdef Q_OS_WIN
    if (!beginObservation(globalRect))
    {
        return m_gdiFallback.capture(globalRect);
    }

    CaptureFrame frame;
    const bool acquired = acquireFrame(&frame, 16);
    endObservation();
    if (acquired && frame.isValid())
    {
        return frame.image;
    }

    return m_gdiFallback.capture(globalRect);
#else
    Q_UNUSED(globalRect)
    return QImage();
#endif
}

#ifdef Q_OS_WIN
bool DxgiDuplicationBackend::ensureDuplicatorForRect(const QRect &globalRect)
{
    if (!globalRect.isValid())
    {
        return false;
    }

    POINT centerPoint;
    centerPoint.x = globalRect.center().x();
    centerPoint.y = globalRect.center().y();
    HMONITOR targetMonitor = MonitorFromPoint(centerPoint, MONITOR_DEFAULTTONEAREST);
    if (targetMonitor == nullptr)
    {
        return false;
    }

    if (m_duplication != nullptr && m_activeMonitor == targetMonitor && m_activeOutputRect.contains(globalRect))
    {
        return true;
    }

    resetDuplicator();

    IDXGIFactory1 *factory = nullptr;
    const HRESULT factoryHr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void **>(&factory));
    if (FAILED(factoryHr))
    {
        m_runtimeAvailable = false;
        return false;
    }

    bool resolved = false;
    for (UINT adapterIndex = 0; !resolved; ++adapterIndex)
    {
        IDXGIAdapter1 *adapter = nullptr;
        if (factory->EnumAdapters1(adapterIndex, &adapter) == DXGI_ERROR_NOT_FOUND)
        {
            break;
        }

        for (UINT outputIndex = 0; !resolved; ++outputIndex)
        {
            IDXGIOutput *output = nullptr;
            if (adapter->EnumOutputs(outputIndex, &output) == DXGI_ERROR_NOT_FOUND)
            {
                break;
            }

            DXGI_OUTPUT_DESC outputDesc;
            const HRESULT descHr = output->GetDesc(&outputDesc);
            if (FAILED(descHr) || outputDesc.Monitor != targetMonitor)
            {
                safeRelease(&output);
                continue;
            }

            IDXGIOutput1 *output1 = nullptr;
            if (FAILED(output->QueryInterface(__uuidof(IDXGIOutput1), reinterpret_cast<void **>(&output1))))
            {
                safeRelease(&output);
                continue;
            }

            static const D3D_FEATURE_LEVEL kFeatureLevels[] = {
                D3D_FEATURE_LEVEL_11_1,
                D3D_FEATURE_LEVEL_11_0,
                D3D_FEATURE_LEVEL_10_1,
                D3D_FEATURE_LEVEL_10_0
            };

            ID3D11Device *device = nullptr;
            ID3D11DeviceContext *context = nullptr;
            D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
            const HRESULT deviceHr = D3D11CreateDevice(adapter,
                                                       D3D_DRIVER_TYPE_UNKNOWN,
                                                       nullptr,
                                                       D3D11_CREATE_DEVICE_BGRA_SUPPORT,
                                                       kFeatureLevels,
                                                       ARRAYSIZE(kFeatureLevels),
                                                       D3D11_SDK_VERSION,
                                                       &device,
                                                       &featureLevel,
                                                       &context);
            if (FAILED(deviceHr))
            {
                safeRelease(&output1);
                safeRelease(&output);
                continue;
            }

            IDXGIOutputDuplication *duplication = nullptr;
            const HRESULT duplicationHr = output1->DuplicateOutput(device, &duplication);
            if (FAILED(duplicationHr))
            {
                safeRelease(&context);
                safeRelease(&device);
                safeRelease(&output1);
                safeRelease(&output);
                continue;
            }

            const QRect outputRect(outputDesc.DesktopCoordinates.left,
                                   outputDesc.DesktopCoordinates.top,
                                   outputDesc.DesktopCoordinates.right - outputDesc.DesktopCoordinates.left,
                                   outputDesc.DesktopCoordinates.bottom - outputDesc.DesktopCoordinates.top);
            if (!outputRect.contains(globalRect))
            {
                safeRelease(&duplication);
                safeRelease(&context);
                safeRelease(&device);
                safeRelease(&output1);
                safeRelease(&output);
                continue;
            }

            m_adapter = adapter;
            m_device = device;
            m_context = context;
            m_duplication = duplication;
            m_activeMonitor = targetMonitor;
            m_activeOutputRect = outputRect;
            resolved = true;

            safeRelease(&output1);
            safeRelease(&output);
        }

        if (!resolved)
        {
            safeRelease(&adapter);
        }
    }

    safeRelease(&factory);
    return resolved;
}

void DxgiDuplicationBackend::resetDuplicator()
{
    safeRelease(&m_stagingTexture);
    safeRelease(&m_duplication);
    safeRelease(&m_context);
    safeRelease(&m_device);
    safeRelease(&m_adapter);
    m_activeMonitor = nullptr;
    m_activeOutputRect = QRect();
}

bool DxgiDuplicationBackend::acquireFrame(CaptureFrame *frame, int timeoutMs)
{
    if (frame == nullptr || m_duplication == nullptr || !observationRect().isValid())
    {
        return false;
    }

    DXGI_OUTDUPL_FRAME_INFO frameInfo;
    ZeroMemory(&frameInfo, sizeof(frameInfo));

    IDXGIResource *resource = nullptr;
    const HRESULT acquireHr = m_duplication->AcquireNextFrame(timeoutMs, &frameInfo, &resource);
    if (acquireHr == DXGI_ERROR_WAIT_TIMEOUT)
    {
        return false;
    }

    if (FAILED(acquireHr))
    {
        if (acquireHr == DXGI_ERROR_ACCESS_LOST)
        {
            resetDuplicator();
        }
        safeRelease(&resource);
        return false;
    }

    ID3D11Texture2D *texture = nullptr;
    const HRESULT textureHr = resource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&texture));
    bool ok = false;
    if (SUCCEEDED(textureHr))
    {
        const QImage image = copyFrameToImage(texture, observationRect());
        if (!image.isNull())
        {
            *frame = buildFrame(image);
            if (frameInfo.LastPresentTime.QuadPart > 0)
            {
                frame->timestampMs = static_cast<qint64>(frameInfo.LastPresentTime.QuadPart / 10000);
            }
            ok = true;
        }
    }

    safeRelease(&texture);
    safeRelease(&resource);
    m_duplication->ReleaseFrame();
    return ok;
}

QImage DxgiDuplicationBackend::copyFrameToImage(ID3D11Texture2D *texture, const QRect &captureRect)
{
    if (texture == nullptr || m_device == nullptr || m_context == nullptr || !m_activeOutputRect.contains(captureRect))
    {
        return m_gdiFallback.capture(captureRect);
    }

    D3D11_TEXTURE2D_DESC textureDesc;
    ZeroMemory(&textureDesc, sizeof(textureDesc));
    texture->GetDesc(&textureDesc);

    if (textureDesc.Format != DXGI_FORMAT_B8G8R8A8_UNORM
        && textureDesc.Format != DXGI_FORMAT_B8G8R8A8_UNORM_SRGB)
    {
        return QImage();
    }

    const bool needNewStaging = (m_stagingTexture == nullptr);
    if (needNewStaging)
    {
        D3D11_TEXTURE2D_DESC stagingDesc = textureDesc;
        stagingDesc.BindFlags = 0;
        stagingDesc.MiscFlags = 0;
        stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        stagingDesc.Usage = D3D11_USAGE_STAGING;
        if (FAILED(m_device->CreateTexture2D(&stagingDesc, nullptr, &m_stagingTexture)))
        {
            return QImage();
        }
    }

    m_context->CopyResource(m_stagingTexture, texture);

    D3D11_MAPPED_SUBRESOURCE mapped;
    ZeroMemory(&mapped, sizeof(mapped));
    if (FAILED(m_context->Map(m_stagingTexture, 0, D3D11_MAP_READ, 0, &mapped)))
    {
        return QImage();
    }

    const int sourceX = captureRect.x() - m_activeOutputRect.x();
    const int sourceY = captureRect.y() - m_activeOutputRect.y();
    const int rowBytes = captureRect.width() * 4;

    QImage image(captureRect.size(), QImage::Format_ARGB32);
    for (int row = 0; row < image.height(); ++row)
    {
        const uchar *source = static_cast<const uchar *>(mapped.pData)
                              + (sourceY + row) * mapped.RowPitch
                              + sourceX * 4;
        std::memcpy(image.scanLine(row), source, static_cast<size_t>(rowBytes));
    }

    m_context->Unmap(m_stagingTexture, 0);
    return LongCaptureBackend::normalizeCapturedImage(image);
}
#endif
