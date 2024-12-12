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

#include "qgsgeotransform.h"
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
#include "qgspolygon.h"
#include "qgsphongtexturedmaterialsettings.h"
#include "qgsmessagelog.h"

#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QPhongMaterial>
#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>

typedef Qt3DRender::QAttribute Qt3DQAttribute;
typedef Qt3DRender::QBuffer Qt3DQBuffer;
typedef Qt3DRender::QGeometry Qt3DQGeometry;
#else
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>

typedef Qt3DCore::QAttribute Qt3DQAttribute;
typedef Qt3DCore::QBuffer Qt3DQBuffer;
typedef Qt3DCore::QGeometry Qt3DQGeometry;
#endif
#include <Qt3DRender/QGeometryRenderer>

/// @cond PRIVATE

// -----------


class QgsBufferedLine3DSymbolHandler : public QgsFeature3DHandler
{
  public:
    QgsBufferedLine3DSymbolHandler( const QgsLine3DSymbol *symbol, const QgsFeatureIds &selectedIds )
      : mSymbol( static_cast<QgsLine3DSymbol *>( symbol->clone() ) )
      , mSelectedIds( selectedIds ) {}

    bool prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames, const QgsVector3D &chunkOrigin ) override;
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
    std::unique_ptr<QgsLine3DSymbol> mSymbol;
    // inputs - generic
    QgsFeatureIds mSelectedIds;

    //! origin (in the map coordinates) for output geometries (e.g. at the center of the chunk)
    QgsVector3D mChunkOrigin;

    // outputs
    LineData outNormal;   //!< Features that are not selected
    LineData outSelected; //!< Features that are selected
};


bool QgsBufferedLine3DSymbolHandler::prepare( const Qgs3DRenderContext &, QSet<QString> &attributeNames, const QgsVector3D &chunkOrigin )
{
  Q_UNUSED( attributeNames )

  mChunkOrigin = chunkOrigin;

  const QgsPhongTexturedMaterialSettings *texturedMaterialSettings = dynamic_cast<const QgsPhongTexturedMaterialSettings *>( mSymbol->materialSettings() );

  outNormal.tessellator.reset( new QgsTessellator( chunkOrigin.x(), chunkOrigin.y(), true, false, false, false, texturedMaterialSettings ? texturedMaterialSettings->requiresTextureCoordinates() : false, 3, texturedMaterialSettings ? texturedMaterialSettings->textureRotation() : 0 ) );
  outSelected.tessellator.reset( new QgsTessellator( chunkOrigin.x(), chunkOrigin.y(), true, false, false, false, texturedMaterialSettings ? texturedMaterialSettings->requiresTextureCoordinates() : false, 3, texturedMaterialSettings ? texturedMaterialSettings->textureRotation() : 0 ) );

  outNormal.tessellator->setOutputZUp( true );
  outSelected.tessellator->setOutputZUp( true );

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

  if ( QgsWkbTypes::flatType( buffered->wkbType() ) == Qgis::WkbType::Polygon )
  {
    QgsPolygon *polyBuffered = static_cast<QgsPolygon *>( buffered );
    processPolygon( polyBuffered, f.id(), mSymbol->offset(), mSymbol->extrusionHeight(), context, out );
  }
  else if ( QgsWkbTypes::flatType( buffered->wkbType() ) == Qgis::WkbType::MultiPolygon )
  {
    QgsMultiPolygon *mpolyBuffered = static_cast<QgsMultiPolygon *>( buffered );
    for ( int i = 0; i < mpolyBuffered->numGeometries(); ++i )
    {
      QgsPolygon *polyBuffered = static_cast<QgsPolygon *>( mpolyBuffered->polygonN( i ) )->clone(); // need to clone individual geometry parts
      processPolygon( polyBuffered, f.id(), mSymbol->offset(), mSymbol->extrusionHeight(), context, out );
    }
    delete buffered;
  }
  mFeatureCount++;
}

