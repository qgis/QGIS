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
#include "qgstessellator.h"
#include "qgs3dmapsettings.h"
//#include "qgsterraingenerator.h"
#include "qgs3dutils.h"

#include "qgsvectorlayer.h"
#include "qgsmultilinestring.h"
#include "qgsmultipolygon.h"
#include "qgsgeos.h"
#include "qgssimplelinematerialsettings.h"

#include "qgsphongtexturedmaterialsettings.h"

#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QGeometryRenderer>

/// @cond PRIVATE

// -----------


class QgsBufferedLine3DSymbolHandler : public QgsFeature3DHandler
{
  public:
    QgsBufferedLine3DSymbolHandler( const QgsLine3DSymbol *symbol, const QgsFeatureIds &selectedIds )
      : mSymbol( static_cast< QgsLine3DSymbol *>( symbol->clone() ) )
      , mSelectedIds( selectedIds ) {}

    bool prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames ) override;
    void processFeature( const QgsFeature &feature, const Qgs3DRenderContext &context ) override;
    void finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context ) override;

  private:

    //! temporary data we will pass to the tessellator
    struct LineData
    {
      std::unique_ptr<QgsTessellator> tessellator;
      QVector<QgsFeatureId> triangleIndexFids;
      QVector<uint> triangleIndexStartingIndices;
    };

    void processPolygon( QgsPolygon *polyBuffered, QgsFeatureId fid, float height, float extrusionHeight, const Qgs3DRenderContext &context, LineData &out );

    void makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, LineData &out, bool selected );

    // input specific for this class
    std::unique_ptr< QgsLine3DSymbol > mSymbol;
    // inputs - generic
    QgsFeatureIds mSelectedIds;

    // outputs
    LineData outNormal;  //!< Features that are not selected
    LineData outSelected;  //!< Features that are selected
};



bool QgsBufferedLine3DSymbolHandler::prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames )
{
  Q_UNUSED( attributeNames )

  const QgsPhongTexturedMaterialSettings *texturedMaterialSettings = dynamic_cast< const QgsPhongTexturedMaterialSettings * >( mSymbol->material() );

  outNormal.tessellator.reset( new QgsTessellator( context.map().origin().x(), context.map().origin().y(), true,
                               false, false, false, texturedMaterialSettings ? texturedMaterialSettings->requiresTextureCoordinates() : false,
                               3,
                               texturedMaterialSettings ? texturedMaterialSettings->textureRotation() : 0 ) );
  outSelected.tessellator.reset( new QgsTessellator( context.map().origin().x(), context.map().origin().y(), true,
                                 false, false, false, texturedMaterialSettings ? texturedMaterialSettings->requiresTextureCoordinates() : false,
                                 3,
                                 texturedMaterialSettings ? texturedMaterialSettings->textureRotation() : 0 ) );

  return true;
}

