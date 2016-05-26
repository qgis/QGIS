/***************************************************************************
    qgsglobetilesource.cpp
    ---------------------
    begin                : August 2010
    copyright            : (C) 2010 by Pirmin Kalberer
                           (C) 2015 Sandro Mani
    email                : pka at sourcepole dot ch
                           smani at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <osgEarth/Registry>
#include <osgEarth/ImageUtils>

#include "qgscrscache.h"
#include "qgsglobetilesource.h"
#include "qgscoordinatetransform.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaprenderercustompainterjob.h"
#include "qgsmaprendererparalleljob.h"

QgsGlobeTileStatistics* QgsGlobeTileStatistics::s_instance = 0;

QgsGlobeTileStatistics::QgsGlobeTileStatistics() : mTileCount( 0 ), mQueueTileCount( 0 )
{
  s_instance =  this;
}

void QgsGlobeTileStatistics::updateTileCount( int change )
{
  mMutex.lock();
  mTileCount += change;
  emit changed( mQueueTileCount, mTileCount );
  mMutex.unlock();
}

void QgsGlobeTileStatistics::updateQueueTileCount( int num )
{
  mMutex.lock();
  mQueueTileCount = num;
  emit changed( mQueueTileCount, mTileCount );
  mMutex.unlock();
}

///////////////////////////////////////////////////////////////////////////////

QgsGlobeTileImage::QgsGlobeTileImage( QgsGlobeTileSource* tileSource, const QgsRectangle& tileExtent, int tileSize , int tileLod )
    : osg::Image()
    , mTileSource( tileSource )
    , mTileExtent( tileExtent )
    , mTileSize( tileSize )
    , mImageUpdatePending( false )
    , mLod( tileLod )
{
#ifdef GLOBE_SHOW_TILE_STATS
  QgsGlobeTileStatistics::instance()->updateTileCount( + 1 );
#endif
  mTileData = new unsigned char[mTileSize * mTileSize * 4];
  std::memset( mTileData, 0, mTileSize * mTileSize * 4 );
#if 0
  setImage( mTileSize, mTileSize, 1, 4, // width, height, depth, internal_format
            GL_BGRA, GL_UNSIGNED_BYTE,
            mTileData, osg::Image::NO_DELETE );

  mLastUpdateTime = osgEarth::DateTime().asTimeStamp();
  mTileSource->mTileUpdateManager.addTile( const_cast<QgsGlobeTileImage*>( this ) );
  mDpi = 72;
  mImageUpdatePending = true;
#else
  QImage qImage( mTileData, mTileSize, mTileSize, QImage::Format_ARGB32_Premultiplied );
  QPainter painter( &qImage );
  QgsMapRendererCustomPainterJob job( createSettings( qImage.logicalDpiX(), mTileSource->mLayerSet ), &painter );
  job.renderSynchronously();

  setImage( mTileSize, mTileSize, 1, 4, // width, height, depth, internal_format
            GL_BGRA, GL_UNSIGNED_BYTE,
            mTileData, osg::Image::NO_DELETE );
  flipVertical();
  mDpi = qImage.logicalDpiX();
  mLastUpdateTime = osgEarth::DateTime().asTimeStamp();
#endif
}

QgsGlobeTileImage::~QgsGlobeTileImage()
{
  mTileSource->mTileUpdateManager.removeTile( this );
  delete[] mTileData;
#ifdef GLOBE_SHOW_TILE_STATS
  QgsGlobeTileStatistics::instance()->updateTileCount( -1 );
#endif
}

bool QgsGlobeTileImage::requiresUpdateCall() const
{
  if ( mLastUpdateTime < mTileSource->mLastModifiedTime )
  {
    mLastUpdateTime = mTileSource->mLastModifiedTime;
    if ( !mTileExtent.intersects( mTileSource->mLastUpdateExtent ) )
    {
      return false;
    }
    mTileSource->mTileUpdateManager.addTile( const_cast<QgsGlobeTileImage*>( this ) );
    mImageUpdatePending = true;
    return true;
  }
  return mImageUpdatePending;
}

QgsMapSettings QgsGlobeTileImage::createSettings( int dpi , const QStringList &layerSet ) const
{
  QgsMapSettings settings;
  settings.setBackgroundColor( QColor( Qt::transparent ) );
  settings.setDestinationCrs( QgsCRSCache::instance()->crsByAuthId( GEO_EPSG_CRS_AUTHID ) );
  settings.setCrsTransformEnabled( true );
  settings.setExtent( mTileExtent );
  settings.setLayers( layerSet );
  settings.setFlag( QgsMapSettings::DrawEditingInfo, false );
  settings.setFlag( QgsMapSettings::DrawLabeling, false );
  settings.setFlag( QgsMapSettings::DrawSelection, false );
  settings.setMapUnits( QGis::Degrees );
  settings.setOutputSize( QSize( mTileSize, mTileSize ) );
  settings.setOutputImageFormat( QImage::Format_ARGB32_Premultiplied );
  settings.setOutputDpi( dpi );
  settings.setCustomRenderFlags( "globe" );
  return settings;
}

void QgsGlobeTileImage::update( osg::NodeVisitor * )
{
  if ( !mUpdatedImage.isNull() )
  {
    QgsDebugMsg( QString( "Updating earth tile image: %1" ).arg( mTileExtent.toString( 5 ) ) );
    std::memcpy( mTileData, mUpdatedImage.bits(), mTileSize * mTileSize * 4 );
    setImage( mTileSize, mTileSize, 1, 4, // width, height, depth, internal_format
              GL_BGRA, GL_UNSIGNED_BYTE,
              mTileData, osg::Image::NO_DELETE );
    flipVertical();
    mUpdatedImage = QImage();
    mImageUpdatePending = false;
  }
}

///////////////////////////////////////////////////////////////////////////////

QgsGlobeTileUpdateManager::QgsGlobeTileUpdateManager( QObject* parent )
    : QObject( parent ), mCurrentTile( 0 ), mRenderer( 0 )
{
  connect( this, SIGNAL( startRendering() ), this, SLOT( start() ) );
  connect( this, SIGNAL( cancelRendering() ), this, SLOT( cancel() ) );
}

QgsGlobeTileUpdateManager::~QgsGlobeTileUpdateManager()
{
#ifdef GLOBE_SHOW_TILE_STATS
  QgsGlobeTileStatistics::instance()->updateQueueTileCount( 0 );
#endif
  mTileQueue.clear();
  mCurrentTile = 0;
  if ( mRenderer )
  {
    mRenderer->cancel();
  }
}

void QgsGlobeTileUpdateManager::addTile( QgsGlobeTileImage *tile )
{
  if ( !mTileQueue.contains( tile ) )
  {
    mTileQueue.append( tile );
#ifdef GLOBE_SHOW_TILE_STATS
    QgsGlobeTileStatistics::instance()->updateQueueTileCount( mTileQueue.size() );
#endif
    qSort( mTileQueue.begin(), mTileQueue.end(), QgsGlobeTileImage::lodSort );
  }
  emit startRendering();
}

void QgsGlobeTileUpdateManager::removeTile( QgsGlobeTileImage *tile )
{
  if ( mCurrentTile == tile )
  {
    mCurrentTile = 0;
    if ( mRenderer )
      emit cancelRendering();
  }
  else if ( mTileQueue.contains( tile ) )
  {
    mTileQueue.removeAll( tile );
#ifdef GLOBE_SHOW_TILE_STATS
    QgsGlobeTileStatistics::instance()->updateQueueTileCount( mTileQueue.size() );
#endif
  }
}

void QgsGlobeTileUpdateManager::start()
{
  if ( mRenderer == 0 && !mTileQueue.isEmpty() )
  {
    mCurrentTile = mTileQueue.takeFirst();
#ifdef GLOBE_SHOW_TILE_STATS
    QgsGlobeTileStatistics::instance()->updateQueueTileCount( mTileQueue.size() );
#endif
    mRenderer = new QgsMapRendererParallelJob( mCurrentTile->createSettings( mCurrentTile->dpi(), mLayerSet ) );
    connect( mRenderer, SIGNAL( finished() ), this, SLOT( renderingFinished() ) );
    mRenderer->start();
  }
}

void QgsGlobeTileUpdateManager::cancel()
{
  if ( mRenderer )
    mRenderer->cancel();
}

void QgsGlobeTileUpdateManager::renderingFinished()
{
  if ( mCurrentTile )
  {
    QImage image = mRenderer->renderedImage();
    mCurrentTile->setUpdatedImage( image );
    mCurrentTile = 0;
  }
  mRenderer->deleteLater();
  mRenderer = 0;
  start();
}

///////////////////////////////////////////////////////////////////////////////

QgsGlobeTileSource::QgsGlobeTileSource( QgsMapCanvas* canvas, const osgEarth::TileSourceOptions& options )
    : TileSource( options )
    , mCanvas( canvas )
    , mLastModifiedTime( 0 )
{
}

osgEarth::TileSource::Status QgsGlobeTileSource::initialize( const osgDB::Options* /*dbOptions*/ )
{
  setProfile( osgEarth::Registry::instance()->getGlobalGeodeticProfile() );
  mLastModifiedTime = osgEarth::DateTime().asTimeStamp();
  return STATUS_OK;
}

