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

#include <Qt3DCore/QTransform>
#include <QtConcurrent>

#include "moc_qgsannotationlayerchunkloader_p.cpp"

///@cond PRIVATE


QgsAnnotationLayerChunkLoader::QgsAnnotationLayerChunkLoader( const QgsAnnotationLayerChunkLoaderFactory *factory, QgsChunkNode *node )
  : QgsChunkLoader( node )
  , mFactory( factory )
  , mRenderContext( factory->mRenderContext )
{
}


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


void QgsAnnotationLayerChunkLoader::start()
{
  QgsChunkNode *node = chunk();
  if ( node->level() < mFactory->mLeafLevel )
  {
    QTimer::singleShot( 0, this, &QgsAnnotationLayerChunkLoader::finished );
    return;
  }

  QgsAnnotationLayer *layer = mFactory->mLayer;
  mLayerName = mFactory->mLayer->name();

  // only a subset of data to be queried
  const QgsRectangle rect = node->box3D().toRectangle();
  // origin for coordinates of the chunk - it is kind of arbitrary, but it should be
  // picked so that the coordinates are relatively small to avoid numerical precision issues
  mChunkOrigin = QgsVector3D( rect.center().x(), rect.center().y(), 0 );

  QgsExpressionContext exprContext;
  exprContext.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) );
  mRenderContext.setExpressionContext( exprContext );

  QgsCoordinateTransform layerToMapTransform( layer->crs(), mRenderContext.crs(), mRenderContext.transformContext() );

  QgsRectangle layerExtent;
  try
  {
    layerExtent = layerToMapTransform.transformBoundingBox( rect, Qgis::TransformDirection::Reverse );
  }
  catch ( QgsCsException &e )
  {
    QgsDebugError( u"Error transforming annotation layer extent to 3d map extent: %1"_s.arg( e.what() ) );
    return;
  }

  const double zOffset = mFactory->mZOffset;
  const Qgis::AltitudeClamping altitudeClamping = mFactory->mClamping;
  bool showCallouts = mFactory->mShowCallouts;
  const QgsTextFormat textFormat = mFactory->mTextFormat;

  // see logic from QgsAnnotationLayerRenderer
  const QStringList itemsList = layer->queryIndex( layerExtent );
  QSet< QString > itemIds( itemsList.begin(), itemsList.end() );

  // we also have NO choice but to clone ALL non-indexed items (i.e. those with a scale-dependent bounding box)
  // since these won't be in the layer's spatial index, and it's too expensive to determine their actual bounding box
  // upfront (we are blocking the main thread right now!)

  // TODO -- come up with some brilliant way to avoid this and also index scale-dependent items ;)
  itemIds.unite( layer->mNonIndexedItems );

  mItemsToRender.reserve( itemIds.size() );
  std::transform( itemIds.begin(), itemIds.end(), std::back_inserter( mItemsToRender ), [layer]( const QString &id ) -> std::unique_ptr< QgsAnnotationItem > {
    return std::unique_ptr< QgsAnnotationItem >( layer->item( id )->clone() );
  } );

  //
  // this will be run in a background thread
  //
  mFutureWatcher = new QFutureWatcher<void>( this );
  connect( mFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsChunkQueueJob::finished );

  const QFuture<void> future = QtConcurrent::run( [this, rect, layerToMapTransform, zOffset, altitudeClamping, showCallouts, textFormat] {
    const QgsEventTracing::ScopedEvent e( u"3D"_s, u"Annotation layer chunk load"_s );

    std::vector< Billboard > billboards;
    billboards.reserve( mItemsToRender.size() );
    QVector< QImage > textures;
    textures.reserve( mItemsToRender.size() );

    std::vector< TextBillboard > textBillboards;
    textBillboards.reserve( mItemsToRender.size() );
    QStringList textBillboardTexts;
    textBillboardTexts.reserve( mItemsToRender.size() );

    auto addTextBillboard = [layerToMapTransform, showCallouts, rect, zOffset, altitudeClamping, this, &textBillboards, &textBillboardTexts]( const QgsPointXY &p, const QString &annotationText, const QgsTextFormat &annotationTextFormat ) {
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
          const float terrainZ = ( altitudeClamping == Qgis::AltitudeClamping::Absolute && !showCallouts ) ? 0 : mRenderContext.terrainRenderingEnabled() && mRenderContext.terrainGenerator() ? static_cast<float>( mRenderContext.terrainGenerator()->heightAt( mapPoint.x(), mapPoint.y(), mRenderContext ) * mRenderContext.terrainSettings()->verticalScale() )
                                                                                                                                                                                               : 0.f;

          switch ( altitudeClamping )
          {
            case Qgis::AltitudeClamping::Absolute:
              z = zOffset;
              break;
            case Qgis::AltitudeClamping::Terrain:
              z = terrainZ;
              break;
            case Qgis::AltitudeClamping::Relative:
              z = terrainZ + zOffset;
              break;
          }

          TextBillboard billboard;
          billboard.position = ( QgsVector3D( mapPoint.x(), mapPoint.y(), z ) - mChunkOrigin ).toVector3D();
          billboard.text = text;
          textBillboards.emplace_back( std::move( billboard ) );
          textBillboardTexts.append( text );

          if ( showCallouts )
          {
            mCalloutLines << QgsLineString( { mapPoint.x(), mapPoint.x() }, { mapPoint.y(), mapPoint.y() }, { terrainZ, z } );
          }

          mZMax = std::max( mZMax, showCallouts ? std::max( 0.0, z ) : z );
          mZMin = std::min( mZMin, showCallouts ? std::min( 0.0, z ) : z );
        }
        catch ( QgsCsException &e )
        {
          QgsDebugError( e.what() );
        }
      }
    };

    for ( const std::unique_ptr< QgsAnnotationItem > &item : std::as_const( mItemsToRender ) )
    {
      if ( mCanceled )
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
            const float terrainZ = ( altitudeClamping == Qgis::AltitudeClamping::Absolute && !showCallouts ) ? 0 : mRenderContext.terrainRenderingEnabled() && mRenderContext.terrainGenerator() ? static_cast<float>( mRenderContext.terrainGenerator()->heightAt( mapPoint.x(), mapPoint.y(), mRenderContext ) * mRenderContext.terrainSettings()->verticalScale() )
                                                                                                                                                                                                 : 0.f;

            switch ( altitudeClamping )
            {
              case Qgis::AltitudeClamping::Absolute:
                z = zOffset;
                break;
              case Qgis::AltitudeClamping::Terrain:
                z = terrainZ;
                break;
              case Qgis::AltitudeClamping::Relative:
                z = terrainZ + zOffset;
                break;
            }

            Billboard billboard;
            billboard.position = ( QgsVector3D( mapPoint.x(), mapPoint.y(), z ) - mChunkOrigin ).toVector3D();
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

            if ( showCallouts )
            {
              mCalloutLines << QgsLineString( { mapPoint.x(), mapPoint.x() }, { mapPoint.y(), mapPoint.y() }, { terrainZ, z } );
            }

            mZMax = std::max( mZMax, showCallouts ? std::max( 0.0, z ) : z );
            mZMin = std::min( mZMin, showCallouts ? std::min( 0.0, z ) : z );
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
    // free memory
    mItemsToRender.clear();

    if ( !textures.isEmpty() )
    {
      const QgsTextureAtlas atlas = QgsTextureAtlasGenerator::createFromImages( textures, 2048 );
      if ( atlas.isValid() )
      {
        mBillboardAtlas = atlas.renderAtlasTexture();
        mBillboardPositions.reserve( static_cast< int >( billboards.size() ) );
        for ( Billboard &billboard : billboards )
        {
          const QRect textureRect = atlas.rect( billboard.textureId );
          QgsBillboardGeometry::BillboardAtlasData geometry;
          geometry.position = billboard.position;
          geometry.textureAtlasOffset = QVector2D( static_cast< float >( textureRect.left() ) / static_cast< float>( mBillboardAtlas.width() ), 1 - ( static_cast< float >( textureRect.bottom() ) / static_cast< float>( mBillboardAtlas.height() ) ) );
          geometry.textureAtlasSize = QVector2D( static_cast< float >( textureRect.width() ) / static_cast< float>( mBillboardAtlas.width() ), static_cast< float>( textureRect.height() ) / static_cast< float>( mBillboardAtlas.height() ) );
          geometry.pixelOffset = QPoint( 0, textureRect.height() / 2 );
          mBillboardPositions.append( geometry );
        }
      }
      else
      {
        QgsDebugError( u"Error encountered building texture atlas"_s );
        mBillboardAtlas = QImage();
      }
    }
    else
    {
      mBillboardAtlas = QImage();
      mBillboardPositions.clear();
    }


    if ( !textBillboardTexts.isEmpty() )
    {
      const QgsFontTextureAtlas atlas = QgsFontTextureAtlasGenerator::create( textFormat, textBillboardTexts );
      if ( atlas.isValid() )
      {
        mTextBillboardAtlas = atlas.renderAtlasTexture();
        mTextBillboardPositions.reserve( static_cast< int >( textBillboards.size() ) );
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
            geometry.textureAtlasOffset = QVector2D( static_cast< float >( textureRect.left() ) / static_cast< float>( mTextBillboardAtlas.width() ), 1 - ( static_cast< float >( textureRect.bottom() ) / static_cast< float>( mTextBillboardAtlas.height() ) ) );
            geometry.textureAtlasSize = QVector2D( static_cast< float >( textureRect.width() ) / static_cast< float>( mTextBillboardAtlas.width() ), static_cast< float>( textureRect.height() ) / static_cast< float>( mTextBillboardAtlas.height() ) );
            const QPointF pixelOffset = atlas.pixelOffsetForGrapheme( billboard.text, graphemeIndex );
            geometry.pixelOffset = QPoint( static_cast< int >( std::round( -xOffset + pixelOffset.x() + 0.5 * textureRect.width() ) ), static_cast< int >( std::round( pixelOffset.y() + 0.5 * textureRect.height() ) ) );
            mTextBillboardPositions.append( geometry );
          }
        }
      }
      else
      {
        QgsDebugError( u"Error encountered building font texture atlas"_s );
        mTextBillboardAtlas = QImage();
      }
    }
    else
    {
      mTextBillboardAtlas = QImage();
      mTextBillboardPositions.clear();
    }
  } );

  // emit finished() as soon as the handler is populated with features
  mFutureWatcher->setFuture( future );
}

