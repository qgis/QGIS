class QgsGlobeFeatureUtils
{
%TypeHeaderCode
#include <qgsglobefeatureutils.h>
%End
  public:
        static inline QgsPointV2 qgsPointFromPoint( const osg::Vec3d& pt );
        static inline osg::Vec3d pointFromQgsPoint( const QgsPointV2& pt );
        static inline osgEarth::Features::LineString* lineStringFromQgsLineString( const QgsLineStringV2* lineString );
        static inline osgEarth::Features::Polygon* polygonFromQgsPolygon( const QgsPolygonV2* polygon );
        static inline osgEarth::Features::Geometry* geometryFromQgsGeometry( const QgsGeometry& geom );
        static osgEarth::Features::Feature* featureFromQgsFeature( QgsVectorLayer* layer, QgsFeature& feat );
        static void setFeatureField( osgEarth::Features::Feature* feature, const QgsField& field, const QVariant& value );
        static osgEarth::Features::FeatureSchema schemaForFields( const QgsFields& fields );
};

