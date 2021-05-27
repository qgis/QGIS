// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.03.18

#pragma once

#include <Graphics/DX11/DXGIOutput.h>

// A simple wrapper for IDXGIAdapter1 objects and enumeration of them.

namespace gte
{
    class DXGIAdapter
    {
    public:
        // Construction and destruction.
        ~DXGIAdapter();
        DXGIAdapter(DXGIAdapter const& object);
        DXGIAdapter(IDXGIAdapter1* adapter = nullptr);

        // Assignment.
        DXGIAdapter& operator=(DXGIAdapter const& object);

        // Member access.
        inline IDXGIAdapter1* GetAdapter() const
        {
            return mAdapter;
        }

        inline DXGI_ADAPTER_DESC1 const& GetDescription() const
        {
            return mDescription;
        }

        inline std::vector<DXGIOutput> const& GetOutputs() const
        {
            return mOutputs;
        }

        // Enumeration of adapters on a machine.
        static void Enumerate(std::vector<DXGIAdapter>& adapters);

        // Choose the most powerful adapter. A discrete GPU is preferred.
        // If not available, Intel Integrated Graphics is preferred. If
        // not available, Microsoft WARP (software implementation of DX11)
        // will be chosen.
        static DXGIAdapter GetMostPowerful();

    private:
        IDXGIAdapter1* mAdapter;
        DXGI_ADAPTER_DESC1 mDescription;
        std::vector<DXGIOutput> mOutputs;
    };
}
