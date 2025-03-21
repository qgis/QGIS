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

    void processPolygon( QgsPolygon *polyBuffered, QgsFeatureId fid, float height, float extrusionHeight, const Qgs3DRenderContext &context, LineData &lineData );

    void makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, LineData &lineData, bool selected );

    // input specific for this class
    std::unique_ptr<QgsLine3DSymbol> mSymbol;
    // inputs - generic
    QgsFeatureIds mSelectedIds;

    //! origin (in the map coordinates) for output geometries (e.g. at the center of the chunk)
    QgsVector3D mChunkOrigin;

    // outputs
    LineData mLineDataNormal;   //!< Features that are not selected
    LineData mLineDataSelected; //!< Features that are selected
};


bool QgsBufferedLine3DSymbolHandler::prepare( const Qgs3DRenderContext &, QSet<QString> &attributeNames, const QgsVector3D &chunkOrigin )
{
  Q_UNUSED( attributeNames )

  mChunkOrigin = chunkOrigin;

  const QgsPhongTexturedMaterialSettings *texturedMaterialSettings = dynamic_cast<const QgsPhongTexturedMaterialSettings *>( mSymbol->materialSettings() );

  const float textureRotation = texturedMaterialSettings ? static_cast<float>( texturedMaterialSettings->textureRotation() ) : 0;
  const bool requiresTextureCoordinates = texturedMaterialSettings ? texturedMaterialSettings->requiresTextureCoordinates() : false;
  mLineDataNormal.tessellator.reset( new QgsTessellator( chunkOrigin.x(), chunkOrigin.y(), true, false, false, false, requiresTextureCoordinates, 3, textureRotation ) );
  mLineDataSelected.tessellator.reset( new QgsTessellator( chunkOrigin.x(), chunkOrigin.y(), true, false, false, false, requiresTextureCoordinates, 3, textureRotation ) );

  mLineDataNormal.tessellator->setOutputZUp( true );
  mLineDataSelected.tessellator->setOutputZUp( true );

  return true;
}

void QgsBufferedLine3DSymbolHandler::processFeature( const QgsFeature &feature, const Qgs3DRenderContext &context )
{
  if ( feature.geometry().isNull() )
    return;

  LineData &lineData = mSelectedIds.contains( feature.id() ) ? mLineDataSelected : mLineDataNormal;

  QgsGeometry geom = feature.geometry();
  const QgsAbstractGeometry *abstractGeom = geom.constGet()->simplifiedTypeRef();

  // segmentize curved geometries if necessary
  if ( QgsWkbTypes::isCurvedType( abstractGeom->wkbType() ) )
  {
    geom = QgsGeometry( abstractGeom->segmentize() );
    abstractGeom = geom.constGet()->simplifiedTypeRef();
  }

  // TODO: configurable
  const int nSegments = 4;
  const Qgis::EndCapStyle endCapStyle = Qgis::EndCapStyle::Round;
  const Qgis::JoinStyle joinStyle = Qgis::JoinStyle::Round;
  const double mitreLimit = 0;

  const QgsGeos engine( abstractGeom );

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
    QgsPolygon *polyBuffered = qgsgeometry_cast<QgsPolygon *>( buffered );
    processPolygon( polyBuffered, feature.id(), mSymbol->offset(), mSymbol->extrusionHeight(), context, lineData );
  }
  else if ( QgsWkbTypes::flatType( buffered->wkbType() ) == Qgis::WkbType::MultiPolygon )
  {
    QgsMultiPolygon *mpolyBuffered = qgsgeometry_cast<QgsMultiPolygon *>( buffered );
    for ( int i = 0; i < mpolyBuffered->numGeometries(); ++i )
    {
      QgsPolygon *polyBuffered = qgsgeometry_cast<QgsPolygon *>( mpolyBuffered->polygonN( i ) )->clone(); // need to clone individual geometry parts
      processPolygon( polyBuffered, feature.id(), mSymbol->offset(), mSymbol->extrusionHeight(), context, lineData );
    }
    delete buffered;
  }
  mFeatureCount++;
}

void QgsBufferedLine3DSymbolHandler::processPolygon( QgsPolygon *polyBuffered, QgsFeatureId fid, float height, float extrusionHeight, const Qgs3DRenderContext &context, LineData &lineData )
{
  Qgs3DUtils::clampAltitudes( polyBuffered, mSymbol->altitudeClamping(), mSymbol->altitudeBinding(), height, context );

  Q_ASSERT( lineData.tessellator->dataVerticesCount() % 3 == 0 );
  const uint startingTriangleIndex = static_cast<uint>( lineData.tessellator->dataVerticesCount() / 3 );
  lineData.triangleIndexStartingIndices.append( startingTriangleIndex );
  lineData.triangleIndexFids.append( fid );
  lineData.tessellator->addPolygon( *polyBuffered, extrusionHeight );
  if ( !lineData.tessellator->error().isEmpty() )
  {
    QgsMessageLog::logMessage( lineData.tessellator->error(), QObject::tr( "3D" ) );
  }

  delete polyBuffered;
}

