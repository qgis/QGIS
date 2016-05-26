/* -*-c++-*-
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
    /**
     * A georeferenced 3D point.
     */
    class GeoPoint
    {
%TypeHeaderCode
#include <osgEarth/GeoData>
using namespace osgEarth;
using namespace osg;
%End
    public:

        /**
         * Constructs a GeoPoint.
         */
        GeoPoint(
            const osgEarth::SpatialReference* srs,
            double x,
            double y,
            double z,
            const AltitudeMode& mode );

        /**
         * Constructs a GeoPoint with X and Y coordinates. The Z defaults
         * to zero with an ALTMODE_RELATIVE altitude mode (i.e., 0 meters
         * above the terrain).
         */
        GeoPoint(
            const osgEarth::SpatialReference* srs,
            double x,
            double y );

        /**
         * Constructs a GeoPoint from a vec3.
         */
        GeoPoint(
            const osgEarth::SpatialReference* srs,
            const osg::Vec3d&       xyz,
            const AltitudeMode&     mode );

        /**
         * Constructs a new GeoPoint by transforming an existing GeoPoint into
         * the specified spatial reference.
         */
        GeoPoint(
            const osgEarth::SpatialReference* srs,
            const GeoPoint&         rhs );

        /**
         * Copy constructor
         */
        GeoPoint(const GeoPoint& rhs);

        /**
         * Constructs an empty (and invalid) geopoint.
         */
        GeoPoint();

        /**
         * Constructs a geopoint from serialization
         */
        GeoPoint( const osgEarth::Config& conf, const osgEarth::SpatialReference* srs =0 );

        /** Dtor */
        virtual ~GeoPoint();

        /**
         * Sets the SRS and coords
         */
        void set(
            const osgEarth::SpatialReference* srs,
            const osg::Vec3d&       xyz,
            const AltitudeMode&     mode );

        void set(
            const osgEarth::SpatialReference* srs,
            double                  x,
            double                  y,
            double                  z,
            const AltitudeMode&     mode );

        // component getter/setters

        double  x() const;

        double  y() const;

        double  z() const;

        double  alt() const;

        const osg::Vec3d& vec3d() const;

        const osgEarth::SpatialReference* getSRS() const;

        const AltitudeMode& altitudeMode() const;

        /**
         * Returns a copy of this geopoint transformed into another SRS.
         */
        GeoPoint transform(const osgEarth::SpatialReference* outSRS) const;

        /**
         * Transforms this geopoint into another SRS.
         */
        bool transform(const osgEarth::SpatialReference* outSRS, GeoPoint& output) const;

        /**
         * Transforms this geopoint's Z coordinate (in place)
         */
        bool transformZ(const AltitudeMode& altMode, const osgEarth::TerrainHeightProvider* t );

        /**
         * Transforms and returns the geopoints Z coordinate.
         */
/*
        bool transformZ(const AltitudeMode& altMode, const osgEarth::TerrainHeightProvider* t, double& out_z) const;
*/
        /**
         * Transforms this geopoint's Z to be absolute w.r.t. the vertical datum
         */
        bool makeAbsolute(const osgEarth::TerrainHeightProvider* t);

        /**
         * Transforms this geopoint's Z to be terrain-relative.
         */
        bool makeRelative(const osgEarth::TerrainHeightProvider* t);

        /**
         * Transforms this GeoPoint to geographic (lat/long) coords in place.
         */
        bool makeGeographic();

        /**
         * Outputs world coordinates corresponding to this point. If the point
         * is ALTMODE_RELATIVE, this will fail because there's no way to resolve
         * the actual Z coordinate. Use the variant of toWorld that takes a
         * Terrain* instead.
         */
        bool toWorld( osg::Vec3d& out_world ) const;

        /**
         * Outputs world coordinates corresponding to this point, passing in a Terrain
         * object that will be used if the point needs to be converted to absolute
         * altitude
         */
        bool toWorld( osg::Vec3d& out_world, const osgEarth::TerrainHeightProvider* terrain ) const;

        /**
         * Converts world coordinates into a geopoint
         */
        bool fromWorld(const osgEarth::SpatialReference* srs, const osg::Vec3d& world);

        /**
         * geopoint into absolute world coords.
         */
/*
        bool createLocalToWorld( Matrixd& out_local2world ) const;
*/
        /**
         * Outputs a matrix that will transform absolute world coordiantes so they are
         * localized into a local tangent place around this geopoint.
         */
/*
        bool createWorldToLocal( Matrixd& out_world2local ) const;
*/

        bool operator == (const GeoPoint& rhs) const;
        bool operator != (const GeoPoint& rhs) const;
        bool isValid() const;

        osgEarth::Config getConfig() const;

    public:
        static GeoPoint INVALID;
    };


    /**
     * A simple circular bounding area consiting of a GeoPoint and a linear radius.
     */
     class GeoCircle
     {
%TypeHeaderCode
#include <osgEarth/GeoData>
using namespace osgEarth;
%End
     public:
         /** Construct an INVALID GeoCircle */
        GeoCircle();

        /** Copy another GoeCircle */
        GeoCircle(const GeoCircle& rhs);

        /** Construct a new GeoCircle */
        GeoCircle(
             const GeoPoint& center,
             double          radius );

        virtual ~GeoCircle();

        /** The center point of the circle */
        const GeoPoint& getCenter() const;
        void setCenter( const GeoPoint& value );

        /** Circle's radius, in linear map units (or meters for a geographic SRS) */
        double getRadius() const;
        void setRadius( double value );

        /** SRS of the center point */
        const osgEarth::SpatialReference* getSRS() const;

        /** Equality test */
        bool operator == ( const GeoCircle& rhs ) const;

        /** Inequality test */
        bool operator != ( const GeoCircle& rhs ) const;

        /** Validity test */
        bool isValid() const;

        /** Transform the GeoCircle to another SRS */
        GeoCircle transform( const osgEarth::SpatialReference* srs ) const;

        /** Transform the GeoCircle to another SRS */
        bool transform( const osgEarth::SpatialReference* srs, GeoCircle& out_circle ) const;

        /** Does this GeoCircle intersect another? */
        bool intersects( const GeoCircle& rhs ) const;

     public:

         static GeoCircle INVALID;
     };


    /**
     * An axis-aligned geospatial extent. A bounding box that is aligned with a
     * spatial reference's coordinate system.
     */
    class GeoExtent
    {
%TypeHeaderCode
#include <osgEarth/GeoData>
using namespace osgEarth;
%End
    public:
        /** Default ctor creates an "invalid" extent */
        GeoExtent();

        /** Contructs a valid extent */
        GeoExtent(
            const osgEarth::SpatialReference* srs,
            double west, double south,
            double east, double north );

        /** Contructs an invalid extent that you can grow with the expandToInclude method */
        GeoExtent( const osgEarth::SpatialReference* srs );

        /** Copy ctor */
        GeoExtent( const GeoExtent& rhs );

        /** Create from Bounds object */
        GeoExtent( const osgEarth::SpatialReference* srs, const Bounds& bounds );

        /** Dtor */
        virtual ~GeoExtent();

        bool operator == ( const GeoExtent& rhs ) const;
        bool operator != ( const GeoExtent& rhs ) const;

        /** Gets the spatial reference system underlying this extent. */
        const osgEarth::SpatialReference* getSRS() const;

        double west() const;
        double east() const;
        double south() const;
        double north() const;

        double xMin() const;
        double xMax() const;
        double yMin() const;
        double yMax() const;

        double width() const;
        double height() const;

        /**
         * Gets the centroid of the bounds
         */
/*
        bool getCentroid( double& out_x, double& out_y ) const;
*/
        osg::Vec3d getCentroid() const;

        /**
         * Returns true is that extent is in a Geographic (lat/long) SRS that spans
         * the antimedirian (180 degrees east/west).
         */
        bool crossesAntimeridian() const;

        /**
         * Returns the raw bounds in a single function call
         */
        void getBounds(double &xmin, double &ymin, double &xmax, double &ymax) const;

        /** True if this object defines a real, valid extent with positive area */
        bool isValid() const;
        bool defined() const;
        bool isInvalid() const;

        /**
         * If this extent crosses the international date line, populates two extents, one for
         * each side, and returns true. Otherwise returns false and leaves the reference
         * parameters untouched.
         */
        bool splitAcrossAntimeridian( GeoExtent& first, GeoExtent& second ) const;

        /**
         * Returns this extent transformed into another spatial reference.
         *
         * NOTE! It is possible that the target SRS will not be able to accomadate the
         * extents of the source SRS. (For example, transforming a full WGS84 extent
         * to Mercator will resultin an error since Mercator does not cover the entire
         * globe.) Consider using Profile:clampAndTransformExtent() instead of using
         * this method directly.
         */
        GeoExtent transform( const osgEarth::SpatialReference* to_srs ) const;

        /**
         * Same as transform(srs) but puts the result in the output extent
         */
/*
        bool transform( const osgEarth::SpatialReference* to_srs, GeoExtent& output ) const;
*/
        /**
         * Returns true if the specified point falls within the bounds of the extent.
         *
         * @param x, y
         *      Coordinates to test
         * @param xy_srs
         *      SRS of input x and y coordinates; if null, the method assumes x and y
         *      are in the same SRS as this object.
         */
        bool contains(double x, double y, const osgEarth::SpatialReference* srs =0) const;

        bool contains(const osg::Vec3d& xy, const osgEarth::SpatialReference* srs =0) const;

        /**
         * Returns true if the point falls within this extent.
         */
        bool contains( const GeoPoint& rhs ) const;

        /**
         * Returns true if this extent fully contains another extent.
         */
        bool contains( const GeoExtent& rhs ) const;

        /**
         * Returns true if this extent fully contains the target bounds.
         */
        bool contains( const Bounds& rhs ) const;

        /**
         * Returns TRUE if this extent intersects another extent.
         * @param[in ] rhs      Extent against which to perform intersect test
         * @param[in ] checkSRS If false, assume the extents have the same SRS (don't check).
         */
        bool intersects( const GeoExtent& rhs, bool checkSRS =true ) const;

        /**
         * Copy of the anonymous bounding box
         */
        Bounds bounds() const;

        /**
         * Gets a geo circle bounding this extent.
         */
        const GeoCircle& getBoundingGeoCircle() const;

        /**
         * Grow this extent to include the specified point (which is assumed to be
         * in the extent's SRS.
         */
        void expandToInclude( double x, double y );
        void expandToInclude( const osg::Vec3d& v );
        void expandToInclude( const Bounds& bounds );

        /**
         * Expand this extent to include the bounds of another extent.
         */
        bool expandToInclude( const GeoExtent& rhs );

        /**
         * Intersect this extent with another extent in the same SRS and return the
         * result.
         */
        GeoExtent intersectionSameSRS( const GeoExtent& rhs ) const; //const Bounds& rhs ) const;

        /**
         * Returns a human-readable string containing the extent data (without the SRS)
         */
/*
        std::string toString() const;
*/
        /**
         *Inflates this GeoExtent by the given ratios
         */
        void scale(double x_scale, double y_scale);

        /**
         * Expands the extent by x and y.
         */
        void expand( double x, double y );

        /**
         *Gets the area of this GeoExtent
         */
        double area() const;


        /**
         * Normalize the longitude values in this GeoExtent
         */
        void normalize();

        /**
         * Given a longitude value, normalize it so that it exists within the GeoExtents longitude frame.
         */
        double normalizeLongitude( double longitude ) const;

    public:
        static GeoExtent INVALID;

    };

    /**
     * A geospatial area with tile data LOD extents
     */
    class DataExtent : public GeoExtent
    {
%TypeHeaderCode
#include <osgEarth/GeoData>
using namespace osgEarth;
%End
    public:
        DataExtent(const GeoExtent& extent);
        DataExtent(const GeoExtent& extent, unsigned minLevel );
        DataExtent(const GeoExtent& extent, unsigned minLevel, unsigned maxLevel);

        /** Dtor */
        virtual ~DataExtent();

        /** The minimum LOD of the extent */
/*
        const optional<unsigned>& minLevel() const;
*/
        /** The maximum LOD of the extent */
/*
        const optional<unsigned>& maxLevel() const;
*/
    };

    typedef std::vector< DataExtent > DataExtentList;
