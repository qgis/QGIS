// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/Camera.h>
#include <Graphics/ConstantBuffer.h>
#include <Graphics/Visual.h>
#include <map>

// The PVWUpdater class is responsible for managing memory associated with
// projection-view-world matrices stored in ConstantBuffer objects that are
// used in shader programs.  The common case is when a vertex shader has a
// 4x4 matrix that is stored in a single constant buffer.
//
// The projection and view matrices are associated with a Camera object.  The
// world matrix is usually the world transform of a Visual object, but it can
// be a matrix unassociated with such objects.  When the projection P, view V
// or world matrix W changes in CPU memory, the PVW product must be recomputed
// in the ConstantBuffer object's CPU memory and uploaded to the GPU memory of
// that object.  The BufferUpdater function object has the responsibility for
// the CPU-to-GPU memory copy.  Its body is usually a single-line statement,
// 'engine->Update(buffer)', where 'engine' is a GraphicsEngine-derived class
// object.
//
// Three update scenarios are supported by PVWUpdater.
//
//   1. If you have a set of world matrices and associated constant buffers
//      that must be updated each frame, and if that set does not change over
//      time or changes infrequently over time, you can subscribe the members
//      of the set as listeners for the PVWUpdater::Update() function call.
//      When a matrix-buffer pair no longer needs to listen, that pair can be
//      unsubscribed as a listener.
//
//   2. If you have a scene graph whose leaf nodes are Visual objects with
//      VisualEffects attached to them, and if you use a Culler object to
//      collect the potentially visible set of objects in the camera's view
//      frustum, you can call culler.ComputeVisibleSet(camera, scene) and
//      then pvwUpdater.Update(culler.GetVisibleSet()).
//
//   3. The subscription model of scenario 1 and the visibility model of
//      scenario 2 are independent of each other. You can manage the updates
//      of a static set of matrix-buffer pairs (for example, the stationary
//      background objects in the world) and a dynamic set of matrix-buffer
//      pairs (for example, the moving objects in the world).

namespace gte
{
    class PVWUpdater
    {
    public:
        // Construction and destruction.
        virtual ~PVWUpdater() = default;
        PVWUpdater();
        PVWUpdater(std::shared_ptr<Camera> const& camera, BufferUpdater const& updater);

        // Member access.  The functions are for deferred construction after
        // a default construction of a PVWUpdater.
        void Set(std::shared_ptr<Camera> const& camera, BufferUpdater const& updater);

        inline std::shared_ptr<Camera> const& GetCamera() const
        {
            return mCamera;
        }

        inline void SetUpdater(BufferUpdater const& updater)
        {
            mUpdater = updater;
        }

        inline BufferUpdater const& GetUpdater() const
        {
            return mUpdater;
        }

        // Functions supporting a static set of matrix-buffer pairs.  The
        // Subscribe functions use the address of the world matrix as a key to
        // a std::map, so be careful to ensure that the world matrix persists
        // until a call to an Unsubscribe function for that matrix.  The
        // return value of Subscribe is 'true' as long as the world matrix and
        // its associated constant buffer are not already subscribed.  The
        // return value of Unsubscribe is 'true' if and only if the world
        // matrix is currently subscribed.

        // This is the most general subscription function where you explicitly
        // provide the world matrix and associated constant buffer.  The input
        // 'pvwMatrixName' is the name specified in the shader program for the
        // PVW matrix and is used in SetMember<Matrix4x4<float>>(...) of the
        // ConstantBuffer object storing that matrix.  The default "pvwMatrix"
        // is used by GTEngine VisualEffect vertex shaders.
        bool Subscribe(Matrix4x4<float> const& worldMatrix,
            std::shared_ptr<ConstantBuffer> const& cbuffer,
            std::string const& pvwMatrixName = "pvwMatrix");

        // The world matrix is visual->worldTransform and the constant buffer
        // is visual->GetEffect()->GetPVWMatrixConstant().  If you subscribe
        // a Visual object, it must exist at least as long as the PVWUpdater
        // object that is managing its CPU-to-GPU memory copies.
        bool Subscribe(std::shared_ptr<Visual> const& visual,
            std::string const& pvwMatrixName = "pvwMatrix");

        bool Unsubscribe(Matrix4x4<float> const& worldMatrix);
        bool Unsubscribe(std::shared_ptr<Visual> const& visual);
        void UnsubscribeAll();

        // After any camera modifictions that change the projection or view
        // matrices, or after any modifications to world matrices of the
        // subscribed pairs, call this function to recompute the PVW matrices
        // in CPU memory of the constant buffers and copy that to GPU memory.
        void Update();


        // Function supporting a dynamic set of matrix-buffer pairs based on
        // the potentially visible set.  Although you can create the visible
        // set using Culler::ComputeVisibleSet, you can equally manage any set
        // of Visual objects to update if you so choose.
        void Update(std::vector<Visual*> const& updateSet);

    protected:
        std::shared_ptr<Camera> mCamera;
        BufferUpdater mUpdater;

        typedef Matrix4x4<float> const* PVWKey;
        typedef std::pair<std::shared_ptr<ConstantBuffer>, std::string> PVWValue;
        std::map<PVWKey, PVWValue> mSubscribers;
    };
}