void QgsBufferedLine3DSymbolHandler::finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context )
{
  // create entity for selected and not selected
  makeEntity( parent, context, mLineDataNormal, false );
  makeEntity( parent, context, mLineDataSelected, true );

  mZMin = std::min( mLineDataNormal.tessellator->zMinimum(), mLineDataSelected.tessellator->zMinimum() );
  mZMax = std::max( mLineDataNormal.tessellator->zMaximum(), mLineDataSelected.tessellator->zMaximum() );
}


void QgsBufferedLine3DSymbolHandler::makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, LineData &lineData, bool selected )
{
  if ( lineData.tessellator->dataVerticesCount() == 0 )
    return; // nothing to show - no need to create the entity

  QgsMaterialContext materialContext;
  materialContext.setIsSelected( selected );
  materialContext.setSelectionColor( context.selectionColor() );
  QgsMaterial *material = mSymbol->materialSettings()->toMaterial( QgsMaterialSettingsRenderingTechnique::Triangles, materialContext );

  // extract vertex buffer data from tessellator
  const QByteArray data( ( const char * ) lineData.tessellator->data().constData(), static_cast<int>( lineData.tessellator->data().count() * sizeof( float ) ) );
  const int nVerts = data.count() / lineData.tessellator->stride();

  const QgsPhongTexturedMaterialSettings *texturedMaterialSettings = dynamic_cast<const QgsPhongTexturedMaterialSettings *>( mSymbol->materialSettings() );

  QgsTessellatedPolygonGeometry *geometry = new QgsTessellatedPolygonGeometry( true, false, false, texturedMaterialSettings ? texturedMaterialSettings->requiresTextureCoordinates() : false );
  geometry->setData( data, nVerts, lineData.triangleIndexFids, lineData.triangleIndexStartingIndices );

  Qt3DRender::QGeometryRenderer *renderer = new Qt3DRender::QGeometryRenderer;
  renderer->setGeometry( geometry );

  // add transform (our geometry has coordinates relative to mChunkOrigin)
  QgsGeoTransform *transform = new QgsGeoTransform;
  transform->setGeoTranslation( mChunkOrigin );

  // make entity
  Qt3DCore::QEntity *entity = new Qt3DCore::QEntity;
  entity->addComponent( renderer );
  entity->addComponent( material );
  entity->addComponent( transform );
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
    void makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, QgsLineVertexData &lineVertexData, bool selected );
    Qt3DExtras::QPhongMaterial *material( const QgsLine3DSymbol &symbol ) const;
    void processMaterialDatadefined( uint verticesCount, const QgsExpressionContext &context, QgsLineVertexData &lineVertexData );

    // input specific for this class
    std::unique_ptr<QgsLine3DSymbol> mSymbol;
    // inputs - generic
    QgsFeatureIds mSelectedIds;

    //! origin (in the map coordinates) for output geometries (e.g. at the center of the chunk)
    QgsVector3D mChunkOrigin;

    // outputs
    QgsLineVertexData mLineDataNormal;   //!< Features that are not selected
    QgsLineVertexData mLineDataSelected; //!< Features that are selected
};


bool QgsThickLine3DSymbolHandler::prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames, const QgsVector3D &chunkOrigin )
{
  Q_UNUSED( attributeNames )

  mChunkOrigin = chunkOrigin;

  mLineDataNormal.withAdjacency = true;
  mLineDataSelected.withAdjacency = true;
  mLineDataNormal.init( mSymbol->altitudeClamping(), mSymbol->altitudeBinding(), mSymbol->offset(), context, chunkOrigin );
  mLineDataSelected.init( mSymbol->altitudeClamping(), mSymbol->altitudeBinding(), mSymbol->offset(), context, chunkOrigin );

  QSet<QString> attrs = mSymbol->dataDefinedProperties().referencedFields( context.expressionContext() );
  attributeNames.unite( attrs );
  attrs = mSymbol->materialSettings()->dataDefinedProperties().referencedFields( context.expressionContext() );
  attributeNames.unite( attrs );

  if ( mSymbol->materialSettings()->dataDefinedProperties().isActive( QgsAbstractMaterialSettings::Property::Ambient ) )
  {
    processMaterialDatadefined( mLineDataNormal.vertices.size(), context.expressionContext(), mLineDataNormal );
    processMaterialDatadefined( mLineDataSelected.vertices.size(), context.expressionContext(), mLineDataSelected );
  }

  return true;
}

