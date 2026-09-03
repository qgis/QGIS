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
#include "qgsline3dsymbol.h"
#include "qgslogger.h"
#include "qgsorientedbox3d.h"
#include "qgspoint3dsymbol.h"
#include "qgspolygon3dsymbol.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerfeatureiterator.h"
#include "qgswkbtypes.h"

#include <QString>
#include <Qt3DCore/QTransform>
#include <Qt3DRender/QGeometryRenderer>
#include <QtConcurrentRun>

#include "moc_qgsvectorlayerchunkloader_p.cpp"

using namespace Qt::StringLiterals;

///@cond PRIVATE


QgsVectorLayerChunkLoader::QgsVectorLayerChunkLoader( const QgsVectorLayerChunkLoaderFactory *factory, QgsChunkNode *node )
  : QgsChunkLoader( node )
  , mFactory( factory )
  , mRenderContext( factory->mRenderContext )
  , mSource( new QgsVectorLayerFeatureSource( factory->mLayer ) )
{}

void QgsVectorLayerChunkLoader::start()
{
  QgsChunkNode *node = chunk();

  QgsVectorLayer *layer = mFactory->mLayer;
  mLayerName = mFactory->mLayer->name();

  QgsFeature3DHandler *handler = QgsApplication::symbol3DRegistry()->createHandlerForSymbol( layer, mFactory->mSymbol.get() );
  if ( !handler )
  {
    QgsDebugError( u"Unknown 3D symbol type for vector layer: "_s + mFactory->mSymbol->type() );
    return;
  }
  mHandler.reset( handler );

  QgsExpressionContext exprContext;
  exprContext.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) );
  exprContext.setFields( layer->fields() );
  mRenderContext.setExpressionContext( exprContext );

  QSet<QString> attributeNames;
  if ( !mHandler->prepare( mRenderContext, attributeNames, node->box3D() ) )
  {
    QgsDebugError( u"Failed to prepare 3D feature handler!"_s );
    return;
  }

  // build the feature request
  // only a subset of data to be queried
  QgsFeatureRequest req;
  req.setSubsetOfAttributes( attributeNames, layer->fields() );

  QgsCoordinateTransform layerToRenderCrs;
  if ( mFactory->mIsGeocentric )
  {
    layerToRenderCrs = QgsCoordinateTransform( layer->crs3D(), mRenderContext.crs(), mRenderContext.transformContext() );

    QgsRectangle filterRect;
    if ( layer->crs().type() == Qgis::CrsType::Geocentric )
    {
      filterRect = QgsVectorLayerChunkLoaderFactory::box3DTransformedExtent( node->box3D(), layerToRenderCrs, Qgis::TransformDirection::Reverse );
    }
    else
    {
      const QgsRectangle lonLatRect = mFactory->nodeIdToLonLatRect( node->tileId() );
      filterRect = Qgs3DUtils::tryReprojectExtent2D( lonLatRect, mFactory->mCrsToLatLon.destinationCrs(), layer->crs(), mRenderContext.transformContext() );
    }
    req.setFilterRect( filterRect );
  }
  else
  {
    req.setCoordinateTransform( QgsCoordinateTransform( layer->crs3D(), mRenderContext.crs(), mRenderContext.transformContext() ) );
    req.setFilterRect( node->box3D().toRectangle() );
  }

  //
  // this will be run in a background thread
  //
  mFutureWatcher = new QFutureWatcher<void>( this );

  connect( mFutureWatcher, &QFutureWatcher<void>::finished, this, [this] {
    if ( !mCanceled )
      mFactory->mNodesAreLeafs[mNode->tileId().text()] = mNodeIsLeaf;
  } );

  connect( mFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsChunkQueueJob::finished );

  const bool isGeocentric = mFactory->mIsGeocentric;
  const QFuture<void> future = QtConcurrent::run( [req = std::move( req ), layerToRenderCrs, isGeocentric, this] {
    const QgsScopedEvent e( u"3D"_s, u"VL chunk load"_s );

    QgsFeature f;
    QgsFeatureIterator fi = mSource->getFeatures( req );
    int featureCount = 0;
    bool featureLimitReached = false;
    while ( fi.nextFeature( f ) )
    {
      if ( mCanceled )
        return;

      if ( ++featureCount > mFactory->mMaxFeatures )
      {
        featureLimitReached = true;
        break;
      }

      if ( isGeocentric )
      {
        QgsGeometry g = f.geometry();
        if ( !g.constGet()->is3D() )
          g.get()->addZValue( 0 );

        g.transform( layerToRenderCrs, Qgis::TransformDirection::Forward, true );
        f.setGeometry( g );
      }

      mRenderContext.expressionContext().setFeature( f );
      mHandler->processFeature( f, mRenderContext );
    }

    if ( !featureLimitReached )
    {
      QgsDebugMsgLevel( u"All features fetched for node: %1"_s.arg( mNode->tileId().text() ), 3 );

      if ( featureCount == 0 || std::max<double>( mNode->box3D().width(), mNode->box3D().height() ) < QgsVectorLayer3DTilingSettings::maximumLeafExtent() )
        mNodeIsLeaf = true;
    }
  } );

  // emit finished() as soon as the handler is populated with features
  mFutureWatcher->setFuture( future );
}

