// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/SamplerState.h>
#include <Graphics/GL45/GL45DrawingState.h>

namespace gte
{
    class GL45SamplerState : public GL45DrawingState
    {
    public:
        // Construction and destruction.
        virtual ~GL45SamplerState();
        GL45SamplerState(SamplerState const* samplerState);
        static std::shared_ptr<GEObject> Create(void* unused, GraphicsObject const* object);

        // Member access.
        inline SamplerState* GetSamplerState()
        {
            return static_cast<SamplerState*>(mGTObject);
        }

    private:
        // Conversions from GTEngine values to GL4 values.
        static GLint const msMode[];
    };
}
