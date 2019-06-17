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
#include "qgslinematerial_p.h"
#include "qgslinevertexdata_p.h"
#include "qgstessellatedpolygongeometry.h"
#include "qgs3dmapsettings.h"
//#include "qgsterraingenerator.h"
#include "qgs3dutils.h"

#include "qgsvectorlayer.h"
#include "qgsmultilinestring.h"
#include "qgsmultipolygon.h"
#include "qgsgeos.h"

#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QGeometryRenderer>

/// @cond PRIVATE


static Qt3DExtras::QPhongMaterial *_material( const QgsLine3DSymbol &symbol )
{
  Qt3DExtras::QPhongMaterial *material = new Qt3DExtras::QPhongMaterial;

  material->setAmbient( symbol.material().ambient() );
  material->setDiffuse( symbol.material().diffuse() );
  material->setSpecular( symbol.material().specular() );
  material->setShininess( symbol.material().shininess() );

  return material;
}


// -----------


class QgsBufferedLine3DSymbolHandler : public QgsFeature3DHandler
{
  public:
    QgsBufferedLine3DSymbolHandler( const QgsLine3DSymbol &symbol, const QgsFeatureIds &selectedIds )
      : mSymbol( symbol ), mSelectedIds( selectedIds ) {}

    bool prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames ) override;
    void processFeature( QgsFeature &feature, const Qgs3DRenderContext &context ) override;
    void finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context ) override;

  private:

    //! temporary data we will pass to the tessellator
    struct LineData
    {
      QList<QgsPolygon *> polygons;
      QList<QgsFeatureId> fids;
    };

    void makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, LineData &out, bool selected );

    // input specific for this class
    const QgsLine3DSymbol &mSymbol;
    // inputs - generic
    QgsFeatureIds mSelectedIds;

    // outputs
    LineData outNormal;  //!< Features that are not selected
    LineData outSelected;  //!< Features that are selected
};



bool QgsBufferedLine3DSymbolHandler::prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames )
{
  Q_UNUSED( context )
  Q_UNUSED( attributeNames )
  return true;
}

void QgsBufferedLine3DSymbolHandler::processFeature( QgsFeature &f, const Qgs3DRenderContext &context )
{
  if ( f.geometry().isNull() )
    return;

  LineData &out = mSelectedIds.contains( f.id() ) ? outSelected : outNormal;

  QgsGeometry geom = f.geometry();

  // segmentize curved geometries if necessary
  if ( QgsWkbTypes::isCurvedType( geom.constGet()->wkbType() ) )
    geom = QgsGeometry( geom.constGet()->segmentize() );

  const QgsAbstractGeometry *g = geom.constGet();

  // TODO: configurable
  const int nSegments = 4;
  const QgsGeometry::EndCapStyle endCapStyle = QgsGeometry::CapRound;
  const QgsGeometry::JoinStyle joinStyle = QgsGeometry::JoinStyleRound;
  const double mitreLimit = 0;

  QgsGeos engine( g );
  QgsAbstractGeometry *buffered = engine.buffer( mSymbol.width() / 2., nSegments, endCapStyle, joinStyle, mitreLimit ); // factory

  if ( QgsWkbTypes::flatType( buffered->wkbType() ) == QgsWkbTypes::Polygon )
  {
    QgsPolygon *polyBuffered = static_cast<QgsPolygon *>( buffered );
    Qgs3DUtils::clampAltitudes( polyBuffered, mSymbol.altitudeClamping(), mSymbol.altitudeBinding(), mSymbol.height(), context.map() );
    out.polygons.append( polyBuffered );
    out.fids.append( f.id() );
  }
  else if ( QgsWkbTypes::flatType( buffered->wkbType() ) == QgsWkbTypes::MultiPolygon )
  {
    QgsMultiPolygon *mpolyBuffered = static_cast<QgsMultiPolygon *>( buffered );
    for ( int i = 0; i < mpolyBuffered->numGeometries(); ++i )
    {
      QgsAbstractGeometry *partBuffered = mpolyBuffered->geometryN( i );
      Q_ASSERT( QgsWkbTypes::flatType( partBuffered->wkbType() ) == QgsWkbTypes::Polygon );
      QgsPolygon *polyBuffered = static_cast<QgsPolygon *>( partBuffered )->clone(); // need to clone individual geometry parts
      Qgs3DUtils::clampAltitudes( polyBuffered, mSymbol.altitudeClamping(), mSymbol.altitudeBinding(), mSymbol.height(), context.map() );
      out.polygons.append( polyBuffered );
      out.fids.append( f.id() );
    }
    delete buffered;
  }
}


