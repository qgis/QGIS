// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GL45/GTGraphicsGL45PCH.h>
#include <Graphics/GL45/GL45TextureRT.h>
using namespace gte;

GL45TextureRT::GL45TextureRT(TextureRT const* texture)
    :
    GL45Texture2(texture)
{
}

std::shared_ptr<GEObject> GL45TextureRT::Create(void*, GraphicsObject const* object)
{
    if (object->GetType() == GT_TEXTURE_RT)
    {
        return std::make_shared<GL45TextureRT>(
            static_cast<TextureRT const*>(object));
    }

    LogError("Invalid object type.");
}

bool GL45TextureRT::CanAutoGenerateMipmaps() const
{
    auto texture = GetTexture();

    return texture && texture->HasMipmaps() && texture->WantAutogenerateMipmaps();
}
