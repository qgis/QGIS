// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/TextureCubeArray.h>
#include <Graphics/GL45/GL45TextureArray.h>

namespace gte
{
    class GL45TextureCubeArray : public GL45TextureArray
    {
    public:
        // Construction and destruction.
        virtual ~GL45TextureCubeArray();
        GL45TextureCubeArray(TextureCubeArray const* texture);
        static std::shared_ptr<GEObject> Create(void* unused, GraphicsObject const* object);

        // Member access.
        inline TextureCubeArray* GetTexture() const
        {
            return static_cast<TextureCubeArray*>(mGTObject);
        }

        // Returns true if mipmaps need to be generated.
        virtual bool CanAutoGenerateMipmaps() const override;

    protected:
        virtual void LoadTextureLevel(unsigned int item, unsigned int level, void const* data) override;
    };
}
