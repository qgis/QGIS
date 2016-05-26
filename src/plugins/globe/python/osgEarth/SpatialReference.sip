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

namespace osgEarth
{

    /**
     * SpatialReference holds information describing the reference ellipsoid/datum
     * and the projection of geospatial data.
     */
    class SpatialReference
    {
%TypeHeaderCode
#include <osgEarth/SpatialReference>
%End
    public:
        /**
         * Creates an SRS from two intialization strings; the first for the horizontal datum and
         * the second for the vertical datum. If you omit the vertical datum, it will default to
         * the geodetic datum for the ellipsoid.
         */
/*
        static SpatialReference* create( const std::string& init, const std::string& vinit ="" );
        static SpatialReference* get   ( const std::string& init, const std::string& vinit ="" );
*/
        /**
         * Attempts to create a spatial reference def from a pre-existing CSN, returning
         * NULL if there is not enough information.
         */
/*
        static SpatialReference* create( osg::CoordinateSystemNode* csn );
*/
        /**
         * Creates an SRS around a pre-existing OGR spatial reference handle. The new
         * SpatialReference object takes ownership of the handle.
         *
         * @param xferOwnership
         *      If true, the SpatialReference object is responsible for releasing the handle
         *      upon destruction.
         */
        static osgEarth::SpatialReference* createFromHandle( void* ogrHandle, bool xferOwnership =false );


    public: // Basic transformations.

        /**
         * Transform a single point from this SRS to another SRS.
         * Returns true if the transformation succeeded.
         */
/*
        virtual bool transform(
            const osg::Vec3d&       input,
            const osgEarth::SpatialReference* outputSRS,
            osg::Vec3d&             output) const;
*/
        /**
         * Transform a collection of points from this SRS to another SRS.
         * Returns true if ALL transforms succeeded, false if at least one failed.
         */
/*
        virtual bool transform(
            std::vector<osg::Vec3d>& input,
            const osgEarth::SpatialReference*  outputSRS ) const;
/*

        /**
         * Transform a 2D point directly. (Convenience function)
         */
        bool transform2D(
            double                  x,
            double                  y,
            const osgEarth::SpatialReference* outputSRS,
            double&                 out_x,
            double&                 out_y ) const;


    public: // Units transformations.

        /**
         * Transforms a distance from the base units of this SRS to the base units of
         * another. If one of the SRS's is geographic (i.e. has angular units), the
         * conversion will assume that the corresponding distance is measured at the
         * equator.
         */
        double transformUnits(
            double                  distance,
            const osgEarth::SpatialReference* outputSRS ) const;


    public: // World transformations.

        /**
         * Transforms a point from this SRS into "world" coordinates. This normalizes
         * the Z coordinate (according to the vertical datum) and converts to ECEF
         * if necessary.
         */
        bool transformToWorld(
            const osg::Vec3d& input,
            osg::Vec3d&       out_world ) const;

        /**
         * Transforms a point from the "world" coordinate system into this spatial
         * reference.
         * @param world
         *      World point to transform
         * @param out_local
         *      Output coords in local (SRS) coords
         * @param worldIsGeocentric
         *      Whether the incoming world coordinates are ECEF/geocentric coords
         * @param out_geodeticZ
         *      (optional) Outputs the geodetic (HAE) Z if applicable
         */
        bool transformFromWorld(
            const osg::Vec3d& world,
            osg::Vec3d&       out_local,
            double*           out_geodeticZ =0 ) const;

    public: // extent transformations.

        /**
         * Transforms a spatial extent to another SRS. The transformed extent will
         * actually be the minimum bounding axis-aligned rectangle that would hold
         * the source extent.
         */
        virtual bool transformExtentToMBR(
            const osgEarth::SpatialReference* to_srs,
            double&                 in_out_xmin,
            double&                 in_out_ymin,
            double&                 in_out_xmax,
            double&                 in_out_ymax ) const;

        virtual bool transformExtentPoints(
            const osgEarth::SpatialReference* to_srs,
            double in_xmin, double in_ymin,
            double in_xmax, double in_ymax,
            double* x, double* y,
            unsigned numx, unsigned numy ) const;


    public: // properties

/*
        typedef std::pair<std::string,std::string> Key;
*/
        /** True if this is a geographic SRS (lat/long/msl) */
        virtual bool isGeographic() const;

        /** True if this is a geodetic SRS (lat/long/hae) */
        virtual bool isGeodetic() const;

        /** True if this is a projected SRS (i.e. local coordinate system) */
        virtual bool isProjected() const;

        /** True if this is an ECEF system (geocentric/meters) */
        virtual bool isECEF() const;

        /** Tests whether this SRS represents a Mercator projection. */
        bool isMercator() const;

        /** Tests whether this SRS represents a Spherical Mercator pseudo-projection. */
        bool isSphericalMercator() const;

        /** Tests whether this SRS represents a polar sterographic projection. */
        bool isNorthPolar() const;
        bool isSouthPolar() const;

        /** Tests whether this SRS is user-defined; i.e. whether it is other than a
            well-known SRS. (i.e. whether the SRS is unsupported by GDAL) */
        virtual bool isUserDefined() const;

