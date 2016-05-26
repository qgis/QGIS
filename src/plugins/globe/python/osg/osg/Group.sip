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

typedef std::vector< ref_ptr<Node> > NodeList;

/** General group node which maintains a list of children.
  * Children are reference counted. This allows children to be shared
  * with memory management handled automatically via osg::Referenced.
*/
class Group : osg::Node
{
%TypeHeaderCode
#include <osg/Group>
%End
    public :


        Group();

//        /** Copy constructor using CopyOp to manage deep vs shallow copy. */
//        Group(const Group&,const CopyOp& copyop=CopyOp::SHALLOW_COPY);
//
//        META_Node(osg, Group);
//
//        virtual Group* asGroup() { return this; }
//        virtual const Group* asGroup() const { return this; }
//
//        virtual void traverse(NodeVisitor& nv);
//
//        /** Add Node to Group.
//          * If node is not NULL and is not contained in Group then increment its
//          * reference count, add it to the child list and dirty the bounding
//          * sphere to force it to recompute on next getBound() and return true for success.
//          * Otherwise return false. Scene nodes can't be added as child nodes.
//        */
//        virtual bool addChild( Node *child );
//
//        /** Insert Node to Group at specific location.
//          * The new child node is inserted into the child list
//          * before the node at the specified index. No nodes
//          * are removed from the group with this operation.
//        */
//        virtual bool insertChild( unsigned int index, Node *child );
//
//        /** Remove Node from Group.
//          * If Node is contained in Group then remove it from the child
//          * list, decrement its reference count, and dirty the
//          * bounding sphere to force it to recompute on next getBound() and
//          * return true for success. If Node is not found then return false
//          * and do not change the reference count of the Node.
//          * Note, do not override, only override removeChildren(,) is required.
//        */
//        inline bool removeChild( Node *child )
//        {
//            unsigned int pos = getChildIndex(child);
//            if (pos<_children.size()) return removeChildren(pos,1);
//            else return false;
//        }
//
//        /** Remove Node from Group.
//          * If Node is contained in Group then remove it from the child
//          * list, decrement its reference count, and dirty the
//          * bounding sphere to force it to recompute on next getBound() and
//          * return true for success. If Node is not found then return false
//          * and do not change the reference count of the Node.
//          * Note, do not override, only override removeChildren(,) is required.
//        */
//        inline bool removeChild( unsigned int pos, unsigned int numChildrenToRemove=1 )
//        {
//            if (pos<_children.size()) return removeChildren(pos,numChildrenToRemove);
//            else return false;
//        }
//
//        /** Remove children from Group.
//          * Note, must be override by subclasses of Group which add per child attributes.*/
//        virtual bool removeChildren(unsigned int pos,unsigned int numChildrenToRemove);
//
//        /** Replace specified Node with another Node.
//          * Equivalent to setChild(getChildIndex(orignChild),node)
//          * See docs for setChild for further details on implementation.
//          */
//        virtual bool replaceChild( Node *origChild, Node* newChild );
//
//        /** Return the number of children nodes. */
//        inline unsigned int getNumChildren() const { return static_cast<unsigned int>(_children.size()); }
//
//        /** Set child node at position i.
//          * Return true if set correctly, false on failure (if node==NULL || i is out of range).
//          * When Set can be successful applied, the algorithm is : decrement the reference count origNode and increment the
//          * reference count of newNode, and dirty the bounding sphere
//          * to force it to recompute on next getBound() and return true.
//          * If origNode is not found then return false and do not
//          * add newNode. If newNode is NULL then return false and do
//          * not remove origNode. Also returns false if newChild is a Scene node.
//        */
//        virtual bool setChild( unsigned  int i, Node* node );
//
//        /** Return child node at position i. */
//        inline Node* getChild( unsigned  int i ) { return _children[i].get(); }
//
//        /** Return child node at position i. */
//        inline const Node* getChild( unsigned  int i ) const { return _children[i].get(); }
//
//        /** Return true if node is contained within Group. */
//        inline bool containsNode( const Node* node ) const
//        {
//
//            for (NodeList::const_iterator itr=_children.begin();
//                 itr!=_children.end();
//                 ++itr)
//            {
//                if (itr->get()==node) return true;
//            }
//            return false;
//        }
//
//        /** Get the index number of child, return a value between
//          * 0 and _children.size()-1 if found, if not found then
//          * return _children.size().
//        */
//        inline unsigned int getChildIndex( const Node* node ) const
//        {
//            for (unsigned int childNum=0;childNum<_children.size();++childNum)
//            {
//                if (_children[childNum]==node) return childNum;
//            }
//            return static_cast<unsigned int>(_children.size()); // node not found.
//        }
//
//        /** Set whether to use a mutex to ensure ref() and unref() are thread safe.*/
//        virtual void setThreadSafeRefUnref(bool threadSafe);
//
//        /** Resize any per context GLObject buffers to specified size. */
//        virtual void resizeGLObjectBuffers(unsigned int maxSize);
//
//        /** If State is non-zero, this function releases any associated OpenGL objects for
//           * the specified graphics context. Otherwise, releases OpenGL objects
//           * for all graphics contexts. */
//        virtual void releaseGLObjects(osg::State* = 0) const;
//
//        virtual BoundingSphere computeBound() const;
//
//    protected:
//
//
//        virtual void childRemoved(unsigned int /*pos*/, unsigned int /*numChildrenToRemove*/) {}
//        virtual void childInserted(unsigned int /*pos*/) {}
//
//        NodeList _children;
//
    private:
        ~Group();

};

};
