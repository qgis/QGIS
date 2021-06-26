// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/PlanarReflectionEffect.h>
using namespace gte;

PlanarReflectionEffect::PlanarReflectionEffect(
    std::vector<std::shared_ptr<Visual>> const& planes,
    std::vector<float> const& reflectances)
    :
    mPlanes(planes),
    mReflectances(reflectances),
    mPlaneOrigins(planes.size()),
    mPlaneNormals(planes.size())
{
    unsigned int const numPlanes = static_cast<unsigned int>(mPlanes.size());
    for (unsigned int i = 0; i < numPlanes; ++i)
    {
        std::shared_ptr<Visual> plane = mPlanes[i];

        // The culling flag is set to "always" because this effect is
        // responsible for drawing the triangle mesh.  This prevents drawing
        // attempts by another scene graph for which 'plane' is a leaf node.
        plane->culling = CULL_ALWAYS;

        // Compute the model-space origin and normal for the plane.

        // Get the position data.
        std::shared_ptr<VertexBuffer> vbuffer = plane->GetVertexBuffer();
        unsigned int vstride = vbuffer->GetElementSize();
        std::set<DFType> required;
        required.insert(DF_R32G32B32_FLOAT);
        required.insert(DF_R32G32B32A32_FLOAT);
        char const* positions = vbuffer->GetChannel(VA_POSITION, 0, required);
        if (!positions)
        {
            LogError("Expecting 3D positions.");
        }

        // Verify the plane topology involves triangles.
        std::shared_ptr<IndexBuffer> ibuffer = plane->GetIndexBuffer();
        IPType primitiveType = ibuffer->GetPrimitiveType();
        if (!(primitiveType & IP_HAS_TRIANGLES))
        {
            LogError("Expecting triangle topology.");
        }

        // Get the vertex indices for the first triangle defining the plane.
        unsigned int v[3];
        if (ibuffer->IsIndexed())
        {
            ibuffer->GetTriangle(i, v[0], v[1], v[2]);
        }
        else if (primitiveType == IP_TRIMESH)
        {
            v[0] = 3 * i;
            v[1] = v[0] + 1;
            v[2] = v[0] + 2;
        }
        else  // primitiveType == IP_TRISTRIP
        {
            int offset = (i & 1);
            v[0] = i + offset;
            v[1] = i + 1 + offset;
            v[2] = i + 2 - offset;
        }

        // Get the model-space positions of the triangle vertices.
        Vector4<float> p[3];
        for (int j = 0; j < 3; ++j)
        {
            auto temp = *reinterpret_cast<Vector3<float> const*>(positions + v[j] * vstride);
            p[j] = HLift<3, float>(temp, 1.0f);
        }

        mPlaneOrigins[i] = p[0];
        mPlaneNormals[i] = UnitCross(p[2] - p[0], p[1] - p[0]);
    }

    // Turn off color writes.
    mNoColorWrites = std::make_shared<BlendState>();
    mNoColorWrites->target[0].mask = 0;
    mNoColorWrites->target[0].enable = true;

    // Blend with a constant alpha.  The blend color is set for each
    // reflecting plane.
    mReflectanceBlend = std::make_shared<BlendState>();
    mReflectanceBlend->target[0].enable = true;
    mReflectanceBlend->target[0].srcColor = BlendState::BM_INV_FACTOR;
    mReflectanceBlend->target[0].dstColor = BlendState::BM_FACTOR;
    mReflectanceBlend->target[0].srcAlpha = BlendState::BM_INV_FACTOR;
    mReflectanceBlend->target[0].dstAlpha = BlendState::BM_FACTOR;

    // For toggling the current cull mode to the opposite of what is active.
    mCullReverse = std::make_shared<RasterizerState>();

    // The depth-stencil passes.  The reference values are set for each
    // reflecting plane.
    mDSPass0 = std::make_shared<DepthStencilState>();
    mDSPass0->depthEnable = true;
    mDSPass0->writeMask = DepthStencilState::MASK_ZERO;
    mDSPass0->comparison = DepthStencilState::LESS_EQUAL;
    mDSPass0->stencilEnable = true;
    mDSPass0->frontFace.fail = DepthStencilState::OP_KEEP;
    mDSPass0->frontFace.depthFail = DepthStencilState::OP_KEEP;
    mDSPass0->frontFace.pass = DepthStencilState::OP_REPLACE;
    mDSPass0->frontFace.comparison = DepthStencilState::ALWAYS;

    mDSPass1 = std::make_shared<DepthStencilState>();
    mDSPass1->depthEnable = true;
    mDSPass1->writeMask = DepthStencilState::MASK_ALL;
    mDSPass1->comparison = DepthStencilState::ALWAYS;
    mDSPass1->stencilEnable = true;
    mDSPass1->frontFace.fail = DepthStencilState::OP_KEEP;
    mDSPass1->frontFace.depthFail = DepthStencilState::OP_KEEP;
    mDSPass1->frontFace.pass = DepthStencilState::OP_KEEP;
    mDSPass1->frontFace.comparison = DepthStencilState::EQUAL;

    mDSPass2 = std::make_shared<DepthStencilState>();
    mDSPass2->depthEnable = true;
    mDSPass2->writeMask = DepthStencilState::MASK_ALL;
    mDSPass2->comparison = DepthStencilState::LESS_EQUAL;
    mDSPass2->stencilEnable = true;
    mDSPass2->frontFace.fail = DepthStencilState::OP_KEEP;
    mDSPass2->frontFace.depthFail = DepthStencilState::OP_KEEP;
    mDSPass2->frontFace.pass = DepthStencilState::OP_ZERO;
    mDSPass2->frontFace.comparison = DepthStencilState::EQUAL;
}

