// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/BoundingSphere.h>
#include <Graphics/Camera.h>
#include <memory>
#include <vector>

// Support for determining the potentially visible set of Visual objects in a
// scene.  The class is designed to support derived classes that provide
// specialized culling and sorting.  One example is a culler that stores
// opaque objects in one set, sorted from front to back, and semitransparent
// objects in another set, sorted from back to front.  Another example is a
// portal system (room-graph visibility) that must maintain a set of unique
// visible objects--one object viewed through two portals should not be
// inserted into the set twice.

namespace gte
{
    class Spatial;
    class Visual;

    enum CullingMode
    {
        // Determine visibility state by comparing the world bounding volume
        // to culling planes.
        CULL_DYNAMIC,

        // Force the object to be culled.  If a Node is culled, its entire
        // subtree is culled.
        CULL_ALWAYS,

        // Never cull the object.  If a Node is never culled, its entire
        // subtree is never culled.  To accomplish this, the first time such
        // a Node is encountered, the 'noCull' parameter is set to 'true' in
        // the recursive chain GetVisibleSet/OnGetVisibleSet.
        CULL_NEVER
    };

    typedef std::vector<Visual*> VisibleSet;

    class Culler
    {
    public:
        // Construction and destruction.
        virtual ~Culler();
        Culler();

        // Access to the stack of world culling planes.  The first 6 planes
        // are those associated with the camera's view frustum.  You may push
        // and pop/ planes to be used in addition to the view frustum planes.
        // The return value for PushPlane is 'true' as long as you have not
        // exceeded the stack capacity.  The return value for PopPlane is
        // 'true' as long as you pop planes you pushed; it is not possible to
        // pop the 6 planes for the view frustum.
        enum { MAX_PLANE_QUANTITY = 32 };
        bool PushPlane(CullingPlane<float> const& plane);
        bool PopPlane();

        // This is the main function you should use for culling within a scene
        // graph.  Traverse the scene graph and construct the potentially
        // visible set relative to the world planes.
        void ComputeVisibleSet(std::shared_ptr<Camera> const& camera,
            std::shared_ptr<Spatial> const& scene);

        // Access to the camera and potentially visible set.
        inline VisibleSet& GetVisibleSet()
        {
            return mVisibleSet;
        }

    protected:
        enum { INITIALLY_VISIBLE = 128 };

        // These classes must make calls into the Culler, but applications are
        // not allowed to.
        friend class Spatial;
        friend class Visual;

        // Compare the object's world bounding sphere against the culling
        // planes.  Only Spatial calls this function.
        bool IsVisible(BoundingSphere<float> const& sphere);

        // The base class behavior is to append the visible object to the end
        // of the visible set (stored as an array).  Derived classes may
        // override this behavior; for example, the array might be maintained
        // as a sorted array for minimizing render state changes or it might
        // be maintained as a unique list of objects for a portal system.
        // Only Visual calls this function.
        virtual void Insert(Visual* visible);

        // See the comments before data member mPlaneState about the bit
        // system for enabling and disabling planes during culling.  Only
        // Spatial calls these functions (during a scene graph traversal).
        inline void SetPlaneState(unsigned int planeState)
        {
            mPlaneState = planeState;
        }

        inline unsigned int GetPlaneState() const
        {
            return mPlaneState;
        }

        void PushViewFrustumPlanes(std::shared_ptr<Camera> const& camera);

        // The world culling planes corresponding to the view frustum plus any
        // additional user-defined culling planes.  The member mPlaneState
        // represents bit flags to store whether or not a plane is active in
        // the culling system.  A bit of 1 means the plane is active;
        // otherwise, the plane is inactive.  An active plane is compared to
        // bounding volumes, whereas an inactive plane is not.  This supports
        // an efficient culling of a hierarchy.  For example, if a node's
        // bounding volume is inside the left plane of the view frustum, then
        // the left plane is set to inactive because the children of the node
        // are automatically all inside the left plane.
        int mPlaneQuantity;
        std::array<CullingPlane<float>, MAX_PLANE_QUANTITY> mPlane;
        unsigned int mPlaneState;

        // The potentially visible set generated by ComputeVisibleSet(scene).
        VisibleSet mVisibleSet;
    };
}