QgsVectorLayerChunkLoader::~QgsVectorLayerChunkLoader()
{
  if ( mFutureWatcher && !mFutureWatcher->isFinished() )
  {
    disconnect( mFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsChunkQueueJob::finished );
    mFutureWatcher->waitForFinished();
  }
}

void QgsVectorLayerChunkLoader::cancel()
{
  mCanceled = true;
}

Qt3DCore::QEntity *QgsVectorLayerChunkLoader::createEntity( Qt3DCore::QEntity *parent )
{
  if ( mHandler->featureCount() == 0 )
  {
    // an empty node, so we return no entity. This tags the node as having no data and effectively removes it.
    // we just make sure first that its initial estimated vertical range does not affect its parents' bboxes calculation
    mNode->setExactBox3D( QgsBox3D() );
    mNode->updateParentBoundingBoxesRecursively();
    return nullptr;
  }

  Qt3DCore::QEntity *entity = new Qt3DCore::QEntity( parent );
  entity->setObjectName( mLayerName + "_" + mNode->tileId().text() );
  mHandler->finalize( entity, mRenderContext );

  // fix the vertical range of the node from the estimated vertical range to the true range
  if ( mHandler->zMinimum() != std::numeric_limits<float>::max() && mHandler->zMaximum() != std::numeric_limits<float>::lowest() )
  {
    QgsBox3D box = mNode->box3D();
    box.setZMinimum( mHandler->zMinimum() );
    box.setZMaximum( mHandler->zMaximum() );
    mNode->setExactBox3D( box );
    mNode->updateParentBoundingBoxesRecursively();
  }

  return entity;
}


///////////////


QgsVectorLayerChunkLoaderFactory::QgsVectorLayerChunkLoaderFactory( const Qgs3DRenderContext &context, QgsVectorLayer *vl, QgsAbstract3DSymbol *symbol, double zMin, double zMax, int maxFeatures )
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

    mRadiusX = mCrsToLatLon.transform( QgsVector3D( 0, 0, 0 ), Qgis::TransformDirection::Reverse ).x();
    mRadiusY = mCrsToLatLon.transform( QgsVector3D( 90, 0, 0 ), Qgis::TransformDirection::Reverse ).y();
    mRadiusZ = mCrsToLatLon.transform( QgsVector3D( 0, 90, 0 ), Qgis::TransformDirection::Reverse ).z();

    QgsRectangle layerExtentLonLat;
    if ( mLayer->crs().type() == Qgis::CrsType::Geocentric )
    {
      const QgsCoordinateTransform layerToLatLon( mLayer->crs(), geographicCrs, context.transformContext() );
      layerExtentLonLat = box3DTransformedExtent( mLayer->extent3D(), layerToLatLon );
    }
    else
    {
      layerExtentLonLat = Qgs3DUtils::tryReprojectExtent2D( mLayer->extent(), mLayer->crs(), geographicCrs, context.transformContext() );
    }

    if ( layerExtentLonLat.isValid() )
    {
      layerExtentLonLat.grow( 0.01 );
      layerExtentLonLat = layerExtentLonLat.intersect( QgsRectangle( -180, -90, 180, 90 ) );
    }

    mRootNodeId = rootTileIdForExtent( layerExtentLonLat );

    const QgsBox3D rootBox3D = tileIdToBox3D( mRootNodeId );
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

