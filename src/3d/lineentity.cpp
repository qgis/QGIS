#include "lineentity.h"

#include "abstract3dsymbol.h"
#include "polygongeometry.h"
#include "map3d.h"
#include "terraingenerator.h"
#include "utils.h"

#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DRender/QGeometryRenderer>

#include "qgsvectorlayer.h"
#include "qgsmultipolygon.h"
#include "qgsgeos.h"


LineEntity::LineEntity( const Map3D &map, QgsVectorLayer *layer, const Line3DSymbol &symbol, Qt3DCore::QNode *parent )
  : Qt3DCore::QEntity( parent )
{
  QgsPointXY origin( map.originX, map.originY );

  Qt3DExtras::QPhongMaterial *material = new Qt3DExtras::QPhongMaterial;
  material->setAmbient( symbol.material.ambient() );
  material->setDiffuse( symbol.material.diffuse() );
  material->setSpecular( symbol.material.specular() );
  material->setShininess( symbol.material.shininess() );
  addComponent( material );

  // TODO: configurable
  int nSegments = 4;
  QgsGeometry::EndCapStyle endCapStyle = QgsGeometry::CapRound;
  QgsGeometry::JoinStyle joinStyle = QgsGeometry::JoinStyleRound;
  double mitreLimit = 0;

  QList<QgsPolygonV2 *> polygons;
  QgsFeature f;
  QgsFeatureRequest request;
  request.setDestinationCrs( map.crs );
  QgsFeatureIterator fi = layer->getFeatures( request );
  while ( fi.nextFeature( f ) )
  {
    if ( f.geometry().isNull() )
      continue;

    QgsAbstractGeometry *g = f.geometry().geometry();

    QgsGeos engine( g );
    QgsAbstractGeometry *buffered = engine.buffer( symbol.width / 2., nSegments, endCapStyle, joinStyle, mitreLimit ); // factory

    if ( QgsWkbTypes::flatType( buffered->wkbType() ) == QgsWkbTypes::Polygon )
    {
      QgsPolygonV2 *polyBuffered = static_cast<QgsPolygonV2 *>( buffered );
      Utils::clampAltitudes( polyBuffered, symbol.altClamping, symbol.altBinding, symbol.height, map );
      polygons.append( polyBuffered );
    }
    else if ( QgsWkbTypes::flatType( buffered->wkbType() ) == QgsWkbTypes::MultiPolygon )
    {
      QgsMultiPolygonV2 *mpolyBuffered = static_cast<QgsMultiPolygonV2 *>( buffered );
      for ( int i = 0; i < mpolyBuffered->numGeometries(); ++i )
      {
        QgsAbstractGeometry *partBuffered = mpolyBuffered->geometryN( i );
        Q_ASSERT( QgsWkbTypes::flatType( partBuffered->wkbType() ) == QgsWkbTypes::Polygon );
        QgsPolygonV2 *polyBuffered = static_cast<QgsPolygonV2 *>( partBuffered )->clone(); // need to clone individual geometry parts
        Utils::clampAltitudes( polyBuffered, symbol.altClamping, symbol.altBinding, symbol.height, map );
        polygons.append( polyBuffered );
      }
      delete buffered;
    }
  }

  geometry = new PolygonGeometry;
  geometry->setPolygons( polygons, origin, /*symbol.height,*/ symbol.extrusionHeight );

  Qt3DRender::QGeometryRenderer *renderer = new Qt3DRender::QGeometryRenderer;
  renderer->setGeometry( geometry );
  addComponent( renderer );
}
