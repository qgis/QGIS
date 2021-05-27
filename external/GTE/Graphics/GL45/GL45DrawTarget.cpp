// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GL45/GTGraphicsGL45PCH.h>
#include <Graphics/DrawTarget.h>
#include <Graphics/GL45/GL45DrawTarget.h>
using namespace gte;

GL45DrawTarget::~GL45DrawTarget ()
{
    glDeleteFramebuffers(1, &mFrameBuffer);
}

GL45DrawTarget::GL45DrawTarget(DrawTarget const* target,
    std::vector<GL45TextureRT*>& rtTextures, GL45TextureDS* dsTexture)
    :
    GEDrawTarget(target),
    mRTTextures(rtTextures),
    mDSTexture(dsTexture),
    mFrameBuffer(0)
{
    LogAssert(target->GetNumTargets() <= rtTextures.size(),
        "DrawTargets has more targets than there are RT textures provided.");

    glGenFramebuffers(1, &mFrameBuffer);
}

std::shared_ptr<GEDrawTarget> GL45DrawTarget::Create(DrawTarget const* target,
    std::vector<GEObject*>& rtTextures, GEObject* dsTexture)
{
    std::vector<GL45TextureRT*> dxRTTextures(rtTextures.size());
    for (size_t i = 0; i < rtTextures.size(); ++i)
    {
        dxRTTextures[i] = static_cast<GL45TextureRT*>(rtTextures[i]);
    }
    GL45TextureDS* dxDSTexture = static_cast<GL45TextureDS*>(dsTexture);

    return std::make_shared<GL45DrawTarget>(target, dxRTTextures, dxDSTexture);
}

void GL45DrawTarget::Enable()
{
    // Save the current viewport settings so they can be restored when
    // Disable is called.
    GLint intVals[4];
    GLdouble doubleVals[2];
    glGetIntegerv(GL_VIEWPORT, intVals);
    glGetDoublev(GL_DEPTH_RANGE, doubleVals);
    mSaveViewportX = intVals[0];
    mSaveViewportY = intVals[1];
    mSaveViewportWidth = intVals[2];
    mSaveViewportHeight = intVals[3];
    mSaveViewportNear = doubleVals[0];
    mSaveViewportFar = doubleVals[1];

    // Set viewport according to draw target;
    auto viewportWidth = static_cast<GLsizei>(mTarget->GetWidth());
    auto viewportHeight = static_cast<GLsizei>(mTarget->GetHeight());
    glViewport(0, 0, viewportWidth, viewportHeight);

    // Set depth range to full.
    glDepthRange(0.0, 1.0);

    // Bind the frame buffer.
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFrameBuffer);

    // Attach depth buffer if there is one.
    if (mDSTexture)
    {
        DFType format = mDSTexture->GetTexture()->GetFormat();
        GLenum attachment;
        if (format == DF_D24_UNORM_S8_UINT)
        {
            attachment = GL_DEPTH_STENCIL_ATTACHMENT;
        }
        else  // for now only DF_D24_UNORM_S8_UINT or DF_D32_FLOAT supported
        {
            attachment = GL_DEPTH_ATTACHMENT;
        }
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, attachment, GL_TEXTURE_2D, mDSTexture->GetGLHandle(), 0);
    }

    // Attach each render target.  Build list of attachments to use for
    // drawing to.
    auto const numTargets = mTarget->GetNumTargets();
    std::vector<GLenum> useDrawBuffers(numTargets);
    for (unsigned i = 0; i < numTargets; ++i)
    {
        auto colorTarget = GL_COLOR_ATTACHMENT0 + i;

        useDrawBuffers[i] = colorTarget;

        auto textureRT = mRTTextures[i];
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, colorTarget, GL_TEXTURE_2D, textureRT->GetGLHandle(), 0);
    }

    // Set which draw buffers to use.
    glDrawBuffers(static_cast<GLsizei>(useDrawBuffers.size()), useDrawBuffers.data());
}

void GL45DrawTarget::Disable()
{
    // Restore to default frame buffer rendering.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Restore viewport.
    glViewport(mSaveViewportX, mSaveViewportY, mSaveViewportWidth, mSaveViewportHeight);
    glDepthRange(mSaveViewportNear, mSaveViewportFar);

    // When done, test each render target texture if it needs to have its
    // mipmaps automatically generated.
    auto const numTargets = mTarget->GetNumTargets();
    for (unsigned i = 0; i < numTargets; ++i)
    {
        auto textureRT = mRTTextures[i];
        if (textureRT->CanAutoGenerateMipmaps())
        {
            textureRT->GenerateMipmaps();
        }
    }
}
