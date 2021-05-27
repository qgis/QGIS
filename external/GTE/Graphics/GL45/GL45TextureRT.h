// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/TextureRT.h>
#include <Graphics/GL45/GL45Texture2.h>

namespace gte
{
    class GL45TextureRT : public GL45Texture2
    {
    public:
        // Construction.
        virtual ~GL45TextureRT() = default;
        GL45TextureRT(TextureRT const* texture);
        static std::shared_ptr<GEObject> Create(void* unused, GraphicsObject const* object);

        // Member access.
        inline TextureRT* GetTexture() const
        {
            return static_cast<TextureRT*>(mGTObject);
        }

        // Returns true of mipmaps need to be generated.
        virtual bool CanAutoGenerateMipmaps() const override;
    };
}
