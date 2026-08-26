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
#include "qgsannotationpictureitem.h"
#include "qgsannotationpointtextitem.h"
#include "qgsannotationrectangletextitem.h"
#include "qgsapplication.h"
#include "qgsbillboardgeometry.h"
#include "qgschunknode.h"
#include "qgseventtracing.h"
#include "qgsexpressioncontextutils.h"
#include "qgsfonttextureatlasgenerator.h"
#include "qgsgeos.h"
#include "qgsgeotransform.h"
#include "qgsimagecache.h"
#include "qgslinematerial_p.h"
#include "qgslinevertexdata_p.h"
#include "qgslogger.h"
#include "qgsmarkersymbol.h"
#include "qgspainting.h"
#include "qgspoint3dbillboardmaterial.h"
#include "qgssvgcache.h"
#include "qgstessellatedpolygongeometry.h"
#include "qgstextdocument.h"
#include "qgstextureatlasgenerator.h"

#include <QString>
#include <QTimer>
#include <Qt3DCore/QTransform>
#include <Qt3DRender/QGeometryRenderer>
#include <QtConcurrentRun>

#include "moc_qgsannotationlayerchunkloader_p.cpp"

using namespace Qt::StringLiterals;

///@cond PRIVATE


QgsAnnotationLayerChunkLoader::QgsAnnotationLayerChunkLoader( const QgsAnnotationLayerChunkLoaderFactory *factory, QgsChunkNode *node )
  : QgsChunkLoader( node )
  , mFactory( factory )
  , mRenderContext( factory->mRenderContext )
{}

namespace
{
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

  /**
   * A group of picture billboards sharing the same source image.
   */
  struct PictureBillboardGroup
  {
      QString path;
      Qgis::BillboardScaleMode scaleMode;
      Qgis::PictureFormat pictureFormat;

      bool operator<( const PictureBillboardGroup &other ) const
      {
        if ( path != other.path )
          return path < other.path;
        return scaleMode < other.scaleMode;
      }
  };

