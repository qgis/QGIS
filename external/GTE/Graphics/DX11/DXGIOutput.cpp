// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.04.13

#include <Graphics/DX11/GTGraphicsDX11PCH.h>
#include <Graphics/DX11/DXGIOutput.h>
using namespace gte;

DXGIOutput::~DXGIOutput()
{
    DX11::SafeRelease(mOutput);
}

DXGIOutput::DXGIOutput(DXGIOutput const& object)
    :
    mOutput(nullptr)
{
    *this = object;
}

DXGIOutput::DXGIOutput(IDXGIOutput* output)
    :
    mOutput(output)
{
    ZeroMemory(&mDescription, sizeof(DXGI_OUTPUT_DESC));
    if (mOutput)
    {
        DX11::SafeAddRef(mOutput);
        DX11Log(mOutput->GetDesc(&mDescription));
    }
}

DXGIOutput& DXGIOutput::operator=(DXGIOutput const& object)
{
    DX11::SafeAddRef(object.mOutput);
    DX11::SafeRelease(mOutput);
    mOutput = object.mOutput;
    mDescription = object.mDescription;
    return *this;
}

HRESULT DXGIOutput::GetDisplayModes(DXGI_FORMAT format,
    std::vector<DXGI_MODE_DESC>& modeDescriptions)
{
    modeDescriptions.clear();

    if (mOutput)
    {
        // The zero value for 'flags' says to return the maximum number of
        // modes, regardless of the DXGI_ENUM_MODES possibilities for flags.
        // We might want to allow for a different value for DX11.1 when
        // stereo modes are available.
        UINT const flags = 0;
        UINT numModes = 0;
        DX11Log(mOutput->GetDisplayModeList(format, flags, &numModes, nullptr));

        if (numModes > 0)
        {
            modeDescriptions.resize(numModes);
            HRESULT hr = mOutput->GetDisplayModeList(format, flags, &numModes, &modeDescriptions[0]);
            DX11Log(hr);
            return hr;
        }

        // No modes available for the requested format.  This is not an
        // error condition.  The caller must test that the array of
        // descriptions has no elements and choose another format that
        // might have modes associated with it.
        return S_OK;
    }

    LogError("Output not yet set.");
}

HRESULT DXGIOutput::FindClosestMatchingMode(DXGI_MODE_DESC const& requested,
    DXGI_MODE_DESC& closest)
{
    if (mOutput)
    {
        HRESULT hr = mOutput->FindClosestMatchingMode(&requested, &closest, nullptr);
        DX11Log(hr);
        return hr;
    }

    LogError("Output not yet set.");
}

void DXGIOutput::Enumerate(IDXGIAdapter* adapter, std::vector<DXGIOutput>& outputs)
{
    outputs.clear();

    if (adapter)
    {
        for (UINT i = 0; /**/; ++i)
        {
            IDXGIOutput* output = nullptr;
            HRESULT hr = adapter->EnumOutputs(i, &output);
            if (hr != DXGI_ERROR_NOT_FOUND)
            {
                outputs.push_back(DXGIOutput(output));
            }
            else
            {
                break;
            }
        }
    }
}
