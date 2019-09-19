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
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QCullFace>
#include <Qt3DRender/QGeometryRenderer>

#include "qgsvectorlayer.h"
#include "qgslinestring.h"
#include "qgsmultipolygon.h"

#include "qgslinevertexdata_p.h"
#include "qgslinematerial_p.h"

/// @cond PRIVATE


class QgsPolygon3DSymbolHandler : public QgsFeature3DHandler
{
  public:
    QgsPolygon3DSymbolHandler( const QgsPolygon3DSymbol &symbol, const QgsFeatureIds &selectedIds )
      : mSymbol( symbol ), mSelectedIds( selectedIds ) {}

    bool prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames ) override;
    void processFeature( QgsFeature &feature, const Qgs3DRenderContext &context ) override;
    void finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context ) override;

  private:

    //! temporary data we will pass to the tessellator
    struct PolygonData
    {
      QList<QgsPolygon *> polygons;
      QList<QgsFeatureId> fids;
      QList<float> extrusionHeightPerPolygon;  // will stay empty if not needed per polygon
    };

    void processPolygon( QgsPolygon *polyClone, QgsFeatureId fid, float height, bool hasDDExtrusion, float extrusionHeight, const Qgs3DRenderContext &context, PolygonData &out );
    void makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, PolygonData &out, bool selected );
    Qt3DExtras::QPhongMaterial *material( const QgsPolygon3DSymbol &symbol ) const;

    // input specific for this class
    const QgsPolygon3DSymbol &mSymbol;
    // inputs - generic
    QgsFeatureIds mSelectedIds;

    // outputs
    PolygonData outNormal;  //!< Features that are not selected
    PolygonData outSelected;  //!< Features that are selected

    QgsLineVertexData outEdges;  //!< When highlighting edges, this holds data for vertex/index buffer
};


bool QgsPolygon3DSymbolHandler::prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames )
{
  outEdges.withAdjacency = true;
  outEdges.init( mSymbol.altitudeClamping(), mSymbol.altitudeBinding(), mSymbol.height(), &context.map() );

  QSet<QString> attrs = mSymbol.dataDefinedProperties().referencedFields( context.expressionContext() );
  attributeNames.unite( attrs );
  return true;
}

void QgsPolygon3DSymbolHandler::processPolygon( QgsPolygon *polyClone, QgsFeatureId fid, float height, bool hasDDExtrusion, float extrusionHeight, const Qgs3DRenderContext &context, PolygonData &out )
{
  if ( mSymbol.edgesEnabled() )
  {
    // add edges before the polygon gets the Z values modified because addLineString() does its own altitude handling
    outEdges.addLineString( *static_cast<const QgsLineString *>( polyClone->exteriorRing() ) );
    for ( int i = 0; i < polyClone->numInteriorRings(); ++i )
      outEdges.addLineString( *static_cast<const QgsLineString *>( polyClone->interiorRing( i ) ) );

    if ( extrusionHeight )
    {
      // add roof and wall edges
      const QgsLineString *exterior = static_cast<const QgsLineString *>( polyClone->exteriorRing() );
      outEdges.addLineString( *exterior, extrusionHeight );
      outEdges.addVerticalLines( *exterior, extrusionHeight );
      for ( int i = 0; i < polyClone->numInteriorRings(); ++i )
      {
        const QgsLineString *interior = static_cast<const QgsLineString *>( polyClone->interiorRing( i ) );
        outEdges.addLineString( *interior, extrusionHeight );
        outEdges.addVerticalLines( *interior, extrusionHeight );
      }
    }
  }

  Qgs3DUtils::clampAltitudes( polyClone, mSymbol.altitudeClamping(), mSymbol.altitudeBinding(), height, context.map() );
  out.polygons.append( polyClone );
  out.fids.append( fid );
  if ( hasDDExtrusion )
    out.extrusionHeightPerPolygon.append( extrusionHeight );
}

