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
  namespace Features
  {
    /**
     * Metadata and schema information for feature data.
     */
    class FeatureProfile
    {
%TypeHeaderCode
#include <osgEarthFeatures/Feature>
%End
    public:
        FeatureProfile( const GeoExtent& extent );

        virtual ~FeatureProfile();

        /** Gets the spatial extents of the features in this profile. */
        const GeoExtent& getExtent() const;

        /** Gets the spatial reference system of feature shapes in this class. */
        const osgEarth::SpatialReference* getSRS() const;

        bool getTiled() const;
        void setTiled(bool tiled);

        int getFirstLevel() const;
        void setFirstLevel(int firstLevel );

        int getMaxLevel() const;
        void setMaxLevel(int maxLevel);

/*
        const Profile* getProfile() const;
        void setProfile( const Profile* profile );
*/
    };

/*
    struct AttributeValueUnion
    {
        std::string stringValue;
        double      doubleValue;
        int         intValue;
        bool        boolValue;

        //Whether the value is set.  A value of false means the value is effectively NULL
        bool        set;
    };

    enum AttributeType
    {
        ATTRTYPE_UNSPECIFIED,
        ATTRTYPE_STRING,
        ATTRTYPE_INT,
        ATTRTYPE_DOUBLE,
        ATTRTYPE_BOOL
    };

    struct AttributeValue
    {
        std::string getString() const;
        double getDouble( double defaultValue =0.0 ) const;
        int getInt( int defaultValue =0 ) const;
        bool getBool( bool defaultValue =false ) const;
    };

    typedef std::map<std::string, AttributeValue> AttributeTable;
*/
    typedef unsigned long FeatureID;

    /**
     * Wraps a FeatureID in a referenced object.
     */
    class RefFeatureID
    {
%TypeHeaderCode
#include <osgEarthFeatures/Feature>
using namespace osgEarth::Features;
%End
    public:
        RefFeatureID( osgEarth::Features::FeatureID fid );
        virtual ~RefFeatureID();

        operator osgEarth::Features::FeatureID () const;
    };

/*
    typedef std::map< std::string, AttributeType > FeatureSchema;
*/
    typedef std::list< osg::ref_ptr<Feature> > FeatureList;

    /**
     * Basic building block of vector feature data.
     */
    class Feature
    {
%TypeHeaderCode
#include <osgEarthFeatures/Feature>
using namespace osgEarth::Features;
%End
    public:
/*
        Feature( Geometry* geom, const osgEarth::SpatialReference* srs, const Style& style =Style(), osgEarth::Features::FeatureID fid =0 );
*/
        /** Copy contructor */
/*
        Feature( const Feature& rhs, const osg::CopyOp& copyop =osg::CopyOp::DEEP_COPY_ALL );
*/
        virtual ~Feature();

    public:

        /**
         * The unique ID of this feature (unique relative to its provider)
         */
        osgEarth::Features::FeatureID getFID() const;

        /**
         * The geometry in this feature.
         */
/*
        void setGeometry( Symbology::Geometry* geom );
        Symbology::Geometry* getGeometry();
        const Symbology::Geometry* getGeometry() const;
*/
        /**
         * The spatial reference of the geometry in this feature.
         */
        const osgEarth::SpatialReference* getSRS() const;
        void setSRS( const osgEarth::SpatialReference* srs );

        /**
         * Computes the bound of this feature in the specified SRS.
         */
/*
        bool getWorldBound( const osgEarth::SpatialReference* srs, osg::BoundingSphered& out_bound ) const;
*/
        /**
         * Gets a polytope, in world coordinates (proj or ECEF) that bounds the
         * geographic extents covered by this feature. This is useful for roughly
         * intersecting the feature with the terrain graph.
         */
/*
        bool getWorldBoundingPolytope( const osgEarth::SpatialReference* srs, osg::Polytope& out_polytope ) const;
*/

/*
        const AttributeTable& getAttrs() const;
*/
/*
        void set( const std::string& name, const std::string& value );
        void set( const std::string& name, double value );
        void set( const std::string& name, int value );
        void set( const std::string& name, bool value );
*/
        /** Sets the attribute to NULL */
/*
        void setNull( const std::string& name );
        void setNull( const std::string& name, AttributeType type );

        bool hasAttr( const std::string& name ) const;

        std::string getString( const std::string& name ) const;
        double getDouble( const std::string& name, double defaultValue =0.0 ) const;
        int getInt( const std::string& name, int defaultValue =0 ) const;
        bool getBool( const std::string& name, bool defaultValue =false ) const;
*/
        /**
         * Gets whether the attribute is set, meaning it is non-NULL
         */
/*
        bool isSet( const std::string& name ) const;
*/

        /** Embedded style. */
/*
        optional<Style>& style();
        const optional<Style>& style();
*/

        /** Geodetic interpolation method. */
/*
        optional<GeoInterpolation>& geoInterp();
        const optional<GeoInterpolation>& geoInterp() const;
*/
        /** Populates the variables of an expression with attribute values and evals the expression. */
/*
        double eval( NumericExpression& expr, FilterContext const* context=0 ) const;
*/
        /** Populates the variables of an expression with attribute values and evals the expression. */
/*
        const std::string& eval( StringExpression& expr, FilterContext const* context=0L ) const;
*/
    public:
        /** Gets a GeoJSON representation of this Feature */
/*
        std::string getGeoJSON();
*/
        /** Gets a FeatureList as a GeoJSON FeatureCollection */
/*
        static std::string featuresToGeoJSON( FeatureList& features);

    public:
        /**
         * Transforms this Feature to the given osgEarth::SpatialReference
         */
        void transform( const osgEarth::SpatialReference* srs );

    protected:
        Feature( osgEarth::Features::FeatureID fid =0 );

        void dirty();
    };
  };
};
