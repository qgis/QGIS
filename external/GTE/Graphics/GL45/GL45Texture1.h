// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/Texture1.h>
#include <Graphics/GL45/GL45TextureSingle.h>

namespace gte
{
    class GL45Texture1 : public GL45TextureSingle
    {
    public:
        // Construction and destruction.
        virtual ~GL45Texture1();
        GL45Texture1(Texture1 const* texture);
        static std::shared_ptr<GEObject> Create(void* unused, GraphicsObject const* object);

        // Member access.
        inline Texture1* GetTexture() const
        {
            return static_cast<Texture1*>(mGTObject);
        }

        // Returns true if mipmaps need to be generated.
        virtual bool CanAutoGenerateMipmaps() const override;

    protected:
        virtual void LoadTextureLevel(unsigned int level, void const* data) override;
    };
}
