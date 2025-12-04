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

#include "qgs3drendercontext.h"
#include "qgs3dutils.h"
#include "qgsgeotransform.h"
#include "qgslinematerial_p.h"
#include "qgslinestring.h"
#include "qgslinevertexdata_p.h"
#include "qgsmessagelog.h"
#include "qgsmultipolygon.h"
#include "qgsphongtexturedmaterialsettings.h"
#include "qgspolygon.h"
#include "qgspolygon3dsymbol.h"
#include "qgspolyhedralsurface.h"
#include "qgstessellatedpolygongeometry.h"
#include "qgstessellator.h"
#include "qgsvectorlayer.h"

#include <Qt3DExtras/QDiffuseMapMaterial>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DRender/QAbstractTextureImage>
#include <Qt3DRender/QCullFace>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QTexture>

/// @cond PRIVATE


class QgsPolygon3DSymbolHandler : public QgsFeature3DHandler
{
  public:
    QgsPolygon3DSymbolHandler( const QgsPolygon3DSymbol *symbol, const QgsFeatureIds &selectedIds )
      : mSymbol( static_cast<QgsPolygon3DSymbol *>( symbol->clone() ) )
      , mSelectedIds( selectedIds ) {}

    bool prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames, const QgsVector3D &chunkOrigin ) override;
    void processFeature( const QgsFeature &f, const Qgs3DRenderContext &context ) override;
    QList<Qt3DCore::QEntity *> finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context ) override;

  private:
    //! temporary data we will pass to the tessellator
    struct PolygonData
    {
        std::unique_ptr<QgsTessellator> tessellator;
        QVector<QgsFeatureId> triangleIndexFids;
        QVector<uint> triangleIndexStartingIndices;
        QByteArray materialDataDefined;
    };

    void processPolygon( const QgsPolygon *poly, QgsFeatureId fid, float offset, float extrusionHeight, const Qgs3DRenderContext &context, PolygonData &out );
    void processMaterialDatadefined( uint verticesCount, const QgsExpressionContext &context, PolygonData &out );
    Qt3DCore::QEntity *makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, PolygonData &out, bool selected );
    QgsMaterial *material( const QgsPolygon3DSymbol *symbol, bool isSelected, const Qgs3DRenderContext &context ) const;

    // input specific for this class
    std::unique_ptr<QgsPolygon3DSymbol> mSymbol;
    // inputs - generic
    QgsFeatureIds mSelectedIds;

    //! origin (in the map coordinates) for output geometries (e.g. at the center of the chunk)
    QgsVector3D mChunkOrigin;

    // outputs
    PolygonData outNormal;   //!< Features that are not selected
    PolygonData outSelected; //!< Features that are selected

    QgsLineVertexData outEdges; //!< When highlighting edges, this holds data for vertex/index buffer
};


bool QgsPolygon3DSymbolHandler::prepare( const Qgs3DRenderContext &context, QSet<QString> &attributeNames, const QgsVector3D &chunkOrigin )
{
  outEdges.withAdjacency = true;
  outEdges.init( mSymbol->altitudeClamping(), mSymbol->altitudeBinding(), 0, context, chunkOrigin );

  mChunkOrigin = chunkOrigin;

  const QgsPhongTexturedMaterialSettings *texturedMaterialSettings = dynamic_cast<const QgsPhongTexturedMaterialSettings *>( mSymbol->materialSettings() );

  auto tessellator = std::make_unique<QgsTessellator>();
  tessellator->setOrigin( chunkOrigin );
  tessellator->setAddNormals( true );
  tessellator->setInvertNormals( mSymbol->invertNormals() );
  tessellator->setBackFacesEnabled( mSymbol->addBackFaces() );
  tessellator->setOutputZUp( true );
  tessellator->setExtrusionFaces( mSymbol->extrusionFaces() );
  tessellator->setTextureRotation( texturedMaterialSettings ? static_cast<float>( texturedMaterialSettings->textureRotation() ) : 0.f );
  tessellator->setAddTextureUVs( texturedMaterialSettings && texturedMaterialSettings->requiresTextureCoordinates() );
  tessellator->setOutputZUp( true );

  outNormal.tessellator = std::move( tessellator );

  tessellator = std::make_unique<QgsTessellator>();
  tessellator->setOrigin( chunkOrigin );
  tessellator->setAddNormals( true );
  tessellator->setInvertNormals( mSymbol->invertNormals() );
  tessellator->setBackFacesEnabled( mSymbol->addBackFaces() );
  tessellator->setOutputZUp( true );
  tessellator->setExtrusionFaces( mSymbol->extrusionFaces() );
  tessellator->setTextureRotation( texturedMaterialSettings ? static_cast<float>( texturedMaterialSettings->textureRotation() ) : 0.f );
  tessellator->setAddTextureUVs( texturedMaterialSettings && texturedMaterialSettings->requiresTextureCoordinates() );
  tessellator->setOutputZUp( true );

  outSelected.tessellator = std::move( tessellator );

  QSet<QString> attrs = mSymbol->dataDefinedProperties().referencedFields( context.expressionContext() );
  attributeNames.unite( attrs );
  attrs = mSymbol->materialSettings()->dataDefinedProperties().referencedFields( context.expressionContext() );
  attributeNames.unite( attrs );
  return true;
}

