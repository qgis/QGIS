// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GL45/GTGraphicsGL45PCH.h>
#include <Graphics/GL45/GL45Texture1.h>
using namespace gte;

GL45Texture1::~GL45Texture1()
{
    glDeleteTextures(1, &mGLHandle);
}

GL45Texture1::GL45Texture1(Texture1 const* texture)
    :
    GL45TextureSingle(texture, GL_TEXTURE_1D, GL_TEXTURE_BINDING_1D)
{
    // Create a texture structure.
    glGenTextures(1, &mGLHandle);
    glBindTexture(GL_TEXTURE_1D, mGLHandle);

    // Allocate (immutable) texture storage for all levels.
    auto const length = texture->GetDimension(0);
    glTexStorage1D(GL_TEXTURE_1D, mNumLevels, mInternalFormat, length);

    Initialize();

    // Cannot leave this texture bound.
    glBindTexture(GL_TEXTURE_1D, 0);

    // Create a staging texture if requested.
    CreateStaging();
}

std::shared_ptr<GEObject> GL45Texture1::Create(void*, GraphicsObject const* object)
{
    if (object->GetType() == GT_TEXTURE1)
    {
        return std::make_shared<GL45Texture1>(
            static_cast<Texture1 const*>(object));
    }

    LogError("Invalid object type.");
}

bool GL45Texture1::CanAutoGenerateMipmaps() const
{
    auto texture = GetTexture();

    return texture && texture->HasMipmaps() && texture->WantAutogenerateMipmaps();
}

void GL45Texture1::LoadTextureLevel(unsigned int level, void const* data)
{
    auto texture = GetTexture();
    if (texture && level < texture->GetNumLevels())
    {
        auto length = texture->GetDimensionFor(level, 0);

        glTexSubImage1D(GL_TEXTURE_1D, level, 0, length,
            mExternalFormat, mExternalType, data);
    }
}
