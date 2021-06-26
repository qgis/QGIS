// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.12.25

#include <Graphics/DX11/GTGraphicsDX11PCH.h>
#include <Graphics/DX11/DX11Engine.h>
#include <Graphics/DX11/DX11BlendState.h>
#include <Graphics/DX11/DX11ComputeShader.h>
#include <Graphics/DX11/DX11ConstantBuffer.h>
#include <Graphics/DX11/DX11DepthStencilState.h>
#include <Graphics/DX11/DX11DrawTarget.h>
#include <Graphics/DX11/DX11GeometryShader.h>
#include <Graphics/DX11/DX11IndexBuffer.h>
#include <Graphics/DX11/DX11IndirectArgumentsBuffer.h>
#include <Graphics/DX11/DX11PixelShader.h>
#include <Graphics/DX11/DX11RasterizerState.h>
#include <Graphics/DX11/DX11RawBuffer.h>
#include <Graphics/DX11/DX11SamplerState.h>
#include <Graphics/DX11/DX11StructuredBuffer.h>
#include <Graphics/DX11/DX11Texture1.h>
#include <Graphics/DX11/DX11Texture1Array.h>
#include <Graphics/DX11/DX11Texture2.h>
#include <Graphics/DX11/DX11Texture2Array.h>
#include <Graphics/DX11/DX11Texture3.h>
#include <Graphics/DX11/DX11TextureCube.h>
#include <Graphics/DX11/DX11TextureCubeArray.h>
#include <Graphics/DX11/DX11TextureBuffer.h>
#include <Graphics/DX11/DX11VertexBuffer.h>
#include <Graphics/DX11/DX11VertexShader.h>
#include <Graphics/DX11/HLSLProgramFactory.h>
#include <Graphics/DX11/HLSLComputeProgram.h>
using namespace gte;

DX11Engine::~DX11Engine()
{
    DX11::FinalRelease(mWaitQuery);

    // The render state objects (and fonts) are destroyed first so that the
    // render state objects are removed from the bridges before they are
    // cleared later in the destructor.
    DestroyDefaultObjects();

    GraphicsObject::UnsubscribeForDestruction(mGOListener);
    mGOListener = nullptr;

    DrawTarget::UnsubscribeForDestruction(mDTListener);
    mDTListener = nullptr;

    if (mGOMap.HasElements())
    {
        if (mWarnOnNonemptyBridges)
        {
            LogWarning("Bridge map is nonempty on destruction.");
        }

        mGOMap.RemoveAll();
    }

    if (mDTMap.HasElements())
    {
        if (mWarnOnNonemptyBridges)
        {
            LogWarning("Draw target map nonempty on destruction.");
        }

        mDTMap.RemoveAll();
    }

    if (mILMap->HasElements())
    {
        if (mWarnOnNonemptyBridges)
        {
            LogWarning("Input layout map nonempty on destruction.");
        }

        mILMap->UnbindAll();
    }
    mILMap = nullptr;

    if (mIsGraphicsDevice)
    {
        DestroyBackBuffer();
        DestroySwapChain();
    }
    DestroyDevice();

    DX11::SafeRelease(mAdapter);
}

DX11Engine::DX11Engine()
{
    mIsGraphicsDevice = false;
    Initialize(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, false);
    if (CreateDevice())
    {
        CreateDefaultObjects();
    }
}

DX11Engine::DX11Engine(IDXGIAdapter* adapter, D3D_DRIVER_TYPE driverType,
    HMODULE softwareModule, UINT flags)
{
    mIsGraphicsDevice = false;
    Initialize(adapter, driverType, softwareModule, flags, false);
    if (CreateDevice())
    {
        CreateDefaultObjects();
    }
}

DX11Engine::DX11Engine(HWND handle, UINT xSize, UINT ySize, bool useDepth24Stencil8)
{
    mIsGraphicsDevice = true;
    Initialize(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, useDepth24Stencil8);

    if (CreateDevice() &&
        CreateSwapChain(handle, xSize, ySize) &&
        CreateBackBuffer(xSize, ySize))
    {
        CreateDefaultObjects();
    }
    else
    {
        DestroyBackBuffer();
        DestroySwapChain();
        DestroyDevice();
    }
}

DX11Engine::DX11Engine(IDXGIAdapter* adapter, HWND handle, UINT xSize,
    UINT ySize, bool useDepth24Stencil8, D3D_DRIVER_TYPE driverType,
    HMODULE softwareModule, UINT flags)
{
    mIsGraphicsDevice = true;
    Initialize(adapter, driverType, softwareModule, flags, useDepth24Stencil8);

    if (CreateDevice() &&
        CreateSwapChain(handle, xSize, ySize) &&
        CreateBackBuffer(xSize, ySize))
    {
        CreateDefaultObjects();
    }
    else
    {
        DestroyBackBuffer();
        DestroySwapChain();
        DestroyDevice();
    }
}

DX11GraphicsObject* DX11Engine::Share(std::shared_ptr<Texture2> const& texture, DX11Engine* engine)
{
    LogAssert(texture != nullptr && engine != nullptr, "Invalid input.");
    LogAssert(texture->IsShared(), "The texture must allow sharing.");

    DX11GraphicsObject* dxTexture = static_cast<DX11GraphicsObject*>(Get(texture));
    if (dxTexture)
    {
        // The texture is already shared by 'this', so nothing to do.
        return dxTexture;
    }

    dxTexture = static_cast<DX11GraphicsObject*>(engine->Get(texture));
    if (dxTexture)
    {
        std::shared_ptr<DX11GraphicsObject> dxShared;
        if (texture->GetType() == GT_TEXTURE2)
        {
            dxShared = std::make_shared<DX11Texture2>(mDevice, static_cast<DX11Texture2 const*>(dxTexture));
        }
        else if (texture->GetType() == GT_TEXTURE_RT)
        {
            dxShared = std::make_shared<DX11TextureRT>(mDevice, static_cast<DX11TextureRT const*>(dxTexture));
        }
        else // texture->GetType() == GT_TEXTURE_DS
        {
            dxShared = std::make_shared<DX11TextureDS>(mDevice, static_cast<DX11TextureDS const*>(dxTexture));
        }
        mGOMap.Insert(texture.get(), dxShared);
        return dxTexture;
    }

    // The texture is not bound to 'engine'; create a new binding for 'this'.
    return static_cast<DX11GraphicsObject*>(Bind(texture));
}

D3D11_MAPPED_SUBRESOURCE DX11Engine::MapForWrite(std::shared_ptr<Resource> const& resource, unsigned int sri)
{
    if (!resource->GetData())
    {
        LogWarning("Resource does not have system memory, creating it.");
        resource->CreateStorage();
    }

    DX11Resource* dxResource = static_cast<DX11Resource*>(Bind(resource));
    return dxResource->MapForWrite(mImmediate, sri);
}

void DX11Engine::Unmap(std::shared_ptr<Resource> const& resource, unsigned int sri)
{
    DX11Resource* dxResource = static_cast<DX11Resource*>(Bind(resource));
    dxResource->Unmap(mImmediate, sri);
}

bool DX11Engine::IsFullscreen(DXGIOutput const& output) const
{
    if (mSwapChain)
    {
        auto desc = output.GetDescription();
        std::wstring name(desc.DeviceName);
        auto iter = mFullscreenState.find(name);
        if (iter != mFullscreenState.end())
        {
            return iter->second;
        }

        // The 'output' has not yet been passed to SetFullscreen, so it is
        // safe to assume that window is not fullscreen.
        return false;
    }

    LogError("This function requires a swap chain.");
}