  struct PictureBillboard
  {
      QVector3D position;
      QSizeF size;
  };
} //namespace

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
    const QgsScopedEvent e( u"3D"_s, u"Annotation layer chunk load"_s );

    std::vector< Billboard > billboards;
    billboards.reserve( mItemsToRender.size() );
    QVector< QImage > textures;
    textures.reserve( static_cast< qsizetype >( mItemsToRender.size() ) );

    std::vector< TextBillboard > textBillboards;
    textBillboards.reserve( mItemsToRender.size() );
    QStringList textBillboardTexts;
    textBillboardTexts.reserve( static_cast< qsizetype >( mItemsToRender.size() ) );

    QMap< PictureBillboardGroup, QVector< PictureBillboard > > groupedPictures;
    mPictureBillboards.reserve( static_cast< qsizetype >( mItemsToRender.size() ) );

    auto addTextBillboard = [layerToMapTransform,
                             showCallouts,
                             rect,
                             zOffset,
                             altitudeClamping,
                             this,
                             &textBillboards,
                             &textBillboardTexts]( const QgsPointXY &p, const QString &annotationText, const QgsTextFormat &annotationTextFormat ) {
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
          const float terrainZ = ( altitudeClamping == Qgis::AltitudeClamping::Absolute && !showCallouts ) ? 0
                                 : mRenderContext.terrainRenderingEnabled() && mRenderContext.terrainGenerator()
                                   ? static_cast<float>( mRenderContext.terrainGenerator()->heightAt( mapPoint.x(), mapPoint.y(), mRenderContext ) * mRenderContext.terrainSettings()->verticalScale() )
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
            const float terrainZ = ( altitudeClamping == Qgis::AltitudeClamping::Absolute && !showCallouts ) ? 0
                                   : mRenderContext.terrainRenderingEnabled() && mRenderContext.terrainGenerator()
                                     ? static_cast<float>( mRenderContext.terrainGenerator()->heightAt( mapPoint.x(), mapPoint.y(), mRenderContext ) * mRenderContext.terrainSettings()->verticalScale() )
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
      else if ( auto pictureItem = dynamic_cast< QgsAnnotationPictureItem * >( annotation ) )
      {
        if ( pictureItem->path().isEmpty() )
          continue;

        if ( pictureItem->placementMode() == Qgis::AnnotationPlacementMode::RelativeToMapFrame )
        {
          // annotations relative to the map frame have no geographic position => ignore
          continue;
        }

        const QgsPointXY p = pictureItem->bounds().center();
        QgsPointXY mapPoint;
        try
        {
          mapPoint = layerToMapTransform.transform( p );
        }
        catch ( QgsCsException &e )
        {
          QgsDebugError( e.what() );
          continue;
        }

        if ( !rect.contains( mapPoint ) )
          continue;

        double z = 0;
        const float terrainZ = ( altitudeClamping == Qgis::AltitudeClamping::Absolute && !showCallouts ) ? 0
                               : mRenderContext.terrainRenderingEnabled() && mRenderContext.terrainGenerator()
                                 ? static_cast<float>( mRenderContext.terrainGenerator()->heightAt( mapPoint.x(), mapPoint.y(), mRenderContext ) * mRenderContext.terrainSettings()->verticalScale() )
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

        PictureBillboardGroup billboardGroup;
        billboardGroup.path = pictureItem->path();
        billboardGroup.pictureFormat = pictureItem->format();
        billboardGroup.scaleMode = pictureItem->billboard3DScaleMode();

        PictureBillboard billboardItem;
        billboardItem.position = ( QgsVector3D( mapPoint.x(), mapPoint.y(), z ) - mChunkOrigin ).toVector3D();
        billboardItem.size = pictureItem->billboard3DSize();
        if ( billboardItem.size.isEmpty() )
        {
          constexpr QSizeF DEFAULT_FIXED_SIZE = QSizeF( 128, 128 );
          constexpr QSizeF DEFAULT_WORLD_SIZE = QSizeF( 20, 20 );
          billboardItem.size = billboardGroup.scaleMode == Qgis::BillboardScaleMode::ViewIndependent ? DEFAULT_FIXED_SIZE : DEFAULT_WORLD_SIZE;
        }

        groupedPictures[billboardGroup].append( billboardItem );

        if ( showCallouts )
        {
          mCalloutLines << QgsLineString( { mapPoint.x(), mapPoint.x() }, { mapPoint.y(), mapPoint.y() }, { terrainZ, z } );
        }

        mZMax = std::max( mZMax, showCallouts ? std::max( 0.0, z ) : z );
        mZMin = std::min( mZMin, showCallouts ? std::min( 0.0, z ) : z );
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
          geometry.textureAtlasOffset = QVector2D(
            static_cast< float >( textureRect.left() ) / static_cast< float>( mBillboardAtlas.width() ),
            1 - ( static_cast< float >( textureRect.bottom() ) / static_cast< float>( mBillboardAtlas.height() ) )
          );
          geometry.textureAtlasSize = QVector2D(
            static_cast< float >( textureRect.width() ) / static_cast< float>( mBillboardAtlas.width() ), static_cast< float>( textureRect.height() ) / static_cast< float>( mBillboardAtlas.height() )
          );
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
            geometry.textureAtlasOffset = QVector2D(
              static_cast< float >( textureRect.left() ) / static_cast< float>( mTextBillboardAtlas.width() ),
              1 - ( static_cast< float >( textureRect.bottom() ) / static_cast< float>( mTextBillboardAtlas.height() ) )
            );
            geometry.textureAtlasSize = QVector2D(
              static_cast< float >( textureRect.width() ) / static_cast< float>( mTextBillboardAtlas.width() ),
              static_cast< float>( textureRect.height() ) / static_cast< float>( mTextBillboardAtlas.height() )
            );
            const QPointF pixelOffset = atlas.pixelOffsetForGrapheme( billboard.text, graphemeIndex );
            geometry.pixelOffset
              = QPoint( static_cast< int >( std::round( -xOffset + pixelOffset.x() + 0.5 * textureRect.width() ) ), static_cast< int >( std::round( pixelOffset.y() + 0.5 * textureRect.height() ) ) );
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

    // picture item billboards, grouped by picture source
    for ( auto it = groupedPictures.constBegin(); it != groupedPictures.constEnd(); ++it )
    {
      QSizeF maxGroupSize( 0, 0 );
      for ( auto picIt = it.value().constBegin(); picIt != it.value().constEnd(); ++picIt )
      {
        if ( picIt->size.width() > maxGroupSize.width() )
        {
          maxGroupSize.setWidth( picIt->size.width() );
        }
        if ( picIt->size.height() > maxGroupSize.height() )
        {
          maxGroupSize.setHeight( picIt->size.height() );
        }
      }

      QImage image;
      bool fitsInCache = false;

      // can't zoom into these billboards, so we can use a fairly conservative texture size
      constexpr int MAXIMUM_PICTURE_TEXTURE_SIZE_FIXED_SIZE = 256;
      // can zoom into these, so we need a larger texture
      constexpr int MAXIMUM_PICTURE_TEXTURE_SIZE_PERSPECTIVE = 1024;
      const int textureSize = it.key().scaleMode == Qgis::BillboardScaleMode::Perspective ? MAXIMUM_PICTURE_TEXTURE_SIZE_PERSPECTIVE : MAXIMUM_PICTURE_TEXTURE_SIZE_FIXED_SIZE;
      switch ( it.key().pictureFormat )
      {
        case Qgis::PictureFormat::Raster:
        {
          const QSize originalSize = QgsApplication::imageCache()->originalSize( it.key().path, true );
          QSize imageSize = originalSize;
          if ( imageSize.isEmpty() )
          {
            imageSize = maxGroupSize.toSize();
          }
          if ( imageSize.width() >= imageSize.height() && imageSize.width() > textureSize )
          {
            imageSize = QSize( textureSize, static_cast< int >( std::round( imageSize.height() * textureSize / imageSize.width() ) ) );
          }
          else if ( imageSize.height() > textureSize )
          {
            imageSize = QSize( static_cast< int >( std::round( imageSize.width() * textureSize / imageSize.height() ) ), textureSize );
          }
          image = QgsApplication::imageCache()->pathAsImage( it.key().path, imageSize, false, 1.0, fitsInCache, true );
          break;
        }

        case Qgis::PictureFormat::SVG:
        {
          const QPicture picture = QgsApplication::svgCache()->svgAsPicture( it.key().path, textureSize, QColor(), QColor(), 1.0, 1.0, false, 0, true );
          if ( !picture.isNull() && picture.boundingRect().width() > 0 && picture.boundingRect().height() > 0 )
          {
            const QRectF picRect = picture.boundingRect();
            QSize imageSize = picRect.size().toSize();
            if ( imageSize.width() >= imageSize.height() )
            {
              imageSize = QSize( textureSize, static_cast< int >( std::round( picRect.height() * static_cast< double >( textureSize ) / picRect.width() ) ) );
            }
            else
            {
              imageSize = QSize( static_cast< int >( std::round( picRect.width() * static_cast< double >( textureSize ) / picRect.height() ) ), textureSize );
            }

            image = QImage( imageSize, QImage::Format_ARGB32_Premultiplied );
            image.fill( Qt::transparent );

            const double scale = static_cast< double >( imageSize.width() ) / picRect.width();

            QPainter painter( &image );
            painter.setRenderHint( QPainter::Antialiasing );
            painter.scale( scale, scale );

            QgsPainting::drawPicture( &painter, QPointF( picRect.width() / 2.0, picRect.height() / 2.0 ), picture );
            painter.end();
          }
          break;
        }

        case Qgis::PictureFormat::Unknown:
          continue;
      }

      PictureBillboards billboard;
      billboard.scaleMode = it.key().scaleMode;
      billboard.image = image;
      billboard.positions.reserve( it.value().size() );
      billboard.sizes.reserve( it.value().size() );
      for ( const PictureBillboard &item : it.value() )
      {
        billboard.positions.append( item.position );
        billboard.sizes.append( item.size );
      }

      mPictureBillboards.append( billboard );
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

  if ( mBillboardPositions.empty() && mTextBillboardPositions.empty() && mPictureBillboards.empty() )
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
    billboardGeometryRenderer->setPrimitiveType( Qt3DRender::QGeometryRenderer::TriangleStrip );
    billboardGeometryRenderer->setGeometry( billboardGeometry );
    billboardGeometryRenderer->setVertexCount( 4 );
    billboardGeometryRenderer->setInstanceCount( mBillboardPositions.count() );

    QgsPoint3DBillboardMaterial *billboardMaterial = new QgsPoint3DBillboardMaterial( QgsPoint3DBillboardMaterial::ExtraAttribute::TextureData | QgsPoint3DBillboardMaterial::ExtraAttribute::PixelOffsets );
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
    billboardGeometryRenderer->setPrimitiveType( Qt3DRender::QGeometryRenderer::TriangleStrip );
    billboardGeometryRenderer->setGeometry( textBillboardGeometry );
    billboardGeometryRenderer->setVertexCount( 4 );
    billboardGeometryRenderer->setInstanceCount( mTextBillboardPositions.count() );

    QgsPoint3DBillboardMaterial *billboardMaterial = new QgsPoint3DBillboardMaterial( QgsPoint3DBillboardMaterial::ExtraAttribute::TextureData | QgsPoint3DBillboardMaterial::ExtraAttribute::PixelOffsets );
    billboardMaterial->setTexture2DFromImage( mTextBillboardAtlas );

    Qt3DCore::QEntity *billboardEntity = new Qt3DCore::QEntity;
    billboardEntity->addComponent( billboardMaterial );
    billboardEntity->addComponent( billboardGeometryRenderer );
    billboardEntity->setParent( entity );
  }

  for ( const PictureBillboards &pictureBillboard : mPictureBillboards )
  {
    QgsBillboardGeometry *pictureGeometry = new QgsBillboardGeometry();
    pictureGeometry->setPositionsAndSizes( pictureBillboard.positions, pictureBillboard.sizes );

    Qt3DRender::QGeometryRenderer *pictureGeometryRenderer = new Qt3DRender::QGeometryRenderer;
    pictureGeometryRenderer->setPrimitiveType( Qt3DRender::QGeometryRenderer::TriangleStrip );
    pictureGeometryRenderer->setGeometry( pictureGeometry );
    pictureGeometryRenderer->setVertexCount( 4 );
    pictureGeometryRenderer->setInstanceCount( static_cast< int >( pictureBillboard.positions.size() ) );

    QgsPoint3DBillboardMaterial *pictureMaterial
      = new QgsPoint3DBillboardMaterial( QgsPoint3DBillboardMaterial::ExtraAttribute::Size | QgsPoint3DBillboardMaterial::ExtraAttribute::VerticalOffset, pictureBillboard.scaleMode );
    pictureMaterial->setTexture2DFromImage( pictureBillboard.image );
    // picture billboards should be vertically anchored to the bottom of the picture
    pictureMaterial->setVerticalOffset( 0.5 );

    Qt3DCore::QEntity *pictureEntity = new Qt3DCore::QEntity;
    pictureEntity->addComponent( pictureMaterial );
    pictureEntity->addComponent( pictureGeometryRenderer );
    pictureEntity->setParent( entity );
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


QgsAnnotationLayerChunkLoaderFactory::QgsAnnotationLayerChunkLoaderFactory(
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

QgsChunkLoader *QgsAnnotationLayerChunkLoaderFactory::createChunkLoader( QgsChunkNode *node ) const
{
  return new QgsAnnotationLayerChunkLoader( this, node );
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
      new QgsAnnotationLayerChunkLoaderFactory( Qgs3DRenderContext::fromMapSettings( map ), layer, 3, clamping, zOffset, showCallouts, calloutLineColor, calloutLineWidth, textFormat, zMin, zMax ),
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
  if ( auto loaderFactory = static_cast<QgsAnnotationLayerChunkLoaderFactory *>( mChunkLoaderFactory ) )
  {
    return loaderFactory->mClamping != Qgis::AltitudeClamping::Absolute;
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