QgsAnnotationLayerChunkLoader::~QgsAnnotationLayerChunkLoader()
{
  if ( mFutureWatcher && !mFutureWatcher->isFinished() )
  {
    disconnect( mFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsChunkQueueJob::finished );
    mFutureWatcher->waitForFinished();
  }
}

void QgsAnnotationLayerChunkLoader::cancel()
{
  mCanceled = true;
}

Qt3DCore::QEntity *QgsAnnotationLayerChunkLoader::createEntity( Qt3DCore::QEntity *parent )
{
  if ( mNode->level() < mFactory->mLeafLevel )
  {
    Qt3DCore::QEntity *entity = new Qt3DCore::QEntity( parent ); // dummy entity
    entity->setObjectName( mLayerName + "_CONTAINER_" + mNode->tileId().text() );
    return entity;
  }

  if ( mBillboardPositions.empty() && mTextBillboardPositions.empty() )
  {
    // an empty node, so we return no entity. This tags the node as having no data and effectively removes it.
    // we just make sure first that its initial estimated vertical range does not affect its parents' bboxes calculation
    mNode->setExactBox3D( QgsBox3D() );
    mNode->updateParentBoundingBoxesRecursively();
    return nullptr;
  }

  Qt3DCore::QEntity *entity = new Qt3DCore::QEntity( parent );
  entity->setObjectName( mLayerName + "_" + mNode->tileId().text() );

  QgsGeoTransform *billboardTransform = new QgsGeoTransform;
  billboardTransform->setGeoTranslation( mChunkOrigin );
  entity->addComponent( billboardTransform );

  if ( !mBillboardPositions.empty() )
  {
    QgsBillboardGeometry *billboardGeometry = new QgsBillboardGeometry();
    billboardGeometry->setBillboardData( mBillboardPositions, true );

    Qt3DRender::QGeometryRenderer *billboardGeometryRenderer = new Qt3DRender::QGeometryRenderer;
    billboardGeometryRenderer->setPrimitiveType( Qt3DRender::QGeometryRenderer::Points );
    billboardGeometryRenderer->setGeometry( billboardGeometry );
    billboardGeometryRenderer->setVertexCount( billboardGeometry->count() );

    QgsPoint3DBillboardMaterial *billboardMaterial = new QgsPoint3DBillboardMaterial( QgsPoint3DBillboardMaterial::Mode::AtlasTextureWithPixelOffsets );
    billboardMaterial->setTexture2DFromImage( mBillboardAtlas );


    Qt3DCore::QEntity *billboardEntity = new Qt3DCore::QEntity;
    billboardEntity->addComponent( billboardMaterial );
    billboardEntity->addComponent( billboardGeometryRenderer );
    billboardEntity->setParent( entity );
  }

  if ( !mTextBillboardPositions.empty() )
  {
    QgsBillboardGeometry *textBillboardGeometry = new QgsBillboardGeometry();
    textBillboardGeometry->setBillboardData( mTextBillboardPositions, true );

    Qt3DRender::QGeometryRenderer *billboardGeometryRenderer = new Qt3DRender::QGeometryRenderer;
    billboardGeometryRenderer->setPrimitiveType( Qt3DRender::QGeometryRenderer::Points );
    billboardGeometryRenderer->setGeometry( textBillboardGeometry );
    billboardGeometryRenderer->setVertexCount( textBillboardGeometry->count() );

    QgsPoint3DBillboardMaterial *billboardMaterial = new QgsPoint3DBillboardMaterial( QgsPoint3DBillboardMaterial::Mode::AtlasTextureWithPixelOffsets );
    billboardMaterial->setTexture2DFromImage( mTextBillboardAtlas );

    Qt3DCore::QEntity *billboardEntity = new Qt3DCore::QEntity;
    billboardEntity->addComponent( billboardMaterial );
    billboardEntity->addComponent( billboardGeometryRenderer );
    billboardEntity->setParent( entity );
  }


  if ( mFactory->mShowCallouts )
  {
    QgsLineVertexData lineData;
    lineData.withAdjacency = true;
    lineData.geocentricCoordinates = false; // mMapSettings->sceneMode() == Qgis::SceneMode::Globe;
    lineData.init( Qgis::AltitudeClamping::Absolute, Qgis::AltitudeBinding::Vertex, 0, mRenderContext, mChunkOrigin );

    for ( const QgsLineString &line : mCalloutLines )
    {
      lineData.addLineString( line, 0, false );
    }

    QgsLineMaterial *mat = new QgsLineMaterial;
    mat->setLineColor( mFactory->mCalloutLineColor );
    mat->setLineWidth( mFactory->mCalloutLineWidth );

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
  if ( mZMin != std::numeric_limits<float>::max() && mZMax != std::numeric_limits<float>::lowest() )
  {
    QgsBox3D box = mNode->box3D();
    box.setZMinimum( mZMin );
    box.setZMaximum( mZMax );
    mNode->setExactBox3D( box );
    mNode->updateParentBoundingBoxesRecursively();
  }
  return entity;
}


///////////////


QgsAnnotationLayerChunkLoaderFactory::QgsAnnotationLayerChunkLoaderFactory( const Qgs3DRenderContext &context, QgsAnnotationLayer *layer, int leafLevel, Qgis::AltitudeClamping clamping, double zOffset, bool showCallouts, const QColor &calloutLineColor, double calloutLineWidth, const QgsTextFormat &textFormat, double zMin, double zMax )
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

  QgsBox3D rootBox3D( context.extent(), zMin, zMax );
  // add small padding to avoid clipping of point features located at the edge of the bounding box
  rootBox3D.grow( 1.0 );
  setupQuadtree( rootBox3D, -1, leafLevel ); // negative root error means that the node does not contain anything
}