void QgsPolygon3DSymbolHandler::processPolygon( const QgsPolygon *poly, QgsFeatureId fid, float offset, float extrusionHeight, const Qgs3DRenderContext &context, PolygonData &out )
{
  std::unique_ptr<QgsPolygon> polyClone( poly->clone() );

  const uint oldVerticesCount = out.tessellator->dataVerticesCount();
  if ( mSymbol->edgesEnabled() )
  {
    // add edges before the polygon gets the Z values modified because addLineString() does its own altitude handling
    outEdges.addLineString( *static_cast<const QgsLineString *>( polyClone->exteriorRing() ), offset );
    for ( int i = 0; i < polyClone->numInteriorRings(); ++i )
      outEdges.addLineString( *static_cast<const QgsLineString *>( polyClone->interiorRing( i ) ), offset );

    if ( extrusionHeight )
    {
      // add roof and wall edges
      const QgsLineString *exterior = static_cast<const QgsLineString *>( polyClone->exteriorRing() );
      outEdges.addLineString( *exterior, extrusionHeight + offset );
      outEdges.addVerticalLines( *exterior, extrusionHeight, offset );
      for ( int i = 0; i < polyClone->numInteriorRings(); ++i )
      {
        const QgsLineString *interior = static_cast<const QgsLineString *>( polyClone->interiorRing( i ) );
        outEdges.addLineString( *interior, extrusionHeight + offset );
        outEdges.addVerticalLines( *interior, extrusionHeight, offset );
      }
    }
  }

  Qgs3DUtils::clampAltitudes( polyClone.get(), mSymbol->altitudeClamping(), mSymbol->altitudeBinding(), offset, context );

  Q_ASSERT( out.tessellator->dataVerticesCount() % 3 == 0 );
  const uint startingTriangleIndex = static_cast<uint>( out.tessellator->dataVerticesCount() / 3 );
  out.triangleIndexStartingIndices.append( startingTriangleIndex );
  out.triangleIndexFids.append( fid );
  out.tessellator->addPolygon( *polyClone, extrusionHeight );
  if ( !out.tessellator->error().isEmpty() )
  {
    QgsMessageLog::logMessage( out.tessellator->error(), QObject::tr( "3D" ) );
  }

  if ( mSymbol->materialSettings()->dataDefinedProperties().hasActiveProperties() )
    processMaterialDatadefined( out.tessellator->dataVerticesCount() - oldVerticesCount, context.expressionContext(), out );
}

void QgsPolygon3DSymbolHandler::processMaterialDatadefined( uint verticesCount, const QgsExpressionContext &context, QgsPolygon3DSymbolHandler::PolygonData &out )
{
  const QByteArray bytes = mSymbol->materialSettings()->dataDefinedVertexColorsAsByte( context );
  out.materialDataDefined.append( bytes.repeated( verticesCount ) );
}