        /** Tests whether coordinates in this SRS form a contiguous space. A non-contiguous
            SRS is one in which adjacent coordinates may not necessarily represent
            adjacent map locations. */
        virtual bool isContiguous() const;

        /** Tests whether this SRS is a Unified Cube projection (osgEarth-internal) */
        virtual bool isCube() const;

        /** Tests whether this SRS is a Local Tangent Plane projection (osgEarth-internal) */
        virtual bool isLTP() const;

        /** Whether this is a geographic plate carre SRS */
        virtual bool isPlateCarre() const;

        /** Gets the readable name of this SRS. */
/*
        const std::string& getName() const;
*/
        /** Gets the underlying reference ellipsoid of this SRS */
/*
        const osg::EllipsoidModel* getEllipsoid() const;
*/
        /** Gets the WKT string */
/*
        const std::string& getWKT() const;
*/
        /** Gets the initialization type (PROJ4, WKT, etc.) */
/*
        const std::string& getInitType() const;
*/
        /** Gets the initialization key. */
/*
        const Key& getKey() const;
*/
        /** Gets the initialization string for the horizontal datum */
/*
        const std::string& getHorizInitString() const;
*/
        /** Gets the initialization string for the vertical datum */
/*
        const std::string& getVertInitString() const;
*/
        /** Gets the datum identifier of this SRS (or empty string if not available) */
/*
        const std::string& getDatumName() const;
*/
        /** Gets the base units of data in this SRS */
/*
        const Units& getUnits() const;
*/
        /** Whether the two SRS are completely equivalent. */
        virtual bool isEquivalentTo( const osgEarth::SpatialReference* rhs ) const;

        /** Whether the two SRS are horizonally equivalent (ignoring the vertical datums) */
        bool isHorizEquivalentTo( const osgEarth::SpatialReference* rhs ) const;

        /** Whether this SRS has the same vertical datum as another. */
        bool isVertEquivalentTo( const osgEarth::SpatialReference* rhs ) const;

        /** Gets a reference to this SRS's underlying geographic SRS. */
        const osgEarth::SpatialReference* getGeographicSRS() const;

        /** Gets a reference to this SRS's underlying geodetic SRS. This is the same as the
            geographic SRS [see getGeographicSRS()] but with a geodetic vertical datum (in
            which Z is expressed as height above the geodetic ellipsoid). */
        const osgEarth::SpatialReference* getGeodeticSRS() const;

        /** Gets the ECEF reference system associated with this SRS' ellipsoid. */
        const osgEarth::SpatialReference* getECEF() const;

        /** Gets the vertical datum. If null, this SRS uses a default geodetic vertical datum */
/*
        const VerticalDatum* getVerticalDatum() const;
*/
        /** Creates a localizer matrix based on a point in this SRS. */
/*
        bool createLocalToWorld( const osg::Vec3d& point, osg::Matrixd& out_local2world ) const;
*/
        /** Create a de-localizer matrix based on a point in this SRS. */
/*
        bool createWorldToLocal( const osg::Vec3d& point, osg::Matrix& out_world2local ) const;
*/
        /** Creates and returns a local trangent plane SRS at the given reference location.
            The reference location is expressed in this object's SRS, but it tangent to
            the globe at getGeographicSRS(). LTP units are in meters. */
        const osgEarth::SpatialReference* createTangentPlaneSRS( const osg::Vec3d& refPos ) const;

        /** Creates a transverse mercator projection centered at the specified longitude. */
/*
        const osgEarth::SpatialReference* createTransMercFromLongitude( const Angular& lon ) const;
*/
        /** Creates a UTM (universal transverse mercator) projection in the UTM zone
            containing the specified longitude. NOTE: this is slightly faster than using
            basic tmerc (transverse mercator) above. */
/*
        const osgEarth::SpatialReference* createUTMFromLonLat( const Angle& lon, const Angle& lat ) const;
*/
        /** Creates a copy of this SRS, but flags the new SRS so that it will operate in
            Plate Carre mode for the purposes of world coordinate conversion. The SRS is
            otherwise mathematically equivalent to its vanilla counterpart. */
        const osgEarth::SpatialReference* createPlateCarreGeographicSRS() const;

        /** Creates a new CSN based on this spatial reference. */
/*
        osg::CoordinateSystemNode* createCoordinateSystemNode() const;
*/
        /** Populates the provided CSN with information from this SRS. */
/*
        bool populateCoordinateSystemNode( osg::CoordinateSystemNode* csn ) const;
*/
        /**
         * Creates a new Locator object based on this spatial reference.
         *
         * @param xmin, ymin, xmax, ymax
         *      Extents of the tile for which to create a locator. These should
         *      be in degrees for a geographic/geocentric scene.
         * @param plate_carre
         *      Set this to true for the special case in which you are using a
         *      geographic SRS with a PROJECTED map (like flat-earth lat/long).
         */
/*
        virtual GeoLocator* createLocator(
            double xmin, double ymin, double xmax, double ymax,
            bool plate_carre =false ) const;
*/


    protected:
        virtual ~SpatialReference();

    protected:

/*
       osgEarth:: SpatialReference( void* handle, const std::string& type );
*/
        SpatialReference( void* handle, bool ownsHandle =true );
        void init();
    };
};