bool DX11Engine::SetFullscreen(DXGIOutput const& output, bool fullscreen)
{
    if (mSwapChain)
    {
        auto desc = output.GetDescription();
        std::wstring name(desc.DeviceName);
        auto iter = mFullscreenState.find(name);
        if (iter == mFullscreenState.end())
        {
            // This is the first time 'output' has been seen, so insert it
            // into the map indicating that the window is not fullscreen.
            iter = mFullscreenState.insert(std::make_pair(name, false)).first;
        }

        if (iter->second != fullscreen)
        {
            HRESULT hr;
            if (fullscreen)
            {
                hr = mSwapChain->SetFullscreenState(TRUE, output.GetOutput());
                if (SUCCEEDED(hr))
                {
                    iter->second = true;
                    return true;
                }
                else
                {
                    LogError("Failed to go fullscreen.");
                }
            }
            else
            {
                hr = mSwapChain->SetFullscreenState(FALSE, nullptr);
                if (SUCCEEDED(hr))
                {
                    iter->second = false;
                    return true;
                }
                else
                {
                    LogError("Failed to go windowed.");
                }
            }
        }
        else
        {
            // The requested state is the current state, so there is
            // nothing to do.
            return false;
        }
    }

    LogError("This function requires a swap chain.");
}

void DX11Engine::ExitFullscreen()
{
    if (mSwapChain)
    {
        BOOL isFullscreen;
        IDXGIOutput* display;
        if (S_OK == mSwapChain->GetFullscreenState(&isFullscreen, &display))
        {
            if (isFullscreen)
            {
                mSwapChain->SetFullscreenState(FALSE, nullptr);
            }
        }
    }
}

void DX11Engine::BeginTimer(DX11PerformanceCounter& counter)
{
    if (counter.mFrequencyQuery)
    {
        mImmediate->Begin(counter.mFrequencyQuery);

        mImmediate->End(counter.mStartTimeQuery);
        while (S_OK != mImmediate->GetData(counter.mStartTimeQuery,
            &counter.mStartTime, sizeof(counter.mStartTime), 0))
        {
            // Wait for end of query.
        }
    }
}

void DX11Engine::EndTimer(DX11PerformanceCounter& counter)
{
    if (counter.mFrequencyQuery)
    {
        mImmediate->End(counter.mFinalTimeQuery);
        while (S_OK != mImmediate->GetData(counter.mFinalTimeQuery,
            &counter.mFinalTime, sizeof(counter.mFinalTime), 0))
        {
            // Wait for end of query.
        }

        mImmediate->End(counter.mFrequencyQuery);
        while (S_OK != mImmediate->GetData(counter.mFrequencyQuery,
            &counter.mTimeStamp, sizeof(counter.mTimeStamp), 0))
        {
            // Wait for end of query.
        }
    }
}

void DX11Engine::Initialize(IDXGIAdapter* adapter, D3D_DRIVER_TYPE driverType,
    HMODULE softwareModule, UINT flags, bool useDepth24Stencil8)
{
    // Initialization of DX11Engine members.
    mAdapter = adapter;
    DX11::SafeAddRef(mAdapter);

    mDriverType = driverType;
    mSoftwareModule = softwareModule;
    mFlags = flags;
    mDevice = nullptr;
    mImmediate = nullptr;
    mFeatureLevel = D3D_FEATURE_LEVEL_9_1;
    mUseDepth24Stencil8 = useDepth24Stencil8;

    mSwapChain = nullptr;
    mColorBuffer = nullptr;
    mColorView = nullptr;
    mDepthStencilBuffer = nullptr;
    mDepthStencilView = nullptr;
    mViewport.TopLeftX = 0.0f;
    mViewport.TopLeftY = 0.0f;
    mViewport.Width = 0.0f;
    mViewport.Height = 0.0f;
    mViewport.MinDepth = 0.0f;
    mViewport.MaxDepth = 0.0f;

    mNumActiveRTs = 0;
    mActiveRT.fill(nullptr);
    mActiveDS = nullptr;
    mSaveViewport = mViewport;
    mSaveRT.fill(nullptr);
    mSaveDS = nullptr;
    mWaitQuery = nullptr;

    mBackBufferStaging = nullptr;

    // Initialization of GraphicsEngine members that depend on DX11.
    mILMap = std::make_unique<DX11InputLayoutManager>();

    mCreateGEObject =
    {
        nullptr,    // GT_GRAPHICS_OBJECT (abstract base)
        nullptr,    // GT_RESOURCE (abstract base)
        nullptr,    // GT_BUFFER (abstract base)
        &DX11ConstantBuffer::Create,
        &DX11TextureBuffer::Create,
        &DX11VertexBuffer::Create,
        &DX11IndexBuffer::Create,
        &DX11StructuredBuffer::Create,
        nullptr,  // TODO:  Implement TypedBuffer
        &DX11RawBuffer::Create,
        &DX11IndirectArgumentsBuffer::Create,
        nullptr,    // GT_TEXTURE (abstract base)
        nullptr,    // GT_TEXTURE_SINGLE (abstract base)
        &DX11Texture1::Create,
        &DX11Texture2::Create,
        &DX11TextureRT::Create,
        &DX11TextureDS::Create,
        &DX11Texture3::Create,
        nullptr,  // GT_TEXTURE_ARRAY (abstract base)
        &DX11Texture1Array::Create,
        &DX11Texture2Array::Create,
        &DX11TextureCube::Create,
        &DX11TextureCubeArray::Create,
        nullptr,    // GT_SHADER (abstract base)
        &DX11VertexShader::Create,
        &DX11GeometryShader::Create,
        &DX11PixelShader::Create,
        &DX11ComputeShader::Create,
        nullptr,    // GT_DRAWING_STATE (abstract base)
        &DX11SamplerState::Create,
        &DX11BlendState::Create,
        &DX11DepthStencilState::Create,
        &DX11RasterizerState::Create
    };

    mCreateGEDrawTarget = &DX11DrawTarget::Create;
}

bool DX11Engine::CreateDevice()
{
    if (mAdapter)
    {
        mDriverType = D3D_DRIVER_TYPE_UNKNOWN;
        return CreateBestMatchingDevice();
    }

    IDXGIFactory1* factory = nullptr;
    HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
    if (FAILED(hr))
    {
        DX11::SafeRelease(factory);
        return false;
    }

    if (mDriverType == D3D_DRIVER_TYPE_HARDWARE)
    {
        for (uint32_t i = 0; /**/; ++i)
        {
            IDXGIAdapter1* adapter = nullptr;
            if (factory->EnumAdapters1(i, &adapter) == DXGI_ERROR_NOT_FOUND)
            {
                DX11::SafeRelease(factory);
                return false;
            }

            DXGI_ADAPTER_DESC1 desc;
            hr = adapter->GetDesc1(&desc);
            if (FAILED(hr))
            {
                DX11::SafeRelease(factory);
                DX11::SafeRelease(adapter);
                return false;
            }

            if (desc.Flags != DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                if (CreateBestMatchingDevice())
                {
                    mAdapter = adapter;
                    DX11::SafeRelease(factory);
                    return true;
                }
            }
            else
            {
                mDriverType = D3D_DRIVER_TYPE_WARP;
                if (CreateBestMatchingDevice())
                {
                    mAdapter = adapter;
                    DX11::SafeRelease(factory);
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }
    }

    DX11::SafeRelease(factory);
    return false;
}

bool DX11Engine::CreateBestMatchingDevice()
{
    // Determine the subarray for creating the device.
    UINT const numFeatureLevels = 7;
    D3D_FEATURE_LEVEL const featureLevels[numFeatureLevels] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1
    };

    for (UINT i = 0; i < numFeatureLevels; ++i)
    {
        HRESULT hr = D3D11CreateDevice(mAdapter, mDriverType, mSoftwareModule,
            mFlags, &featureLevels[i], 1, D3D11_SDK_VERSION,
            &mDevice, &mFeatureLevel, &mImmediate);
        if (SUCCEEDED(hr))
        {
            mGEObjectCreator = mDevice;
            if (i == 0 || i == 1)
            {
                HLSLProgramFactory::defaultVersion = "5_0";
            }
            else if (i == 2 || i == 3)
            {
                HLSLProgramFactory::defaultVersion = "4_0";
            }
            else if (i == 4)
            {
                HLSLProgramFactory::defaultVersion = "4_0_level_9_3";
            }
            else
            {
                HLSLProgramFactory::defaultVersion = "4_0_level_9_1";
            }
            return true;
        }
    }

    LogError("Failed to create device");
}

bool DX11Engine::CreateSwapChain(HWND handle, UINT xSize, UINT ySize)
{
    mXSize = xSize;
    mYSize = ySize;

    struct DXGIInterfaces
    {
        DXGIInterfaces()
            :
            device(nullptr),
            adapter(nullptr),
            factory(nullptr)
        {
        }

        ~DXGIInterfaces()
        {
            DX11::SafeRelease(factory);
            DX11::SafeRelease(adapter);
            DX11::SafeRelease(device);
        }

        IDXGIDevice* device;
        IDXGIAdapter* adapter;
        IDXGIFactory1* factory;
    };

    DXGIInterfaces dxgi;
    DX11Log(mDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgi.device)));
    DX11Log(dxgi.device->GetAdapter(&dxgi.adapter));
    DX11Log(dxgi.adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgi.factory)));

    DXGI_SWAP_CHAIN_DESC desc;
    desc.BufferDesc.Width = xSize;
    desc.BufferDesc.Height = ySize;
    desc.BufferDesc.RefreshRate.Numerator = 0;
    desc.BufferDesc.RefreshRate.Denominator = 1;
    desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = 2;
    desc.OutputWindow = handle;
    desc.Windowed = TRUE;
    desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    desc.Flags = 0;

    DX11Log(dxgi.factory->CreateSwapChain(dxgi.device, &desc, &mSwapChain));