void QgsBufferedLine3DSymbolHandler::processFeature( const QgsFeature &f, const Qgs3DRenderContext &context )
{
  if ( f.geometry().isNull() )
    return;

  LineData &out = mSelectedIds.contains( f.id() ) ? outSelected : outNormal;

  QgsGeometry geom = f.geometry();
  const QgsAbstractGeometry *g = geom.constGet()->simplifiedTypeRef();

  // segmentize curved geometries if necessary
  if ( QgsWkbTypes::isCurvedType( g->wkbType() ) )
  {
    geom = QgsGeometry( g->segmentize() );
    g = geom.constGet()->simplifiedTypeRef();
  }

  // TODO: configurable
  const int nSegments = 4;
  const Qgis::EndCapStyle endCapStyle = Qgis::EndCapStyle::Round;
  const Qgis::JoinStyle joinStyle = Qgis::JoinStyle::Round;
  const double mitreLimit = 0;

  const QgsGeos engine( g );

  double width = mSymbol->width();
  if ( qgsDoubleNear( width, 0 ) )
  {
    // a zero-width buffered line should be treated like a "wall" or "fence" -- we fake this by bumping the width to a very tiny amount,
    // so that we get a very narrow polygon shape to work with...
    width = 0.001;
  }

  QgsAbstractGeometry *buffered = engine.buffer( width / 2., nSegments, endCapStyle, joinStyle, mitreLimit ); // factory
  if ( !buffered )
    return;

  if ( QgsWkbTypes::flatType( buffered->wkbType() ) == QgsWkbTypes::Polygon )
  {
    QgsPolygon *polyBuffered = static_cast<QgsPolygon *>( buffered );
    processPolygon( polyBuffered, f.id(), mSymbol->height(), mSymbol->extrusionHeight(), context, out );
  }
  else if ( QgsWkbTypes::flatType( buffered->wkbType() ) == QgsWkbTypes::MultiPolygon )
  {
    QgsMultiPolygon *mpolyBuffered = static_cast<QgsMultiPolygon *>( buffered );
    for ( int i = 0; i < mpolyBuffered->numGeometries(); ++i )
    {
      QgsPolygon *polyBuffered = static_cast<QgsPolygon *>( mpolyBuffered->polygonN( i ) )->clone(); // need to clone individual geometry parts
      processPolygon( polyBuffered, f.id(), mSymbol->height(), mSymbol->extrusionHeight(), context, out );
    }
    delete buffered;
  }
}

void QgsBufferedLine3DSymbolHandler::processPolygon( QgsPolygon *polyBuffered, QgsFeatureId fid, float height, float extrusionHeight, const Qgs3DRenderContext &context, LineData &out )
{
  Qgs3DUtils::clampAltitudes( polyBuffered, mSymbol->altitudeClamping(), mSymbol->altitudeBinding(), height, context.map() );

  Q_ASSERT( out.tessellator->dataVerticesCount() % 3 == 0 );
  const uint startingTriangleIndex = static_cast<uint>( out.tessellator->dataVerticesCount() / 3 );
  out.triangleIndexStartingIndices.append( startingTriangleIndex );
  out.triangleIndexFids.append( fid );
  out.tessellator->addPolygon( *polyBuffered, extrusionHeight );
  delete polyBuffered;
}

void QgsBufferedLine3DSymbolHandler::finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context )
{
  // create entity for selected and not selected
  makeEntity( parent, context, outNormal, false );
  makeEntity( parent, context, outSelected, true );

  mZMin = std::min( outNormal.tessellator->zMinimum(), outSelected.tessellator->zMinimum() );
  mZMax = std::max( outNormal.tessellator->zMaximum(), outSelected.tessellator->zMaximum() );
}


void QgsBufferedLine3DSymbolHandler::makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, LineData &out, bool selected )
{
  if ( out.tessellator->dataVerticesCount() == 0 )
    return;  // nothing to show - no need to create the entity

  QgsMaterialContext materialContext;
  materialContext.setIsSelected( selected );
  materialContext.setSelectionColor( context.map().selectionColor() );
  Qt3DRender::QMaterial *mat = mSymbol->material()->toMaterial( QgsMaterialSettingsRenderingTechnique::Triangles, materialContext );

  // extract vertex buffer data from tessellator
  const QByteArray data( ( const char * )out.tessellator->data().constData(), out.tessellator->data().count() * sizeof( float ) );
  const int nVerts = data.count() / out.tessellator->stride();

  const QgsPhongTexturedMaterialSettings *texturedMaterialSettings = dynamic_cast< const QgsPhongTexturedMaterialSettings * >( mSymbol->material() );

  QgsTessellatedPolygonGeometry *geometry = new QgsTessellatedPolygonGeometry( true, false, false,
      texturedMaterialSettings ? texturedMaterialSettings->requiresTextureCoordinates() : false );
  geometry->setData( data, nVerts, out.triangleIndexFids, out.triangleIndexStartingIndices );

  Qt3DRender::QGeometryRenderer *renderer = new Qt3DRender::QGeometryRenderer;
  renderer->setGeometry( geometry );

  // make entity
  Qt3DCore::QEntity *entity = new Qt3DCore::QEntity;
  entity->addComponent( renderer );
  entity->addComponent( mat );
  entity->setParent( parent );

  if ( !selected )
    entity->findChild<Qt3DRender::QGeometryRenderer *>()->setObjectName( QStringLiteral( "main" ) ); // temporary measure to distinguish between "selected" and "main"

  // cppcheck wrongly believes entity will leak
  // cppcheck-suppress memleak
}