QgsChunkLoader *QgsVectorLayerChunkLoaderFactory::createChunkLoader( QgsChunkNode *node ) const
{
  return new QgsVectorLayerChunkLoader( this, node );
}

QgsChunkNode *QgsVectorLayerChunkLoaderFactory::createRootNode() const
{
  if ( mIsGeocentric )
    return new QgsChunkNode( mRootNodeId, mRootBox3D, mRootError );

  return QgsQuadtreeChunkLoaderFactory::createRootNode();
}

bool QgsVectorLayerChunkLoaderFactory::canCreateChildren( QgsChunkNode *node )
{
  return mNodesAreLeafs.contains( node->tileId().text() );
}

QVector<QgsChunkNode *> QgsVectorLayerChunkLoaderFactory::createChildren( QgsChunkNode *node ) const
{
  if ( mNodesAreLeafs.value( node->tileId().text(), false ) )
    return {};

  if ( !mIsGeocentric )
    return QgsQuadtreeChunkLoaderFactory::createChildren( node );

  QVector<QgsChunkNode *> children;
  if ( mMaxLevel != -1 && node->level() >= mMaxLevel )
    return children;

  const QgsChunkNodeId nodeId = node->tileId();
  const float childError = node->error() / 2;

  if ( nodeId.d == 0 )
  {
    const QgsChunkNodeId westId( 1, 0, 0 );
    const QgsChunkNodeId eastId( 1, 1, 0 );
    children << new QgsChunkNode( westId, tileIdToBox3D( westId ), childError, node );
    children << new QgsChunkNode( eastId, tileIdToBox3D( eastId ), childError, node );
    return children;
  }

  for ( int i = 0; i < 4; ++i )
  {
    const int dx = i & 1, dy = !!( i & 2 );
    const QgsChunkNodeId childId( nodeId.d + 1, nodeId.x * 2 + dx, nodeId.y * 2 + dy );
    children << new QgsChunkNode( childId, tileIdToBox3D( childId ), childError, node );
  }
  return children;
}

QgsRectangle QgsVectorLayerChunkLoaderFactory::nodeIdToLonLatRect( QgsChunkNodeId id )
{
  if ( id.d == 0 )
    return QgsRectangle( -180, -90, 180, 90 );

  const double tileSize = 180.0 / std::pow( 2.0, id.d - 1 );
  const double lonMin = id.x * tileSize - 180.0;
  const double latMin = id.y * tileSize - 90.0;
  return QgsRectangle( lonMin, latMin, lonMin + tileSize, latMin + tileSize );
}

QgsChunkNodeId QgsVectorLayerChunkLoaderFactory::rootTileIdForExtent( const QgsRectangle &lonLatExtent )
{
  QgsChunkNodeId id( 0, 0, 0 );
  if ( !lonLatExtent.isValid() )
    return id;

  bool descended = true;
  while ( descended )
  {
    descended = false;
    const int childCount = id.d == 0 ? 2 : 4;
    for ( int i = 0; i < childCount; ++i )
    {
      const int dx = i & 1, dy = id.d == 0 ? 0 : !!( i & 2 );
      const QgsChunkNodeId childId( id.d + 1, id.x * 2 + dx, id.y * 2 + dy );
      if ( nodeIdToLonLatRect( childId ).contains( lonLatExtent ) )
      {
        id = childId;
        descended = true;
        break;
      }
    }
  }
  return id;
}

