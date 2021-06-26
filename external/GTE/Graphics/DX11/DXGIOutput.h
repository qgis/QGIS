// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/DX11/DX11.h>
#include <vector>

// A simple wrapper for IDXGIOutput objects and enumeration of them.

namespace gte
{
    class DXGIOutput
    {
    public:
        // Construction and destruction.
        ~DXGIOutput();
        DXGIOutput(DXGIOutput const& object);
        DXGIOutput(IDXGIOutput* output = nullptr);

        // Assignment.
        DXGIOutput& operator=(DXGIOutput const& object);

        // Member access.
        inline IDXGIOutput* GetOutput() const
        {
            return mOutput;
        }

        inline DXGI_OUTPUT_DESC const& GetDescription() const
        {
            return mDescription;
        }

        // Queries for information about the output/monitor.  The modes that
        // support the format are returned, possibly an empty list.  The
        // returned HRESULT is one of the DXGI_ERROR values.
        HRESULT GetDisplayModes(DXGI_FORMAT format,
            std::vector<DXGI_MODE_DESC>& modeDescriptions);

        // Find a mode that matches as closely as possible the requested mode.
        // The returned HRESULT is one of the DXGI_ERROR values.
        HRESULT FindClosestMatchingMode(DXGI_MODE_DESC const& requested,
            DXGI_MODE_DESC& closest);

        // Enumeration of monitors attached to the adapter (if any).
        static void Enumerate(IDXGIAdapter* adapter, std::vector<DXGIOutput>& outputs);

    private:
        IDXGIOutput* mOutput;
        DXGI_OUTPUT_DESC mDescription;
    };
}