// --------------


class QgsSimpleLine3DSymbolHandler : public QgsFeature3DHandler
{
  public:
    QgsSimpleLine3DSymbolHandler( const QgsLine3DSymbol *symbol, const QgsFeatureIds &selectedIds )
      : mSymbol( static_cast< QgsLine3DSymbol *>( symbol->clone() ) )
      , mSelectedIds( selectedIds )
    {
    }

    bool prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames ) override;
    void processFeature( const QgsFeature &feature, const Qgs3DRenderContext &context ) override;
    void finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context ) override;

  private:

    void makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, QgsLineVertexData &out, bool selected );
    Qt3DExtras::QPhongMaterial *material( const QgsLine3DSymbol &symbol ) const;

    // input specific for this class
    std::unique_ptr< QgsLine3DSymbol > mSymbol;
    // inputs - generic
    QgsFeatureIds mSelectedIds;

    // outputs
    QgsLineVertexData outNormal;  //!< Features that are not selected
    QgsLineVertexData outSelected;  //!< Features that are selected
};



bool QgsSimpleLine3DSymbolHandler::prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames )
{
  Q_UNUSED( attributeNames )

  outNormal.init( mSymbol->altitudeClamping(), mSymbol->altitudeBinding(), mSymbol->height(), &context.map() );
  outSelected.init( mSymbol->altitudeClamping(), mSymbol->altitudeBinding(), mSymbol->height(), &context.map() );

  return true;
}

void QgsSimpleLine3DSymbolHandler::processFeature( const QgsFeature &f, const Qgs3DRenderContext &context )
{
  Q_UNUSED( context )
  if ( f.geometry().isNull() )
    return;

  QgsLineVertexData &out = mSelectedIds.contains( f.id() ) ? outSelected : outNormal;

  const QgsGeometry geom = f.geometry();
  const QgsAbstractGeometry *g = geom.constGet();
  if ( const QgsLineString *ls = qgsgeometry_cast<const QgsLineString *>( g ) )
  {
    out.addLineString( *ls );
  }
  else if ( const QgsMultiLineString *mls = qgsgeometry_cast<const QgsMultiLineString *>( g ) )
  {
    for ( int nGeom = 0; nGeom < mls->numGeometries(); ++nGeom )
    {
      const QgsLineString *ls = mls->lineStringN( nGeom );
      out.addLineString( *ls );
    }
  }
}

void QgsSimpleLine3DSymbolHandler::finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context )
{
  // create entity for selected and not selected
  makeEntity( parent, context, outNormal, false );
  makeEntity( parent, context, outSelected, true );

  updateZRangeFromPositions( outNormal.vertices );
  updateZRangeFromPositions( outSelected.vertices );
}


void QgsSimpleLine3DSymbolHandler::makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, QgsLineVertexData &out, bool selected )
{
  if ( out.indexes.isEmpty() )
    return;

  // material (only ambient color is used for the color)

  QgsMaterialContext materialContext;
  materialContext.setIsSelected( selected );
  materialContext.setSelectionColor( context.map().selectionColor() );
  Qt3DRender::QMaterial *mat = mSymbol->material()->toMaterial( QgsMaterialSettingsRenderingTechnique::Lines, materialContext );

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
    QgsThickLine3DSymbolHandler( const QgsLine3DSymbol *symbol, const QgsFeatureIds &selectedIds )
      : mSymbol( static_cast< QgsLine3DSymbol * >( symbol->clone() ) )
      , mSelectedIds( selectedIds )
    {
    }

    bool prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames ) override;
    void processFeature( const QgsFeature &feature, const Qgs3DRenderContext &context ) override;
    void finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context ) override;

  private:


    void makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, QgsLineVertexData &out, bool selected );
    Qt3DExtras::QPhongMaterial *material( const QgsLine3DSymbol &symbol ) const;

    // input specific for this class
    std::unique_ptr< QgsLine3DSymbol > mSymbol;
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
  outNormal.init( mSymbol->altitudeClamping(), mSymbol->altitudeBinding(), mSymbol->height(), &context.map() );
  outSelected.init( mSymbol->altitudeClamping(), mSymbol->altitudeBinding(), mSymbol->height(), &context.map() );

  return true;
}