QgsRectangle QgsVectorLayerChunkLoaderFactory::box3DTransformedExtent( const QgsBox3D &box3D, const QgsCoordinateTransform &transform, Qgis::TransformDirection direction )
{
  QgsRectangle rect;
  const QVector<QgsVector3D> corners = QgsOrientedBox3D::fromBox3D( box3D ).corners();
  for ( const QgsVector3D &corner : corners )
  {
    try
    {
      const QgsVector3D transformed = transform.transform( corner, direction );
      if ( rect.isNull() )
        rect = QgsRectangle( transformed.x(), transformed.y(), transformed.x(), transformed.y() );
      else
        rect.combineExtentWith( transformed.x(), transformed.y() );
    }
    catch ( const QgsCsException & )
    {
      QgsDebugError( u"Failed to transform box3D corner while computing extent"_s );
    }
  }

  return rect;
}

QgsBox3D QgsVectorLayerChunkLoaderFactory::tileIdToBox3D( QgsChunkNodeId id ) const
{
  if ( id.d == 0 )
    return QgsBox3D( -mRadiusX, -mRadiusY, -mRadiusZ, mRadiusX, mRadiusY, mRadiusZ );

  if ( id.d == 1 )
  {
    return id.x == 0 ? QgsBox3D( -mRadiusX, -mRadiusY, -mRadiusZ, mRadiusX, 0, mRadiusZ ) : QgsBox3D( -mRadiusX, 0, -mRadiusZ, mRadiusX, mRadiusY, mRadiusZ );
  }

  const QgsRectangle rect = nodeIdToLonLatRect( id );
  QVector<double> x = { rect.xMinimum(), rect.xMinimum(), rect.xMaximum(), rect.xMaximum() };
  QVector<double> y = { rect.yMinimum(), rect.yMaximum(), rect.yMinimum(), rect.yMaximum() };
  QVector<double> z = { 0.0, 0.0, 0.0, 0.0 };

  mCrsToLatLon.transformCoords( x.size(), x.data(), y.data(), z.data(), Qgis::TransformDirection::Reverse );

  QgsBox3D box3D( QgsVector3D( x[0], y[0], z[0] ), QgsVector3D( x[1], y[1], z[1] ) );
  for ( int i = 2; i < x.size(); ++i )
    box3D.combineWith( x[i], y[i], z[i] );

  return box3D;
}


///////////////


QgsVectorLayerChunkedEntity::QgsVectorLayerChunkedEntity(
  Qgs3DMapSettings *map, QgsVectorLayer *vl, double zMin, double zMax, const QgsVectorLayer3DTilingSettings &tilingSettings, QgsAbstract3DSymbol *symbol
)
  : QgsAbstractFeatureBasedChunkedEntity( map, 3, new QgsVectorLayerChunkLoaderFactory( Qgs3DRenderContext::fromMapSettings( map ), vl, symbol, zMin, zMax, tilingSettings.maximumChunkFeatures() ), true )
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
  QgsVectorLayerChunkLoaderFactory *loaderFactory = static_cast<QgsVectorLayerChunkLoaderFactory *>( mChunkLoaderFactory );
  if ( loaderFactory )
  {
    QString symbolType = loaderFactory->mSymbol.get()->type();
    if ( symbolType == "line" )
    {
      QgsLine3DSymbol *lineSymbol = static_cast<QgsLine3DSymbol *>( loaderFactory->mSymbol.get() );
      if ( lineSymbol && lineSymbol->altitudeClamping() == Qgis::AltitudeClamping::Absolute )
      {
        return false;
      }
    }
    else if ( symbolType == "point" )
    {
      QgsPoint3DSymbol *pointSymbol = static_cast<QgsPoint3DSymbol *>( loaderFactory->mSymbol.get() );
      if ( pointSymbol && pointSymbol->altitudeClamping() == Qgis::AltitudeClamping::Absolute )
      {
        return false;
      }
    }
    else if ( symbolType == "polygon" )
    {
      QgsPolygon3DSymbol *polygonSymbol = static_cast<QgsPolygon3DSymbol *>( loaderFactory->mSymbol.get() );
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
