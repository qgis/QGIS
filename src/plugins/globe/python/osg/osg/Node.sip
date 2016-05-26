/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/


namespace osg {

/** A vector of Nodes pointers which is used to describe the path from a root node to a descendant.*/
typedef std::vector< Node* > NodePath;

/** A vector of NodePath, typically used to describe all the paths from a node to the potential root nodes it has.*/
typedef std::vector< NodePath > NodePathList;

/** A vector of NodePath, typically used to describe all the paths from a node to the potential root nodes it has.*/
typedef std::vector< Matrix > MatrixList;

/** Base class for all internal nodes in the scene graph.
    Provides interface for most common node operations (Composite Pattern).
*/
class Node : public osg::Object
{
%TypeHeaderCode
#include <osg/Node>
%End
    public:

        /** Construct a node.
            Initialize the parent list to empty, node name to "" and
            bounding sphere dirty flag to true.*/
        Node();
//
//        /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
//        Node(const Node&,const CopyOp& copyop=CopyOp::SHALLOW_COPY);
//
//        /** clone an object of the same type as the node.*/
//        virtual Object* cloneType() const { return new Node(); }
//
//        /** return a clone of a node, with Object* return type.*/
//        virtual Object* clone(const CopyOp& copyop) const { return new Node(*this,copyop); }
//
//        /** return true if this and obj are of the same kind of object.*/
//        virtual bool isSameKindAs(const Object* obj) const { return dynamic_cast<const Node*>(obj)!=NULL; }
//
//        /** return the name of the node's library.*/
//        virtual const char* libraryName() const { return "osg"; }
//
//        /** return the name of the node's class type.*/
//        virtual const char* className() const { return "Node"; }
//
//        /** convert 'this' into a Group pointer if Node is a Group, otherwise return 0.
//          * Equivalent to dynamic_cast<Group*>(this).*/
//        virtual Group* asGroup() { return 0; }
//        /** convert 'const this' into a const Group pointer if Node is a Group, otherwise return 0.
//          * Equivalent to dynamic_cast<const Group*>(this).*/
//        virtual const Group* asGroup() const { return 0; }
//
//        /** Convert 'this' into a Transform pointer if Node is a Transform, otherwise return 0.
//          * Equivalent to dynamic_cast<Transform*>(this).*/
//        virtual Transform* asTransform() { return 0; }
//
//        /** convert 'const this' into a const Transform pointer if Node is a Transform, otherwise return 0.
//          * Equivalent to dynamic_cast<const Transform*>(this).*/
//        virtual const Transform* asTransform() const { return 0; }
//
//        /** Convert 'this' into a Switch pointer if Node is a Switch, otherwise return 0.
//          * Equivalent to dynamic_cast<Switch*>(this).*/
//        virtual Switch* asSwitch() { return 0; }
//
//        /** convert 'const this' into a const Switch pointer if Node is a Switch, otherwise return 0.
//          * Equivalent to dynamic_cast<const Switch*>(this).*/
//        virtual const Switch* asSwitch() const { return 0; }
//
//        /** Convert 'this' into a Geode pointer if Node is a Geode, otherwise return 0.
//          * Equivalent to dynamic_cast<Geode*>(this).*/
//        virtual Geode* asGeode() { return 0; }
//
//        /** convert 'const this' into a const Geode pointer if Node is a Geode, otherwise return 0.
//          * Equivalent to dynamic_cast<const Geode*>(this).*/
//        virtual const Geode* asGeode() const { return 0; }
//
//        /** Convert 'this' into a Transform pointer if Node is a Terrain, otherwise return 0.
//          * Equivalent to dynamic_cast<Terrrain*>(this).*/
//        virtual osgTerrain::Terrain* asTerrain() { return 0; }
//
//        /** convert 'const this' into a const Terrain pointer if Node is a Terrain, otherwise return 0.
//          * Equivalent to dynamic_cast<const Terrain*>(this).*/
//        virtual const osgTerrain::Terrain* asTerrain() const { return 0; }
//
//
//        /** Visitor Pattern : calls the apply method of a NodeVisitor with this node's type.*/
//        virtual void accept(NodeVisitor& nv);
//        /** Traverse upwards : calls parents' accept method with NodeVisitor.*/
//        virtual void ascend(NodeVisitor& nv);
//        /** Traverse downwards : calls children's accept method with NodeVisitor.*/
//        virtual void traverse(NodeVisitor& /*nv*/) {}
//
//        /** A vector of osg::Group pointers which is used to store the parent(s) of node.*/
//        typedef std::vector<Group*> ParentList;
//
//        /** Get the parent list of node. */
//        inline const ParentList& getParents() const { return _parents; }
//
//        /** Get the a copy of parent list of node. A copy is returned to
//          * prevent modification of the parent list.*/
//        inline ParentList getParents() { return _parents; }
//
//        inline Group* getParent(unsigned int i)  { return _parents[i]; }
//
//        /**
//         * Get a single const parent of node.
//         * @param i index of the parent to get.
//         * @return the parent i.
//         */
//        inline const Group* getParent(unsigned int i) const  { return _parents[i]; }
//
//        /**
//         * Get the number of parents of node.
//         * @return the number of parents of this node.
//         */
//        inline unsigned int getNumParents() const { return static_cast<unsigned int>(_parents.size()); }
//
//        /** Get the list of node paths parent paths.
//          * The optional Node* haltTraversalAtNode allows the user to prevent traversal beyond a specifed node. */
//        NodePathList getParentalNodePaths(osg::Node* haltTraversalAtNode=0) const;
//
//        /** Get the list of matrices that transform this node from local coordinates to world coordinates.
//          * The optional Node* haltTraversalAtNode allows the user to prevent traversal beyond a specifed node. */
//        MatrixList getWorldMatrices(const osg::Node* haltTraversalAtNode=0) const;
//
//
//        /** Set update node callback, called during update traversal. */
//        void setUpdateCallback(NodeCallback* nc);
//
//        /** Get update node callback, called during update traversal. */
//        inline NodeCallback* getUpdateCallback() { return _updateCallback.get(); }
//
//        /** Get const update node callback, called during update traversal. */
//        inline const NodeCallback* getUpdateCallback() const { return _updateCallback.get(); }
//
//        /** Convenience method that sets the update callback of the node if it doesn't exist, or nest it into the existing one. */
//        inline void addUpdateCallback(NodeCallback* nc) {
//            if (nc != NULL) {
//                if (_updateCallback.valid()) _updateCallback->addNestedCallback(nc);
//                else setUpdateCallback(nc);
//            }
//        }
//
//        /** Convenience method that removes a given callback from a node, even if that callback is nested. There is no error return in case the given callback is not found. */
//        inline void removeUpdateCallback(NodeCallback* nc) {
//            if (nc != NULL && _updateCallback.valid()) {
//                if (_updateCallback == nc) setUpdateCallback(nc->getNestedCallback());        // replace the callback by the nested one
//                else _updateCallback->removeNestedCallback(nc);
//            }
//        }
//
//        /** Get the number of Children of this node which require Update traversal,
//          * since they have an Update Callback attached to them or their children.*/
//        inline unsigned int getNumChildrenRequiringUpdateTraversal() const { return _numChildrenRequiringUpdateTraversal; }
//
//
//        /** Set event node callback, called during event traversal. */
//        void setEventCallback(NodeCallback* nc);
//
//        /** Get event node callback, called during event traversal. */
//        inline NodeCallback* getEventCallback() { return _eventCallback.get(); }
//
//        /** Get const event node callback, called during event traversal. */
//        inline const NodeCallback* getEventCallback() const { return _eventCallback.get(); }
//
//        /** Convenience method that sets the event callback of the node if it doesn't exist, or nest it into the existing one. */
//        inline void addEventCallback(NodeCallback* nc) {
//            if (nc != NULL) {
//                if (_eventCallback.valid()) _eventCallback->addNestedCallback(nc);
//                else setEventCallback(nc);
//            }
//        }
//
//        /** Convenience method that removes a given callback from a node, even if that callback is nested. There is no error return in case the given callback is not found. */
//        inline void removeEventCallback(NodeCallback* nc) {
//            if (nc != NULL && _eventCallback.valid()) {
//                if (_eventCallback == nc) setEventCallback(nc->getNestedCallback());        // replace the callback by the nested one
//                else _eventCallback->removeNestedCallback(nc);
//            }
//        }
//
//        /** Get the number of Children of this node which require Event traversal,
//          * since they have an Event Callback attached to them or their children.*/
//        inline unsigned int getNumChildrenRequiringEventTraversal() const { return _numChildrenRequiringEventTraversal; }
//
//
//        /** Set cull node callback, called during cull traversal. */
//        void setCullCallback(NodeCallback* nc) { _cullCallback = nc; }
//
//        /** Get cull node callback, called during cull traversal. */
//        inline NodeCallback* getCullCallback() { return _cullCallback.get(); }
//
//        /** Get const cull node callback, called during cull traversal. */
//        inline const NodeCallback* getCullCallback() const { return _cullCallback.get(); }
//
//        /** Convenience method that sets the cull callback of the node if it doesn't exist, or nest it into the existing one. */
//        inline void addCullCallback(NodeCallback* nc) {
//            if (nc != NULL) {
//                if (_cullCallback.valid()) _cullCallback->addNestedCallback(nc);
//                else setCullCallback(nc);
//            }
//        }
//
//        /** Convenience method that removes a given callback from a node, even if that callback is nested. There is no error return in case the given callback is not found. */
//        inline void removeCullCallback(NodeCallback* nc) {
//            if (nc != NULL && _cullCallback.valid()) {
//                if (_cullCallback == nc) setCullCallback(nc->getNestedCallback());        // replace the callback by the nested one
//                else _cullCallback->removeNestedCallback(nc);
//            }
//        }
//
//        /** Set the view frustum/small feature culling of this node to be active or inactive.
//          * The default value is true for _cullingActive. Used as a guide
//          * to the cull traversal.*/
//        void setCullingActive(bool active);
//
//        /** Get the view frustum/small feature _cullingActive flag for this node. Used as a guide
//          * to the cull traversal.*/
//        inline bool getCullingActive() const { return _cullingActive; }
//
//        /** Get the number of Children of this node which have culling disabled.*/
//        inline unsigned int getNumChildrenWithCullingDisabled() const { return _numChildrenWithCullingDisabled; }
//
//        /** Return true if this node can be culled by view frustum, occlusion or small feature culling during the cull traversal.
//          * Note, returns true only if no children have culling disabled, and the local _cullingActive flag is true.*/
//        inline bool isCullingActive() const { return _numChildrenWithCullingDisabled==0 && _cullingActive && getBound().valid(); }
//
//        /** Get the number of Children of this node which are or have OccluderNode's.*/
//        inline unsigned int getNumChildrenWithOccluderNodes() const { return _numChildrenWithOccluderNodes; }
//
//
//        /** return true if this node is an OccluderNode or the subgraph below this node are OccluderNodes.*/
//        bool containsOccluderNodes() const;
//
//
//        /**
//        * This is a set of bits (flags) that represent the Node.
//        * The default value is 0xffffffff (all bits set).
//        *
//        * The most common use of these is during traversal of the scene graph.
//        * For instance, when traversing the scene graph the osg::NodeVisitor does a bitwise
//        * AND of its TraversalMask with the Node#s NodeMask to
//        * determine if the Node should be processed/traversed.
//        *
//        * For example, if a Node has a NodeMask value of 0x02 (only 2nd bit set)
//        * and the osg::Camera has a CullMask of 0x4 (2nd bit not set) then during cull traversal,
//        * which takes it's TraversalMask from the Camera's CullMask, the node and any children
//        * would be ignored and thereby treated as "culled" and thus not rendered.
//        * Conversely, if the osg::Camera CullMask were 0x3 (2nd bit set) then the node
//        * would be processed and child Nodes would be examined.
//        */
//        typedef unsigned int NodeMask;
//        /** Set the node mask.*/
//        inline void setNodeMask(NodeMask nm) { _nodeMask = nm; }
//        /** Get the node Mask.*/
//        inline NodeMask getNodeMask() const { return _nodeMask; }
//
//
//
//        /** Set the node's StateSet.*/
//        void setStateSet(osg::StateSet* stateset);
//
//        /** return the node's StateSet, if one does not already exist create it
//          * set the node and return the newly created StateSet. This ensures
//          * that a valid StateSet is always returned and can be used directly.*/
//        osg::StateSet* getOrCreateStateSet();
//
//        /** Return the node's StateSet. returns NULL if a stateset is not attached.*/
//        inline osg::StateSet* getStateSet() { return _stateset.get(); }
//
//        /** Return the node's const StateSet. Returns NULL if a stateset is not attached.*/
//        inline const osg::StateSet* getStateSet() const { return _stateset.get(); }
//
//
//        /** A vector of std::string's which are used to describe the object.*/
//        typedef std::vector<std::string> DescriptionList;
//
//        /** Set the list of string descriptions.*/
//        void setDescriptions(const DescriptionList& descriptions);
//
//        /** Get the description list of the node.*/
//        DescriptionList& getDescriptions();
//
//        /** Get the const description list of the const node.*/
//        const DescriptionList& getDescriptions() const;
//
//
//        /** Get a single const description of the const node.*/
//        const std::string& getDescription(unsigned int i) const;
//
//        /** Get a single description of the node.*/
//        std::string& getDescription(unsigned int i);
//
//        /** Get the number of descriptions of the node.*/
//        unsigned int getNumDescriptions() const;
//
//        /** Add a description string to the node.*/
//        void addDescription(const std::string& desc);
//
//
//        /** Set the initial bounding volume to use when computing the overall bounding volume.*/
//        void setInitialBound(const osg::BoundingSphere& bsphere) { _initialBound = bsphere; dirtyBound(); }
//
//        /** Set the initial bounding volume to use when computing the overall bounding volume.*/
//        const BoundingSphere& getInitialBound() const { return _initialBound; }
//
//        /** Mark this node's bounding sphere dirty.
//            Forcing it to be computed on the next call to getBound().*/
//        void dirtyBound();
//
//        /** Get the bounding sphere of node.
//           Using lazy evaluation computes the bounding sphere if it is 'dirty'.*/
//        inline const BoundingSphere& getBound() const
//        {
//            if(!_boundingSphereComputed)
//            {
//                _boundingSphere = _initialBound;
//                if (_computeBoundCallback.valid())
//                    _boundingSphere.expandBy(_computeBoundCallback->computeBound(*this));
//                else
//                    _boundingSphere.expandBy(computeBound());
//
//                _boundingSphereComputed = true;
//            }
//            return _boundingSphere;
//        }
//
//
//        /** Compute the bounding sphere around Node's geometry or children.
//            This method is automatically called by getBound() when the bounding
//            sphere has been marked dirty via dirtyBound().*/
//        virtual BoundingSphere computeBound() const;
//
//        /** Callback to allow users to override the default computation of bounding volume.*/
//        struct ComputeBoundingSphereCallback : public osg::Object
//        {
//            ComputeBoundingSphereCallback() {}
//
//            ComputeBoundingSphereCallback(const ComputeBoundingSphereCallback&,const CopyOp&) {}
//
//            META_Object(osg,ComputeBoundingSphereCallback);
//
//           virtual BoundingSphere computeBound(const osg::Node&) const { return BoundingSphere(); }
//        };
//
//        /** Set the compute bound callback to override the default computeBound.*/
//        void setComputeBoundingSphereCallback(ComputeBoundingSphereCallback* callback) { _computeBoundCallback = callback; }
//
//        /** Get the compute bound callback.*/
//        ComputeBoundingSphereCallback* getComputeBoundingSphereCallback() { return _computeBoundCallback.get(); }
//
//        /** Get the const compute bound callback.*/
//        const ComputeBoundingSphereCallback* getComputeBoundingSphereCallback() const { return _computeBoundCallback.get(); }
//
//        /** Set whether to use a mutex to ensure ref() and unref() are thread safe.*/
//        virtual void setThreadSafeRefUnref(bool threadSafe);
//
//        /** Resize any per context GLObject buffers to specified size. */
//        virtual void resizeGLObjectBuffers(unsigned int /*maxSize*/);
//
//        /** If State is non-zero, this function releases any associated OpenGL objects for
//           * the specified graphics context. Otherwise, releases OpenGL objects
//           * for all graphics contexts. */
//        virtual void releaseGLObjects(osg::State* = 0) const;
//
//
//    protected:
//
//
//
//        BoundingSphere                          _initialBound;
//        ref_ptr<ComputeBoundingSphereCallback>  _computeBoundCallback;
//        mutable BoundingSphere                  _boundingSphere;
//        mutable bool                            _boundingSphereComputed;
//
//        void addParent(osg::Group* node);
//        void removeParent(osg::Group* node);
//
//        ParentList _parents;
//        friend class osg::Group;
//        friend class osg::Drawable;
//        friend class osg::StateSet;
//
//        ref_ptr<NodeCallback> _updateCallback;
//        unsigned int _numChildrenRequiringUpdateTraversal;
//        void setNumChildrenRequiringUpdateTraversal(unsigned int num);
//
//        ref_ptr<NodeCallback> _eventCallback;
//        unsigned int _numChildrenRequiringEventTraversal;
//        void setNumChildrenRequiringEventTraversal(unsigned int num);
//
//        ref_ptr<NodeCallback> _cullCallback;
//
//        bool _cullingActive;
//        unsigned int _numChildrenWithCullingDisabled;
//        void setNumChildrenWithCullingDisabled(unsigned int num);
//
//        unsigned int _numChildrenWithOccluderNodes;
//        void setNumChildrenWithOccluderNodes(unsigned int num);
//
//        NodeMask _nodeMask;
//
//        ref_ptr<StateSet> _stateset;
//
  private:

        /** Node destructor. Note, is protected so that Nodes cannot
            be deleted other than by being dereferenced and the reference
            count being zero (see osg::Referenced), preventing the deletion
            of nodes which are still in use. This also means that
            Nodes cannot be created on stack i.e Node node will not compile,
            forcing all nodes to be created on the heap i.e Node* node
            = new Node().*/
        virtual ~Node();

};

};
