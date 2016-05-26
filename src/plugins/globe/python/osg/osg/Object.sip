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

//// forward declare
//
///** META_Object macro define the standard clone, isSameKindAs and className methods.
//  * Use when subclassing from Object to make it more convenient to define
//  * the standard pure virtual clone, isSameKindAs and className methods
//  * which are required for all Object subclasses.*/
//#define META_Object(library,name) \
//        virtual osg::Object* cloneType() const { return new name (); } \
//        virtual osg::Object* clone(const osg::CopyOp& copyop) const { return new name (*this,copyop); } \
//        virtual bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const name *>(obj)!=NULL; } \
//        virtual const char* libraryName() const { return #library; }\
//        virtual const char* className() const { return #name; }

/** Base class/standard interface for objects which require IO support,
    cloning and reference counting.
    Based on GOF Composite, Prototype and Template Method patterns.
*/
class Object // : public Referenced
{
%TypeHeaderCode
#include <osg/Object>
%End
    public:


//
//        inline explicit Object(bool threadSafeRefUnref):Referenced(threadSafeRefUnref),_dataVariance(UNSPECIFIED),_userDataContainer(0) {}
//
//        /** Copy constructor, optional CopyOp object can be used to control
//          * shallow vs deep copying of dynamic data.*/
//        Object(const Object&,const CopyOp& copyop=CopyOp::SHALLOW_COPY);
//
//        /** Clone the type of an object, with Object* return type.
//            Must be defined by derived classes.*/
//        virtual Object* cloneType() const = 0;
//
//        /** Clone an object, with Object* return type.
//            Must be defined by derived classes.*/
//        virtual Object* clone(const CopyOp&) const = 0;
//
//        virtual bool isSameKindAs(const Object*) const { return true; }
//
//        /** return the name of the object's library. Must be defined
//            by derived classes. The OpenSceneGraph convention is that the
//            namespace of a library is the same as the library name.*/
        virtual const char* libraryName() const = 0;
//
//        /** return the name of the object's class type. Must be defined
//            by derived classes.*/
        virtual const char* className() const = 0;
//
//
//        /** Set whether to use a mutex to ensure ref() and unref() are thread safe.*/
//        virtual void setThreadSafeRefUnref(bool threadSafe);
//
//        /** Set the name of object using C++ style string.*/
//        virtual void setName( const std::string& name ) { _name = name; }
//
//        /** Set the name of object using a C style string.*/
//        inline void setName( const char* name )
//        {
//            if (name) setName(std::string(name));
//            else setName(std::string());
//        }
//
//        /** Get the name of object.*/
//        inline const std::string& getName() const { return _name; }
//
//
//        enum DataVariance
//        {
//            DYNAMIC,
//            STATIC,
//            UNSPECIFIED
//        };
//
//        /** Set the data variance of this object.
//           * Can be set to either STATIC for values that do not change over the lifetime of the object,
//           * or DYNAMIC for values that vary over the lifetime of the object. The DataVariance value
//           * can be used by routines such as optimization codes that wish to share static data.
//           * UNSPECIFIED is used to specify that the DataVariance hasn't been set yet. */
//        inline void setDataVariance(DataVariance dv) { _dataVariance = dv; }
//
//        /** Get the data variance of this object.*/
//        inline DataVariance getDataVariance() const { return _dataVariance; }
//
//        /** Compute the DataVariance based on an assessment of callback etc.*/
//        virtual void computeDataVariance() {}
//
//
//        /** set the UserDataContainer object.*/
//        void setUserDataContainer(osg::UserDataContainer* udc);
//
//        /** get the UserDataContainer attached to this object.*/
//        osg::UserDataContainer* getUserDataContainer() { return _userDataContainer; }
//
//        /** get the const UserDataContainer attached to this object.*/
//        const osg::UserDataContainer* getUserDataContainer() const { return _userDataContainer; }
//
//        /** Convinience method that returns the UserDataContainer, and if one doesn't already exist creates and assigns
//         * a DefaultUserDataContainer to the Object and then return this new UserDataContainer.*/
//        osg::UserDataContainer* getOrCreateUserDataContainer();
//
//
//        /**
//         * Set user data, data must be subclassed from Referenced to allow
//         * automatic memory handling.  If your own data isn't directly
//         * subclassed from Referenced then create an adapter object
//         * which points to your own object and handles the memory addressing.
//         */
//        virtual void setUserData(Referenced* obj);
//
//        /** Get user data.*/
//        virtual Referenced* getUserData();
//
//        /** Get const user data.*/
//        virtual const Referenced* getUserData() const;
//
//
//
//        /** Convinience method that casts the named UserObject to osg::TemplateValueObject<T> and gets the value.
//          * To use this template method you need to include the osg/ValueObject header.*/
//        template<typename T>
//        bool getUserValue(const std::string& name, T& value) const;
//
//        /** Convinience method that creates the osg::TemplateValueObject<T> to store the
//          * specified value and adds it as a named UserObject.
//          * To use this template method you need to include the osg/ValueObject header. */
//        template<typename T>
//        void setUserValue(const std::string& name, const T& value);
//
//
//        /** Resize any per context GLObject buffers to specified size. */
//        virtual void resizeGLObjectBuffers(unsigned int /*maxSize*/) {}
//
//        /** If State is non-zero, this function releases any associated OpenGL objects for
//           * the specified graphics context. Otherwise, releases OpenGL objects
//           * for all graphics contexts. */
//        virtual void releaseGLObjects(osg::State* = 0) const {}
//
//
//    protected:
//
//        /** Object destructor. Note, is protected so that Objects cannot
//            be deleted other than by being dereferenced and the reference
//            count being zero (see osg::Referenced), preventing the deletion
//            of nodes which are still in use. This also means that
//            Nodes cannot be created on stack i.e Node node will not compile,
//            forcing all nodes to be created on the heap i.e Node* node
//            = new Node().*/
//
//        std::string _name;
//        DataVariance _dataVariance;
//
//        osg::UserDataContainer* _userDataContainer;
    private:

        /** Construct an object. Note Object is a pure virtual base class
            and therefore cannot be constructed on its own, only derived
            classes which override the clone and className methods are
            concrete classes and can be constructed.*/
        Object();

        /** Disallow any copy operator.*/
        Object& operator = (const Object&);
        virtual ~Object();
};

//template<typename T>
//T* clone(const T* t, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
//{
//    if (t)
//    {
//        osg::ref_ptr<osg::Object> obj = t->clone(copyop);
//
//        T* ptr = dynamic_cast<T*>(obj.get());
//        if (ptr)
//        {
//            obj.release();
//            return ptr;
//        }
//        else
//        {
//            OSG_WARN<<"Warning: osg::clone(const T*, osg::CopyOp&) cloned object not of type T, returning NULL."<<std::endl;
//            return 0;
//        }
//    }
//    else
//    {
//        OSG_WARN<<"Warning: osg::clone(const T*, osg::CopyOp&) passed null object to clone, returning NULL."<<std::endl;
//        return 0;
//    }
//}
//
//template<typename T>
//T* clone(const T* t, const std::string& name, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
//{
//    T* newObject = osg::clone(t, copyop);
//    if (newObject)
//    {
//        newObject->setName(name);
//        return newObject;
//    }
//    else
//    {
//        OSG_WARN<<"Warning: osg::clone(const T*, const std::string&, const osg::CopyOp) passed null object to clone, returning NULL."<<std::endl;
//        return 0;
//    }
//}
//
//template<typename T>
//T* cloneType(const T* t)
//{
//    if (t)
//    {
//        osg::ref_ptr<osg::Object> obj = t->cloneType();
//
//        T* ptr = dynamic_cast<T*>(obj.get());
//        if (ptr)
//        {
//            obj.release();
//            return ptr;
//        }
//        else
//        {
//            OSG_WARN<<"Warning: osg::cloneType(const T*) cloned object not of type T, returning NULL."<<std::endl;
//            return 0;
//        }
//    }
//    else
//    {
//        OSG_WARN<<"Warning: osg::cloneType(const T*) passed null object to clone, returning NULL."<<std::endl;
//        return 0;
//    }
//}
//

}; // namespace osg