void QgsBufferedLine3DSymbolHandler::processPolygon( QgsPolygon *polyBuffered, QgsFeatureId fid, float height, float extrusionHeight, const Qgs3DRenderContext &context, LineData &out )
{
  Qgs3DUtils::clampAltitudes( polyBuffered, mSymbol->altitudeClamping(), mSymbol->altitudeBinding(), height, context );

  Q_ASSERT( out.tessellator->dataVerticesCount() % 3 == 0 );
  const uint startingTriangleIndex = static_cast<uint>( out.tessellator->dataVerticesCount() / 3 );
  out.triangleIndexStartingIndices.append( startingTriangleIndex );
  out.triangleIndexFids.append( fid );
  out.tessellator->addPolygon( *polyBuffered, extrusionHeight );
  if ( !out.tessellator->error().isEmpty() )
  {
    QgsMessageLog::logMessage( out.tessellator->error(), QObject::tr( "3D" ) );
  }

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
    return; // nothing to show - no need to create the entity

  QgsMaterialContext materialContext;
  materialContext.setIsSelected( selected );
  materialContext.setSelectionColor( context.selectionColor() );
  QgsMaterial *mat = mSymbol->materialSettings()->toMaterial( QgsMaterialSettingsRenderingTechnique::Triangles, materialContext );

  // extract vertex buffer data from tessellator
  const QByteArray data( ( const char * ) out.tessellator->data().constData(), out.tessellator->data().count() * sizeof( float ) );
  const int nVerts = data.count() / out.tessellator->stride();

  const QgsPhongTexturedMaterialSettings *texturedMaterialSettings = dynamic_cast<const QgsPhongTexturedMaterialSettings *>( mSymbol->materialSettings() );

  QgsTessellatedPolygonGeometry *geometry = new QgsTessellatedPolygonGeometry( true, false, false, texturedMaterialSettings ? texturedMaterialSettings->requiresTextureCoordinates() : false );
  geometry->setData( data, nVerts, out.triangleIndexFids, out.triangleIndexStartingIndices );

  Qt3DRender::QGeometryRenderer *renderer = new Qt3DRender::QGeometryRenderer;
  renderer->setGeometry( geometry );

  // add transform (our geometry has coordinates relative to mChunkOrigin)
  QgsGeoTransform *tr = new QgsGeoTransform;
  tr->setGeoTranslation( mChunkOrigin );

  // make entity
  Qt3DCore::QEntity *entity = new Qt3DCore::QEntity;
  entity->addComponent( renderer );
  entity->addComponent( mat );
  entity->addComponent( tr );
  entity->setParent( parent );

  if ( !selected )
    renderer->setProperty( Qgs3DTypes::PROP_NAME_3D_RENDERER_FLAG, Qgs3DTypes::Main3DRenderer ); // temporary measure to distinguish between "selected" and "main"

  // cppcheck wrongly believes entity will leak
  // cppcheck-suppress memleak
}


// --------------


class QgsThickLine3DSymbolHandler : public QgsFeature3DHandler
{
  public:
    QgsThickLine3DSymbolHandler( const QgsLine3DSymbol *symbol, const QgsFeatureIds &selectedIds )
      : mSymbol( static_cast<QgsLine3DSymbol *>( symbol->clone() ) )
      , mSelectedIds( selectedIds )
    {
    }

    bool prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames, const QgsVector3D &chunkOrigin ) override;
    void processFeature( const QgsFeature &feature, const Qgs3DRenderContext &context ) override;
    void finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context ) override;

  private:
    void makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, QgsLineVertexData &out, bool selected );
    Qt3DExtras::QPhongMaterial *material( const QgsLine3DSymbol &symbol ) const;
    void processMaterialDatadefined( uint verticesCount, const QgsExpressionContext &context, QgsLineVertexData &out );

    // input specific for this class
    std::unique_ptr<QgsLine3DSymbol> mSymbol;
    // inputs - generic
    QgsFeatureIds mSelectedIds;

    //! origin (in the map coordinates) for output geometries (e.g. at the center of the chunk)
    QgsVector3D mChunkOrigin;

    // outputs
    QgsLineVertexData outNormal;   //!< Features that are not selected
    QgsLineVertexData outSelected; //!< Features that are selected
};


