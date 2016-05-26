/* -*-c++-*- */
/* osgEarth - Dynamic map generation toolkit for OpenSceneGraph
 * Copyright 2008-2013 Pelican Mapping
 * http://osgearth.org
 *
 * osgEarth is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

namespace osgEarth
{
    /**
     * A tracking revision. See "class Revisioned" below for details.
     */
    struct Revision // header-only; no export
    {
%TypeHeaderCode
#include <osgEarth/Revisioning>
%End
        Revision();
        Revision(int init);
        void reset();
        operator int();
        // int operator++();
        // int operator--();
    };


    /**
     * Base class for a revisioned object. A Revisioned object is one that keeps track
     * of its state with a version number. Other objects can then hold a Revision object
     * (see above) and use it to see if they are up to date with this object, thereby
     * enabling passive data model synchronization.
     *
     * A Revisioned object is useful in the case where one or many objects want to
     * track the state of a single object. If you want the opposite - where one
     * object tracks the state of a multitude (or a hierarchy) of data model objects,
     * use TrackDirty instead.
     */
    class Revisioned /* no export; header-only */
    {
%TypeHeaderCode
#include <osgEarth/Revisioning>
%End
    public:
        /**
         * Marks this object as dirty by increasing the revision number.
         * If the object has parents, it will mark those dirty as well.
         */
        void dirty();

        /**
         * Synchronizes the external revision number with this revision, effectively
         * bringing the external object up to date.
         */
        virtual void sync( osgEarth::Revision& externalRevision ) const;

        /**
         * Returns true if the external object is at a different revision that this object.
         */
        bool outOfSyncWith( const osgEarth::Revision& externalRevision) const;

        /**
         * Returns true if the external object is at the same revision as this object.
         */
        virtual bool inSyncWith( const osgEarth::Revision& externalRevision ) const;

    protected:
//
//        /** Marks this object as always being dirty (i.e. inSyncWith() will always return false) */
//        void setAlwaysDirty( bool value )
//        {
//            _alwaysDirty = value;
//        }
//
    private:

        Revisioned();
        /** Dtor */
        ~Revisioned();
    };


//    /**
//     * A TrackedMutable object is an object that can mark itself, and optionally
//     * its dependent parents, dirty. This is analagous to OSG's "dirtyBound"
//     * concept that can propagate up through a scene graph.
//     *
//     * The use case is when you want to detect changes somewhere in an object
//     * group or hierarchy, and then update something based on the modular
//     * changes. This is the opposite of the Revisioned pattern (in which many
//     * objects are tracking the state of one.)
//     *
//     * Note: objects always start out dirty.
//     */
//    class OSGEARTH_EXPORT DirtyNotifier
//    {
//    public:
//        DirtyNotifier();
//        virtual ~DirtyNotifier() { }
//
//        /**
//         * Marks the object dirty and notifies parents
//         */
//        virtual void setDirty();
//
//        /**
//         * Marks the object not dirty.
//         */
//        virtual void resetDirty() { _counter->_count = 0; }
//
//        /**
//         * Is this object dirty?
//         */
//        virtual bool isDirty() const { return _counter->_count > 0; }
//
//        /**
//         * Adds a dependent parent that will be dirtied if this object if dirtied.
//         */
//        virtual void addParent( DirtyNotifier* parent );
//
//        /**
//         * Removes a dependent parent previously added by addParent.
//         */
//        virtual void removeParent( DirtyNotifier* parent );
//
//    private:
//        // this pattern is used to we can track parents with an observer pointer.
//        struct DirtyCounter : public osg::Referenced
//        {
//            DirtyCounter(DirtyNotifier* owner) : _owner(owner), _count(1) { }
//            DirtyNotifier* _owner;
//            int            _count;
//            friend class DirtyNotifer;
//        };
//        osg::ref_ptr<DirtyCounter>                     _counter;
//        std::vector< osg::observer_ptr<DirtyCounter> > _parents;
//    };
//
//
//    /**
//     * A simple but thread-safe "dirty" object that support only one client.
//     */
//    template<typename T>
//    struct SimpleMutable
//    {
//        SimpleMutable() : _dirty(1) { }
//        SimpleMutable(const T& value) : _value(value), _dirty(1) { }
//        operator const T&() const { return _value; }
//        const T* operator ->() const { return &_value; }
//        SimpleMutable& operator = (const T& value) { _value = value; _dirty.exchange(1); return *this; }
//        bool changed(bool reset=true) const { unsigned r = reset? _dirty.exchange(0) : (unsigned)_dirty; return r==1; }
//        void clean() { _dirty.exchange(0); }
//    private:
//        T                           _value;
//        mutable OpenThreads::Atomic _dirty;
//    };

};