void QgsPolygon3DSymbolHandler::processFeature( const QgsFeature &f, const Qgs3DRenderContext &context )
{
  if ( f.geometry().isNull() )
    return;

  PolygonData &out = mSelectedIds.contains( f.id() ) ? outSelected : outNormal;

  QgsGeometry geom = f.geometry();
  const QgsAbstractGeometry *g = geom.constGet()->simplifiedTypeRef();

  // segmentize curved geometries if necessary
  if ( QgsWkbTypes::isCurvedType( g->wkbType() ) )
  {
    geom = QgsGeometry( g->segmentize() );
    g = geom.constGet()->simplifiedTypeRef();
  }

  const QgsPropertyCollection &ddp = mSymbol->dataDefinedProperties();
  const bool hasDDHeight = ddp.isActive( QgsAbstract3DSymbol::Property::Height );
  const bool hasDDExtrusion = ddp.isActive( QgsAbstract3DSymbol::Property::ExtrusionHeight );

  float offset = mSymbol->offset();
  float extrusionHeight = mSymbol->extrusionHeight();
  if ( hasDDHeight )
    offset = static_cast<float>( ddp.valueAsDouble( QgsAbstract3DSymbol::Property::Height, context.expressionContext(), offset ) );
  if ( hasDDExtrusion )
    extrusionHeight = ddp.valueAsDouble( QgsAbstract3DSymbol::Property::ExtrusionHeight, context.expressionContext(), extrusionHeight );

  if ( const QgsPolygon *poly = qgsgeometry_cast<const QgsPolygon *>( g ) )
  {
    processPolygon( poly, f.id(), offset, extrusionHeight, context, out );
  }
  else if ( const QgsMultiPolygon *mpoly = qgsgeometry_cast<const QgsMultiPolygon *>( g ) )
  {
    for ( int i = 0; i < mpoly->numGeometries(); ++i )
    {
      const QgsPolygon *poly = mpoly->polygonN( i );
      processPolygon( poly, f.id(), offset, extrusionHeight, context, out );
    }
  }
  else if ( const QgsGeometryCollection *gc = qgsgeometry_cast<const QgsGeometryCollection *>( g ) )
  {
    for ( int i = 0; i < gc->numGeometries(); ++i )
    {
      const QgsAbstractGeometry *g2 = gc->geometryN( i );
      if ( QgsWkbTypes::flatType( g2->wkbType() ) == Qgis::WkbType::Polygon )
      {
        const QgsPolygon *poly = static_cast<const QgsPolygon *>( g2 );
        processPolygon( poly, f.id(), offset, extrusionHeight, context, out );
      }
    }
  }
  else if ( const QgsPolyhedralSurface *polySurface = qgsgeometry_cast<const QgsPolyhedralSurface *>( g ) )
  {
    for ( int i = 0; i < polySurface->numPatches(); ++i )
    {
      const QgsPolygon *poly = polySurface->patchN( i );
      processPolygon( poly, f.id(), offset, extrusionHeight, context, out );
    }
  }
  else
    qWarning() << "not a polygon";

  mFeatureCount++;
}

QList<Qt3DCore::QEntity *> QgsPolygon3DSymbolHandler::finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context )
{
  QList<Qt3DCore::QEntity *> createdEntities;
  // create entity for selected and not selected
  Qt3DCore::QEntity *normalEntity = makeEntity( parent, context, outNormal, false );
  if ( normalEntity )
  {
    createdEntities.append( normalEntity );
  }

  Qt3DCore::QEntity *selectedEntity = makeEntity( parent, context, outSelected, true );
  if ( selectedEntity )
  {
    createdEntities.append( selectedEntity );
  }

  mZMin = std::min( outNormal.tessellator->zMinimum(), outSelected.tessellator->zMinimum() );
  mZMax = std::max( outNormal.tessellator->zMaximum(), outSelected.tessellator->zMaximum() );

  // add entity for edges
  if ( mSymbol->edgesEnabled() && !outEdges.indexes.isEmpty() )
  {
    QgsLineMaterial *mat = new QgsLineMaterial;
    mat->setLineColor( mSymbol->edgeColor() );
    mat->setLineWidth( mSymbol->edgeWidth() );

    Qt3DCore::QEntity *entity = new Qt3DCore::QEntity;
    entity->setObjectName( parent->objectName() + "_EDGES" );

    // geometry renderer
    Qt3DRender::QGeometryRenderer *renderer = new Qt3DRender::QGeometryRenderer;
    renderer->setPrimitiveType( Qt3DRender::QGeometryRenderer::LineStripAdjacency );
    renderer->setGeometry( outEdges.createGeometry( entity ) );
    renderer->setVertexCount( outEdges.indexes.count() );
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

    createdEntities.append( entity );
  }

  return createdEntities;
}


