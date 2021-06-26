// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GL45/GTGraphicsGL45PCH.h>
#include <Graphics/GL45/GL45TextureDS.h>
using namespace gte;

GL45TextureDS::GL45TextureDS(TextureDS const* texture)
    :
    GL45Texture2(texture)
{
}

std::shared_ptr<GEObject> GL45TextureDS::Create(void*, GraphicsObject const* object)
{
    if (object->GetType() == GT_TEXTURE_DS)
    {
        return std::make_shared<GL45TextureDS>(
            static_cast<TextureDS const*>(object));
    }

    LogError("Invalid object type.");
}

bool GL45TextureDS::CanAutoGenerateMipmaps() const
{
    return false;
}