void PlanarReflectionEffect::Draw(std::shared_ptr<GraphicsEngine> const& engine,
    VisibleSet const& visibleSet, PVWUpdater& pvwMatrices)
{
    // Save the global state, to be restored later.
    std::shared_ptr<BlendState> saveBState = engine->GetBlendState();
    std::shared_ptr<DepthStencilState> saveDSState = engine->GetDepthStencilState();
    std::shared_ptr<RasterizerState> saveRState = engine->GetRasterizerState();

    // The depth range will be modified during drawing, so save the current
    // depth range for restoration later.
    float minDepth, maxDepth;
    engine->GetDepthRange(minDepth, maxDepth);

    // Get the camera to store post-world transformations.
    std::shared_ptr<Camera> camera = pvwMatrices.GetCamera();

    // Get the current cull mode and reverse it.  Allow for models that are
    // not necessarily set up with front or back face culling.
    if (saveRState->cullMode == RasterizerState::CULL_BACK)
    {
        mCullReverse->cullMode = RasterizerState::CULL_FRONT;
    }
    else if (saveRState->cullMode == RasterizerState::CULL_FRONT)
    {
        mCullReverse->cullMode = RasterizerState::CULL_BACK;
    }
    else
    {
        mCullReverse->cullMode = RasterizerState::CULL_NONE;
    }
    engine->Bind(mCullReverse);

    unsigned int const numPlanes = static_cast<unsigned int>(mPlanes.size());
    for (unsigned int i = 0, reference = 1; i < numPlanes; ++i, ++reference)
    {
        std::shared_ptr<Visual> plane = mPlanes[i];

        // Render the plane to the stencil buffer only; that is, there are no
        // color writes or depth writes.  The depth buffer is read so that
        // plane pixels occluded by other already drawn geometry are not
        // drawn.  The stencil buffer value for pixels from plane i is (i+1).
        // The stencil buffer is updated at a pixel only when the depth test
        // passes at that pixel (the plane pixel is visible):
        //   frontFace.fail is always false, so value KEEP is irrelevant
        //   frontFace.depthFail = true, KEEP current stencil value
        //   frontFace.pass = false, REPLACE current stencil value with (i+1)
        mDSPass0->reference = reference;
        engine->SetDepthStencilState(mDSPass0);
        engine->SetBlendState(mNoColorWrites);
        engine->Draw(plane);

        // Render the plane again.  The stencil buffer comparison is EQUAL,
        // so the color and depth are updated only at pixels generated by the
        // plane; the stencil values for such pixels is necessarily (i+1).
        // The depth buffer comparison is ALWAYS and the depth range settings
        // cause the depth to be updated to maximum depth at all pixels
        // where the stencil values are (i+1).  This allows us to draw the
        // reflected object on the plane.  Color writes are enabled, because
        // the portion of the plane not covered by the reflected object must
        // be drawn because it is visible.
        mDSPass1->reference = reference;
        engine->SetDepthStencilState(mDSPass1);
        engine->SetDefaultBlendState();
        engine->SetDepthRange(maxDepth, maxDepth);
        engine->Draw(plane);
        engine->SetDepthRange(minDepth, maxDepth);

        // Render the reflected object only at pixels corresponding to those
        // drawn for the current plane; that is, where the stencil buffer
        // value is (i+1).  The reflection matrix is constructed from the
        // plane in world coordinates and must be applied in the
        // transformation pipeline before the world-to-view matrix is applied;
        // thus, we insert the reflection matrix into the pipeline via
        // SetPreViewMatrix.  Because the pvw-matrices are dependent on this,
        // each time the full transformation is computed we must update the
        // pvw matrices in the constant buffers for the shaders.  NOTE:  The
        // reflected objects will generate pixels whose depth is larger than
        // that for the reflecting plane.  This is not a problem, because we
        // will later draw the plane again and blend its pixels with the
        // reflected object pixels, after which the depth buffer values are
        // updated to the plane pixel depths.
        Matrix4x4<float> wMatrix = plane->worldTransform;
        Vector4<float> origin = DoTransform(wMatrix, mPlaneOrigins[i]);
        Vector4<float> normal = DoTransform(wMatrix, mPlaneNormals[i]);
        Normalize(normal);
        camera->SetPreViewMatrix(MakeReflection(origin, normal));
        pvwMatrices.Update();
        engine->SetDefaultDepthStencilState();
        engine->SetRasterizerState(mCullReverse);
        for (auto const& visual : visibleSet)
        {
            engine->Draw(visual);
        }                       
        engine->SetRasterizerState(saveRState);
        camera->SetPreViewMatrix(Matrix4x4<float>::Identity());
        pvwMatrices.Update();

        // Render the plane a third time and blend its colors with the
        // colors of the reflect objects.  The blending occurs only at the
        // pixels corresponding to the current plane; that is, where the
        // stencil values are (i+1).  The stencil values are cleared (set to
        // zero) at pixels where the depth test passes.  The blending uses
        // the reflectance value for the plane,
        //   (1 - reflectance) * plane.rgba + reflectance * backbuffer.rgba
        mDSPass2->reference = reference;
        mReflectanceBlend->blendColor = { mReflectances[i], mReflectances[i],
            mReflectances[i], mReflectances[i] };
        engine->SetDepthStencilState(mDSPass2);
        engine->SetBlendState(mReflectanceBlend);
        engine->Draw(plane);
    }

    // Restore the global state that existed before this function call.
    engine->SetBlendState(saveBState);
    engine->SetDepthStencilState(saveDSState);
    engine->SetRasterizerState(saveRState);

    // Render the objects using a normal drawing pass.
    for (auto const& visual : visibleSet)
    {
        engine->Draw(visual);
    }
}

std::pair<Vector4<float>, Vector4<float>> PlanarReflectionEffect::GetPlane(int i) const
{
    if (0 <= i && i < GetNumPlanes())
    {
        return std::make_pair(mPlaneOrigins[i], mPlaneNormals[i]);
    }
    else
    {
        return std::make_pair(Vector4<float>::Zero(), Vector4<float>::Zero());
    }
}

std::shared_ptr<Visual> PlanarReflectionEffect::GetPlaneVisual(int i) const
{
    if (0 <= i && i < GetNumPlanes())
    {
        return mPlanes[i];
    }
    else
    {
        return nullptr;
    }
}

void PlanarReflectionEffect::SetReflectance(int i, float reflectance)
{
    if (0 <= i && i < GetNumPlanes())
    {
        mReflectances[i] = reflectance;
    }
}

float PlanarReflectionEffect::GetReflectance(int i) const
{
    if (0 <= i && i < GetNumPlanes())
    {
        return mReflectances[i];
    }
    else
    {
        return 0.0f;
    }
}
