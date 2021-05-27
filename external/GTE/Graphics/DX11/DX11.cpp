// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/DX11/GTGraphicsDX11PCH.h>
#include <Graphics/DX11/DX11.h>
#include <comdef.h>
#include <codecvt>
#include <locale>
using namespace gte;

HRESULT DX11::SetPrivateName(ID3D11DeviceChild* object, std::string const& name)
{
    HRESULT hr;
    if (object && name != "")
    {
        hr = object->SetPrivateData(WKPDID_D3DDebugObjectName,
            static_cast<UINT>(name.length()), name.c_str());
    }
    else
    {
        // Callers are allowed to call this function with a null input
        // or with an empty name (for convenience).
        hr = S_OK;
    }
    return hr;
}

HRESULT DX11::SetPrivateName(IDXGIObject* object, std::string const& name)
{
    HRESULT hr;
    if (object && name != "")
    {
        hr = object->SetPrivateData(WKPDID_D3DDebugObjectName,
            static_cast<UINT>(name.length()), name.c_str());
    }
    else
    {
        // Callers are allowed to call this function with a null input
        // or with an empty name (for convenience).
        hr = S_OK;
    }
    return hr;
}

void DX11::Log(HRESULT hr, char const* file, char const* function, int line)
{
    if (FAILED(hr))
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> convl;
        std::string message = convl.to_bytes(_com_error(hr).ErrorMessage());
        Logger(file, function, line, message).Error();
    }
}
