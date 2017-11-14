/***************************************************************************
  qgsline3dsymbol_p.cpp
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsline3dsymbol_p.h"

#include "qgsline3dsymbol.h"
#include "qgstessellatedpolygongeometry.h"
#include "qgs3dmapsettings.h"
//#include "qgsterraingenerator.h"
#include "qgs3dutils.h"

#include "qgsvectorlayer.h"
#include "qgsmultipolygon.h"
#include "qgsgeos.h"

/// @cond PRIVATE

QgsLine3DSymbolEntity::QgsLine3DSymbolEntity( const Qgs3DMapSettings &map, QgsVectorLayer *layer, const QgsLine3DSymbol &symbol, Qt3DCore::QNode *parent )
  : Qt3DCore::QEntity( parent )
{
  addEntityForSelectedLines( map, layer, symbol );
  addEntityForNotSelectedLines( map, layer, symbol );
}

Qt3DExtras::QPhongMaterial *QgsLine3DSymbolEntity::material( const QgsLine3DSymbol &symbol ) const
{
  Qt3DExtras::QPhongMaterial *material = new Qt3DExtras::QPhongMaterial;

  material->setAmbient( symbol.material().ambient() );
  material->setDiffuse( symbol.material().diffuse() );
  material->setSpecular( symbol.material().specular() );
  material->setShininess( symbol.material().shininess() );

  return material;
}

void QgsLine3DSymbolEntity::addEntityForSelectedLines( const Qgs3DMapSettings &map, QgsVectorLayer *layer, const QgsLine3DSymbol &symbol )
{
  // build the default material
  Qt3DExtras::QPhongMaterial *mat = material( symbol );

  // update the material with selection colors
  mat->setDiffuse( map.selectionColor() );
  mat->setAmbient( map.selectionColor().darker() );

  // build the feature request to select features
  QgsFeatureRequest req;
  req.setDestinationCrs( map.crs() );
  req.setFilterFids( layer->selectedFeatureIds() );

  // build the entity
  QgsLine3DSymbolEntityNode *entity = new QgsLine3DSymbolEntityNode( map, layer, symbol, req );
  entity->addComponent( mat );
  entity->setParent( this );
}

void QgsLine3DSymbolEntity::addEntityForNotSelectedLines( const Qgs3DMapSettings &map, QgsVectorLayer *layer, const QgsLine3DSymbol &symbol )
{
  // build the default material
  Qt3DExtras::QPhongMaterial *mat = material( symbol );

  // build the feature request to select features
  QgsFeatureRequest req;
  req.setDestinationCrs( map.crs() );

  QgsFeatureIds notSelected = layer->allFeatureIds();
  notSelected.subtract( layer->selectedFeatureIds() );
  req.setFilterFids( notSelected );

  // build the entity
  QgsLine3DSymbolEntityNode *entity = new QgsLine3DSymbolEntityNode( map, layer, symbol, req );
  entity->addComponent( mat );
  entity->setParent( this );
}

QgsLine3DSymbolEntityNode::QgsLine3DSymbolEntityNode( const Qgs3DMapSettings &map, QgsVectorLayer *layer, const QgsLine3DSymbol &symbol, const QgsFeatureRequest &req, Qt3DCore::QNode *parent )
  : Qt3DCore::QEntity( parent )
{
  addComponent( renderer( map, symbol, layer, req ) );
}

Qt3DRender::QGeometryRenderer *QgsLine3DSymbolEntityNode::renderer( const Qgs3DMapSettings &map, const QgsLine3DSymbol &symbol, const QgsVectorLayer *layer, const QgsFeatureRequest &request )
{
  QgsPointXY origin( map.originX(), map.originY() );

  // TODO: configurable
  int nSegments = 4;
  QgsGeometry::EndCapStyle endCapStyle = QgsGeometry::CapRound;
  QgsGeometry::JoinStyle joinStyle = QgsGeometry::JoinStyleRound;
  double mitreLimit = 0;

  QList<QgsPolygon *> polygons;
  QgsFeature f;
  QgsFeatureIterator fi = layer->getFeatures( request );
  while ( fi.nextFeature( f ) )
  {
    if ( f.geometry().isNull() )
      continue;

    QgsGeometry geom = f.geometry();

    // segmentize curved geometries if necessary
    if ( QgsWkbTypes::isCurvedType( geom.constGet()->wkbType() ) )
      geom = QgsGeometry( geom.constGet()->segmentize() );

    const QgsAbstractGeometry *g = geom.constGet();

    QgsGeos engine( g );
    QgsAbstractGeometry *buffered = engine.buffer( symbol.width() / 2., nSegments, endCapStyle, joinStyle, mitreLimit ); // factory

    if ( QgsWkbTypes::flatType( buffered->wkbType() ) == QgsWkbTypes::Polygon )
    {
      QgsPolygon *polyBuffered = static_cast<QgsPolygon *>( buffered );
      Qgs3DUtils::clampAltitudes( polyBuffered, symbol.altitudeClamping(), symbol.altitudeBinding(), symbol.height(), map );
      polygons.append( polyBuffered );
    }
    else if ( QgsWkbTypes::flatType( buffered->wkbType() ) == QgsWkbTypes::MultiPolygon )
    {
      QgsMultiPolygon *mpolyBuffered = static_cast<QgsMultiPolygon *>( buffered );
      for ( int i = 0; i < mpolyBuffered->numGeometries(); ++i )
      {
        QgsAbstractGeometry *partBuffered = mpolyBuffered->geometryN( i );
        Q_ASSERT( QgsWkbTypes::flatType( partBuffered->wkbType() ) == QgsWkbTypes::Polygon );
        QgsPolygon *polyBuffered = static_cast<QgsPolygon *>( partBuffered )->clone(); // need to clone individual geometry parts
        Qgs3DUtils::clampAltitudes( polyBuffered, symbol.altitudeClamping(), symbol.altitudeBinding(), symbol.height(), map );
        polygons.append( polyBuffered );
      }
      delete buffered;
    }
  }

  mGeometry = new QgsTessellatedPolygonGeometry;
  mGeometry->setPolygons( polygons, origin, symbol.extrusionHeight() );

  Qt3DRender::QGeometryRenderer *renderer = new Qt3DRender::QGeometryRenderer;
  renderer->setGeometry( mGeometry );

  return renderer;
}

/// @endcond
