/***************************************************************************
  qgsannotationlayerchunkloader_p.cpp
  --------------------------------------
  Date                 : September 2025
  Copyright            : (C) 2025 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsannotationlayerchunkloader_p.h"

#include "qgs3dutils.h"
#include "qgsabstract3dsymbol.h"
#include "qgsabstractterrainsettings.h"
#include "qgsannotationitem.h"
#include "qgsannotationlayer.h"
#include "qgsannotationlinetextitem.h"
#include "qgsannotationmarkeritem.h"
#include "qgsannotationpointtextitem.h"
#include "qgsannotationrectangletextitem.h"
#include "qgsbillboardgeometry.h"
#include "qgschunkloader.h"
#include "qgschunknode.h"
#include "qgseventtracing.h"
#include "qgsexpressioncontextutils.h"
#include "qgsfonttextureatlasgenerator.h"
#include "qgsgeos.h"
#include "qgsgeotransform.h"
#include "qgslinematerial_p.h"
#include "qgslinevertexdata_p.h"
#include "qgslogger.h"
#include "qgsmarkersymbol.h"
#include "qgspoint3dbillboardmaterial.h"
#include "qgstessellatedpolygongeometry.h"
#include "qgstextdocument.h"
#include "qgstextureatlasgenerator.h"

#include <QString>
#include <QTimer>
#include <Qt3DCore/QTransform>
#include <Qt3DRender/QGeometryRenderer>
#include <QtConcurrentRun>

using namespace Qt::StringLiterals;

///@cond PRIVATE

struct Billboard
{
    QVector3D position;
    int textureId = -1;
    const QgsMarkerSymbol *markerSymbol = nullptr;
};

struct TextBillboard
{
    QVector3D position;
    QString text;
};

QgsAnnotationLayerChunkLoader::QgsAnnotationLayerChunkLoader(
  const Qgs3DRenderContext &context,
  QgsAnnotationLayer *layer,
  int leafLevel,
  Qgis::AltitudeClamping clamping,
  double zOffset,
  bool showCallouts,
  const QColor &calloutLineColor,
  double calloutLineWidth,
  const QgsTextFormat &textFormat,
  double zMin,
  double zMax
)
  : mRenderContext( context )
  , mLayer( layer )
  , mLeafLevel( leafLevel )
  , mClamping( clamping )
  , mZOffset( zOffset )
  , mShowCallouts( showCallouts )
  , mCalloutLineColor( calloutLineColor )
  , mCalloutLineWidth( calloutLineWidth )
  , mTextFormat( textFormat )
{
  if ( context.crs().type() == Qgis::CrsType::Geocentric )
  {
    // TODO: add support for handling of annotation layers
    // (we're using dummy quadtree here to make sure the empty extent does not break the scene completely)
    QgsDebugError( u"Annotation layers in globe scenes are not supported yet!"_s );
    setupQuadtree( QgsBox3D( -1e7, -1e7, -1e7, 1e7, 1e7, 1e7 ), -1, leafLevel );
    return;
  }

  // choose the smaller root extent between context and mLayer ones:
  QgsRectangle extent = context.extent();
  const QgsRectangle layerExtentInMapCrs = Qgs3DUtils::tryReprojectExtent2D( mLayer->extent(), mLayer->crs(), context.crs(), context.transformContext() );
  if ( layerExtentInMapCrs.isValid() )
  {
    extent = context.extent().intersect( layerExtentInMapCrs );
  }
  if ( extent.isValid() )
  {
    QgsBox3D rootBox3D( extent, zMin, zMax );

    // add small padding to avoid clipping of point features located at the edge of the bounding box
    rootBox3D.grow( 1.0 );
    setupQuadtree( rootBox3D, -1, leafLevel ); // negative root error means that the node does not contain anything
  }
}

QFuture<QgsChunkLoaderResult> QgsAnnotationLayerChunkLoader::loadChunk( QgsChunkNode *node )
{
  if ( node->level() < mLeafLevel )
  {
    return QtFuture::makeReadyValueFuture<QgsChunkLoaderResult>( { [this, node]( Qt3DCore::QEntity *parent ) {
      Qt3DCore::QEntity *entity = new Qt3DCore::QEntity( parent ); // dummy entity
      entity->setObjectName( mLayer->name() + "_CONTAINER_" + node->tileId().text() );
      return entity;
    } } );
  }

  auto renderContext = mRenderContext; // Copy for constness

  // only a subset of data to be queried
  const QgsRectangle rect = node->box3D().toRectangle();
  // origin for coordinates of the chunk - it is kind of arbitrary, but it should be
  // picked so that the coordinates are relatively small to avoid numerical precision issues
  QgsVector3D chunkOrigin = QgsVector3D( rect.center().x(), rect.center().y(), 0 );

  QgsExpressionContext exprContext;
  exprContext.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( mLayer ) );
  renderContext.setExpressionContext( exprContext );

  QgsCoordinateTransform layerToMapTransform( mLayer->crs(), mRenderContext.crs(), mRenderContext.transformContext() );

  QgsRectangle layerExtent;
  try
  {
    layerExtent = layerToMapTransform.transformBoundingBox( rect, Qgis::TransformDirection::Reverse );
  }
  catch ( QgsCsException &e )
  {
    QgsDebugError( u"Error transforming annotation layer extent to 3d map extent: %1"_s.arg( e.what() ) );
    return {};
  }

  // see logic from QgsAnnotationLayerRenderer
  const QStringList itemsList = mLayer->queryIndex( layerExtent );
  QSet< QString > itemIds( itemsList.begin(), itemsList.end() );

  // we also have NO choice but to clone ALL non-indexed items (i.e. those with a scale-dependent bounding box)
  // since these won't be in the layer's spatial index, and it's too expensive to determine their actual bounding box
  // upfront (we are blocking the main thread right now!)

  // TODO -- come up with some brilliant way to avoid this and also index scale-dependent items ;)
  itemIds.unite( mLayer->mNonIndexedItems );

  std::vector< std::unique_ptr< QgsAnnotationItem > > itemsToRender;
  itemsToRender.reserve( itemIds.size() );
  std::transform( itemIds.begin(), itemIds.end(), std::back_inserter( itemsToRender ), [this]( const QString &id ) -> std::unique_ptr< QgsAnnotationItem > {
    return std::unique_ptr< QgsAnnotationItem >( mLayer->item( id )->clone() );
  } );

  //
  // this will be run in a background thread
  //
  return QtConcurrent::run( [this, node, rect, layerToMapTransform, itemsToRender = std::move( itemsToRender ), chunkOrigin]( QPromise<QgsChunkLoaderResult> &promise ) {
    const QgsScopedEvent e( u"3D"_s, u"Annotation layer chunk load"_s );

    QVector< QgsLineString > calloutLines;
    double zMin = std::numeric_limits< double >::max();
    double zMax = std::numeric_limits< double >::lowest();

    std::vector< Billboard > billboards;
    billboards.reserve( itemsToRender.size() );
    QVector< QImage > textures;
    textures.reserve( itemsToRender.size() );

    std::vector< TextBillboard > textBillboards;
    textBillboards.reserve( itemsToRender.size() );
    QStringList textBillboardTexts;
    textBillboardTexts.reserve( itemsToRender.size() );

    auto addTextBillboard = [layerToMapTransform,
                             rect,
                             this,
                             &textBillboards,
                             &textBillboardTexts,
                             &chunkOrigin,
                             &calloutLines,
                             &zMax,
                             &zMin]( const QgsPointXY &p, const QString &annotationText, const QgsTextFormat &annotationTextFormat ) {
      QString text = annotationText;
      if ( annotationTextFormat.allowHtmlFormatting() )
      {
        // strip HTML characters, we don't support those in 3D
        const QgsTextDocument document = QgsTextDocument::fromTextAndFormat( { text }, annotationTextFormat );
        text = document.toPlainText().join( ' ' );
      }
      if ( !text.isEmpty() )
      {
        try
        {
          const QgsPointXY mapPoint = layerToMapTransform.transform( p );
          if ( !rect.contains( mapPoint ) )
            return;

          double z = 0;
          const float terrainZ = ( mClamping == Qgis::AltitudeClamping::Absolute && !mShowCallouts ) ? 0
                                 : mRenderContext.terrainRenderingEnabled() && mRenderContext.terrainGenerator()
                                   ? static_cast<float>( mRenderContext.terrainGenerator()->heightAt( mapPoint.x(), mapPoint.y(), mRenderContext ) * mRenderContext.terrainSettings()->verticalScale() )
                                   : 0.f;

          switch ( mClamping )
          {
            case Qgis::AltitudeClamping::Absolute:
              z = mZOffset;
              break;
            case Qgis::AltitudeClamping::Terrain:
              z = terrainZ;
              break;
            case Qgis::AltitudeClamping::Relative:
              z = terrainZ + mZOffset;
              break;
          }

          TextBillboard billboard;
          billboard.position = ( QgsVector3D( mapPoint.x(), mapPoint.y(), z ) - chunkOrigin ).toVector3D();
          billboard.text = text;
          textBillboards.emplace_back( std::move( billboard ) );
          textBillboardTexts.append( text );

          if ( mShowCallouts )
          {
            calloutLines << QgsLineString( { mapPoint.x(), mapPoint.x() }, { mapPoint.y(), mapPoint.y() }, { terrainZ, z } );
          }

          zMax = std::max( zMax, mShowCallouts ? std::max( 0.0, z ) : z );
          zMin = std::min( zMin, mShowCallouts ? std::min( 0.0, z ) : z );
        }
        catch ( QgsCsException &e )
        {
          QgsDebugError( e.what() );
        }
      }
    };

    for ( const std::unique_ptr< QgsAnnotationItem > &item : std::as_const( itemsToRender ) )
    {
      if ( promise.isCanceled() )
        break;

      QgsAnnotationItem *annotation = item.get();

      if ( !annotation->enabled() )
        continue;

      if ( QgsAnnotationMarkerItem *marker = dynamic_cast< QgsAnnotationMarkerItem * >( annotation ) )
      {
        if ( marker->symbol() )
        {
          QgsPointXY p = marker->geometry();
          try
          {
            const QgsPointXY mapPoint = layerToMapTransform.transform( p );
            if ( !rect.contains( mapPoint ) )
              continue;

            double z = 0;
            const float terrainZ = ( mClamping == Qgis::AltitudeClamping::Absolute && !mShowCallouts ) ? 0
                                   : mRenderContext.terrainRenderingEnabled() && mRenderContext.terrainGenerator()
                                     ? static_cast<float>( mRenderContext.terrainGenerator()->heightAt( mapPoint.x(), mapPoint.y(), mRenderContext ) * mRenderContext.terrainSettings()->verticalScale() )
                                     : 0.f;

            switch ( mClamping )
            {
              case Qgis::AltitudeClamping::Absolute:
                z = mZOffset;
                break;
              case Qgis::AltitudeClamping::Terrain:
                z = terrainZ;
                break;
              case Qgis::AltitudeClamping::Relative:
                z = terrainZ + mZOffset;
                break;
            }

            Billboard billboard;
            billboard.position = ( QgsVector3D( mapPoint.x(), mapPoint.y(), z ) - chunkOrigin ).toVector3D();
            billboard.textureId = -1;

            for ( const Billboard &existingBillboard : billboards )
            {
              if ( existingBillboard.markerSymbol && marker->symbol()->rendersIdenticallyTo( existingBillboard.markerSymbol ) )
              {
                // marker symbol has been reused => reuse existing texture to minimize size of texture atlas
                billboard.textureId = existingBillboard.textureId;
                break;
              }
            }

            if ( billboard.textureId < 0 )
            {
              // could not match to previously considered marker, have to render and add to texture atlas
              billboard.markerSymbol = marker->symbol();
              billboard.textureId = textures.size();
              textures.append( QgsPoint3DBillboardMaterial::renderSymbolToImage( marker->symbol(), mRenderContext ) );
            }
            billboards.emplace_back( std::move( billboard ) );

            if ( mShowCallouts )
            {
              calloutLines << QgsLineString( { mapPoint.x(), mapPoint.x() }, { mapPoint.y(), mapPoint.y() }, { terrainZ, z } );
            }

            zMax = std::max( zMax, mShowCallouts ? std::max( 0.0, z ) : z );
            zMin = std::min( zMin, mShowCallouts ? std::min( 0.0, z ) : z );
          }
          catch ( QgsCsException &e )
          {
            QgsDebugError( e.what() );
          }
        }
      }
      else if ( QgsAnnotationPointTextItem *pointText = dynamic_cast< QgsAnnotationPointTextItem * >( annotation ) )
      {
        addTextBillboard( pointText->point(), pointText->text(), pointText->format() );
      }
      else if ( QgsAnnotationLineTextItem *lineText = dynamic_cast< QgsAnnotationLineTextItem * >( annotation ) )
      {
        QgsGeos geos( lineText->geometry() );
        std::unique_ptr< QgsPoint > point( geos.pointOnSurface() );
        if ( point )
        {
          addTextBillboard( *point, lineText->text(), lineText->format() );
        }
      }
      else if ( QgsAnnotationRectangleTextItem *rectText = dynamic_cast< QgsAnnotationRectangleTextItem * >( annotation ) )
      {
        switch ( rectText->placementMode() )
        {
          case Qgis::AnnotationPlacementMode::SpatialBounds:
          case Qgis::AnnotationPlacementMode::FixedSize:
          {
            addTextBillboard( rectText->bounds().center(), rectText->text(), rectText->format() );
            break;
          }
          case Qgis::AnnotationPlacementMode::RelativeToMapFrame:
            // ignore these annotations, they don't have a fix map position
            break;
        }
      }
    }

    QImage billboardAtlas;
    QVector< QgsBillboardGeometry::BillboardAtlasData > billboardPositions;
    if ( !textures.isEmpty() )
    {
      const QgsTextureAtlas atlas = QgsTextureAtlasGenerator::createFromImages( textures, 2048 );
      if ( atlas.isValid() )
      {
        billboardAtlas = atlas.renderAtlasTexture();
        billboardPositions.reserve( static_cast< int >( billboards.size() ) );
        for ( Billboard &billboard : billboards )
        {
          const QRect textureRect = atlas.rect( billboard.textureId );
          QgsBillboardGeometry::BillboardAtlasData geometry;
          geometry.position = billboard.position;
          geometry.textureAtlasOffset = QVector2D(
            static_cast< float >( textureRect.left() ) / static_cast< float>( billboardAtlas.width() ),
            1 - ( static_cast< float >( textureRect.bottom() ) / static_cast< float>( billboardAtlas.height() ) )
          );
          geometry.textureAtlasSize
            = QVector2D( static_cast< float >( textureRect.width() ) / static_cast< float>( billboardAtlas.width() ), static_cast< float>( textureRect.height() ) / static_cast< float>( billboardAtlas.height() ) );
          geometry.pixelOffset = QPoint( 0, textureRect.height() / 2 );
          billboardPositions.append( geometry );
        }
      }
      else
      {
        QgsDebugError( u"Error encountered building texture atlas"_s );
        billboardAtlas = QImage();
      }
    }
    else
    {
      billboardAtlas = QImage();
      billboardPositions.clear();
    }

    QImage textBillboardAtlas;
    QVector< QgsBillboardGeometry::BillboardAtlasData > textBillboardPositions;
    if ( !textBillboardTexts.isEmpty() )
    {
      const QgsFontTextureAtlas atlas = QgsFontTextureAtlasGenerator::create( mTextFormat, textBillboardTexts );
      if ( atlas.isValid() )
      {
        textBillboardAtlas = atlas.renderAtlasTexture();
        textBillboardPositions.reserve( static_cast< int >( textBillboards.size() ) );
        for ( TextBillboard &billboard : textBillboards )
        {
          int graphemeIndex = 0;
          const int graphemeCount = atlas.graphemeCount( billboard.text );
          // horizontally center text over point
          const double xOffset = atlas.totalWidth( billboard.text ) / 2.0;
          for ( ; graphemeIndex < graphemeCount; ++graphemeIndex )
          {
            const QRect textureRect = atlas.textureRectForGrapheme( billboard.text, graphemeIndex );
            QgsBillboardGeometry::BillboardAtlasData geometry;
            geometry.position = billboard.position;
            geometry.textureAtlasOffset = QVector2D(
              static_cast< float >( textureRect.left() ) / static_cast< float>( textBillboardAtlas.width() ),
              1 - ( static_cast< float >( textureRect.bottom() ) / static_cast< float>( textBillboardAtlas.height() ) )
            );
            geometry.textureAtlasSize = QVector2D(
              static_cast< float >( textureRect.width() ) / static_cast< float>( textBillboardAtlas.width() ),
              static_cast< float>( textureRect.height() ) / static_cast< float>( textBillboardAtlas.height() )
            );
            const QPointF pixelOffset = atlas.pixelOffsetForGrapheme( billboard.text, graphemeIndex );
            geometry.pixelOffset
              = QPoint( static_cast< int >( std::round( -xOffset + pixelOffset.x() + 0.5 * textureRect.width() ) ), static_cast< int >( std::round( pixelOffset.y() + 0.5 * textureRect.height() ) ) );
            textBillboardPositions.append( geometry );
          }
        }
      }
      else
      {
        QgsDebugError( u"Error encountered building font texture atlas"_s );
        textBillboardAtlas = QImage();
      }
    }
    else
    {
      textBillboardAtlas = QImage();
      textBillboardPositions.clear();
    }

    promise.addResult(
      { [billboardPositions, textBillboardPositions, node, this, billboardAtlas, textBillboardAtlas, chunkOrigin, calloutLines, zMax, zMin]( Qt3DCore::QEntity *parent ) -> Qt3DCore::QEntity * {
        if ( billboardPositions.empty() && textBillboardPositions.empty() )
        {
          // an empty node, so we return no entity. This tags the node as having no data and effectively removes it.
          // we just make sure first that its initial estimated vertical range does not affect its parents' bboxes calculation
          node->setExactBox3D( QgsBox3D() );
          node->updateParentBoundingBoxesRecursively();
          return nullptr;
        }

        Qt3DCore::QEntity *entity = new Qt3DCore::QEntity( parent );
        entity->setObjectName( mLayer->name() + "_" + node->tileId().text() );

        QgsGeoTransform *billboardTransform = new QgsGeoTransform;
        billboardTransform->setGeoTranslation( chunkOrigin );
        entity->addComponent( billboardTransform );

        if ( !billboardPositions.empty() )
        {
          QgsBillboardGeometry *billboardGeometry = new QgsBillboardGeometry();
          billboardGeometry->setBillboardData( billboardPositions, true );

          Qt3DRender::QGeometryRenderer *billboardGeometryRenderer = new Qt3DRender::QGeometryRenderer;
          billboardGeometryRenderer->setPrimitiveType( Qt3DRender::QGeometryRenderer::TriangleStrip );
          billboardGeometryRenderer->setGeometry( billboardGeometry );
          billboardGeometryRenderer->setVertexCount( 4 );
          billboardGeometryRenderer->setInstanceCount( billboardGeometry->count() );

          QgsPoint3DBillboardMaterial *billboardMaterial = new QgsPoint3DBillboardMaterial( QgsPoint3DBillboardMaterial::Mode::AtlasTextureWithPixelOffsets );
          billboardMaterial->setTexture2DFromImage( billboardAtlas );


          Qt3DCore::QEntity *billboardEntity = new Qt3DCore::QEntity;
          billboardEntity->addComponent( billboardMaterial );
          billboardEntity->addComponent( billboardGeometryRenderer );
          billboardEntity->setParent( entity );
        }

        if ( !textBillboardPositions.empty() )
        {
          QgsBillboardGeometry *textBillboardGeometry = new QgsBillboardGeometry();
          textBillboardGeometry->setBillboardData( textBillboardPositions, true );

          Qt3DRender::QGeometryRenderer *billboardGeometryRenderer = new Qt3DRender::QGeometryRenderer;
          billboardGeometryRenderer->setPrimitiveType( Qt3DRender::QGeometryRenderer::TriangleStrip );
          billboardGeometryRenderer->setGeometry( textBillboardGeometry );
          billboardGeometryRenderer->setVertexCount( 4 );
          billboardGeometryRenderer->setInstanceCount( textBillboardGeometry->count() );

          QgsPoint3DBillboardMaterial *billboardMaterial = new QgsPoint3DBillboardMaterial( QgsPoint3DBillboardMaterial::Mode::AtlasTextureWithPixelOffsets );
          billboardMaterial->setTexture2DFromImage( textBillboardAtlas );

          Qt3DCore::QEntity *billboardEntity = new Qt3DCore::QEntity;
          billboardEntity->addComponent( billboardMaterial );
          billboardEntity->addComponent( billboardGeometryRenderer );
          billboardEntity->setParent( entity );
        }


        if ( mShowCallouts )
        {
          QgsLineVertexData lineData;
          lineData.withAdjacency = true;
          lineData.geocentricCoordinates = false; // mMapSettings->sceneMode() == Qgis::SceneMode::Globe;
          lineData.init( Qgis::AltitudeClamping::Absolute, Qgis::AltitudeBinding::Vertex, 0, mRenderContext, chunkOrigin );

          for ( const QgsLineString &line : calloutLines )
          {
            lineData.addLineString( line, 0, false );
          }

          QgsLineMaterial *mat = new QgsLineMaterial;
          mat->setLineColor( mCalloutLineColor );
          mat->setLineWidth( mCalloutLineWidth );

          Qt3DCore::QEntity *calloutEntity = new Qt3DCore::QEntity;
          calloutEntity->setObjectName( parent->objectName() + "_CALLOUTS" );

          // geometry renderer
          Qt3DRender::QGeometryRenderer *calloutRenderer = new Qt3DRender::QGeometryRenderer;
          calloutRenderer->setPrimitiveType( Qt3DRender::QGeometryRenderer::LineStripAdjacency );
          calloutRenderer->setGeometry( lineData.createGeometry( calloutEntity ) );
          calloutRenderer->setVertexCount( lineData.indexes.count() );
          calloutRenderer->setPrimitiveRestartEnabled( true );
          calloutRenderer->setRestartIndexValue( 0 );

          // make entity
          calloutEntity->addComponent( calloutRenderer );
          calloutEntity->addComponent( mat );

          calloutEntity->setParent( entity );
        }

        // fix the vertical range of the node from the estimated vertical range to the true range
        if ( zMin != std::numeric_limits<float>::max() && zMax != std::numeric_limits<float>::lowest() )
        {
          QgsBox3D box = node->box3D();
          box.setZMinimum( zMin );
          box.setZMaximum( zMax );
          node->setExactBox3D( box );
          node->updateParentBoundingBoxesRecursively();
        }
        return entity;
      } }
    );
  } );
}


///////////////

QgsAnnotationLayerChunkedEntity::QgsAnnotationLayerChunkedEntity(
  Qgs3DMapSettings *map,
  QgsAnnotationLayer *layer,
  Qgis::AltitudeClamping clamping,
  double zOffset,
  bool showCallouts,
  const QColor &calloutLineColor,
  double calloutLineWidth,
  const QgsTextFormat &textFormat,
  double zMin,
  double zMax
)
  : QgsAbstractFeatureBasedChunkedEntity(
      map,
      -1, // max. allowed screen error (negative tau means that we need to go until leaves are reached)
      new QgsAnnotationLayerChunkLoader( Qgs3DRenderContext::fromMapSettings( map ), layer, 3, clamping, zOffset, showCallouts, calloutLineColor, calloutLineWidth, textFormat, zMin, zMax ),
      true
    )
{
  onTerrainElevationOffsetChanged();
}

QgsAnnotationLayerChunkedEntity::~QgsAnnotationLayerChunkedEntity()
{
  // cancel / wait for jobs
  cancelActiveJobs();
}

// if the AltitudeClamping is `Absolute`, do not apply the offset
bool QgsAnnotationLayerChunkedEntity::applyTerrainOffset() const
{
  if ( auto loader = static_cast<QgsAnnotationLayerChunkLoader *>( mChunkLoader ) )
  {
    return loader->mClamping != Qgis::AltitudeClamping::Absolute;
  }
  return true;
}

QList<QgsRayCastHit> QgsAnnotationLayerChunkedEntity::rayIntersection( const QgsRay3D &ray, const QgsRayCastContext &context ) const
{
  Q_UNUSED( ray )
  Q_UNUSED( context )
  return {};
}

/// @endcond