void QgsBufferedLine3DSymbolHandler::finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context )
{
  // create entity for selected and not selected
  makeEntity( parent, context, outNormal, false );
  makeEntity( parent, context, outSelected, true );
}


void QgsBufferedLine3DSymbolHandler::makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, LineData &out, bool selected )
{
  if ( out.polygons.isEmpty() )
    return;  // nothing to show - no need to create the entity

  Qt3DExtras::QPhongMaterial *mat = _material( mSymbol );
  if ( selected )
  {
    // update the material with selection colors
    mat->setDiffuse( context.map().selectionColor() );
    mat->setAmbient( context.map().selectionColor().darker() );
  }

  QgsPointXY origin( context.map().origin().x(), context.map().origin().y() );
  QgsTessellatedPolygonGeometry *geometry = new QgsTessellatedPolygonGeometry;
  geometry->setPolygons( out.polygons, out.fids, origin, mSymbol.extrusionHeight() );

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


// --------------


class QgsSimpleLine3DSymbolHandler : public QgsFeature3DHandler
{
  public:
    QgsSimpleLine3DSymbolHandler( const QgsLine3DSymbol &symbol, const QgsFeatureIds &selectedIds )
      : mSymbol( symbol ), mSelectedIds( selectedIds )
    {
    }

    bool prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames ) override;
    void processFeature( QgsFeature &feature, const Qgs3DRenderContext &context ) override;
    void finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context ) override;

  private:

    void makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, QgsLineVertexData &out, bool selected );
    Qt3DExtras::QPhongMaterial *material( const QgsLine3DSymbol &symbol ) const;

    // input specific for this class
    const QgsLine3DSymbol &mSymbol;
    // inputs - generic
    QgsFeatureIds mSelectedIds;

    // outputs
    QgsLineVertexData outNormal;  //!< Features that are not selected
    QgsLineVertexData outSelected;  //!< Features that are selected
};



bool QgsSimpleLine3DSymbolHandler::prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames )
{
  Q_UNUSED( attributeNames )

  outNormal.init( mSymbol.altitudeClamping(), mSymbol.altitudeBinding(), mSymbol.height(), &context.map() );
  outSelected.init( mSymbol.altitudeClamping(), mSymbol.altitudeBinding(), mSymbol.height(), &context.map() );

  return true;
}

void QgsSimpleLine3DSymbolHandler::processFeature( QgsFeature &f, const Qgs3DRenderContext &context )
{
  Q_UNUSED( context )
  if ( f.geometry().isNull() )
    return;

  QgsLineVertexData &out = mSelectedIds.contains( f.id() ) ? outSelected : outNormal;

  QgsGeometry geom = f.geometry();
  const QgsAbstractGeometry *g = geom.constGet();
  if ( const QgsLineString *ls = qgsgeometry_cast<const QgsLineString *>( g ) )
  {
    out.addLineString( *ls );
  }
  else if ( const QgsMultiLineString *mls = qgsgeometry_cast<const QgsMultiLineString *>( g ) )
  {
    for ( int nGeom = 0; nGeom < mls->numGeometries(); ++nGeom )
    {
      const QgsLineString *ls = qgsgeometry_cast<const QgsLineString *>( mls->geometryN( nGeom ) );
      out.addLineString( *ls );
    }
  }
}

void QgsSimpleLine3DSymbolHandler::finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context )
{
  // create entity for selected and not selected
  makeEntity( parent, context, outNormal, false );
  makeEntity( parent, context, outSelected, true );
}


void QgsSimpleLine3DSymbolHandler::makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, QgsLineVertexData &out, bool selected )
{
  if ( out.indexes.isEmpty() )
    return;

  // material (only ambient color is used for the color)

  Qt3DExtras::QPhongMaterial *mat = _material( mSymbol );
  if ( selected )
  {
    // update the material with selection colors
    mat->setAmbient( context.map().selectionColor() );
  }

  // geometry renderer

  Qt3DCore::QEntity *entity = new Qt3DCore::QEntity;

  Qt3DRender::QGeometry *geom = out.createGeometry( entity );

  Qt3DRender::QGeometryRenderer *renderer = new Qt3DRender::QGeometryRenderer;
  renderer->setPrimitiveType( Qt3DRender::QGeometryRenderer::LineStrip );
  renderer->setGeometry( geom );
  renderer->setVertexCount( out.indexes.count() );
  renderer->setPrimitiveRestartEnabled( true );
  renderer->setRestartIndexValue( 0 );

  // make entity
  entity->addComponent( renderer );
  entity->addComponent( mat );
  entity->setParent( parent );
}



// --------------