#if defined(GTE_GRAPHICS_USE_NAMED_OBJECTS)
    DX11Log(DX11::SetPrivateName(mSwapChain, "DX11Engine::mSwapChain"));
#endif

    return true;
}

bool DX11Engine::CreateBackBuffer(UINT xSize, UINT ySize)
{
    struct BackBuffer
    {
        BackBuffer()
        {
            SetToNull();
        }

        ~BackBuffer()
        {
            DX11::SafeRelease(depthStencilView);
            DX11::SafeRelease(depthStencilBuffer);
            DX11::SafeRelease(colorView);
            DX11::SafeRelease(colorBuffer);
        }

        void SetToNull()
        {
            colorBuffer = nullptr;
            colorView = nullptr;
            depthStencilBuffer = nullptr;
            depthStencilView = nullptr;
        }

        ID3D11Texture2D* colorBuffer;
        ID3D11RenderTargetView* colorView;
        ID3D11Texture2D* depthStencilBuffer;
        ID3D11DepthStencilView* depthStencilView;
    };

    BackBuffer bb;

    // Create the color buffer and its view.
    DX11Log(mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&bb.colorBuffer)));

#if defined(GTE_GRAPHICS_USE_NAMED_OBJECTS)
    DX11Log(DX11::SetPrivateName(bb.colorBuffer, "DX11Engine::mColorBuffer"));
#endif

    DX11Log(mDevice->CreateRenderTargetView(bb.colorBuffer, nullptr, &bb.colorView));

#if defined(GTE_GRAPHICS_USE_NAMED_OBJECTS)
    DX11Log(DX11::SetPrivateName(bb.colorView, "DX11Engine::mColorView"));
#endif

    D3D11_TEXTURE2D_DESC bbdesc;
    bb.colorBuffer->GetDesc(&bbdesc);
    bbdesc.BindFlags = 0;
    bbdesc.CPUAccessFlags = D3D10_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
    bbdesc.Usage = D3D11_USAGE_STAGING;
    mBackBufferStaging = nullptr;
    DX11Log(mDevice->CreateTexture2D(&bbdesc, nullptr, &mBackBufferStaging));

    // Create the depth-stencil buffer and its view.
    D3D11_TEXTURE2D_DESC desc;
    desc.Width = xSize;
    desc.Height = ySize;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = (mUseDepth24Stencil8 ? DXGI_FORMAT_D24_UNORM_S8_UINT : DXGI_FORMAT_D32_FLOAT);
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_NONE;
    desc.MiscFlags = D3D11_RESOURCE_MISC_NONE;

    DX11Log(mDevice->CreateTexture2D(&desc, nullptr, &bb.depthStencilBuffer));

#if defined(GTE_GRAPHICS_USE_NAMED_OBJECTS)
    DX11Log(DX11::SetPrivateName(bb.depthStencilBuffer, "DX11Engine::mDepthStencilBuffer"));
#endif

    DX11Log(mDevice->CreateDepthStencilView(bb.depthStencilBuffer, nullptr, &bb.depthStencilView));

#if defined(GTE_GRAPHICS_USE_NAMED_OBJECTS)
    DX11Log(DX11::SetPrivateName(bb.depthStencilView, "DX11Engine::mDepthStencilView"));
#endif

    // The back buffer has been created successfully.  Transfer the
    // resources to 'this' members.
    mColorBuffer = bb.colorBuffer;
    mColorView = bb.colorView;
    mDepthStencilBuffer = bb.depthStencilBuffer;
    mDepthStencilView = bb.depthStencilView;
    bb.SetToNull();

    mViewport.Width = static_cast<float>(xSize);
    mViewport.Height = static_cast<float>(ySize);
    mViewport.TopLeftX = 0.0f;
    mViewport.TopLeftY = 0.0f;
    mViewport.MinDepth = 0.0f;
    mViewport.MaxDepth = 1.0f;
    mImmediate->RSSetViewports(1, &mViewport);

    mNumActiveRTs = 1;
    mActiveRT[0] = mColorView;
    mActiveDS = mDepthStencilView;
    mImmediate->OMSetRenderTargets(1, mActiveRT.data(), mActiveDS);
    return true;
}

void DX11Engine::CreateDefaultObjects()
{
    std::shared_ptr<HLSLProgramFactory> factory = std::make_shared<HLSLProgramFactory>();
    mDefaultFont = std::make_shared<FontArialW400H18>(factory, 256);
    SetDefaultFont();

    CreateDefaultGlobalState();
}

void DX11Engine::DestroyDefaultObjects()
{
    if (mDefaultFont)
    {
        mDefaultFont = nullptr;
        mActiveFont = nullptr;
    }

    DestroyDefaultGlobalState();
}

bool DX11Engine::DestroyDevice()
{
    // TODO: The mSwapChain->Present() call sometimes creates a second
    // ID3D11Context object in order to create an ID3D11RenderTargetView.
    // On exit from an application, the FinalRelease(mDevice) call throws
    // an exception because the number of references to mDevice is positive.
    // These references come from the second context. The time of destruction
    // of that context is nondeterministic (probably a timing issue with
    // threading), so the exception does not occur every time.
    //
    // The problem appears to be the introduction of the "flip" flags for
    // swap chain creation. The back buffer is unbound for writing during the
    // Present call. I have tried to figure out how to Magic Dance, but am
    // not yet successful. For now, I am using SafeRelease that does not
    // throw an exception.

    //return DX11::FinalRelease(mImmediate) == 0 && DX11::FinalRelease(mDevice) == 0;
    return DX11::SafeRelease(mImmediate) == 0 && DX11::SafeRelease(mDevice) == 0;
}

bool DX11Engine::DestroySwapChain()
{
    DX11::FinalRelease(mSwapChain);
    return true;
}