Qt3DCore::QEntity *QgsPolygon3DSymbolHandler::makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, PolygonData &polyData, bool selected )
{
  if ( polyData.tessellator->dataVerticesCount() == 0 )
    return nullptr; // nothing to show - no need to create the entity

  QgsMaterial *mat = material( mSymbol.get(), selected, context );

  // extract vertex buffer data from tessellator
  const QByteArray data( reinterpret_cast<const char *>( polyData.tessellator->data().constData() ), static_cast<int>( polyData.tessellator->data().count() * sizeof( float ) ) );
  const int nVerts = data.count() / polyData.tessellator->stride();

  const QgsPhongTexturedMaterialSettings *texturedMaterialSettings = dynamic_cast<const QgsPhongTexturedMaterialSettings *>( mSymbol->materialSettings() );

  QgsTessellatedPolygonGeometry *geometry = new QgsTessellatedPolygonGeometry( true, mSymbol->invertNormals(), mSymbol->addBackFaces(), texturedMaterialSettings && texturedMaterialSettings->requiresTextureCoordinates() );

  geometry->setData( data, nVerts, polyData.triangleIndexFids, polyData.triangleIndexStartingIndices );

  if ( mSymbol->materialSettings()->dataDefinedProperties().hasActiveProperties() )
    mSymbol->materialSettings()->applyDataDefinedToGeometry( geometry, nVerts, polyData.materialDataDefined );

  Qt3DRender::QGeometryRenderer *renderer = new Qt3DRender::QGeometryRenderer;
  renderer->setGeometry( geometry );

  // add transform (our geometry has coordinates relative to mChunkOrigin)
  QgsGeoTransform *tr = new QgsGeoTransform;
  tr->setGeoTranslation( mChunkOrigin );

  // make entity
  Qt3DCore::QEntity *entity = new Qt3DCore::QEntity;
  entity->setObjectName( parent->objectName() + "_CHUNK_MESH" );
  entity->addComponent( renderer );
  entity->addComponent( mat );
  entity->addComponent( tr );
  entity->setParent( parent );

  if ( !selected )
    renderer->setProperty( Qgs3DTypes::PROP_NAME_3D_RENDERER_FLAG, Qgs3DTypes::Main3DRenderer ); // temporary measure to distinguish between "selected" and "main"

  return entity;
}

// front/back side culling
static void applyCullingMode( Qgs3DTypes::CullingMode cullingMode, QgsMaterial *material )
{
  const auto techniques = material->effect()->techniques();
  for ( auto tit = techniques.constBegin(); tit != techniques.constEnd(); ++tit )
  {
    auto renderPasses = ( *tit )->renderPasses();
    for ( auto rpit = renderPasses.begin(); rpit != renderPasses.end(); ++rpit )
    {
      Qt3DRender::QCullFace *cullFace = new Qt3DRender::QCullFace;
      cullFace->setMode( Qgs3DUtils::qt3DcullingMode( cullingMode ) );
      ( *rpit )->addRenderState( cullFace );
    }
  }
}

QgsMaterial *QgsPolygon3DSymbolHandler::material( const QgsPolygon3DSymbol *symbol, bool isSelected, const Qgs3DRenderContext &context ) const
{
  QgsMaterialContext materialContext;
  materialContext.setIsSelected( isSelected );
  materialContext.setSelectionColor( context.selectionColor() );

  const bool dataDefined = mSymbol->materialSettings()->dataDefinedProperties().hasActiveProperties();
  QgsMaterial *material = symbol->materialSettings()->toMaterial( dataDefined ? QgsMaterialSettingsRenderingTechnique::TrianglesDataDefined : QgsMaterialSettingsRenderingTechnique::Triangles, materialContext );
  applyCullingMode( symbol->cullingMode(), material );
  return material;
}


// --------------


namespace Qgs3DSymbolImpl
{


  QgsFeature3DHandler *handlerForPolygon3DSymbol( QgsVectorLayer *layer, const QgsAbstract3DSymbol *symbol )
  {
    const QgsPolygon3DSymbol *polygonSymbol = dynamic_cast<const QgsPolygon3DSymbol *>( symbol );
    if ( !polygonSymbol )
      return nullptr;

    return new QgsPolygon3DSymbolHandler( polygonSymbol, layer->selectedFeatureIds() );
  }
} // namespace Qgs3DSymbolImpl

/// @endcond
