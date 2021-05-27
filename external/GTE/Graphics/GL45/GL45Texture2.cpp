// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GL45/GTGraphicsGL45PCH.h>
#include <Graphics/GL45/GL45Texture2.h>
using namespace gte;

GL45Texture2::~GL45Texture2()
{
    glDeleteTextures(1, &mGLHandle);
}

GL45Texture2::GL45Texture2(Texture2 const* texture)
    :
    GL45TextureSingle(texture, GL_TEXTURE_2D, GL_TEXTURE_BINDING_2D)
{
    // Create a texture structure.
    glGenTextures(1, &mGLHandle);
    glBindTexture(GL_TEXTURE_2D, mGLHandle);

    // Allocate (immutable) texture storage for all levels.
    auto const width = texture->GetDimension(0);
    auto const height = texture->GetDimension(1);
    glTexStorage2D(GL_TEXTURE_2D, mNumLevels, mInternalFormat, width, height);

    Initialize();

    // Cannot leave this texture bound.
    glBindTexture(GL_TEXTURE_2D, 0);

    // Create a staging texture if requested.
    CreateStaging();
}

std::shared_ptr<GEObject> GL45Texture2::Create(void*, GraphicsObject const* object)
{
    if (object->GetType() == GT_TEXTURE2)
    {
        return std::make_shared<GL45Texture2>(
            static_cast<Texture2 const*>(object));
    }

    LogError("Invalid object type.");
}

bool GL45Texture2::CanAutoGenerateMipmaps() const
{
    auto texture = GetTexture();

    return texture &&
        texture->HasMipmaps() &&
        texture->WantAutogenerateMipmaps() &&
        !texture->IsShared();
}

void GL45Texture2::LoadTextureLevel(unsigned int level, void const* data)
{
    auto texture = GetTexture();
    if (texture && level < texture->GetNumLevels())
    {
        auto width = texture->GetDimension(0);
        auto height = texture->GetDimension(1);

        glTexSubImage2D(GL_TEXTURE_2D, level, 0, 0, width, height,
            mExternalFormat, mExternalType, data);
    }
}