bool QgsThickLine3DSymbolHandler::prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames, const QgsVector3D &chunkOrigin )
{
  Q_UNUSED( attributeNames )

  mChunkOrigin = chunkOrigin;

  outNormal.withAdjacency = true;
  outSelected.withAdjacency = true;
  outNormal.init( mSymbol->altitudeClamping(), mSymbol->altitudeBinding(), mSymbol->offset(), context, chunkOrigin );
  outSelected.init( mSymbol->altitudeClamping(), mSymbol->altitudeBinding(), mSymbol->offset(), context, chunkOrigin );

  QSet<QString> attrs = mSymbol->dataDefinedProperties().referencedFields( context.expressionContext() );
  attributeNames.unite( attrs );
  attrs = mSymbol->materialSettings()->dataDefinedProperties().referencedFields( context.expressionContext() );
  attributeNames.unite( attrs );

  if ( mSymbol->materialSettings()->dataDefinedProperties().isActive( QgsAbstractMaterialSettings::Property::Ambient ) )
  {
    processMaterialDatadefined( outNormal.vertices.size(), context.expressionContext(), outNormal );
    processMaterialDatadefined( outSelected.vertices.size(), context.expressionContext(), outSelected );
  }

  return true;
}

void QgsThickLine3DSymbolHandler::processFeature( const QgsFeature &f, const Qgs3DRenderContext &context )
{
  Q_UNUSED( context )
  if ( f.geometry().isNull() )
    return;

  QgsLineVertexData &out = mSelectedIds.contains( f.id() ) ? outSelected : outNormal;

  const int oldVerticesCount = out.vertices.size();

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

  if ( mSymbol->materialSettings()->dataDefinedProperties().isActive( QgsAbstractMaterialSettings::Property::Ambient ) )
    processMaterialDatadefined( out.vertices.size() - oldVerticesCount, context.expressionContext(), out );

  mFeatureCount++;
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
  materialContext.setSelectionColor( context.selectionColor() );
  QgsMaterial *mat = mSymbol->materialSettings()->toMaterial( QgsMaterialSettingsRenderingTechnique::Lines, materialContext );
  if ( !mat )
  {
    const QgsSimpleLineMaterialSettings defaultMaterial;
    mat = defaultMaterial.toMaterial( QgsMaterialSettingsRenderingTechnique::Lines, materialContext );
  }

  if ( QgsLineMaterial *lineMaterial = dynamic_cast<QgsLineMaterial *>( mat ) )
    lineMaterial->setLineWidth( mSymbol->width() );

  Qt3DCore::QEntity *entity = new Qt3DCore::QEntity;

  // geometry renderer
  Qt3DRender::QGeometryRenderer *renderer = new Qt3DRender::QGeometryRenderer;
  renderer->setPrimitiveType( Qt3DRender::QGeometryRenderer::LineStripAdjacency );
  Qt3DQGeometry *geometry = out.createGeometry( entity );

  if ( mSymbol->materialSettings()->dataDefinedProperties().isActive( QgsAbstractMaterialSettings::Property::Ambient ) )
    mSymbol->materialSettings()->applyDataDefinedToGeometry( geometry, out.vertices.size(), out.materialDataDefined );

  renderer->setGeometry( geometry );

  renderer->setVertexCount( out.indexes.count() );
  renderer->setPrimitiveRestartEnabled( true );
  renderer->setRestartIndexValue( 0 );

  // add transform (our geometry has coordinates relative to mChunkOrigin)
  QgsGeoTransform *tr = new QgsGeoTransform;
  tr->setGeoTranslation( mChunkOrigin );

  // make entity
  entity->addComponent( renderer );
  entity->addComponent( mat );
  entity->addComponent( tr );
  entity->setParent( parent );
}

void QgsThickLine3DSymbolHandler::processMaterialDatadefined( uint verticesCount, const QgsExpressionContext &context, QgsLineVertexData &out )
{
  const QByteArray bytes = mSymbol->materialSettings()->dataDefinedVertexColorsAsByte( context );
  out.materialDataDefined.append( bytes.repeated( verticesCount ) );
}


// --------------


namespace Qgs3DSymbolImpl
{

  QgsFeature3DHandler *handlerForLine3DSymbol( QgsVectorLayer *layer, const QgsAbstract3DSymbol *symbol )
  {
    const QgsLine3DSymbol *lineSymbol = dynamic_cast<const QgsLine3DSymbol *>( symbol );
    if ( !lineSymbol )
      return nullptr;

    if ( lineSymbol->renderAsSimpleLines() )
      return new QgsThickLine3DSymbolHandler( lineSymbol, layer->selectedFeatureIds() );
    else
      return new QgsBufferedLine3DSymbolHandler( lineSymbol, layer->selectedFeatureIds() );
  }
} // namespace Qgs3DSymbolImpl

/// @endcond
