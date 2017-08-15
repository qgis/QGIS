#include "lineentity.h"

#include "qgsline3dsymbol.h"
#include "polygongeometry.h"
#include "map3d.h"
#include "terraingenerator.h"
#include "utils.h"

#include "qgsvectorlayer.h"
#include "qgsmultipolygon.h"
#include "qgsgeos.h"


LineEntity::LineEntity( const Map3D &map, QgsVectorLayer *layer, const QgsLine3DSymbol &symbol, Qt3DCore::QNode *parent )
  : Qt3DCore::QEntity( parent )
{
  addEntityForSelectedLines( map, layer, symbol );
  addEntityForNotSelectedLines( map, layer, symbol );
}

Qt3DExtras::QPhongMaterial *LineEntity::material( const QgsLine3DSymbol &symbol ) const
{
  Qt3DExtras::QPhongMaterial *material = new Qt3DExtras::QPhongMaterial;

  material->setAmbient( symbol.material.ambient() );
  material->setDiffuse( symbol.material.diffuse() );
  material->setSpecular( symbol.material.specular() );
  material->setShininess( symbol.material.shininess() );

  return material;
}

void LineEntity::addEntityForSelectedLines( const Map3D &map, QgsVectorLayer *layer, const QgsLine3DSymbol &symbol )
{
  // build the default material
  Qt3DExtras::QPhongMaterial *mat = material( symbol );

  // update the material with selection colors
  mat->setDiffuse( map.selectionColor() );
  mat->setAmbient( map.selectionColor().darker() );

  // build the feature request to select features
  QgsFeatureRequest req;
  req.setDestinationCrs( map.crs );
  req.setFilterFids( layer->selectedFeatureIds() );

  // build the entity
  LineEntityNode *entity = new LineEntityNode( map, layer, symbol, req );
  entity->addComponent( mat );
  entity->setParent( this );
}

void LineEntity::addEntityForNotSelectedLines( const Map3D &map, QgsVectorLayer *layer, const QgsLine3DSymbol &symbol )
{
  // build the default material
  Qt3DExtras::QPhongMaterial *mat = material( symbol );

  // build the feature request to select features
  QgsFeatureRequest req;
  req.setDestinationCrs( map.crs );

  QgsFeatureIds notSelected = layer->allFeatureIds();
  notSelected.subtract( layer->selectedFeatureIds() );
  req.setFilterFids( notSelected );

  // build the entity
  LineEntityNode *entity = new LineEntityNode( map, layer, symbol, req );
  entity->addComponent( mat );
  entity->setParent( this );
}

LineEntityNode::LineEntityNode( const Map3D &map, QgsVectorLayer *layer, const QgsLine3DSymbol &symbol, const QgsFeatureRequest &req, Qt3DCore::QNode *parent )
  : Qt3DCore::QEntity( parent )
{
  addComponent( renderer( map, symbol, layer, req ) );
}

Qt3DRender::QGeometryRenderer *LineEntityNode::renderer( const Map3D &map, const QgsLine3DSymbol &symbol, const QgsVectorLayer *layer, const QgsFeatureRequest &request )
{
  QgsPointXY origin( map.originX, map.originY );

  // TODO: configurable
  int nSegments = 4;
  QgsGeometry::EndCapStyle endCapStyle = QgsGeometry::CapRound;
  QgsGeometry::JoinStyle joinStyle = QgsGeometry::JoinStyleRound;
  double mitreLimit = 0;

  QList<QgsPolygonV2 *> polygons;
  QgsFeature f;
  QgsFeatureIterator fi = layer->getFeatures( request );
  while ( fi.nextFeature( f ) )
  {
    if ( f.geometry().isNull() )
      continue;

    QgsGeometry geom = f.geometry();

    // segmentize curved geometries if necessary
    if ( QgsWkbTypes::isCurvedType( geom.geometry()->wkbType() ) )
      geom = QgsGeometry( geom.geometry()->segmentize() );

    QgsAbstractGeometry *g = geom.geometry();

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

  mGeometry = new PolygonGeometry;
  mGeometry->setPolygons( polygons, origin, /*symbol.height,*/ symbol.extrusionHeight );

  Qt3DRender::QGeometryRenderer *renderer = new Qt3DRender::QGeometryRenderer;
  renderer->setGeometry( mGeometry );

  return renderer;
}
