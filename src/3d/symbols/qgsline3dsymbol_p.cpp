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
#include "qgsmultilinestring.h"
#include "qgsmultipolygon.h"
#include "qgsgeos.h"

#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>

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
  req.setDestinationCrs( map.crs(), map.transformContext() );
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
  req.setDestinationCrs( map.crs(), map.transformContext() );

  QgsFeatureIds notSelected = layer->allFeatureIds();
  notSelected.subtract( layer->selectedFeatureIds() );
  req.setFilterFids( notSelected );

  // build the entity
  QgsLine3DSymbolEntityNode *entity = new QgsLine3DSymbolEntityNode( map, layer, symbol, req );
  entity->findChild<Qt3DRender::QGeometryRenderer *>()->setObjectName( QStringLiteral( "main" ) ); // temporary measure to distinguish between "selected" and "main"
  entity->addComponent( mat );
  entity->setParent( this );
}

QgsLine3DSymbolEntityNode::QgsLine3DSymbolEntityNode( const Qgs3DMapSettings &map, QgsVectorLayer *layer, const QgsLine3DSymbol &symbol, const QgsFeatureRequest &req, Qt3DCore::QNode *parent )
  : Qt3DCore::QEntity( parent )
{
  addComponent( symbol.renderAsSimpleLines() ? rendererSimple( map, symbol, layer, req ) : renderer( map, symbol, layer, req ) );
}

Qt3DRender::QGeometryRenderer *QgsLine3DSymbolEntityNode::renderer( const Qgs3DMapSettings &map, const QgsLine3DSymbol &symbol, const QgsVectorLayer *layer, const QgsFeatureRequest &request )
{
  QgsPointXY origin( map.origin().x(), map.origin().y() );

  // TODO: configurable
  int nSegments = 4;
  QgsGeometry::EndCapStyle endCapStyle = QgsGeometry::CapRound;
  QgsGeometry::JoinStyle joinStyle = QgsGeometry::JoinStyleRound;
  double mitreLimit = 0;

  QList<QgsPolygon *> polygons;
  QList<QgsFeatureId> fids;
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
      fids.append( f.id() );
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
        fids.append( f.id() );
      }
      delete buffered;
    }
  }

  mGeometry = new QgsTessellatedPolygonGeometry;
  mGeometry->setPolygons( polygons, fids, origin, symbol.extrusionHeight() );

  Qt3DRender::QGeometryRenderer *renderer = new Qt3DRender::QGeometryRenderer;
  renderer->setGeometry( mGeometry );

  return renderer;
}


Qt3DRender::QGeometryRenderer *QgsLine3DSymbolEntityNode::rendererSimple( const Qgs3DMapSettings &map, const QgsLine3DSymbol &symbol, const QgsVectorLayer *layer, const QgsFeatureRequest &request )
{
  QVector<QVector3D> vertices;
  vertices << QVector3D();  // the first index is invalid, we use it for primitive restart
  QVector<unsigned int> indexes;

  QgsPoint centroid;
  QgsPointXY origin( map.origin().x(), map.origin().y() );
  QgsFeature f;
  QgsFeatureIterator fi = layer->getFeatures( request );
  while ( fi.nextFeature( f ) )
  {
    if ( f.geometry().isNull() )
      continue;

    if ( symbol.altitudeBinding() == Qgs3DTypes::AltBindCentroid )
      centroid = QgsPoint( f.geometry().centroid().asPoint() );

    QgsGeometry geom = f.geometry();
    const QgsAbstractGeometry *g = geom.constGet();
    if ( const QgsLineString *ls = qgsgeometry_cast<const QgsLineString *>( g ) )
    {
      for ( int i = 0; i < ls->vertexCount(); ++i )
      {
        QgsPoint p = ls->pointN( i );
        float z = Qgs3DUtils::clampAltitude( p, symbol.altitudeClamping(), symbol.altitudeBinding(), symbol.height(), centroid, map );
        vertices << QVector3D( p.x() - map.origin().x(), z, -( p.y() - map.origin().y() ) );
        indexes << vertices.count() - 1;
      }
    }
    else if ( const QgsMultiLineString *mls = qgsgeometry_cast<const QgsMultiLineString *>( g ) )
    {
      for ( int nGeom = 0; nGeom < mls->numGeometries(); ++nGeom )
      {
        const QgsLineString *ls = qgsgeometry_cast<const QgsLineString *>( mls->geometryN( nGeom ) );
        for ( int i = 0; i < ls->vertexCount(); ++i )
        {
          QgsPoint p = ls->pointN( i );
          float z = Qgs3DUtils::clampAltitude( p, symbol.altitudeClamping(), symbol.altitudeBinding(), symbol.height(), centroid, map );
          vertices << QVector3D( p.x() - map.origin().x(), z, -( p.y() - map.origin().y() ) );
          indexes << vertices.count() - 1;
        }
        indexes << 0;  // add primitive restart
      }
    }

    indexes << 0;  // add primitive restart
  }

  QByteArray vertexBufferData;
  vertexBufferData.resize( vertices.size() * 3 * sizeof( float ) );
  float *rawVertexArray = reinterpret_cast<float *>( vertexBufferData.data() );
  int idx = 0;
  for ( const auto &v : qgis::as_const( vertices ) )
  {
    rawVertexArray[idx++] = v.x();
    rawVertexArray[idx++] = v.y();
    rawVertexArray[idx++] = v.z();
  }

  QByteArray indexBufferData;
  indexBufferData.resize( indexes.size() * sizeof( int ) );
  unsigned int *rawIndexArray = reinterpret_cast<unsigned int *>( indexBufferData.data() );
  idx = 0;
  for ( unsigned int indexVal : qgis::as_const( indexes ) )
  {
    rawIndexArray[idx++] = indexVal;
  }

  Qt3DRender::QBuffer *vertexBuffer = new Qt3DRender::QBuffer( Qt3DRender::QBuffer::VertexBuffer, this );
  vertexBuffer->setData( vertexBufferData );

  Qt3DRender::QBuffer *indexBuffer = new Qt3DRender::QBuffer( Qt3DRender::QBuffer::IndexBuffer, this );
  indexBuffer->setData( indexBufferData );

  Qt3DRender::QAttribute *positionAttribute = new Qt3DRender::QAttribute( this );
  positionAttribute->setAttributeType( Qt3DRender::QAttribute::VertexAttribute );
  positionAttribute->setBuffer( vertexBuffer );
  positionAttribute->setVertexBaseType( Qt3DRender::QAttribute::Float );
  positionAttribute->setVertexSize( 3 );
  positionAttribute->setName( Qt3DRender::QAttribute::defaultPositionAttributeName() );

  Qt3DRender::QAttribute *indexAttribute = new Qt3DRender::QAttribute( this );
  indexAttribute->setAttributeType( Qt3DRender::QAttribute::IndexAttribute );
  indexAttribute->setBuffer( indexBuffer );
  indexAttribute->setVertexBaseType( Qt3DRender::QAttribute::UnsignedInt );

  Qt3DRender::QGeometry *geom = new Qt3DRender::QGeometry;
  geom->addAttribute( positionAttribute );
  geom->addAttribute( indexAttribute );

  Qt3DRender::QGeometryRenderer *renderer = new Qt3DRender::QGeometryRenderer;
  renderer->setPrimitiveType( Qt3DRender::QGeometryRenderer::LineStrip );
  renderer->setGeometry( geom );
  renderer->setVertexCount( vertices.count() );
  renderer->setPrimitiveRestartEnabled( true );
  renderer->setRestartIndexValue( 0 );
  return renderer;
}

/// @endcond