bool DX11Engine::DestroyBackBuffer()
{
    if (mImmediate)
    {
        ID3D11RenderTargetView* rtView = nullptr;
        ID3D11DepthStencilView* dsView = nullptr;
        mImmediate->OMSetRenderTargets(1, &rtView, dsView);
    }

    mActiveRT[0] = nullptr;
    mActiveDS = nullptr;

    return DX11::FinalRelease(mColorView) == 0
        && DX11::FinalRelease(mColorBuffer) == 0
        && DX11::FinalRelease(mBackBufferStaging) == 0
        && DX11::FinalRelease(mDepthStencilView) == 0
        && DX11::FinalRelease(mDepthStencilBuffer) == 0;
}

uint64_t DX11Engine::DrawPrimitive(VertexBuffer const* vbuffer, IndexBuffer const* ibuffer)
{
    UINT numActiveVertices = vbuffer->GetNumActiveElements();
    UINT vertexOffset = vbuffer->GetOffset();

    UINT numActiveIndices = ibuffer->GetNumActiveIndices();
    UINT firstIndex = ibuffer->GetFirstIndex();
    IPType type = ibuffer->GetPrimitiveType();

    switch (type)
    {
    case IPType::IP_POLYPOINT:
        mImmediate->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
        break;
    case IPType::IP_POLYSEGMENT_DISJOINT:
        mImmediate->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
        break;
    case IPType::IP_POLYSEGMENT_CONTIGUOUS:
        mImmediate->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
        break;
    case IPType::IP_TRIMESH:
        mImmediate->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        break;
    case IPType::IP_TRISTRIP:
        mImmediate->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        break;
    case IPType::IP_POLYSEGMENT_DISJOINT_ADJ:
        mImmediate->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ);
        break;
    case IPType::IP_POLYSEGMENT_CONTIGUOUS_ADJ:
        mImmediate->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ);
        break;
    case IPType::IP_TRIMESH_ADJ:
        mImmediate->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ);
        break;
    case IPType::IP_TRISTRIP_ADJ:
        mImmediate->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ);
        break;
    default:
        LogError("Unknown primitive topology = " + std::to_string(type));
    }

    ID3D11Query* occlusionQuery = nullptr;
    uint64_t numPixelsDrawn = 0;
    if (mAllowOcclusionQuery)
    {
        occlusionQuery = BeginOcclusionQuery();
    }

    if (ibuffer->IsIndexed())
    {
        if (numActiveIndices > 0)
        {
            mImmediate->DrawIndexed(numActiveIndices, firstIndex, vertexOffset);
        }
    }
    else
    {
        if (numActiveVertices > 0)
        {
            mImmediate->Draw(numActiveVertices, vertexOffset);
        }
    }

    if (mAllowOcclusionQuery)
    {
        numPixelsDrawn = EndOcclusionQuery(occlusionQuery);
    }

    return numPixelsDrawn;
}

ID3D11Query* DX11Engine::BeginOcclusionQuery()
{
    D3D11_QUERY_DESC desc;
    desc.Query = D3D11_QUERY_OCCLUSION;
    desc.MiscFlags = D3D11_QUERY_MISC_NONE;
    ID3D11Query* occlusionQuery = nullptr;
    HRESULT hr = mDevice->CreateQuery(&desc, &occlusionQuery);
    if (SUCCEEDED(hr))
    {
        mImmediate->Begin(occlusionQuery);
        return occlusionQuery;
    }

    LogError("CreateQuery failed.");
}

uint64_t DX11Engine::EndOcclusionQuery(ID3D11Query* occlusionQuery)
{
    if (occlusionQuery)
    {
        mImmediate->End(occlusionQuery);
        UINT64 data = 0;
        UINT size = sizeof(UINT64);
        while (S_OK != mImmediate->GetData(occlusionQuery, &data, size, 0))
        {
            // Wait for end of query.
        }
        DX11::SafeRelease(occlusionQuery);
        return data;
    }

    LogError("No query provided.");
}

bool DX11Engine::EnableShaders(std::shared_ptr<VisualEffect> const& effect,
    DX11VertexShader*& dxVShader, DX11GeometryShader*& dxGShader, DX11PixelShader*& dxPShader)
{
    dxVShader = nullptr;
    dxGShader = nullptr;
    dxPShader = nullptr;

    // Get the active vertex shader.
    LogAssert(effect->GetVertexShader(), "Effect must have a vertex shader.");
    dxVShader = static_cast<DX11VertexShader*>(Bind(effect->GetVertexShader()));

    // Get the active geometry shader (if any).
    if (effect->GetGeometryShader())
    {
        dxGShader = static_cast<DX11GeometryShader*>(Bind(effect->GetGeometryShader()));
    }

    // Get the active pixel shader.
    LogAssert(effect->GetPixelShader(), "Effect must have a pixel shader.");
    dxPShader = static_cast<DX11PixelShader*>(Bind(effect->GetPixelShader()));

    // Enable the shaders and resources.
    Enable(effect->GetVertexShader().get(), dxVShader);
    Enable(effect->GetPixelShader().get(), dxPShader);
    if (dxGShader)
    {
        Enable(effect->GetGeometryShader().get(), dxGShader);
    }

    return true;
}

void DX11Engine::DisableShaders(std::shared_ptr<VisualEffect> const& effect,
    DX11VertexShader* dxVShader, DX11GeometryShader* dxGShader, DX11PixelShader* dxPShader)
{
    if (dxGShader)
    {
        Disable(effect->GetGeometryShader().get(), dxGShader);
    }
    Disable(effect->GetPixelShader().get(), dxPShader);
    Disable(effect->GetVertexShader().get(), dxVShader);
}

void DX11Engine::Enable(Shader const* shader, DX11Shader* dxShader)
{
    dxShader->Enable(mImmediate);
    EnableCBuffers(shader, dxShader);
    EnableTBuffers(shader, dxShader);
    EnableSBuffers(shader, dxShader);
    EnableRBuffers(shader, dxShader);
    EnableTextures(shader, dxShader);
    EnableTextureArrays(shader, dxShader);
    EnableSamplers(shader, dxShader);
}

void DX11Engine::Disable(Shader const* shader, DX11Shader* dxShader)
{
    DisableSamplers(shader, dxShader);
    DisableTextureArrays(shader, dxShader);
    DisableTextures(shader, dxShader);
    DisableRBuffers(shader, dxShader);
    DisableSBuffers(shader, dxShader);
    DisableTBuffers(shader, dxShader);
    DisableCBuffers(shader, dxShader);
    dxShader->Disable(mImmediate);
}

void DX11Engine::EnableCBuffers(Shader const* shader, DX11Shader* dxShader)
{
    int const index = ConstantBuffer::shaderDataLookup;
    for (auto const& cb : shader->GetData(index))
    {
        if (cb.object)
        {
            DX11ConstantBuffer* dxCB = static_cast<DX11ConstantBuffer*>(Bind(cb.object));
            if (dxCB)
            {
                dxShader->EnableCBuffer(mImmediate, cb.bindPoint, dxCB->GetDXBuffer());
            }
            else
            {
                LogError("Failed to bind constant buffer.");
            }
        }
        else
        {
            LogError(cb.name + " is null constant buffer.");
        }
    }
}

void DX11Engine::DisableCBuffers(Shader const* shader, DX11Shader* dxShader)
{
    int const index = ConstantBuffer::shaderDataLookup;
    for (auto const& cb : shader->GetData(index))
    {
        dxShader->DisableCBuffer(mImmediate, cb.bindPoint);
    }
}

void DX11Engine::EnableTBuffers(Shader const* shader, DX11Shader* dxShader)
{
    int const index = TextureBuffer::shaderDataLookup;
    for (auto const& tb : shader->GetData(index))
    {
        if (tb.object)
        {
            DX11TextureBuffer* dxTB = static_cast<DX11TextureBuffer*>(Bind(tb.object));
            if (dxTB)
            {
                dxShader->EnableSRView(mImmediate, tb.bindPoint, dxTB->GetSRView());
            }
            else
            {
                LogError("Failed to bind texture buffer.");
            }
        }
        else
        {
            LogError(tb.name + " is null texture buffer.");
        }
    }
}

