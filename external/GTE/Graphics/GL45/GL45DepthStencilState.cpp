// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GL45/GTGraphicsGL45PCH.h>
#include <Graphics/GL45/GL45DepthStencilState.h>
using namespace gte;

GL45DepthStencilState::GL45DepthStencilState(DepthStencilState const* depthStencilState)
    :
    GL45DrawingState(depthStencilState)
{
    mDepthEnable = (depthStencilState->depthEnable ? GL_TRUE : GL_FALSE);
    mWriteMask = msWriteMask[depthStencilState->writeMask];
    mComparison = msComparison[depthStencilState->comparison];
    mStencilEnable = (depthStencilState->stencilEnable ? GL_TRUE : GL_FALSE);
    mStencilReadMask = depthStencilState->stencilReadMask;
    mStencilWriteMask = depthStencilState->stencilWriteMask;
    DepthStencilState::Face front = depthStencilState->frontFace;
    mFrontFace.onFail = msOperation[front.fail];
    mFrontFace.onZFail = msOperation[front.depthFail];
    mFrontFace.onZPass = msOperation[front.pass];
    mFrontFace.comparison = msComparison[front.comparison];
    DepthStencilState::Face back = depthStencilState->backFace;
    mBackFace.onFail = msOperation[back.fail];
    mBackFace.onZFail = msOperation[back.depthFail];
    mBackFace.onZPass = msOperation[back.pass];
    mBackFace.comparison = msComparison[back.comparison];
    mReference = depthStencilState->reference;
}

std::shared_ptr<GEObject> GL45DepthStencilState::Create(void*, GraphicsObject const* object)
{
    if (object->GetType() == GT_DEPTH_STENCIL_STATE)
    {
        return std::make_shared<GL45DepthStencilState>(
            static_cast<DepthStencilState const*>(object));
    }

    LogError("Invalid object type.");
}

void GL45DepthStencilState::Enable()
{
    if (mDepthEnable)
    {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(mComparison);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }

    glDepthMask(mWriteMask);

    if (mStencilEnable)
    {
        glEnable(GL_STENCIL_TEST);

        glStencilFuncSeparate(GL_FRONT, mFrontFace.comparison, mReference, mStencilReadMask);
        glStencilMaskSeparate(GL_FRONT, mStencilWriteMask);
        glStencilOpSeparate(GL_FRONT, mFrontFace.onFail, mFrontFace.onZFail, mFrontFace.onZPass);
        glStencilFuncSeparate(GL_BACK, mBackFace.comparison, mReference, mStencilReadMask);
        glStencilMaskSeparate(GL_BACK, mStencilWriteMask);
        glStencilOpSeparate(GL_BACK, mBackFace.onFail, mBackFace.onZFail, mBackFace.onZPass);
    }
    else
    {
        glDisable(GL_STENCIL_TEST);
    }
}


GLboolean const GL45DepthStencilState::msWriteMask[] =
{
    GL_FALSE,
    GL_TRUE
};

GLenum const GL45DepthStencilState::msComparison[] =
{
    GL_NEVER,
    GL_LESS,
    GL_EQUAL,
    GL_LEQUAL,
    GL_GREATER,
    GL_NOTEQUAL,
    GL_GEQUAL,
    GL_ALWAYS
};

GLenum const GL45DepthStencilState::msOperation[] =
{
    GL_KEEP,
    GL_ZERO,
    GL_REPLACE,
    GL_INCR,  // TODO: DX11 has INCR_SAT that clamps the result
    GL_DECR,  // TODO: DX11 has INCR_SAT that clamps the result
    GL_INVERT,
    GL_INCR,
    GL_DECR
};
