// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/Camera.h>
#include <Graphics/Node.h>

namespace gte
{
    class TrackObject
    {
    public:
        // Construction and destruction.
        virtual ~TrackObject() = default;
        TrackObject();
        TrackObject(int xSize, int ySize, std::shared_ptr<Camera> const& camera);

        // Member access.  The Set functions are for deferred construction
        // after a default construction of a trackball.
        void Set(int xSize, int ySize, std::shared_ptr<Camera> const& camera);

        inline int GetXSize() const
        {
            return mXSize;
        }

        inline int GetYSize() const
        {
            return mYSize;
        }

        inline std::shared_ptr<Camera> const& GetCamera() const
        {
            return mCamera;
        }

        // Set the root node that the track object manipulates.  NOTE:  The
        // constructor creates a default root node named mRoot.  The Set(...)
        // function replaces mRoot by the caller-specified 'root', which can
        // be a non-Node object.  If you have called Attach while the
        // default-constructed mRoot node is active, the SetRoot call will
        // replace mRoot and you lose the information about the previous mRoot
        // descendants.  If you intend on having a different root that is
        // Node-based, call Set(...) first and then use Attach.
        inline void Set(std::shared_ptr<Spatial> const& root)
        {
            mRoot = root;
        }

        inline std::shared_ptr<Spatial> const& GetRoot() const
        {
            return mRoot;
        }

        inline Matrix4x4<float> const& GetOrientation() const
        {
            return mRoot->worldTransform.GetRotation();
        }

        // The standard use is to set the initial point via a mouse button
        // click in a window rectangle.  Mark the trackball as active.  Once
        // the mouse button is released, mark the trackball as inactive.
        inline void SetActive(bool active)
        {
            mActive = active;
        }

        inline bool GetActive() const
        {
            return mActive;
        }

        // The root node is the top-level node of a hierarchy whose local
        // transformation is the trackball orientation relative to the
        // specified camera.  The camera directions {D,U,R} act as the world
        // coordinate system.  NOTE:  These functions are successful only
        // when the 'object' is a Node-based object.  If the 'object' is
        // a Visual-based object, the functions do nothing.
        void Attach(std::shared_ptr<Spatial> const& object);
        void Detach(std::shared_ptr<Spatial> const& object);
        void DetachAll();

        // Update the root in the scene graph sense.
        inline void Update(double applicationTime = 0.0)
        {
            mRoot->Update(applicationTime);
        }

        // Set the initial point of the track object.  The current track
        // object orientation is recorded.
        void SetInitialPoint(int x, int y);

        // Set the final point of the track object.  The track object
        // orientation is updated by the incremental rotation implied by the
        // initial and final points.
        void SetFinalPoint(int x, int y);

    protected:
        virtual void OnSetInitialPoint() = 0;
        virtual void OnSetFinalPoint() = 0;

        void NormalizeAndUpdateRoot(Matrix4x4<float>& rotate);

        std::shared_ptr<Camera> mCamera;
        std::shared_ptr<Spatial> mRoot;
        int mXSize, mYSize;
        float mX0, mY0, mX1, mY1, mMultiplier;
        bool mActive, mValid;
    };
}
