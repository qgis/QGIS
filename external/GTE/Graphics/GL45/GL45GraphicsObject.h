// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/GEObject.h>
#include <Graphics/GL45/GL45.h>
#include <Mathematics/Logger.h>

namespace gte
{
    class GL45GraphicsObject : public GEObject
    {
    public:
        // Abstract base class.
        virtual ~GL45GraphicsObject() = default;
    protected:
        GL45GraphicsObject(GraphicsObject const* gtObject);

    public:
        // Member access.
        inline GLuint GetGLHandle() const
        {
            return mGLHandle;
        }

        // Support for debugging.
        virtual void SetName(std::string const& name) override;

    protected:
        GLuint mGLHandle;
    };
}
