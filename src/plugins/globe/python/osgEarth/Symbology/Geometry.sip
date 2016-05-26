/* -*-c++-*-
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

    /** Options for the Geometry::buffer() operation. */
    class BufferParameters
    {
%TypeHeaderCode
#include <osgEarthSymbology/Geometry>
using namespace osgEarth::Symbology;
%End
    public:
        enum CapStyle  { CAP_DEFAULT, CAP_SQUARE, CAP_ROUND, CAP_FLAT };
        enum JoinStyle { JOIN_ROUND, JOIN_MITRE, JOIN_BEVEL};
        BufferParameters( CapStyle capStyle =CAP_DEFAULT, JoinStyle joinStyle = JOIN_ROUND, int cornerSegs =0, bool singleSided=false, bool leftSide=false );
        CapStyle  _capStyle;
        JoinStyle _joinStyle;
        int       _cornerSegs; // # of line segment making up a rounded corner
        bool      _singleSided; //Whether or not to do a single sided buffer
        bool      _leftSide;    //If doing a single sided buffer are we buffering to the left?  If false, buffer to the right
    };

    typedef std::vector<osg::Vec3d> Vec3dVector;

    /**
     * Baseline geometry class. All Geometry objects derive from this
     * class, even MultiGeometry.
     */
    class Geometry
    {
%TypeHeaderCode
#include <osgEarthSymbology/Geometry>
using namespace osgEarth::Symbology;
%End
    public:
        Geometry( int capacity =0 );
        Geometry( const Geometry& rhs );
//        Geometry( const osg::Vec3dVector* toCopy );

        /** Dtor - intentionally public */
        virtual ~Geometry();

    public:
        enum Type {
            TYPE_UNKNOWN,
            TYPE_POINTSET,
            TYPE_LINESTRING,
            TYPE_RING,
            TYPE_POLYGON,
            TYPE_MULTI
        };

        enum Orientation {
            ORIENTATION_CCW,
            ORIENTATION_CW,
            ORIENTATION_DEGENERATE
        };

        // true if osgEarth is compiled for buffering
        static bool hasBufferOperation();

    public:
        /**
         * Gets the total number of points in this geometry.
         */
        virtual int getTotalPointCount() const;

        /**
         * Gets the total number of geometry components
         */
        virtual unsigned getNumComponents() const;

        /**
         * Gets the total number of geometries; it is the total of all parts of all
         * components. Also can be seen as the number of Geometry objects that would
         * be returned by a full GeometryIterator.
         */
        virtual unsigned getNumGeometries() const;

        /**
         * Converts this geometry to another type. This function will return "this" if
         * the type is the same, and will return NULL if the conversion is impossible.
         */
        virtual Geometry* cloneAs( const Geometry::Type& newType ) const;

        /**
         * Gets the bounds of this geometry
         */
        virtual Bounds getBounds() const;

        /**
         * Whether the geometry is lines
         */
        bool isLinear() const;

        /**
         * Localizes this geometry relative to its centroid, and returns the localization
         * offset.
         */
        osg::Vec3d localize();

        /**
         * Reverses a call the localize(), given the same offset returned by that method.
         */
        void delocalize( const osg::Vec3d& offset );

        /**
         * Reorders the points in the geometry so that, if the last point was connected
         * to the first in a ring, they would be would in the specified direction.
         */
        virtual void rewind( Orientation ori );

        /**
         * Get the winding orientation of the geometry (if you consider the last point
         * to connect back to the first in a ring.)
         */
        Orientation getOrientation() const;

    public:
        virtual Type getType() const;
        virtual Type getComponentType() const;
        virtual bool isValid() const;

        virtual Geometry* clone() const;

        void push_back(const osg::Vec3d& v );
        void push_back(double x, double y);
        void push_back(double x, double y, double z);

    protected:
    };

    /**
     * An unordered collections of points.
     */
    class PointSet : Geometry
    {
%TypeHeaderCode
#include <osgEarthSymbology/Geometry>
using namespace osgEarth::Symbology;
%End
    public:
        PointSet( int capacity =0 );
        PointSet( const PointSet& rhs );

        /** Dtor */
        virtual ~PointSet();

    public:
        virtual Type getType() const;
    };

    /**
     * An ordered set of points forming a single contiguous line string.
     */
    class LineString : Geometry
    {
%TypeHeaderCode
#include <osgEarthSymbology/Geometry>
using namespace osgEarth::Symbology;
%End
    public:
        LineString( int capacity =0 );
        LineString( const LineString& rhs );

        /** Dtor */
        virtual ~LineString();

        double getLength() const;
        bool getSegment(double length, osg::Vec3d& start, osg::Vec3d& end);

    public:
        virtual Type getType() const;
        virtual bool isValid() const;
    };

    /**
     * A Ring is a closed region. It is open (the first and last
     * points are not the same). It has an orientation, i.e. it is either
     * wound clockwise or counter-clockwise.
     */
    class Ring : Geometry
    {
%TypeHeaderCode
#include <osgEarthSymbology/Geometry>
using namespace osgEarth::Symbology;
%End
    public:
        Ring( int capacity =0 );
        Ring( const Ring& ring );

        /** Dtor */
        virtual ~Ring();

        // override
        virtual Geometry* cloneAs( const Geometry::Type& newType ) const;

        // tests whether the point falls withing the ring
        virtual bool contains2D( double x, double y ) const;

        // gets the signed area of a part that is known to be open.
        virtual double getSignedArea2D() const;

        // ensures that the first and last points are not idential.
        virtual void open();

        // opens and winds the ring in the specified direction
        virtual void rewind( Orientation ori );

    public:
        virtual Type getType() const;
        virtual bool isValid() const;
    };

    /**
     * A Polygon is a geometry that consists of one outer boundary Ring, and
     * zero or more inner "hole" rings. The boundary ring is would CCW, and the
     * inner "holes" are wound CW.
     */
    class Polygon : Ring
    {
%TypeHeaderCode
#include <osgEarthSymbology/Geometry>
using namespace osgEarth::Symbology;
%End
    public:
        Polygon( int capacity =0 );
        Polygon( const Polygon& rhs );

        /** Dtor */
        virtual ~Polygon();

    public:
        virtual Type getType() const;
        virtual int getTotalPointCount() const;

        virtual unsigned getNumGeometries() const;

        // tests whether the point falls withing the polygon (but not its holes)
        virtual bool contains2D( double x, double y ) const;

        virtual void open();
    };

    /**
     * A collection of multiple geometries (aka, a "multi-part" geometry).
     */
    class MultiGeometry : Geometry
    {
%TypeHeaderCode
#include <osgEarthSymbology/Geometry>
using namespace osgEarth::Symbology;
%End
    public:
        MultiGeometry();
        MultiGeometry( const MultiGeometry& rhs );

        /** Dtor */
        virtual ~MultiGeometry();

    public:
        virtual Type getType() const;
        virtual Type getComponentType() const;
        virtual int getTotalPointCount() const;
        virtual unsigned getNumComponents() const;

        virtual unsigned getNumGeometries() const;

        // override
        virtual Geometry* cloneAs( const Geometry::Type& newType ) const;
        virtual bool isValid() const;
        virtual Bounds getBounds() const;
        virtual void rewind( Orientation ori );

    public:

        Geometry* add( Geometry* geom );
    };

    /**
     * Iterates over a Geometry object, returning each component Geometry
     * in turn. The iterator automatically traverses MultiGeometry objects,
     * returning their components. The interator NEVER returns an actual
     * MultiGeometry object.
     */
    class GeometryIterator
    {
%TypeHeaderCode
#include <osgEarthSymbology/Geometry>
using namespace osgEarth::Symbology;
%End
    public:
        /**
         * New iterator.
         *
         * traversePolyHoles: set to TRUE to return Polygons AND Polygon hole geometries;
         *                    set to FALSE to only return Polygon (outer ring).
         *
         * traverseMultiGeometry: set to TRUE to return MG children, but never an MG itself;
         *                        set to FALSE to return MGs and NOT their children.
         */
        GeometryIterator(
            Geometry* geom,
            bool      traversePolygonHoles  =true );

        bool hasMore() const;
        Geometry* next();
    };

    class ConstGeometryIterator
    {
%TypeHeaderCode
#include <osgEarthSymbology/Geometry>
using namespace osgEarth::Symbology;
%End
    public:
        /**
         * New iterator.
         *
         * traversePolyHoles: set to TRUE to return Polygons AND Polygon hole geometries;
         *                    set to FALSE to only return Polygon (outer ring).
         *
         * traverseMultiGeometry: set to TRUE to return MG children, but never an MG itself;
         *                        set to FALSE to return MGs and NOT their children.
         */
        ConstGeometryIterator(
            const Geometry* geom,
            bool            traversePolygonHoles  =true );

        bool hasMore() const;
        const Geometry* next();
    };

    typedef std::pair<osg::Vec3d, osg::Vec3d> Segment;

    class ConstSegmentIterator
    {
%TypeHeaderCode
#include <osgEarthSymbology/Geometry>
using namespace osgEarth::Symbology;
%End
    public:
        ConstSegmentIterator( const Geometry* verts, bool forceClosedLoop =false );
        bool hasMore() const;
    };