void DX11Engine::DisableTBuffers(Shader const* shader, DX11Shader* dxShader)
{
    int const index = TextureBuffer::shaderDataLookup;
    for (auto const& tb : shader->GetData(index))
    {
        dxShader->DisableSRView(mImmediate, tb.bindPoint);
    }
}

void DX11Engine::EnableSBuffers(Shader const* shader, DX11Shader* dxShader)
{
    int const index = StructuredBuffer::shaderDataLookup;
    for (auto const& sb : shader->GetData(index))
    {
        if (sb.object)
        {
            DX11StructuredBuffer* dxSB = static_cast<DX11StructuredBuffer*>(Bind(sb.object));
            if (dxSB)
            {
                if (sb.isGpuWritable)
                {
                    StructuredBuffer* gtSB = static_cast<StructuredBuffer*>(sb.object.get());

                    unsigned int numActive = (gtSB->GetKeepInternalCount() ?
                        0xFFFFFFFFu : gtSB->GetNumActiveElements());
                    dxShader->EnableUAView(mImmediate, sb.bindPoint, dxSB->GetUAView(), numActive);
                }
                else
                {
                    dxShader->EnableSRView(mImmediate, sb.bindPoint, dxSB->GetSRView());
                }
            }
            else
            {
                LogError("Failed to bind structured buffer.");
            }
        }
        else
        {
            LogError(sb.name + " is null structured buffer.");
        }
    }
}

void DX11Engine::DisableSBuffers(Shader const* shader, DX11Shader* dxShader)
{
    int const index = StructuredBuffer::shaderDataLookup;
    for (auto const& sb : shader->GetData(index))
    {
        if (sb.isGpuWritable)
        {
            dxShader->DisableUAView(mImmediate, sb.bindPoint);
        }
        else
        {
            dxShader->DisableSRView(mImmediate, sb.bindPoint);
        }
    }
}

void DX11Engine::EnableRBuffers(Shader const* shader, DX11Shader* dxShader)
{
    int const index = RawBuffer::shaderDataLookup;
    for (auto const& rb : shader->GetData(index))
    {
        if (rb.object)
        {
            DX11RawBuffer* dxRB = static_cast<DX11RawBuffer*>(Bind(rb.object));
            if (dxRB)
            {
                if (rb.isGpuWritable)
                {
                    dxShader->EnableUAView(mImmediate, rb.bindPoint, dxRB->GetUAView(), 0xFFFFFFFFu);
                }
                else
                {
                    dxShader->EnableSRView(mImmediate, rb.bindPoint, dxRB->GetSRView());
                }
            }
            else
            {
                LogError("Failed to bind byte-address buffer.");
            }
        }
        else
        {
            LogError(rb.name + " is null byte-address buffer.");
        }
    }
}

void DX11Engine::DisableRBuffers(Shader const* shader, DX11Shader* dxShader)
{
    int const index = RawBuffer::shaderDataLookup;
    for (auto const& rb : shader->GetData(index))
    {
        if (rb.isGpuWritable)
        {
            dxShader->DisableUAView(mImmediate, rb.bindPoint);
        }
        else
        {
            dxShader->DisableSRView(mImmediate, rb.bindPoint);
        }
    }
}

void DX11Engine::EnableTextures(Shader const* shader, DX11Shader* dxShader)
{
    int const index = TextureSingle::shaderDataLookup;
    for (auto const& tx : shader->GetData(index))
    {
        if (tx.object)
        {
            DX11TextureSingle* dxTX = static_cast<DX11TextureSingle*>(Bind(tx.object));
            if (dxTX)
            {
                if (tx.isGpuWritable)
                {
                    dxShader->EnableUAView(mImmediate, tx.bindPoint, dxTX->GetUAView(), 0xFFFFFFFFu);
                }
                else
                {
                    dxShader->EnableSRView(mImmediate, tx.bindPoint, dxTX->GetSRView());
                }
            }
            else
            {
                LogError("Failed to bind texture.");
            }
        }
        else
        {
            LogError(tx.name + " is null texture.");
        }
    }
}

void DX11Engine::DisableTextures(Shader const* shader, DX11Shader* dxShader)
{
    int const index = TextureSingle::shaderDataLookup;
    for (auto const& tx : shader->GetData(index))
    {
        if (tx.isGpuWritable)
        {
            dxShader->DisableUAView(mImmediate, tx.bindPoint);
        }
        else
        {
            dxShader->DisableSRView(mImmediate, tx.bindPoint);
        }
    }
}

void DX11Engine::EnableTextureArrays(Shader const* shader, DX11Shader* dxShader)
{
    int const index = TextureArray::shaderDataLookup;
    for (auto const& ta : shader->GetData(index))
    {
        if (ta.object)
        {
            DX11TextureArray* dxTA = static_cast<DX11TextureArray*>(Bind(ta.object));
            if (dxTA)
            {
                if (ta.isGpuWritable)
                {
                    dxShader->EnableUAView(mImmediate, ta.bindPoint, dxTA->GetUAView(), 0xFFFFFFFFu);
                }
                else
                {
                    dxShader->EnableSRView(mImmediate, ta.bindPoint, dxTA->GetSRView());
                }
            }
            else
            {
                LogError("Failed to bind texture array.");
            }
        }
        else
        {
            LogError(ta.name + " is null texture array.");
        }
    }
}

void DX11Engine::DisableTextureArrays(Shader const* shader,
    DX11Shader* dxShader)
{
    int const index = TextureArray::shaderDataLookup;
    for (auto const& ta : shader->GetData(index))
    {
        if (ta.isGpuWritable)
        {
            dxShader->DisableUAView(mImmediate, ta.bindPoint);
        }
        else
        {
            dxShader->DisableSRView(mImmediate, ta.bindPoint);
        }
    }
}

void DX11Engine::EnableSamplers(Shader const* shader, DX11Shader* dxShader)
{
    int const index = SamplerState::shaderDataLookup;
    for (auto const& ss : shader->GetData(index))
    {
        if (ss.object)
        {
            DX11SamplerState* dxSS = static_cast<DX11SamplerState*>(Bind(ss.object));
            if (dxSS)
            {
                dxShader->EnableSampler(mImmediate, ss.bindPoint, dxSS->GetDXSamplerState());
            }
            else
            {
                LogError("Failed to bind sampler state.");
            }
        }
        else
        {
            LogError(ss.name + " is null sampler state.");
        }
    }
}

void DX11Engine::DisableSamplers(Shader const* shader, DX11Shader* dxShader)
{
    int const index = SamplerState::shaderDataLookup;
    for (auto const& ss : shader->GetData(index))
    {
        dxShader->DisableSampler(mImmediate, ss.bindPoint);
    }
}

//----------------------------------------------------------------------------
// Public overrides from GraphicsEngine
//----------------------------------------------------------------------------
void DX11Engine::SetViewport(int x, int y, int w, int h)
{
    UINT numViewports = 1;
    mImmediate->RSGetViewports(&numViewports, &mViewport);
    LogAssert(1 == numViewports, "Failed to get viewport.");

    mViewport.TopLeftX = static_cast<float>(x);
    mViewport.TopLeftY = static_cast<float>(y);
    mViewport.Width = static_cast<float>(w);
    mViewport.Height = static_cast<float>(h);
    mImmediate->RSSetViewports(1, &mViewport);
}

void DX11Engine::GetViewport(int& x, int& y, int& w, int& h) const
{
    UINT numViewports = 1;
    D3D11_VIEWPORT viewport;
    mImmediate->RSGetViewports(&numViewports, &viewport);
    LogAssert(1 == numViewports, "Failed to get viewport.");

    x = static_cast<unsigned int>(viewport.TopLeftX);
    y = static_cast<unsigned int>(viewport.TopLeftY);
    w = static_cast<unsigned int>(viewport.Width);
    h = static_cast<unsigned int>(viewport.Height);
}