QgsChunkLoader *QgsAnnotationLayerChunkLoaderFactory::createChunkLoader( QgsChunkNode *node ) const
{
  return new QgsAnnotationLayerChunkLoader( this, node );
}


///////////////


QgsAnnotationLayerChunkedEntity::QgsAnnotationLayerChunkedEntity( Qgs3DMapSettings *map, QgsAnnotationLayer *layer, Qgis::AltitudeClamping clamping, double zOffset, bool showCallouts, const QColor &calloutLineColor, double calloutLineWidth, const QgsTextFormat &textFormat, double zMin, double zMax )
  : QgsChunkedEntity( map,
                      -1, // max. allowed screen error (negative tau means that we need to go until leaves are reached)
                      new QgsAnnotationLayerChunkLoaderFactory( Qgs3DRenderContext::fromMapSettings( map ), layer, 3, clamping, zOffset, showCallouts, calloutLineColor, calloutLineWidth, textFormat, zMin, zMax ), true )
{
  mTransform = new Qt3DCore::QTransform;
  if ( applyTerrainOffset() )
  {
    mTransform->setTranslation( QVector3D( 0.0f, 0.0f, static_cast<float>( map->terrainSettings()->elevationOffset() ) ) );
  }
  this->addComponent( mTransform );

  connect( map, &Qgs3DMapSettings::terrainSettingsChanged, this, &QgsAnnotationLayerChunkedEntity::onTerrainElevationOffsetChanged );
}

QgsAnnotationLayerChunkedEntity::~QgsAnnotationLayerChunkedEntity()
{
  // cancel / wait for jobs
  cancelActiveJobs();
}

// if the AltitudeClamping is `Absolute`, do not apply the offset
bool QgsAnnotationLayerChunkedEntity::applyTerrainOffset() const
{
  if ( auto loaderFactory = static_cast<QgsAnnotationLayerChunkLoaderFactory *>( mChunkLoaderFactory ) )
  {
    return loaderFactory->mClamping != Qgis::AltitudeClamping::Absolute;
  }
  return true;
}

void QgsAnnotationLayerChunkedEntity::onTerrainElevationOffsetChanged()
{
  QgsDebugMsgLevel( u"QgsAnnotationLayerChunkedEntity::onTerrainElevationOffsetChanged"_s, 2 );
  float newOffset = static_cast<float>( qobject_cast<Qgs3DMapSettings *>( sender() )->terrainSettings()->elevationOffset() );
  if ( !applyTerrainOffset() )
  {
    newOffset = 0.0;
  }
  mTransform->setTranslation( QVector3D( 0.0f, 0.0f, newOffset ) );
}


/// @endcond
