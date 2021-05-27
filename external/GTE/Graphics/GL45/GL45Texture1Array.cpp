// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GL45/GTGraphicsGL45PCH.h>
#include <Graphics/GL45/GL45Texture1Array.h>
using namespace gte;

GL45Texture1Array::~GL45Texture1Array()
{
    glDeleteTextures(1, &mGLHandle);
}

GL45Texture1Array::GL45Texture1Array(Texture1Array const* texture)
    :
    GL45TextureArray(texture, GL_TEXTURE_1D_ARRAY, GL_TEXTURE_BINDING_1D_ARRAY)
{
    // Create a texture structure.
    glGenTextures(1, &mGLHandle);
    glBindTexture(GL_TEXTURE_1D_ARRAY, mGLHandle);

    // Allocate (immutable) texture storage for all levels.
    auto const length = texture->GetDimension(0);
    auto const numItems = texture->GetNumItems();
    glTexStorage2D(GL_TEXTURE_1D_ARRAY, mNumLevels, mInternalFormat, length, numItems);

    Initialize();

    // Cannot leave this texture bound.
    glBindTexture(GL_TEXTURE_1D_ARRAY, 0);

    // Create a staging texture if requested.
    CreateStaging();
}

std::shared_ptr<GEObject> GL45Texture1Array::Create(void*, GraphicsObject const* object)
{
    if (object->GetType() == GT_TEXTURE1_ARRAY)
    {
        return std::make_shared<GL45Texture1Array>(
            static_cast<Texture1Array const*>(object));
    }

    LogError("Invalid object type.");
}

bool GL45Texture1Array::CanAutoGenerateMipmaps() const
{
    auto texture = GetTexture();

    return texture && texture->HasMipmaps() && texture->WantAutogenerateMipmaps();
}

void GL45Texture1Array::LoadTextureLevel(unsigned int item, unsigned int level, void const* data)
{
    auto texture = GetTexture();
    if (texture && level < texture->GetNumLevels())
    {
        auto const length = texture->GetDimensionFor(level, 0);

        // For Texture1Array, use the 2D calls where the slice (or item) is
        // the second dimension.  Only updating one slice for the specified
        // level.
        glTexSubImage2D(GL_TEXTURE_1D_ARRAY, level, 0, item, length, 1,
            mExternalFormat, mExternalType, data);
    }
}