void QgsPolygon3DSymbolHandler::processFeature( QgsFeature &f, const Qgs3DRenderContext &context )
{
  if ( f.geometry().isNull() )
    return;

  PolygonData &out = mSelectedIds.contains( f.id() ) ? outSelected : outNormal;

  QgsGeometry geom = f.geometry();

  // segmentize curved geometries if necessary
  if ( QgsWkbTypes::isCurvedType( geom.constGet()->wkbType() ) )
    geom = QgsGeometry( geom.constGet()->segmentize() );

  const QgsAbstractGeometry *g = geom.constGet();

  const QgsPropertyCollection &ddp = mSymbol.dataDefinedProperties();
  bool hasDDHeight = ddp.isActive( QgsAbstract3DSymbol::PropertyHeight );
  bool hasDDExtrusion = ddp.isActive( QgsAbstract3DSymbol::PropertyExtrusionHeight );

  float height = mSymbol.height();
  float extrusionHeight = mSymbol.extrusionHeight();
  if ( hasDDHeight )
    height = ddp.valueAsDouble( QgsAbstract3DSymbol::PropertyHeight, context.expressionContext(), height );
  if ( hasDDExtrusion )
    extrusionHeight = ddp.valueAsDouble( QgsAbstract3DSymbol::PropertyExtrusionHeight, context.expressionContext(), extrusionHeight );

  if ( const QgsPolygon *poly = qgsgeometry_cast< const QgsPolygon *>( g ) )
  {
    QgsPolygon *polyClone = poly->clone();
    processPolygon( polyClone, f.id(), height, hasDDExtrusion, extrusionHeight, context, out );
  }
  else if ( const QgsMultiPolygon *mpoly = qgsgeometry_cast< const QgsMultiPolygon *>( g ) )
  {
    for ( int i = 0; i < mpoly->numGeometries(); ++i )
    {
      const QgsAbstractGeometry *g2 = mpoly->geometryN( i );
      Q_ASSERT( QgsWkbTypes::flatType( g2->wkbType() ) == QgsWkbTypes::Polygon );
      QgsPolygon *polyClone = static_cast< const QgsPolygon *>( g2 )->clone();
      processPolygon( polyClone, f.id(), height, hasDDExtrusion, extrusionHeight, context, out );
    }
  }
  else if ( const QgsGeometryCollection *gc = qgsgeometry_cast< const QgsGeometryCollection *>( g ) )
  {
    for ( int i = 0; i < gc->numGeometries(); ++i )
    {
      const QgsAbstractGeometry *g2 = gc->geometryN( i );
      if ( QgsWkbTypes::flatType( g2->wkbType() ) == QgsWkbTypes::Polygon )
      {
        QgsPolygon *polyClone = static_cast< const QgsPolygon *>( g2 )->clone();
        processPolygon( polyClone, f.id(), height, hasDDExtrusion, extrusionHeight, context, out );
      }
    }
  }
  else
    qDebug() << "not a polygon";
}


void QgsPolygon3DSymbolHandler::finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context )
{
  // create entity for selected and not selected
  makeEntity( parent, context, outNormal, false );
  makeEntity( parent, context, outSelected, true );

  // add entity for edges
  if ( mSymbol.edgesEnabled() && !outEdges.indexes.isEmpty() )
  {
    QgsLineMaterial *mat = new QgsLineMaterial;
    mat->setLineColor( mSymbol.edgeColor() );
    mat->setLineWidth( mSymbol.edgeWidth() );

    Qt3DCore::QEntity *entity = new Qt3DCore::QEntity;

    // geometry renderer
    Qt3DRender::QGeometryRenderer *renderer = new Qt3DRender::QGeometryRenderer;
    renderer->setPrimitiveType( Qt3DRender::QGeometryRenderer::LineStripAdjacency );
    renderer->setGeometry( outEdges.createGeometry( entity ) );
    renderer->setVertexCount( outEdges.indexes.count() );
    renderer->setPrimitiveRestartEnabled( true );
    renderer->setRestartIndexValue( 0 );

    // make entity
    entity->addComponent( renderer );
    entity->addComponent( mat );
    entity->setParent( parent );
  }
}


void QgsPolygon3DSymbolHandler::makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, PolygonData &out, bool selected )
{
  if ( out.polygons.isEmpty() )
    return;  // nothing to show - no need to create the entity

  Qt3DExtras::QPhongMaterial *mat = material( mSymbol );
  if ( selected )
  {
    // update the material with selection colors
    mat->setDiffuse( context.map().selectionColor() );
    mat->setAmbient( context.map().selectionColor().darker() );
  }

  QgsPointXY origin( context.map().origin().x(), context.map().origin().y() );
  QgsTessellatedPolygonGeometry *geometry = new QgsTessellatedPolygonGeometry;
  geometry->setInvertNormals( mSymbol.invertNormals() );
  geometry->setAddBackFaces( mSymbol.addBackFaces() );
  geometry->setPolygons( out.polygons, out.fids, origin, mSymbol.extrusionHeight(), out.extrusionHeightPerPolygon );

  Qt3DRender::QGeometryRenderer *renderer = new Qt3DRender::QGeometryRenderer;
  renderer->setGeometry( geometry );

  // make entity
  Qt3DCore::QEntity *entity = new Qt3DCore::QEntity;
  entity->addComponent( renderer );
  entity->addComponent( mat );
  entity->setParent( parent );

  if ( !selected )
    entity->findChild<Qt3DRender::QGeometryRenderer *>()->setObjectName( QStringLiteral( "main" ) ); // temporary measure to distinguish between "selected" and "main"
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

Qt3DExtras::QPhongMaterial *QgsPolygon3DSymbolHandler::material( const QgsPolygon3DSymbol &symbol ) const
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


// --------------


namespace Qgs3DSymbolImpl
{


  QgsFeature3DHandler *handlerForPolygon3DSymbol( QgsVectorLayer *layer, const QgsPolygon3DSymbol &symbol )
  {
    return new QgsPolygon3DSymbolHandler( symbol, layer->selectedFeatureIds() );
  }

  Qt3DCore::QEntity *entityForPolygon3DSymbol( const Qgs3DMapSettings &map, QgsVectorLayer *layer, const QgsPolygon3DSymbol &symbol )
  {
    QgsFeature3DHandler *handler = handlerForPolygon3DSymbol( layer, symbol );
    Qt3DCore::QEntity *e = entityFromHandler( handler, map, layer );
    delete handler;
    return e;
  }

}

/// @endcond
