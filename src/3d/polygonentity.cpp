#include "polygonentity.h"

#include "abstract3dsymbol.h"
#include "polygongeometry.h"
#include "map3d.h"
#include "terraingenerator.h"
#include "utils.h"

#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DCore/QTransform>

#include "qgsvectorlayer.h"
#include "qgsmultipolygon.h"



PolygonEntity::PolygonEntity( const Map3D &map, QgsVectorLayer *layer, const Polygon3DSymbol &symbol, Qt3DCore::QNode *parent )
  : Qt3DCore::QEntity( parent )
{
  QgsPointXY origin( map.originX, map.originY );

  Qt3DExtras::QPhongMaterial *material = new Qt3DExtras::QPhongMaterial;
  material->setAmbient( symbol.material.ambient() );
  material->setDiffuse( symbol.material.diffuse() );
  material->setSpecular( symbol.material.specular() );
  material->setShininess( symbol.material.shininess() );
  addComponent( material );

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

    if ( QgsWkbTypes::flatType( g->wkbType() ) == QgsWkbTypes::Polygon )
    {
      QgsPolygonV2 *poly = static_cast<QgsPolygonV2 *>( g );
      QgsPolygonV2 *polyClone = poly->clone();
      Utils::clampAltitudes( polyClone, symbol.altClamping, symbol.altBinding, symbol.height, map );
      polygons.append( polyClone );
    }
    else if ( QgsWkbTypes::flatType( g->wkbType() ) == QgsWkbTypes::MultiPolygon )
    {
      QgsMultiPolygonV2 *mpoly = static_cast<QgsMultiPolygonV2 *>( g );
      for ( int i = 0; i < mpoly->numGeometries(); ++i )
      {
        QgsAbstractGeometry *g2 = mpoly->geometryN( i );
        Q_ASSERT( QgsWkbTypes::flatType( g2->wkbType() ) == QgsWkbTypes::Polygon );
        QgsPolygonV2 *polyClone = static_cast<QgsPolygonV2 *>( g2 )->clone();
        Utils::clampAltitudes( polyClone, symbol.altClamping, symbol.altBinding, symbol.height, map );
        polygons.append( polyClone );
      }
    }
    else
      qDebug() << "not a polygon";
  }

  geometry = new PolygonGeometry;
  geometry->setPolygons( polygons, origin, symbol.extrusionHeight );

  Qt3DRender::QGeometryRenderer *renderer = new Qt3DRender::QGeometryRenderer;
  renderer->setGeometry( geometry );
  addComponent( renderer );

  Qt3DCore::QTransform *tform = new Qt3DCore::QTransform;
  tform->setTranslation( QVector3D( 0, 0, 0 ) );
  addComponent( tform );
}