void DX11Engine::SetDepthRange(float zmin, float zmax)
{
    UINT numViewports = 1;
    mImmediate->RSGetViewports(&numViewports, &mViewport);
    LogAssert(1 == numViewports, "Failed to get viewport.");

    mViewport.MinDepth = zmin;
    mViewport.MaxDepth = zmax;
    mImmediate->RSSetViewports(1, &mViewport);
}

void DX11Engine::GetDepthRange(float& zmin, float& zmax) const
{
    UINT numViewports = 1;
    D3D11_VIEWPORT viewport;
    mImmediate->RSGetViewports(&numViewports, &viewport);
    LogAssert(1 == numViewports, "Failed to get viewport.");

    zmin = viewport.MinDepth;
    zmax = viewport.MaxDepth;
}

bool DX11Engine::Resize(unsigned int w, unsigned int h)
{
    // Release the previous back buffer before resizing.
    if (DestroyBackBuffer())
    {
        // Attempt to resize the back buffer to the incoming width and height.
        DXGI_SWAP_CHAIN_DESC desc;
        mSwapChain->GetDesc(&desc);
        HRESULT hr = mSwapChain->ResizeBuffers(desc.BufferCount, w, h,
            DXGI_FORMAT_R8G8B8A8_UNORM, 0);
        if (SUCCEEDED(hr))
        {
            // The attempt succeeded, so create new color and depth-stencil
            // objects.
            return CreateBackBuffer(w, h);
        }

        // The attempt to resize failed, so restore the back buffer to its
        // previous width and height.
        w = desc.BufferDesc.Width;
        h = desc.BufferDesc.Height;
        hr = mSwapChain->ResizeBuffers(desc.BufferCount, w, h,
            DXGI_FORMAT_R8G8B8A8_UNORM, 0);
        if (SUCCEEDED(hr))
        {
            return CreateBackBuffer(w, h);
        }
    }
    return false;
}

void DX11Engine::ClearColorBuffer()
{
    ID3D11RenderTargetView* rtViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = { nullptr };
    ID3D11DepthStencilView* dsView = nullptr;

    mImmediate->OMGetRenderTargets(mNumActiveRTs, rtViews, &dsView);
    DX11::SafeRelease(dsView);
    for (unsigned int i = 0; i < mNumActiveRTs; ++i)
    {
        if (rtViews[i])
        {
            mImmediate->ClearRenderTargetView(rtViews[i], mClearColor.data());
            DX11::SafeRelease(rtViews[i]);
        }
    }
}

void DX11Engine::ClearDepthBuffer()
{
    ID3D11DepthStencilView* dsView = nullptr;
    ID3D11RenderTargetView* rtView = nullptr;
    mImmediate->OMGetRenderTargets(1, &rtView, &dsView);
    DX11::SafeRelease(rtView);
    if (dsView)
    {
        mImmediate->ClearDepthStencilView(dsView, D3D11_CLEAR_DEPTH, mClearDepth, 0);
        DX11::SafeRelease(dsView);
    }
}

void DX11Engine::ClearStencilBuffer()
{
    ID3D11DepthStencilView* dsView = nullptr;
    ID3D11RenderTargetView* rtView = nullptr;
    mImmediate->OMGetRenderTargets(1, &rtView, &dsView);
    DX11::SafeRelease(rtView);
    if (dsView)
    {
        mImmediate->ClearDepthStencilView(dsView, D3D11_CLEAR_STENCIL, 0.0f,
            static_cast<UINT8>(mClearStencil));
        DX11::SafeRelease(dsView);
    }
}

void DX11Engine::ClearBuffers()
{
    ID3D11RenderTargetView* rtViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = { nullptr };
    ID3D11DepthStencilView* dsView = nullptr;

    mImmediate->OMGetRenderTargets(mNumActiveRTs, rtViews, &dsView);
    for (unsigned int i = 0; i < mNumActiveRTs; ++i)
    {
        if (rtViews[i])
        {
            mImmediate->ClearRenderTargetView(rtViews[i], mClearColor.data());
            DX11::SafeRelease(rtViews[i]);
        }
    }
    if (dsView)
    {
        mImmediate->ClearDepthStencilView(dsView,
            D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, mClearDepth,
            static_cast<UINT8>(mClearStencil));
        DX11::SafeRelease(dsView);
    }
}

void DX11Engine::DisplayColorBuffer(unsigned int syncInterval)
{
    // The swap must occur on the thread in which the device was created.
    mSwapChain->Present(syncInterval, 0);
}

void DX11Engine::SetBlendState(std::shared_ptr<BlendState> const& state)
{
    if (state)
    {
        if (state != mActiveBlendState)
        {
            DX11BlendState* dxState = static_cast<DX11BlendState*>(Bind(state));
            if (dxState)
            {
                dxState->Enable(mImmediate);
                mActiveBlendState = state;
            }
            else
            {
                LogError("Failed to bind blend state.");
            }
        }
    }
    else
    {
        LogError("Input state is null.");
    }
}

void DX11Engine::SetDepthStencilState(std::shared_ptr<DepthStencilState> const& state)
{
    if (state)
    {
        if (state != mActiveDepthStencilState)
        {
            DX11DepthStencilState* dxState = static_cast<DX11DepthStencilState*>(Bind(state));
            if (dxState)
            {
                dxState->Enable(mImmediate);
                mActiveDepthStencilState = state;
            }
            else
            {
                LogError("Failed to bind depth-stencil state.");
            }
        }
    }
    else
    {
        LogError("Input state is null.");
    }
}

void DX11Engine::SetRasterizerState(std::shared_ptr<RasterizerState> const& state)
{
    if (state)
    {
        if (state != mActiveRasterizerState)
        {
            DX11RasterizerState* dxState = static_cast<DX11RasterizerState*>(Bind(state));
            if (dxState)
            {
                dxState->Enable(mImmediate);
                mActiveRasterizerState = state;
            }
            else
            {
                LogError("Failed to bind rasterizer state.");
            }
        }
    }
    else
    {
        LogError("Input state is null.");
    }
}

void DX11Engine::Enable(std::shared_ptr<DrawTarget> const& target)
{
    DX11DrawTarget* dxTarget = static_cast<DX11DrawTarget*>(Bind(target));
    dxTarget->Enable(mImmediate);
    mNumActiveRTs = target->GetNumTargets();
}

void DX11Engine::Disable(std::shared_ptr<DrawTarget> const& target)
{
    DX11DrawTarget* dxTarget = static_cast<DX11DrawTarget*>(Bind(target));
    if (dxTarget)
    {
        dxTarget->Disable(mImmediate);
        mNumActiveRTs = 1;

        // The assumption is that Disable is called after you have written
        // the draw target outputs.  If the render targets want automatic
        // mipmap generation, we do so here.
        if (target->WantAutogenerateRTMipmaps())
        {
            unsigned int const numTargets = target->GetNumTargets();
            for (unsigned int i = 0; i < numTargets; ++i)
            {
                DX11Texture* dxTexture = static_cast<DX11Texture*>(Get(target->GetRTTexture(i)));
                ID3D11ShaderResourceView* srView = dxTexture->GetSRView();
                if (srView)
                {
                    mImmediate->GenerateMips(dxTexture->GetSRView());
                }
            }
        }
    }
}

DX11Texture2* DX11Engine::BindTexture(std::shared_ptr<Texture2> const& texture,
    ID3D11Texture2D* dxTexture, ID3D11ShaderResourceView* dxSRView)
{
    LogAssert(texture != nullptr && dxTexture != nullptr && dxSRView != nullptr,
        "Attempt to bind a null object.");

    GraphicsObject const* gtObject = texture.get();
    std::shared_ptr<GEObject> geObject;
    if (!mGOMap.Get(gtObject, geObject))
    {
        geObject = std::make_shared<DX11Texture2>(texture.get(), dxTexture, dxSRView);
        LogAssert(geObject, "Null object.  Out of memory?");
        mGOMap.Insert(gtObject, geObject);
#if defined(GTE_GRAPHICS_USE_NAMED_OBJECTS)
        geObject->SetName(texture->GetName());
#endif
    }
    return static_cast<DX11Texture2*>(geObject.get());
}

