// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GL45/GTGraphicsGL45PCH.h>
#include <Graphics/GL45/GL45Texture3.h>
using namespace gte;

GL45Texture3::~GL45Texture3()
{
    glDeleteTextures(1, &mGLHandle);
}

GL45Texture3::GL45Texture3(Texture3 const* texture)
    :
    GL45TextureSingle(texture, GL_TEXTURE_3D, GL_TEXTURE_BINDING_3D)
{
    // Create a texture structure.
    glGenTextures(1, &mGLHandle);
    glBindTexture(GL_TEXTURE_3D, mGLHandle);

    // Allocate (immutable) texture storage for all levels.
    auto const width = texture->GetDimension(0);
    auto const height = texture->GetDimension(1);
    auto const depth = texture->GetDimension(2);
    glTexStorage3D(GL_TEXTURE_3D, mNumLevels, mInternalFormat, width, height, depth);

    Initialize();

    // Cannot leave this texture bound.
    glBindTexture(GL_TEXTURE_3D, 0);

    // Create a staging texture if requested.
    CreateStaging();
}

std::shared_ptr<GEObject> GL45Texture3::Create(void*, GraphicsObject const* object)
{
    if (object->GetType() == GT_TEXTURE3)
    {
        return std::make_shared<GL45Texture3>(
            static_cast<Texture3 const*>(object));
    }

    LogError("Invalid object type.");
}

bool GL45Texture3::CanAutoGenerateMipmaps() const
{
    auto texture = GetTexture();

    return texture && texture->HasMipmaps() && texture->WantAutogenerateMipmaps();
}

void GL45Texture3::LoadTextureLevel(unsigned int level, void const* data)
{
    auto texture = GetTexture();
    if (texture && level < texture->GetNumLevels())
    {
        auto width = texture->GetDimensionFor(level, 0);
        auto height = texture->GetDimensionFor(level, 1);
        auto depth = texture->GetDimensionFor(level, 2);

        glTexSubImage3D(GL_TEXTURE_3D, level, 0, 0, 0, width, height, depth,
            mExternalFormat, mExternalType, data);
    }
}