osg::Image* QgsGlobeTileSource::createImage( const osgEarth::TileKey& key, osgEarth::ProgressCallback* progress )
{
  Q_UNUSED( progress );

  int tileSize = getPixelsPerTile();
  if ( tileSize <= 0 )
  {
    return osgEarth::ImageUtils::createEmptyImage();
  }

  double xmin, ymin, xmax, ymax;
  key.getExtent().getBounds( xmin, ymin, xmax, ymax );
  QgsRectangle tileExtent( xmin, ymin, xmax, ymax );

  QgsDebugMsg( QString( "Create earth tile image: %1" ).arg( tileExtent.toString( 5 ) ) );
  return new QgsGlobeTileImage( this, tileExtent, getPixelsPerTile(), key.getLOD() );
}

bool QgsGlobeTileSource::hasDataInExtent( const osgEarth::GeoExtent &extent ) const
{
  osgEarth::Bounds bounds = extent.bounds();
  QgsRectangle requestExtent( bounds.xMin(), bounds.yMin(), bounds.xMax(), bounds.yMax() );
  return requestExtent.intersects( mViewExtent );
}

bool QgsGlobeTileSource::hasData( const osgEarth::TileKey& key ) const
{
  const osgEarth::GeoExtent& tileExtent = key.getExtent();
  QgsRectangle rect( tileExtent.xMin(), tileExtent.yMin(), tileExtent.xMax(), tileExtent.yMax() );
  return rect.intersects( mViewExtent );
}

void QgsGlobeTileSource::refresh( const QgsRectangle& updateExtent )
{
  osgEarth::TimeStamp old = mLastModifiedTime;
  mLastModifiedTime = osgEarth::DateTime().asTimeStamp();
  mTileUpdateManager.updateLayerSet( mLayerSet );
  mLastUpdateExtent = updateExtent;
  QgsDebugMsg( QString( "Updated QGIS map layer modified time from %1 to %2" ).arg( old ).arg( mLastModifiedTime ) );
  mViewExtent = QgsCoordinateTransformCache::instance()->transform( mCanvas->mapSettings().destinationCrs().authid(), GEO_EPSG_CRS_AUTHID )->transform( mCanvas->fullExtent() );
}

void QgsGlobeTileSource::setLayerSet( const QStringList &layerSet )
{
  mLayerSet = layerSet;
}

const QStringList& QgsGlobeTileSource::layerSet() const
{
  return mLayerSet;
}