class QgsThickLine3DSymbolHandler : public QgsFeature3DHandler
{
  public:
    QgsThickLine3DSymbolHandler( const QgsLine3DSymbol &symbol, const QgsFeatureIds &selectedIds )
      : mSymbol( symbol ), mSelectedIds( selectedIds )
    {
    }

    bool prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames ) override;
    void processFeature( QgsFeature &feature, const Qgs3DRenderContext &context ) override;
    void finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context ) override;

  private:


    void makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, QgsLineVertexData &out, bool selected );
    Qt3DExtras::QPhongMaterial *material( const QgsLine3DSymbol &symbol ) const;

    // input specific for this class
    const QgsLine3DSymbol &mSymbol;
    // inputs - generic
    QgsFeatureIds mSelectedIds;

    // outputs
    QgsLineVertexData outNormal;  //!< Features that are not selected
    QgsLineVertexData outSelected;  //!< Features that are selected
};



bool QgsThickLine3DSymbolHandler::prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames )
{
  Q_UNUSED( attributeNames )

  outNormal.withAdjacency = true;
  outSelected.withAdjacency = true;
  outNormal.init( mSymbol.altitudeClamping(), mSymbol.altitudeBinding(), mSymbol.height(), &context.map() );
  outSelected.init( mSymbol.altitudeClamping(), mSymbol.altitudeBinding(), mSymbol.height(), &context.map() );

  return true;
}

void QgsThickLine3DSymbolHandler::processFeature( QgsFeature &f, const Qgs3DRenderContext &context )
{
  Q_UNUSED( context )
  if ( f.geometry().isNull() )
    return;

  QgsLineVertexData &out = mSelectedIds.contains( f.id() ) ? outSelected : outNormal;

  QgsGeometry geom = f.geometry();
  const QgsAbstractGeometry *g = geom.constGet();
  if ( const QgsLineString *ls = qgsgeometry_cast<const QgsLineString *>( g ) )
  {
    out.addLineString( *ls );
  }
  else if ( const QgsMultiLineString *mls = qgsgeometry_cast<const QgsMultiLineString *>( g ) )
  {
    for ( int nGeom = 0; nGeom < mls->numGeometries(); ++nGeom )
    {
      const QgsLineString *ls = qgsgeometry_cast<const QgsLineString *>( mls->geometryN( nGeom ) );
      out.addLineString( *ls );
    }
  }
}

void QgsThickLine3DSymbolHandler::finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context )
{
  // create entity for selected and not selected
  makeEntity( parent, context, outNormal, false );
  makeEntity( parent, context, outSelected, true );
}


void QgsThickLine3DSymbolHandler::makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, QgsLineVertexData &out, bool selected )
{
  if ( out.indexes.isEmpty() )
    return;

  // material (only ambient color is used for the color)

  QgsLineMaterial *mat = new QgsLineMaterial;
  mat->setLineColor( mSymbol.material().ambient() );
  mat->setLineWidth( mSymbol.width() );
  if ( selected )
  {
    // update the material with selection colors
    mat->setLineColor( context.map().selectionColor() );
  }

  Qt3DCore::QEntity *entity = new Qt3DCore::QEntity;

  // geometry renderer
  Qt3DRender::QGeometryRenderer *renderer = new Qt3DRender::QGeometryRenderer;
  renderer->setPrimitiveType( Qt3DRender::QGeometryRenderer::LineStripAdjacency );
  renderer->setGeometry( out.createGeometry( entity ) );
  renderer->setVertexCount( out.indexes.count() );
  renderer->setPrimitiveRestartEnabled( true );
  renderer->setRestartIndexValue( 0 );

  // make entity
  entity->addComponent( renderer );
  entity->addComponent( mat );
  entity->setParent( parent );
}


// --------------


namespace Qgs3DSymbolImpl
{

  QgsFeature3DHandler *handlerForLine3DSymbol( QgsVectorLayer *layer, const QgsLine3DSymbol &symbol )
  {
    if ( symbol.renderAsSimpleLines() )
      return new QgsThickLine3DSymbolHandler( symbol, layer->selectedFeatureIds() );
    //return new QgsSimpleLine3DSymbolHandler( symbol, layer->selectedFeatureIds() );
    else
      return new QgsBufferedLine3DSymbolHandler( symbol, layer->selectedFeatureIds() );
  }

  Qt3DCore::QEntity *entityForLine3DSymbol( const Qgs3DMapSettings &map, QgsVectorLayer *layer, const QgsLine3DSymbol &symbol )
  {
    QgsFeature3DHandler *handler = handlerForLine3DSymbol( layer, symbol );
    Qt3DCore::QEntity *e = entityFromHandler( handler, map, layer );
    delete handler;
    return e;
  }
}

/// @endcond
