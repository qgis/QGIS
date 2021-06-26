// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/ControlledObject.h>
#include <Graphics/Culler.h>
#include <Mathematics/Transform.h>
#include <string>

// Support for a spatial hierarchy of objects.  Class Spatial has a parent
// pointer.  Class Node derives from Spatial has an array of child pointers.
// The leaf nodes of the hierarchy are either graphical or audial.  Class
// Visual derives from Spatial and represents graphical data.  Class Audial
// derives from Spatial and represents sound data.

namespace gte
{
    class Camera;

    class Spatial : public ControlledObject
    {
    public:
        // Abstract base class.
        virtual ~Spatial();

        // Update of geometric state.  The function computes world
        // transformations on the downward pass of the scene graph traversal
        // and world bounding volumes on the upward pass of the traversal.
        // The object that calls the update is the initiator.  Other objects
        // visited during the update are not initiators.  The application
        // time is in milliseconds.
        void Update(double applicationTime = 0.0, bool initiator = true);

        // Access to the parent object, which is null for the root of the
        // hierarchy.
        inline Spatial* GetParent()
        {
            return mParent;
        }

        // Allow user-readable names for nodes in a scene graph.
        std::string name;

        // Local and world transforms.  In some situations you might need to
        // set the world transform directly and bypass the Spatial::Update()
        // mechanism, in which case worldTransformIsCurrent should be set to
        // 'true'.
        Transform<float> localTransform;
        Transform<float> worldTransform;
        bool worldTransformIsCurrent;

        // World bound access.  In some situations you might want to set the
        // world bound directly and bypass the Spatial::Update() mechanism, in
        // which case worldBoundIsCurrent flag should be set to 'true'.
        BoundingSphere<float> worldBound;
        CullingMode culling;
        bool worldBoundIsCurrent;

    public:
        // Support for hierarchical culling.
        void OnGetVisibleSet(Culler& culler, std::shared_ptr<Camera> const& camera,
            bool noCull);

        virtual void GetVisibleSet(Culler& culler,
            std::shared_ptr<Camera> const& camera, bool noCull) = 0;

        // Access to the parent object.  Node calls this during attach/detach
        // of children.
        inline void SetParent(Spatial* parent)
        {
            mParent = parent;
        }

    protected:
        // Constructor accessible by Node, Visual, and Audial.
        Spatial();

        // Support for geometric updates.
        virtual void UpdateWorldData(double applicationTime);
        virtual void UpdateWorldBound() = 0;
        void PropagateBoundToRoot();

    private:
        // Support for a hierarchical scene graph.  Spatial provides the
        // parent pointer.  Node provides the child pointers.  The parent
        // pointer is not shared to avoid reference-count cycles between
        // mParent and 'this.  Because the pointer links are set internally
        // rather than by an external manager, it is not possible to use
        // std::weak_ptr to avoid the cycle because we do not know the
        // shared_ptr object that owns mParent.
        Spatial* mParent;
    };
}
