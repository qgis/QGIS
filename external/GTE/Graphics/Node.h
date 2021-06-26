// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2021.04.26

#pragma once

#include <Graphics/Spatial.h>

// This class represents grouping nodes in a spatial hierarchy.

namespace gte
{
    class Node : public Spatial
    {
    public:
        // Construction and destruction.
        virtual ~Node();
        Node() = default;

        // This is the current number of elements in the child array.  These
        // elements are not all guaranteed to be non-null.  Thus, when you
        // iterate over the array and access children with GetChild(...),
        // you should verify the child pointer is not null before
        // dereferencing it.
        int GetNumChildren() const;

        // Attach a child to this node.  If the function succeeds, the return
        // value is the index i of the array where the child was stored, in
        // which case 0 <= i < GetNumChildren().  The first available empty
        // slot of the child array is used for storage.  If all slots are
        // filled, the child is appended to the array (potentially causing a
        // reallocation of the array).
        // 
        // The function fails when 'child' is null or when 'child' already has
        // a parent, in which case the return value is -1.  The nodes form a
        // tree, not a more general directed acyclic graph.  A consequence is
        // that a node cannot have more than one parent.  For example,
        //     Node* node0 = <some node>;
        //     Spatial* child = <some child>;
        //     int index = node0->AttachChild(child);
        //     Node* node1 = <some node>;
        //
        //     // This asserts because 'child' already has a parent (node0).
        //     node1->AttachChild(child);
        //
        //     // The correct way to give 'child' a new parent.
        //     node0->DetachChild(child);  // or node0->DetachChildAt(index);
        //     node1->AttachChild(child);
        //
        //     // In the last example before the DetachChild call, if 'child'
        //     // is referenced only by node0, the detach will cause 'child'
        //     // to be deleted (Node internally reference counts its
        //     // children).  If you want to keep 'child' around for later
        //     // use, do the following.
        //     Spatial::SP saveChild = SPCreate(node0->GetChild(0));
        //     node0->DetachChild(saveChild);
        //     node1->AttachChild(saveChild);
        int AttachChild(std::shared_ptr<Spatial> const& child);

        // Detach a child from this node.  If the 'child' is non-null and in
        // the array, the return value is the index in the array that had
        // stored the child; otherwise, the function returns -1.
        int DetachChild(std::shared_ptr<Spatial> const& child);

        // Detach a child from this node.  If 0 <= i < GetNumChildren(), the
        // return value is the child at index i; otherwise, the function
        // returns null.
        std::shared_ptr<Spatial> DetachChildAt(int i);

        // Detach all children from this node.
        void DetachAllChildren();

        // The same comments for AttachChild apply here regarding the
        // inability to have multiple parents.  If 0 <= i < GetNumChildren(),
        // the function succeeds and returns i.  If i is out of range, the
        // function *still* succeeds, appending the child to the end of the
        // array.  The return value is the previous child stored at index i.
        std::shared_ptr<Spatial> SetChild(int i, std::shared_ptr<Spatial> const& child);

        // Get the child at the specified index. If 0 <= i < GetNumChildren(),
        // the function succeeds and returns the child at that index.  Keep in
        // mind that child[i] could very well be null.  If i is out of range,
        // the function returns null.
        std::shared_ptr<Spatial> GetChild(int i);

    protected:
        // Support for geometric updates.
        virtual void UpdateWorldData(double applicationTime) override;
        virtual void UpdateWorldBound() override;

        // Support for hierarchical culling.
        virtual void GetVisibleSet(Culler& culler,
            std::shared_ptr<Camera> const& camera, bool noCull) override;

        // Child pointers.
        std::vector<std::shared_ptr<Spatial>> mChild;
    };
}
