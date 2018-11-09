/***************************************************************************
  qgspolygon3dsymbol_p.cpp
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

#include "qgspolygon3dsymbol_p.h"

#include "qgspolygon3dsymbol.h"
#include "qgstessellatedpolygongeometry.h"
#include "qgs3dmapsettings.h"
#include "qgs3dutils.h"

#include <Qt3DCore/QTransform>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QCullFace>

#include "qgsvectorlayer.h"
#include "qgsmultipolygon.h"


static QgsExpressionContext _expressionContext3D()
{
  QgsExpressionContext ctx;
  ctx << QgsExpressionContextUtils::globalScope()
      << QgsExpressionContextUtils::projectScope( QgsProject::instance() );
  return ctx;
}

static QSet<QString> _requiredAttributes( const QgsPolygon3DSymbol &symbol, QgsVectorLayer *layer )
{
  QgsExpressionContext ctx( _expressionContext3D() );
  ctx.setFields( layer->fields() );
  //symbol.dataDefinedProperties().prepare( ctx );
  return symbol.dataDefinedProperties().referencedFields( ctx );
}


/// @cond PRIVATE

QgsPolygon3DSymbolEntity::QgsPolygon3DSymbolEntity( const Qgs3DMapSettings &map, QgsVectorLayer *layer, const QgsPolygon3DSymbol &symbol, Qt3DCore::QNode *parent )
  : Qt3DCore::QEntity( parent )
{
  addEntityForSelectedPolygons( map, layer, symbol );
  addEntityForNotSelectedPolygons( map, layer, symbol );
}

void QgsPolygon3DSymbolEntity::addEntityForSelectedPolygons( const Qgs3DMapSettings &map, QgsVectorLayer *layer, const QgsPolygon3DSymbol &symbol )
{
  // build the default material
  Qt3DExtras::QPhongMaterial *mat = material( symbol );

  // update the material with selection colors
  mat->setDiffuse( map.selectionColor() );
  mat->setAmbient( map.selectionColor().darker() );

  // build a transform function
  Qt3DCore::QTransform *tform = new Qt3DCore::QTransform;
  tform->setTranslation( QVector3D( 0, 0, 0 ) );

  // build the feature request to select features
  QgsFeatureRequest req;
  req.setDestinationCrs( map.crs(), map.transformContext() );
  req.setSubsetOfAttributes( _requiredAttributes( symbol, layer ), layer->fields() );
  req.setFilterFids( layer->selectedFeatureIds() );

  // build the entity
  QgsPolygon3DSymbolEntityNode *entity = new QgsPolygon3DSymbolEntityNode( map, layer, symbol, req );
  entity->addComponent( mat );
  entity->addComponent( tform );
  entity->setParent( this );
}

void QgsPolygon3DSymbolEntity::addEntityForNotSelectedPolygons( const Qgs3DMapSettings &map, QgsVectorLayer *layer, const QgsPolygon3DSymbol &symbol )
{
  // build the default material
  Qt3DExtras::QPhongMaterial *mat = material( symbol );

  // build a transform function
  Qt3DCore::QTransform *tform = new Qt3DCore::QTransform;
  tform->setTranslation( QVector3D( 0, 0, 0 ) );

  // build the feature request to select features
  QgsFeatureRequest req;
  req.setSubsetOfAttributes( _requiredAttributes( symbol, layer ), layer->fields() );
  req.setDestinationCrs( map.crs(), map.transformContext() );

  QgsFeatureIds notSelected = layer->allFeatureIds();
  notSelected.subtract( layer->selectedFeatureIds() );
  req.setFilterFids( notSelected );

  // build the entity
  QgsPolygon3DSymbolEntityNode *entity = new QgsPolygon3DSymbolEntityNode( map, layer, symbol, req );
  entity->findChild<Qt3DRender::QGeometryRenderer *>()->setObjectName( QStringLiteral( "main" ) ); // temporary measure to distinguish between "selected" and "main"
  entity->addComponent( mat );
  entity->addComponent( tform );
  entity->setParent( this );
}

static Qt3DRender::QCullFace::CullingMode _qt3DcullingMode( Qgs3DTypes::CullingMode mode )
{
  switch ( mode )
  {
    case Qgs3DTypes::NoCulling:    return Qt3DRender::QCullFace::NoCulling;
    case Qgs3DTypes::Front:        return Qt3DRender::QCullFace::Front;
    case Qgs3DTypes::Back:         return Qt3DRender::QCullFace::Back;
    case Qgs3DTypes::FrontAndBack: return Qt3DRender::QCullFace::FrontAndBack;
  }
  return Qt3DRender::QCullFace::NoCulling;
}

Qt3DExtras::QPhongMaterial *QgsPolygon3DSymbolEntity::material( const QgsPolygon3DSymbol &symbol ) const
{
  Qt3DExtras::QPhongMaterial *material = new Qt3DExtras::QPhongMaterial;

  // front/back side culling
  auto techniques = material->effect()->techniques();
  for ( auto tit = techniques.constBegin(); tit != techniques.constEnd(); ++tit )
  {
    auto renderPasses = ( *tit )->renderPasses();
    for ( auto rpit = renderPasses.begin(); rpit != renderPasses.end(); ++rpit )
    {
      Qt3DRender::QCullFace *cullFace = new Qt3DRender::QCullFace;
      cullFace->setMode( _qt3DcullingMode( symbol.cullingMode() ) );
      ( *rpit )->addRenderState( cullFace );
    }
  }

  material->setAmbient( symbol.material().ambient() );
  material->setDiffuse( symbol.material().diffuse() );
  material->setSpecular( symbol.material().specular() );
  material->setShininess( symbol.material().shininess() );
  return material;
}

QgsPolygon3DSymbolEntityNode::QgsPolygon3DSymbolEntityNode( const Qgs3DMapSettings &map, QgsVectorLayer *layer, const QgsPolygon3DSymbol &symbol, const QgsFeatureRequest &req, Qt3DCore::QNode *parent )
  : Qt3DCore::QEntity( parent )
{
  addComponent( renderer( map, symbol, layer, req ) );
}

Qt3DRender::QGeometryRenderer *QgsPolygon3DSymbolEntityNode::renderer( const Qgs3DMapSettings &map, const QgsPolygon3DSymbol &symbol, const QgsVectorLayer *layer, const QgsFeatureRequest &request )
{
  QgsPointXY origin( map.origin().x(), map.origin().y() );
  QList<QgsPolygon *> polygons;
  QList<QgsFeatureId> fids;
  QList<float> extrusionHeightPerPolygon;  // will stay empty if not needed per polygon

  QgsExpressionContext ctx( _expressionContext3D() );
  ctx.setFields( layer->fields() );

  const QgsPropertyCollection &ddp = symbol.dataDefinedProperties();
  bool hasDDHeight = ddp.isActive( QgsAbstract3DSymbol::PropertyHeight );
  bool hasDDExtrusion = ddp.isActive( QgsAbstract3DSymbol::PropertyExtrusionHeight );

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

    ctx.setFeature( f );
    float height = symbol.height();
    float extrusionHeight = symbol.extrusionHeight();
    if ( hasDDHeight )
      height = ddp.valueAsDouble( QgsAbstract3DSymbol::PropertyHeight, ctx, height );
    if ( hasDDExtrusion )
      extrusionHeight = ddp.valueAsDouble( QgsAbstract3DSymbol::PropertyExtrusionHeight, ctx, extrusionHeight );

    if ( const QgsPolygon *poly = qgsgeometry_cast< const QgsPolygon *>( g ) )
    {
      QgsPolygon *polyClone = poly->clone();
      Qgs3DUtils::clampAltitudes( polyClone, symbol.altitudeClamping(), symbol.altitudeBinding(), height, map );
      polygons.append( polyClone );
      fids.append( f.id() );
      if ( hasDDExtrusion )
        extrusionHeightPerPolygon.append( extrusionHeight );
    }
    else if ( const QgsMultiPolygon *mpoly = qgsgeometry_cast< const QgsMultiPolygon *>( g ) )
    {
      for ( int i = 0; i < mpoly->numGeometries(); ++i )
      {
        const QgsAbstractGeometry *g2 = mpoly->geometryN( i );
        Q_ASSERT( QgsWkbTypes::flatType( g2->wkbType() ) == QgsWkbTypes::Polygon );
        QgsPolygon *polyClone = static_cast< const QgsPolygon *>( g2 )->clone();
        Qgs3DUtils::clampAltitudes( polyClone, symbol.altitudeClamping(), symbol.altitudeBinding(), height, map );
        polygons.append( polyClone );
        fids.append( f.id() );
        if ( hasDDExtrusion )
          extrusionHeightPerPolygon.append( extrusionHeight );
      }
    }
    else
      qDebug() << "not a polygon";
  }

  mGeometry = new QgsTessellatedPolygonGeometry;
  mGeometry->setInvertNormals( symbol.invertNormals() );
  mGeometry->setAddBackFaces( symbol.addBackFaces() );
  mGeometry->setPolygons( polygons, fids, origin, symbol.extrusionHeight(), extrusionHeightPerPolygon );

  Qt3DRender::QGeometryRenderer *renderer = new Qt3DRender::QGeometryRenderer;
  renderer->setGeometry( mGeometry );

  return renderer;
}

/// @endcond
