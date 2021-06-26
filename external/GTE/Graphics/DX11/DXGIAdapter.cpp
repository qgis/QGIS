// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.04.13

#include <Graphics/DX11/GTGraphicsDX11PCH.h>
#include <Graphics/DX11/DXGIAdapter.h>
using namespace gte;

DXGIAdapter::~DXGIAdapter()
{
    DX11::SafeRelease(mAdapter);
}

DXGIAdapter::DXGIAdapter(DXGIAdapter const& object)
    :
    mAdapter(nullptr)
{
    *this = object;
}

DXGIAdapter::DXGIAdapter(IDXGIAdapter1* adapter)
    :
    mAdapter(adapter)
{
    ZeroMemory(&mDescription, sizeof(DXGI_ADAPTER_DESC1));
    if (mAdapter)
    {
        DX11::SafeAddRef(mAdapter);
        DX11Log(mAdapter->GetDesc1(&mDescription));
    }
}

DXGIAdapter& DXGIAdapter::operator=(DXGIAdapter const& object)
{
    DX11::SafeAddRef(object.mAdapter);
    DX11::SafeRelease(mAdapter);
    mAdapter = object.mAdapter;
    mDescription = object.mDescription;
    mOutputs = object.mOutputs;
    return *this;
}

void DXGIAdapter::Enumerate(std::vector<DXGIAdapter>& adapters)
{
    adapters.clear();

    IDXGIFactory1* factory = nullptr;
    DX11Log(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&factory));

    if (factory)
    {
        for (UINT i = 0; /**/; ++i)
        {
            IDXGIAdapter1* adapter = nullptr;
            HRESULT hr = factory->EnumAdapters1(i, &adapter);
            if (hr != DXGI_ERROR_NOT_FOUND)
            {
                adapters.push_back(DXGIAdapter(adapter));
            }
            else
            {
                break;
            }
        }

        DX11::SafeRelease(factory);
    }
}

DXGIAdapter DXGIAdapter::GetMostPowerful()
{
    // NVIDIA Vendor ID:    0x000010de
    // AMD Vendor ID:       0x00001002
    // Intel Vendor ID:     0x00008086
    // Microsoft Vendor ID: 0x00001414, L"Microsoft Basic Render Driver"

    std::vector<DXGIAdapter> adapters;
    Enumerate(adapters);
    std::vector<size_t> discreteAdapters;
    size_t intelAdapter = std::numeric_limits<size_t>::max();
    size_t microsoftAdapter = std::numeric_limits<size_t>::max();
    for (size_t i = 0; i < adapters.size(); ++i)
    {
        auto desc = adapters[i].GetDescription();
        if (desc.VendorId == 0x00008086u)
        {
            intelAdapter = i;
        }
        else if (desc.VendorId == 0x00001414u)
        {
            microsoftAdapter = i;
        }
        else
        {
            discreteAdapters.push_back(i);
        }
    }

    IDXGIAdapter1* adapter = nullptr;
    if (discreteAdapters.size() > 0)
    {
        adapter = adapters[discreteAdapters.front()].GetAdapter();
    }
    else if (intelAdapter != std::numeric_limits<size_t>::max())
    {
        adapter = adapters[intelAdapter].GetAdapter();
    }
    else if (microsoftAdapter != std::numeric_limits<size_t>::max())
    {
        adapter = adapters[microsoftAdapter].GetAdapter();
    }
    else
    {
        // If you reach this error, please report a bug to Geometric Tools.
        LogError("DXGI adapter enumeration failed to find a GPU.");
    }

    return DXGIAdapter(adapter);
}