bool DX11Engine::Update(std::shared_ptr<Buffer> const& buffer)
{
    if (!buffer->GetData())
    {
        LogWarning("Buffer does not have system memory, creating it.");
        buffer->CreateStorage();
    }

    DX11Buffer* dxBuffer = static_cast<DX11Buffer*>(Bind(buffer));
    return dxBuffer->Update(mImmediate);
}

bool DX11Engine::Update(std::shared_ptr<TextureSingle> const& texture)
{
    if (!texture->GetData())
    {
        LogWarning("Texture does not have system memory, creating it.");
        texture->CreateStorage();
    }

    DX11Texture* dxTexture = static_cast<DX11Texture*>(Bind(texture));
    return dxTexture->Update(mImmediate);
}

bool DX11Engine::Update(std::shared_ptr<TextureSingle> const& texture, unsigned int level)
{
    if (!texture->GetData())
    {
        LogWarning("Texture does not have system memory, creating it.");
        texture->CreateStorage();
    }

    DX11Texture* dxTexture = static_cast<DX11Texture*>(Bind(texture));
    unsigned int sri = texture->GetIndex(0, level);
    return dxTexture->Update(mImmediate, sri);
}

bool DX11Engine::Update(std::shared_ptr<TextureArray> const& textureArray)
{
    if (!textureArray->GetData())
    {
        LogWarning("Texture array does not have system memory, creating it.");
        textureArray->CreateStorage();
    }

    DX11TextureArray* dxTextureArray = static_cast<DX11TextureArray*>(
        Bind(textureArray));
    return dxTextureArray->Update(mImmediate);
}

bool DX11Engine::Update(std::shared_ptr<TextureArray> const& textureArray, unsigned int item, unsigned int level)
{
    if (!textureArray->GetData())
    {
        LogWarning("Texture array does not have system memory, creating it.");
        textureArray->CreateStorage();
    }

    DX11TextureArray* dxTextureArray = static_cast<DX11TextureArray*>(
        Bind(textureArray));
    unsigned int sri = textureArray->GetIndex(item, level);
    return dxTextureArray->Update(mImmediate, sri);
}

bool DX11Engine::CopyCpuToGpu(std::shared_ptr<Buffer> const& buffer)
{
    if (!buffer->GetData())
    {
        LogWarning("Buffer does not have system memory, creating it.");
        buffer->CreateStorage();
    }

    DX11Buffer* dxBuffer = static_cast<DX11Buffer*>(Bind(buffer));
    return dxBuffer->CopyCpuToGpu(mImmediate);
}

bool DX11Engine::CopyCpuToGpu(std::shared_ptr<TextureSingle> const& texture)
{
    if (!texture->GetData())
    {
        LogWarning("Texture does not have system memory, creating it.");
        texture->CreateStorage();
    }

    DX11Texture* dxTexture = static_cast<DX11Texture*>(Bind(texture));
    return dxTexture->CopyCpuToGpu(mImmediate);
}

bool DX11Engine::CopyCpuToGpu(std::shared_ptr<TextureSingle> const& texture, unsigned int level)
{
    if (!texture->GetData())
    {
        LogWarning("Texture does not have system memory, creating it.");
        texture->CreateStorage();
    }

    DX11Texture* dxTexture = static_cast<DX11Texture*>(Bind(texture));
    unsigned int sri = texture->GetIndex(0, level);
    return dxTexture->CopyCpuToGpu(mImmediate, sri);
}

bool DX11Engine::CopyCpuToGpu(std::shared_ptr<TextureArray> const& textureArray)
{
    if (!textureArray->GetData())
    {
        LogWarning("Texture array does not have system memory, creating it.");
        textureArray->CreateStorage();
    }

    DX11TextureArray* dxTextureArray = static_cast<DX11TextureArray*>(
        Bind(textureArray));
    return dxTextureArray->CopyCpuToGpu(mImmediate);
}

bool DX11Engine::CopyCpuToGpu(std::shared_ptr<TextureArray> const& textureArray, unsigned int item, unsigned int level)
{
    if (!textureArray->GetData())
    {
        LogWarning("Texture array does not have system memory, creating it.");
        textureArray->CreateStorage();
    }

    DX11TextureArray* dxTextureArray = static_cast<DX11TextureArray*>(
        Bind(textureArray));
    unsigned int sri = textureArray->GetIndex(item, level);
    return dxTextureArray->CopyCpuToGpu(mImmediate, sri);
}

bool DX11Engine::CopyGpuToCpu(std::shared_ptr<Buffer> const& buffer)
{
    if (!buffer->GetData())
    {
        LogWarning("Buffer does not have system memory, creating it.");
        buffer->CreateStorage();
    }

    DX11Buffer* dxBuffer = static_cast<DX11Buffer*>(Bind(buffer));
    return dxBuffer->CopyGpuToCpu(mImmediate);
}

bool DX11Engine::CopyGpuToCpu(std::shared_ptr<TextureSingle> const& texture)
{
    if (!texture->GetData())
    {
        LogWarning("Texture does not have system memory, creating it.");
        texture->CreateStorage();
    }

    DX11Texture* dxTexture = static_cast<DX11Texture*>(Bind(texture));
    return dxTexture->CopyGpuToCpu(mImmediate);
}

bool DX11Engine::CopyGpuToCpu(std::shared_ptr<TextureSingle> const& texture, unsigned int level)
{
    if (!texture->GetData())
    {
        LogWarning("Texture does not have system memory, creating it.");
        texture->CreateStorage();
    }

    DX11Texture* dxTexture = static_cast<DX11Texture*>(Bind(texture));
    unsigned int sri = texture->GetIndex(0, level);
    return dxTexture->CopyGpuToCpu(mImmediate, sri);
}

bool DX11Engine::CopyGpuToCpu(std::shared_ptr<TextureArray> const& textureArray)
{
    if (!textureArray->GetData())
    {
        LogWarning("Texture array does not have system memory, creating it.");
        textureArray->CreateStorage();
    }

    DX11TextureArray* dxTextureArray = static_cast<DX11TextureArray*>(
        Bind(textureArray));
    return dxTextureArray->CopyGpuToCpu(mImmediate);
}

bool DX11Engine::CopyGpuToCpu(std::shared_ptr<TextureArray> const& textureArray, unsigned int item, unsigned int level)
{
    if (!textureArray->GetData())
    {
        LogWarning("Texture array does not have system memory, creating it.");
        textureArray->CreateStorage();
    }

    DX11TextureArray* dxTextureArray = static_cast<DX11TextureArray*>(
        Bind(textureArray));
    unsigned int sri = textureArray->GetIndex(item, level);
    return dxTextureArray->CopyGpuToCpu(mImmediate, sri);
}

void DX11Engine::CopyGpuToGpu(
    std::shared_ptr<Buffer> const& buffer0,
    std::shared_ptr<Buffer> const& buffer1)
{
    DX11Buffer* dxBuffer0 = static_cast<DX11Buffer*>(Bind(buffer0));
    DX11Buffer* dxBuffer1 = static_cast<DX11Buffer*>(Bind(buffer1));
    dxBuffer0->CopyGpuToGpu(mImmediate, dxBuffer1->GetDXResource());
}

void DX11Engine::CopyGpuToGpu(
    std::shared_ptr<TextureSingle> const& texture0,
    std::shared_ptr<TextureSingle> const& texture1)
{
    DX11Texture* dxTexture0 = static_cast<DX11Texture*>(Bind(texture0));
    DX11Texture* dxTexture1 = static_cast<DX11Texture*>(Bind(texture1));
    dxTexture0->CopyGpuToGpu(mImmediate, dxTexture1->GetDXResource());
}

