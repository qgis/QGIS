/***************************************************************************
  qgsvectorlayerchunkloader_p.cpp
  --------------------------------------
  Date                 : July 2019
  Copyright            : (C) 2019 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorlayerchunkloader_p.h"

#include "qgs3dsymbolregistry.h"
#include "qgs3dutils.h"
#include "qgsabstract3dsymbol.h"
#include "qgsabstractterrainsettings.h"
#include "qgsabstractvectorlayer3drenderer.h"
#include "qgsapplication.h"
#include "qgschunknode.h"
#include "qgseventtracing.h"
#include "qgsexception.h"
#include "qgsexpressioncontextutils.h"
#include "qgsfeature3dhandler_p.h"
#include "qgsgeometry.h"
#include "qgsglobeutils_p.h"
#include "qgsline3dsymbol.h"
#include "qgslogger.h"
#include "qgspoint3dsymbol.h"
#include "qgspolygon3dsymbol.h"
#include "qgsthreadingutils.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerfeatureiterator.h"
#include "qgswkbtypes.h"

#include <QFuture>
#include <QString>
#include <Qt3DCore/QTransform>
#include <Qt3DRender/QGeometryRenderer>
#include <QtConcurrentRun>

#include "moc_qgsvectorlayerchunkloader_p.cpp"

using namespace Qt::StringLiterals;

///@cond PRIVATE

///////////////


QgsVectorLayerChunkLoader::QgsVectorLayerChunkLoader( const Qgs3DRenderContext &context, QgsVectorLayer *vl, QgsAbstract3DSymbol *symbol, double zMin, double zMax, int maxFeatures )
  : mRenderContext( context )
  , mLayer( vl )
  , mSymbol( symbol->clone() )
  , mMaxFeatures( maxFeatures )
{
  if ( context.crs().type() == Qgis::CrsType::Geocentric )
  {
    // TODO: add support for handling of vector layers (other than points)
    if ( QgsWkbTypes::geometryType( mLayer->wkbType() ) != Qgis::GeometryType::Point )
    {
      // (we're using dummy quadtree here to make sure the empty extent does not break the scene completely)
      QgsDebugError( u"Non-point vector layers in globe scenes are not supported yet!"_s );
      setupQuadtree( QgsBox3D( -7e6, -7e6, -7e6, 7e6, 7e6, 7e6 ), -1, 3 );
      return;
    }

    mIsGeocentric = true;

    const QgsCoordinateReferenceSystem geographicCrs = context.crs().toGeographicCrs();
    mCrsToLatLon = QgsCoordinateTransform( context.crs(), geographicCrs, context.transformContext() );
    mCrsToLatLon.setBallparkTransformsAreAppropriate( true );

    try
    {
      mRadius = QgsGlobeUtils::ellipsoidRadius( mCrsToLatLon );
    }
    catch ( QgsCsException &e )
    {
      QgsDebugError( u"Error transforming globe ellipsoid extent: %1"_s.arg( e.what() ) );
      setupQuadtree( QgsBox3D( -7e6, -7e6, -7e6, 7e6, 7e6, 7e6 ), -1, 3 );
      return;
    }

    // choose the smaller root extent between context and mLayer ones
    QgsRectangle layerExtentLonLat;
    if ( mLayer->crs().type() == Qgis::CrsType::Geocentric )
    {
      QgsCoordinateTransform layerToLatLon( mLayer->crs(), geographicCrs, context.transformContext() );
      layerToLatLon.setBallparkTransformsAreAppropriate( true );
      try
      {
        layerExtentLonLat = layerToLatLon.transformBox3D( mLayer->extent3D() ).toRectangle();
      }
      catch ( const QgsCsException &e )
      {
        QgsDebugError( u"Error transforming layer extent to lat/lon: %1"_s.arg( e.what() ) );
      }
    }
    else
    {
      layerExtentLonLat = Qgs3DUtils::tryReprojectExtent2D( mLayer->extent(), mLayer->crs(), geographicCrs, context.transformContext() );
    }

    if ( layerExtentLonLat.isValid() )
    {
      // add small padding to avoid clipping of point features located at the edge of the bounding box
      layerExtentLonLat.grow( 0.01 );
      layerExtentLonLat = layerExtentLonLat.intersect( QgsRectangle( -180, -90, 180, 90 ) );
    }

    mRootNodeId = QgsGlobeUtils::findSmallestIdContainingExtent( layerExtentLonLat );

    QgsBox3D rootBox3D;
    if ( mRootNodeId.d == 0 )
      rootBox3D = QgsBox3D( -mRadius.x(), -mRadius.y(), -mRadius.z(), mRadius.x(), mRadius.y(), mRadius.z() );
    else if ( mRootNodeId.d == 1 )
      rootBox3D = mRootNodeId.x == 1 ? QgsBox3D( -mRadius.x(), 0, -mRadius.z(), mRadius.x(), mRadius.y(), mRadius.z() )
                                     : QgsBox3D( -mRadius.x(), -mRadius.y(), -mRadius.z(), mRadius.x(), 0, mRadius.z() );
    else
      rootBox3D = QgsGlobeUtils::nodeIdToBox3D( mRootNodeId, mCrsToLatLon );

    const float rootError = static_cast<float>( std::max<double>( rootBox3D.width(), rootBox3D.height() ) * QgsVectorLayer3DTilingSettings::tileGeometryErrorRatio() );
    setupQuadtree( rootBox3D, rootError );
    return;
  }

  QgsRectangle extent = context.extent();
  const QgsRectangle layerExtentInMapCrs = Qgs3DUtils::tryReprojectExtent2D( mLayer->extent(), mLayer->crs(), context.crs(), context.transformContext() );
  if ( layerExtentInMapCrs.isValid() )
  {
    extent = context.extent().intersect( layerExtentInMapCrs );
  }
  if ( extent.isValid() )
  {
    QgsBox3D rootBox3D( extent, zMin, zMax );
    rootBox3D.grow( 1.0 );

    const float rootError = static_cast<float>( std::max<double>( rootBox3D.width(), rootBox3D.height() ) * QgsVectorLayer3DTilingSettings::tileGeometryErrorRatio() );
    setupQuadtree( rootBox3D, rootError );
  }
}

QFuture<QgsChunkLoaderResult> QgsVectorLayerChunkLoader::loadChunk( QgsChunkNode *node )
{
  QgsFeature3DHandler *handlerPtr = QgsApplication::symbol3DRegistry()->createHandlerForSymbol( mLayer, mSymbol.get() );
  if ( !handlerPtr )
  {
    QgsDebugError( u"Unknown 3D symbol type for vector layer: "_s + mSymbol->type() );
    return QtFuture::makeReadyValueFuture( QgsChunkLoaderResult::sEmpty );
  }
  // Needs to be in shared_ptr instead of unique_ptr so it can be captured in copyable std::function
  std::shared_ptr<QgsFeature3DHandler> handler( handlerPtr );

  Qgs3DRenderContext renderCtx = mRenderContext; // Copy, since we mutate it locally per-chunk

  QgsExpressionContext exprContext;
  exprContext.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( mLayer ) );
  exprContext.setFields( mLayer->fields() );
  renderCtx.setExpressionContext( exprContext );

  QSet<QString> attributeNames;
  if ( !handler->prepare( renderCtx, attributeNames, node->box3D() ) )
  {
    QgsDebugError( u"Failed to prepare 3D feature handler!"_s );
    return QtFuture::makeReadyValueFuture( QgsChunkLoaderResult::sEmpty );
  }

  // build the feature request
  // only a subset of data to be queried
  QgsFeatureRequest req;
  req.setSubsetOfAttributes( attributeNames, mLayer->fields() );

  QgsCoordinateTransform layerToRenderCrs;
  if ( mIsGeocentric )
  {
    layerToRenderCrs = QgsCoordinateTransform( mLayer->crs3D(), mRenderContext.crs(), mRenderContext.transformContext() );
    layerToRenderCrs.setBallparkTransformsAreAppropriate( true );

    QgsRectangle filterRect;
    if ( mLayer->crs().type() == Qgis::CrsType::Geocentric )
    {
      try
      {
        filterRect = layerToRenderCrs.transformBox3D( node->box3D(), Qgis::TransformDirection::Reverse ).toRectangle();
      }
      catch ( const QgsCsException & )
      {
        QgsDebugError( u"Error transforming node box3D to layer CRS"_s );
      }
    }
    else
    {
      const QgsRectangle lonLatRect = QgsGlobeUtils::nodeIdToLonLatRect( node->tileId() );
      filterRect = Qgs3DUtils::tryReprojectExtent2D( lonLatRect, mCrsToLatLon.destinationCrs(), mLayer->crs(), mRenderContext.transformContext() );
    }
    req.setFilterRect( filterRect );
  }
  else
  {
    req.setCoordinateTransform( QgsCoordinateTransform( mLayer->crs3D(), mRenderContext.crs(), mRenderContext.transformContext() ) );
    req.setFilterRect( node->box3D().toRectangle() );
  }


  auto source = std::make_unique<QgsVectorLayerFeatureSource>( mLayer );

  QPointer<QgsVectorLayerChunkLoader> weakThis = this;
  return QtConcurrent::run(
    [req = std::move( req ), source = std::move( source ), handler = std::move( handler ), this, renderCtx, node, layerToRenderCrs, maxFeatures = mMaxFeatures, isGeocentric = mIsGeocentric, weakThis](
      QPromise<QgsChunkLoaderResult> &promise
    ) mutable {
      const QgsScopedEvent e( u"3D"_s, u"VL chunk load"_s );

      QgsFeature f;
      QgsFeatureIterator fi = source->getFeatures( req );
      int featureCount = 0;
      bool featureLimitReached = false;
      while ( fi.nextFeature( f ) )
      {
        if ( promise.isCanceled() )
          return;

        if ( ++featureCount > maxFeatures )
        {
          featureLimitReached = true;
          break;
        }

        if ( isGeocentric )
        {
          QgsGeometry g = f.geometry();
          if ( !g.constGet()->is3D() )
            g.get()->addZValue( 0 );

          try
          {
            g.transform( layerToRenderCrs, Qgis::TransformDirection::Forward, true );
          }
          catch ( QgsCsException &e )
          {
            QgsDebugError( u"Error transforming feature %1 geometry to globe CRS: %2"_s.arg( f.id() ).arg( e.what() ) );
            continue;
          }
          f.setGeometry( g );
        }

        renderCtx.expressionContext().setFeature( f );
        handler->processFeature( f, renderCtx );
      }

      bool nodeIsLeaf = false;
      if ( !featureLimitReached )
      {
        QgsDebugMsgLevel( u"All features fetched for node: %1"_s.arg( node->tileId().text() ), 3 );

        if ( featureCount == 0 || std::max<double>( node->box3D().width(), node->box3D().height() ) < QgsVectorLayer3DTilingSettings::maximumLeafExtent() )
          nodeIsLeaf = true;
      }

      QgsThreadingUtils::runOnMainThread( [weakThis, nodeIsLeaf, key = node->tileId().text()]() {
        if ( weakThis )
        {
          QMutexLocker<QMutex> locker( &weakThis->mNodesAreLeafsMutex );
          weakThis->mNodesAreLeafs[key] = nodeIsLeaf;
        }
      } );

      promise.addResult( QgsChunkLoaderResult { [this, handler, node, renderCtx]( Qt3DCore::QEntity *parent ) -> Qt3DCore::QEntity * {
        QGIS_CHECK_MAIN_THREAD_ACCESS
        if ( handler->featureCount() == 0 )
        {
          // an empty node, so we return no entity. This tags the node as having no data and effectively removes it.
          // we just make sure first that its initial estimated vertical range does not affect its parents' bboxes calculation
          node->setExactBox3D( QgsBox3D() );
          node->updateParentBoundingBoxesRecursively();
          return nullptr;
        }

        Qt3DCore::QEntity *entity = new Qt3DCore::QEntity( parent );
        entity->setObjectName( mLayer->name() + "_" + node->tileId().text() );
        handler->finalize( entity, renderCtx );

        // fix the vertical range of the node from the estimated vertical range to the true range
        if ( handler->zMinimum() != std::numeric_limits<float>::max() && handler->zMaximum() != std::numeric_limits<float>::lowest() )
        {
          QgsBox3D box = node->box3D();
          box.setZMinimum( handler->zMinimum() );
          box.setZMaximum( handler->zMaximum() );
          node->setExactBox3D( box );
          node->updateParentBoundingBoxesRecursively();
        }

        return entity;
      } } );
    }
  );
}

QgsChunkNode *QgsVectorLayerChunkLoader::createRootNode() const
{
  if ( mIsGeocentric )
    return new QgsChunkNode( mRootNodeId, mRootBox3D, mRootError );

  return QgsQuadtreeChunkLoader::createRootNode();
}

QFuture<QVector<QgsChunkNode *>> QgsVectorLayerChunkLoader::createChildren( QgsChunkNode *node )
{
  {
    QMutexLocker locker( &mNodesAreLeafsMutex );
    if ( mNodesAreLeafs.value( node->tileId().text(), false ) )
      return QtFuture::makeReadyValueFuture( QVector<QgsChunkNode *> {} );
  }

  if ( !mIsGeocentric )
    return QgsQuadtreeChunkLoader::createChildren( node );

  QVector<QgsChunkNode *> children;
  if ( mMaxLevel != -1 && node->level() >= mMaxLevel )
    return QtFuture::makeReadyValueFuture( children );

  const QgsChunkNodeId nodeId = node->tileId();
  const float childError = node->error() / 2;

  if ( nodeId.d == 0 )
  {
    const QgsChunkNodeId westId( 1, 0, 0 );
    const QgsChunkNodeId eastId( 1, 1, 0 );
    children << new QgsChunkNode( westId, QgsBox3D( -mRadius.x(), -mRadius.y(), -mRadius.z(), mRadius.x(), 0, mRadius.z() ), childError, node );
    children << new QgsChunkNode( eastId, QgsBox3D( -mRadius.x(), 0, -mRadius.z(), mRadius.x(), mRadius.y(), mRadius.z() ), childError, node );
    return QtFuture::makeReadyValueFuture( children );
  }

  for ( int i = 0; i < 4; ++i )
  {
    const int dx = i & 1, dy = !!( i & 2 );
    const QgsChunkNodeId childId( nodeId.d + 1, nodeId.x * 2 + dx, nodeId.y * 2 + dy );
    children << new QgsChunkNode( childId, QgsGlobeUtils::nodeIdToBox3D( childId, mCrsToLatLon ), childError, node );
  }
  return QtFuture::makeReadyValueFuture( children );
}


///////////////


QgsVectorLayerChunkedEntity::QgsVectorLayerChunkedEntity(
  Qgs3DMapSettings *map, QgsVectorLayer *vl, double zMin, double zMax, const QgsVectorLayer3DTilingSettings &tilingSettings, QgsAbstract3DSymbol *symbol
)
  : QgsAbstractFeatureBasedChunkedEntity( map, 3, new QgsVectorLayerChunkLoader( Qgs3DRenderContext::fromMapSettings( map ), vl, symbol, zMin, zMax, tilingSettings.maximumChunkFeatures() ), true )
{
  onTerrainElevationOffsetChanged();
  setShowBoundingBoxes( tilingSettings.showBoundingBoxes() );
}

QgsVectorLayerChunkedEntity::~QgsVectorLayerChunkedEntity()
{
  // cancel / wait for jobs
  cancelActiveJobs();
}

// if the AltitudeClamping is `Absolute`, do not apply the offset
bool QgsVectorLayerChunkedEntity::applyTerrainOffset() const
{
  QgsVectorLayerChunkLoader *loader = static_cast<QgsVectorLayerChunkLoader *>( mChunkLoader );
  if ( loader )
  {
    QString symbolType = loader->mSymbol.get()->type();
    if ( symbolType == "line" )
    {
      QgsLine3DSymbol *lineSymbol = static_cast<QgsLine3DSymbol *>( loader->mSymbol.get() );
      if ( lineSymbol && lineSymbol->altitudeClamping() == Qgis::AltitudeClamping::Absolute )
      {
        return false;
      }
    }
    else if ( symbolType == "point" )
    {
      QgsPoint3DSymbol *pointSymbol = static_cast<QgsPoint3DSymbol *>( loader->mSymbol.get() );
      if ( pointSymbol && pointSymbol->altitudeClamping() == Qgis::AltitudeClamping::Absolute )
      {
        return false;
      }
    }
    else if ( symbolType == "polygon" )
    {
      QgsPolygon3DSymbol *polygonSymbol = static_cast<QgsPolygon3DSymbol *>( loader->mSymbol.get() );
      if ( polygonSymbol && polygonSymbol->altitudeClamping() == Qgis::AltitudeClamping::Absolute )
      {
        return false;
      }
    }
    else
    {
      QgsDebugMsgLevel( u"QgsVectorLayerChunkedEntity::applyTerrainOffset, unhandled symbol type %1"_s.arg( symbolType ), 2 );
    }
  }

  return true;
}

/// @endcond