void QgsThickLine3DSymbolHandler::processFeature( const QgsFeature &feature, const Qgs3DRenderContext &context )
{
  Q_UNUSED( context )
  if ( feature.geometry().isNull() )
    return;

  QgsLineVertexData &lineVertexData = mSelectedIds.contains( feature.id() ) ? mLineDataSelected : mLineDataNormal;

  const int oldVerticesCount = lineVertexData.vertices.size();

  QgsGeometry geom = feature.geometry();
  const QgsAbstractGeometry *abstractGeom = geom.constGet()->simplifiedTypeRef();

  // segmentize curved geometries if necessary
  if ( QgsWkbTypes::isCurvedType( abstractGeom->wkbType() ) )
  {
    geom = QgsGeometry( abstractGeom->segmentize() );
    abstractGeom = geom.constGet()->simplifiedTypeRef();
  }

  if ( const QgsLineString *lineString = qgsgeometry_cast<const QgsLineString *>( abstractGeom ) )
  {
    lineVertexData.addLineString( *lineString );
  }
  else if ( const QgsMultiLineString *multiLineString = qgsgeometry_cast<const QgsMultiLineString *>( abstractGeom ) )
  {
    for ( int nGeom = 0; nGeom < multiLineString->numGeometries(); ++nGeom )
    {
      const QgsLineString *lineString = multiLineString->lineStringN( nGeom );
      lineVertexData.addLineString( *lineString );
    }
  }

  if ( mSymbol->materialSettings()->dataDefinedProperties().isActive( QgsAbstractMaterialSettings::Property::Ambient ) )
    processMaterialDatadefined( lineVertexData.vertices.size() - oldVerticesCount, context.expressionContext(), lineVertexData );

  mFeatureCount++;
}

void QgsThickLine3DSymbolHandler::finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context )
{
  // create entity for selected and not selected
  makeEntity( parent, context, mLineDataNormal, false );
  makeEntity( parent, context, mLineDataSelected, true );

  updateZRangeFromPositions( mLineDataNormal.vertices );
  updateZRangeFromPositions( mLineDataSelected.vertices );
}


void QgsThickLine3DSymbolHandler::makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, QgsLineVertexData &lineVertexData, bool selected )
{
  if ( lineVertexData.indexes.isEmpty() )
    return;

  // material (only ambient color is used for the color)
  QgsMaterialContext materialContext;
  materialContext.setIsSelected( selected );
  materialContext.setSelectionColor( context.selectionColor() );
  QgsMaterial *material = mSymbol->materialSettings()->toMaterial( QgsMaterialSettingsRenderingTechnique::Lines, materialContext );
  if ( !material )
  {
    const QgsSimpleLineMaterialSettings defaultMaterial;
    material = defaultMaterial.toMaterial( QgsMaterialSettingsRenderingTechnique::Lines, materialContext );
  }

  if ( QgsLineMaterial *lineMaterial = dynamic_cast<QgsLineMaterial *>( material ) )
    lineMaterial->setLineWidth( mSymbol->width() );

  Qt3DCore::QEntity *entity = new Qt3DCore::QEntity;

  // geometry renderer
  Qt3DRender::QGeometryRenderer *renderer = new Qt3DRender::QGeometryRenderer;
  renderer->setPrimitiveType( Qt3DRender::QGeometryRenderer::LineStripAdjacency );
  Qt3DQGeometry *geometry = lineVertexData.createGeometry( entity );

  if ( mSymbol->materialSettings()->dataDefinedProperties().isActive( QgsAbstractMaterialSettings::Property::Ambient ) )
    mSymbol->materialSettings()->applyDataDefinedToGeometry( geometry, lineVertexData.vertices.size(), lineVertexData.materialDataDefined );

  renderer->setGeometry( geometry );

  renderer->setVertexCount( lineVertexData.indexes.count() );
  renderer->setPrimitiveRestartEnabled( true );
  renderer->setRestartIndexValue( 0 );

  // add transform (our geometry has coordinates relative to mChunkOrigin)
  QgsGeoTransform *transform = new QgsGeoTransform;
  transform->setGeoTranslation( mChunkOrigin );

  // make entity
  entity->addComponent( renderer );
  entity->addComponent( material );
  entity->addComponent( transform );
  entity->setParent( parent );
}

void QgsThickLine3DSymbolHandler::processMaterialDatadefined( uint verticesCount, const QgsExpressionContext &context, QgsLineVertexData &lineVertexData )
{
  const QByteArray bytes = mSymbol->materialSettings()->dataDefinedVertexColorsAsByte( context );
  lineVertexData.materialDataDefined.append( bytes.repeated( static_cast<int>( verticesCount ) ) );
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