void DX11Engine::CopyGpuToGpu(
    std::shared_ptr<TextureSingle> const& texture0,
    std::shared_ptr<TextureSingle> const& texture1,
    unsigned int level)
{
    DX11Texture* dxTexture0 = static_cast<DX11Texture*>(Bind(texture0));
    DX11Texture* dxTexture1 = static_cast<DX11Texture*>(Bind(texture1));
    unsigned int sri = texture0->GetIndex(0, level);
    dxTexture0->CopyGpuToGpu(mImmediate, dxTexture1->GetDXResource(), sri);
}

void DX11Engine::CopyGpuToGpu(
    std::shared_ptr<TextureArray> const& textureArray0,
    std::shared_ptr<TextureArray> const& textureArray1)
{
    DX11TextureArray* dxTextureArray0 = static_cast<DX11TextureArray*>(Bind(textureArray0));
    DX11TextureArray* dxTextureArray1 = static_cast<DX11TextureArray*>(Bind(textureArray1));
    return dxTextureArray0->CopyGpuToGpu(mImmediate, dxTextureArray1->GetDXResource());
}

void DX11Engine::CopyGpuToGpu(
    std::shared_ptr<TextureArray> const& textureArray0,
    std::shared_ptr<TextureArray> const& textureArray1,
    unsigned int item, unsigned int level)
{
    DX11TextureArray* dxTextureArray0 = static_cast<DX11TextureArray*>(Bind(textureArray0));
    DX11TextureArray* dxTextureArray1 = static_cast<DX11TextureArray*>(Bind(textureArray1));
    unsigned int sri = textureArray0->GetIndex(item, level);
    return dxTextureArray0->CopyGpuToGpu(mImmediate, dxTextureArray1->GetDXResource(), sri);
}

bool DX11Engine::GetNumActiveElements(std::shared_ptr<StructuredBuffer> const& buffer)
{
    DX11GraphicsObject* dxObject = static_cast<DX11GraphicsObject*>(Get(buffer));
    if (dxObject)
    {
        DX11StructuredBuffer* dxSBuffer =
            static_cast<DX11StructuredBuffer*>(dxObject);
        return dxSBuffer->GetNumActiveElements(mImmediate);
    }
    return false;
}

bool DX11Engine::BindProgram(std::shared_ptr<ComputeProgram> const& program)
{
    auto hlslProgram = std::dynamic_pointer_cast<HLSLComputeProgram>(program);
    if (hlslProgram)
    {
        auto cshader = hlslProgram->GetComputeShader();
        if (cshader)
        {
            return GraphicsEngine::Bind(cshader) != nullptr;
        }
    }
    return false;
}

void DX11Engine::Execute(std::shared_ptr<ComputeProgram> const& program,
    unsigned int numXGroups, unsigned int numYGroups, unsigned int numZGroups)
{
    auto hlslProgram = std::dynamic_pointer_cast<HLSLComputeProgram>(program);
    if (hlslProgram && numXGroups > 0 && numYGroups > 0 && numZGroups > 0)
    {
        auto cshader = hlslProgram->GetComputeShader();
        if (cshader)
        {
            DX11ComputeShader* dxCShader = static_cast<DX11ComputeShader*>(Bind(cshader));
            Enable(cshader.get(), dxCShader);
            mImmediate->Dispatch(numXGroups, numYGroups, numZGroups);
            Disable(cshader.get(), dxCShader);
        }
        else
        {
            LogError("Invalid input parameter.");
        }
    }
}

void DX11Engine::WaitForFinish()
{
    if (!mWaitQuery)
    {
        D3D11_QUERY_DESC desc;
        desc.Query = D3D11_QUERY_EVENT;
        desc.MiscFlags = D3D11_QUERY_MISC_NONE;
        DX11Log(mDevice->CreateQuery(&desc, &mWaitQuery));
    }

    mImmediate->End(mWaitQuery);
    BOOL data = 0;
    UINT size = sizeof(BOOL);
    while (S_OK != mImmediate->GetData(mWaitQuery, &data, size, 0))
    {
        // Wait for end of query.
    }
}

void DX11Engine::Flush()
{
    mImmediate->Flush();
}

void DX11Engine::CopyBackBuffer(std::shared_ptr<Texture2>& texture)
{
    if (!mColorBuffer)
    {
        return;
    }

    // The format and dimensions of the texture must match those of the
    // back buffer.
    D3D11_TEXTURE2D_DESC desc;
    mColorBuffer->GetDesc(&desc);
    if (!texture ||
        texture->GetFormat() != DF_R8G8B8A8_UNORM ||
        texture->GetWidth() != desc.Width ||
        texture->GetHeight() != desc.Height)
    {
        texture = std::make_shared<Texture2>(DF_R8G8B8A8_UNORM, desc.Width, desc.Height);
    }

    // Copy the back buffer to the staging texture.
    mImmediate->CopyResource(mBackBufferStaging, mColorBuffer);

    // Map the staging texture and copy it to the input texture.
    D3D11_MAPPED_SUBRESOURCE subresource;
    DX11Log(mImmediate->Map(mBackBufferStaging, 0, D3D11_MAP_READ_WRITE, 0, &subresource));
    uint32_t const pitch = 4 * desc.Width;
    uint8_t const* source = static_cast<uint8_t const*>(subresource.pData);
    size_t const numBytes = 4 * static_cast<size_t>(desc.Width);
    uint8_t* target = texture->Get<uint8_t>();
    for (uint32_t i = 0; i < desc.Height; ++i)
    {
        std::memcpy(target, source, numBytes);
        source += subresource.RowPitch;
        target += pitch;
    }
}

uint64_t DX11Engine::DrawPrimitive(std::shared_ptr<VertexBuffer> const& vbuffer,
    std::shared_ptr<IndexBuffer> const& ibuffer, std::shared_ptr<VisualEffect> const& effect)
{
    uint64_t numPixelsDrawn = 0;
    DX11VertexShader* dxVShader;
    DX11GeometryShader* dxGShader;
    DX11PixelShader* dxPShader;
    if (EnableShaders(effect, dxVShader, dxGShader, dxPShader))
    {
        // Enable the vertex buffer and input layout.
        DX11VertexBuffer* dxVBuffer = nullptr;
        DX11InputLayout* dxLayout = nullptr;
        if (vbuffer->StandardUsage())
        {
            dxVBuffer = static_cast<DX11VertexBuffer*>(Bind(vbuffer));
            DX11InputLayoutManager* manager = static_cast<DX11InputLayoutManager*>(mILMap.get());
            dxLayout = manager->Bind(mDevice, vbuffer.get(), effect->GetVertexShader().get());
            dxVBuffer->Enable(mImmediate);
            dxLayout->Enable(mImmediate);
        }
        else
        {
            mImmediate->IASetInputLayout(nullptr);
        }

        // Enable the index buffer.
        DX11IndexBuffer* dxIBuffer = nullptr;
        if (ibuffer->IsIndexed())
        {
            dxIBuffer = static_cast<DX11IndexBuffer*>(Bind(ibuffer));
            dxIBuffer->Enable(mImmediate);
        }

        numPixelsDrawn = DrawPrimitive(vbuffer.get(), ibuffer.get());

        // Disable the vertex buffer and input layout.
        if (vbuffer->StandardUsage())
        {
            dxVBuffer->Disable(mImmediate);
            dxLayout->Disable(mImmediate);
        }

        // Disable the index buffer.
        if (dxIBuffer)
        {
            dxIBuffer->Disable(mImmediate);
        }

        DisableShaders(effect, dxVShader, dxGShader, dxPShader);
    }
    return numPixelsDrawn;
}