void QgsThickLine3DSymbolHandler::processFeature( const QgsFeature &f, const Qgs3DRenderContext &context )
{
  Q_UNUSED( context )
  if ( f.geometry().isNull() )
    return;

  QgsLineVertexData &out = mSelectedIds.contains( f.id() ) ? outSelected : outNormal;

  QgsGeometry geom = f.geometry();
  const QgsAbstractGeometry *g = geom.constGet()->simplifiedTypeRef();

  // segmentize curved geometries if necessary
  if ( QgsWkbTypes::isCurvedType( g->wkbType() ) )
  {
    geom = QgsGeometry( g->segmentize() );
    g = geom.constGet()->simplifiedTypeRef();
  }

  if ( const QgsLineString *ls = qgsgeometry_cast<const QgsLineString *>( g ) )
  {
    out.addLineString( *ls );
  }
  else if ( const QgsMultiLineString *mls = qgsgeometry_cast<const QgsMultiLineString *>( g ) )
  {
    for ( int nGeom = 0; nGeom < mls->numGeometries(); ++nGeom )
    {
      const QgsLineString *ls = mls->lineStringN( nGeom );
      out.addLineString( *ls );
    }
  }
}

void QgsThickLine3DSymbolHandler::finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context )
{
  // create entity for selected and not selected
  makeEntity( parent, context, outNormal, false );
  makeEntity( parent, context, outSelected, true );

  updateZRangeFromPositions( outNormal.vertices );
  updateZRangeFromPositions( outSelected.vertices );
}


void QgsThickLine3DSymbolHandler::makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, QgsLineVertexData &out, bool selected )
{
  if ( out.indexes.isEmpty() )
    return;

  // material (only ambient color is used for the color)
  QgsMaterialContext materialContext;
  materialContext.setIsSelected( selected );
  materialContext.setSelectionColor( context.map().selectionColor() );
  Qt3DRender::QMaterial *mat = mSymbol->material()->toMaterial( QgsMaterialSettingsRenderingTechnique::Lines, materialContext );
  if ( !mat )
  {
    const QgsSimpleLineMaterialSettings defaultMaterial;
    mat = defaultMaterial.toMaterial( QgsMaterialSettingsRenderingTechnique::Lines, materialContext );
  }

  if ( QgsLineMaterial *lineMaterial = dynamic_cast< QgsLineMaterial * >( mat ) )
    lineMaterial->setLineWidth( mSymbol->width() );

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

  QgsFeature3DHandler *handlerForLine3DSymbol( QgsVectorLayer *layer, const QgsAbstract3DSymbol *symbol )
  {
    const QgsLine3DSymbol *lineSymbol = dynamic_cast< const QgsLine3DSymbol * >( symbol );
    if ( !lineSymbol )
      return nullptr;

    if ( lineSymbol->renderAsSimpleLines() )
      return new QgsThickLine3DSymbolHandler( lineSymbol, layer->selectedFeatureIds() );
    //return new QgsSimpleLine3DSymbolHandler( symbol, layer->selectedFeatureIds() );
    else
      return new QgsBufferedLine3DSymbolHandler( lineSymbol, layer->selectedFeatureIds() );
  }

  Qt3DCore::QEntity *entityForLine3DSymbol( const Qgs3DMapSettings &map, QgsVectorLayer *layer, const QgsLine3DSymbol &symbol )
  {
    QgsFeature3DHandler *handler = handlerForLine3DSymbol( layer, &symbol );
    Qt3DCore::QEntity *e = entityFromHandler( handler, map, layer );
    delete handler;
    return e;
  }
}

/// @endcond
