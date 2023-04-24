/***************************************************************************
  qgsterraintexturegenerator_p.cpp
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

#include "qgsterraintexturegenerator_p.h"

#include <qgsmaprenderercustompainterjob.h>
#include <qgsmaprenderersequentialjob.h>
#include <qgsmapsettings.h>
#include <qgsmapthemecollection.h>
#include <qgsproject.h>

#include "qgs3dmapsettings.h"

#include "qgseventtracing.h"
#include "qgsgeometry.h"
#include "qgspointxy.h"
#include "qgsrectangle.h"

///@cond PRIVATE

QgsTerrainTextureGenerator::QgsTerrainTextureGenerator( const Qgs3DMapSettings &map )
  : mMap( map )
  , mLastJobId( 0 )
  , mTextureSize( QSize( mMap.mapTileResolution(), mMap.mapTileResolution() ) )
{
}

int QgsTerrainTextureGenerator::render( const QgsRectangle &extent, QgsChunkNodeId tileId, const QString &debugText )
{
  QgsMapSettings mapSettings( baseMapSettings() );
  mapSettings.setExtent( extent );
  QSize size = QSize( mTextureSize );
  if ( mMap.terrainGenerator()->type() == QgsTerrainGenerator::Flat )
  {
    // The flat terrain generator might have non-square tiles, clipped at the scene's extent.
    // We need to produce non-square textures for those cases.
    const QgsRectangle clippedExtent = extent.intersect( mMap.extent() );
    if ( !qgsDoubleNear( clippedExtent.width(), clippedExtent.height() ) )
    {
      if ( clippedExtent.height() > clippedExtent.width() )
        size.setWidth( std::round( size.width() * clippedExtent.width() / clippedExtent.height() ) );
      else if ( clippedExtent.height() < clippedExtent.width() )
        size.setHeight( std::round( size.height() * clippedExtent.height() / clippedExtent.width() ) );
    }
    mapSettings.setExtent( clippedExtent );
  }
  mapSettings.setOutputSize( size );

  QgsEventTracing::addEvent( QgsEventTracing::AsyncBegin, QStringLiteral( "3D" ), QStringLiteral( "Texture" ), tileId.text() );

  QgsMapRendererSequentialJob *job = new QgsMapRendererSequentialJob( mapSettings );
  connect( job, &QgsMapRendererJob::finished, this, &QgsTerrainTextureGenerator::onRenderingFinished );

  JobData jobData;
  jobData.jobId = ++mLastJobId;
  jobData.tileId = tileId;
  jobData.job = job;
  jobData.extent = extent;
  jobData.debugText = debugText;

  mJobs.insert( job, jobData ); //store job data just before launching the job
  job->start();

  // QgsDebugMsgLevel( QStringLiteral("added job: %1 .... in queue: %2").arg( jobData.jobId ).arg( jobs.count() ), 2);
  return jobData.jobId;
}

void QgsTerrainTextureGenerator::cancelJob( int jobId )
{
  for ( const JobData &jd : std::as_const( mJobs ) )
  {
    if ( jd.jobId == jobId )
    {
      // QgsDebugMsgLevel( QStringLiteral("canceling job %1").arg( jobId ), 2 );
      jd.job->cancelWithoutBlocking();
      disconnect( jd.job, &QgsMapRendererJob::finished, this, &QgsTerrainTextureGenerator::onRenderingFinished );
      jd.job->deleteLater();
      mJobs.remove( jd.job );
      return;
    }
  }
  Q_ASSERT( false && "requested job ID does not exist!" );
}

void QgsTerrainTextureGenerator::waitForFinished()
{
  for ( auto it = mJobs.keyBegin(); it != mJobs.keyEnd(); it++ )
    disconnect( *it, &QgsMapRendererJob::finished, this, &QgsTerrainTextureGenerator::onRenderingFinished );
  QVector<QgsMapRendererSequentialJob *> toBeDeleted;
  for ( auto it = mJobs.constBegin(); it != mJobs.constEnd(); it++ )
  {
    QgsMapRendererSequentialJob *mapJob = it.key();
    mapJob->waitForFinished();
    JobData jobData = it.value();
    toBeDeleted.push_back( mapJob );

    QImage img = mapJob->renderedImage();
    QImage clippedImage = generateRenderedImage( img, jobData );

    // pass QImage further
    emit tileReady( jobData.jobId, clippedImage );
  }

  for ( QgsMapRendererSequentialJob *mapJob : toBeDeleted )
  {
    mJobs.remove( mapJob );
    mapJob->deleteLater();
  }
}

void QgsTerrainTextureGenerator::onRenderingFinished()
{
  QgsMapRendererSequentialJob *mapJob = static_cast<QgsMapRendererSequentialJob *>( sender() );

  Q_ASSERT( mJobs.contains( mapJob ) );
  JobData jobData = mJobs.value( mapJob );

  QImage img = mapJob->renderedImage();
  QImage clippedImage = generateRenderedImage( img, jobData );

  mapJob->deleteLater();
  mJobs.remove( mapJob );

  // QgsDebugMsgLevel( QStringLiteral("finished job %1 ... in queue: %2").arg( jobData.jobId).arg( jobs.count() ), 2 );

  QgsEventTracing::addEvent( QgsEventTracing::AsyncEnd, QStringLiteral( "3D" ), QStringLiteral( "Texture" ), jobData.tileId.text() );

  // pass QImage further
  emit tileReady( jobData.jobId, clippedImage );
}

QgsMapSettings QgsTerrainTextureGenerator::baseMapSettings()
{
  QgsMapSettings mapSettings;

  mapSettings.setOutputSize( mTextureSize );
  mapSettings.setDestinationCrs( mMap.crs() );
  mapSettings.setBackgroundColor( QColor( 0, 0, 0, 0 ) );
  mapSettings.setFlag( Qgis::MapSettingsFlag::DrawLabeling, mMap.showLabels() );
  mapSettings.setFlag( Qgis::MapSettingsFlag::Render3DMap );
  mapSettings.setTransformContext( mMap.transformContext() );
  mapSettings.setPathResolver( mMap.pathResolver() );
  mapSettings.setRendererUsage( mMap.rendererUsage() );

  QList<QgsMapLayer *> layers;
  QgsMapThemeCollection *mapThemes = mMap.mapThemeCollection();
  QString mapThemeName = mMap.terrainMapTheme();
  if ( mapThemeName.isEmpty() || !mapThemes || !mapThemes->hasMapTheme( mapThemeName ) )
  {
    layers = mMap.layers();
  }
  else
  {
    layers = mapThemes->mapThemeVisibleLayers( mapThemeName );
    mapSettings.setLayerStyleOverrides( mapThemes->mapThemeStyleOverrides( mapThemeName ) );
  }
  layers.erase( std::remove_if( layers.begin(),
                                layers.end(),
  []( const QgsMapLayer * layer ) { return layer->renderer3D(); } ),
  layers.end() );
  mapSettings.setLayers( layers );

  return mapSettings;
}

QImage QgsTerrainTextureGenerator::generateRenderedImage( const QImage &initialImage, const JobData &jobData ) const
{
  // the result texture is the intersection between the tile extent and 3D scene extent
  // the intersection is in the 3D world coordinates
  QgsRectangle textureExtent = jobData.extent;
  QgsGeometry textureExtentGeom = QgsGeometry::fromRect( textureExtent );
  QgsGeometry intersection = textureExtentGeom.intersection( mMap.rotatedExtent() );

  // the intersection is in the 3D world coordinates
  // the coordinates need to be to converted in the map coordinates
  // 1. bring back the coordinates in the {0-resolution; 0-resolution} range
  int resolution = std::max( initialImage.width(), initialImage.height() );
  QTransform scaling = QTransform::fromScale( resolution / textureExtent.width(), resolution / textureExtent.height() );
  scaling.translate( -textureExtent.xMinimum(), -textureExtent.yMinimum() );
  intersection.transform( scaling );

  // 2. the Y coordinates need to be inverted
  QVector<QgsPointXY> invertedPoints;
  QgsPolylineXY intersectionPolygon = intersection.asPolygon().at( 0 );
  for ( int i = 0; i < intersectionPolygon.size(); i++ )
  {
    const QgsPointXY point = intersectionPolygon.at( i );
    invertedPoints << QgsPointXY( point.x(), initialImage.height() - point.y() );
  }
  QgsPolygonXY invertedPolygon;
  invertedPolygon << invertedPoints;
  QgsGeometry intersectionMapCoords = QgsGeometry::fromPolygonXY( invertedPolygon );

  // the pixels outside the intersection are transparent
  // the painter uses the computed geometry to draw only the initialImage inside the intersection
  QImage clippedImage( initialImage.size(), initialImage.format() );
  clippedImage.fill( qRgba( 0, 0, 0, 0 ) );
  QPainter imagePainter( &clippedImage );
  imagePainter.setClipPath( intersectionMapCoords.constGet()->asQPainterPath() );
  imagePainter.drawImage( QPoint( 0, 0 ), initialImage );

  if ( mMap.showTerrainTilesInfo() )
  {
    // extra tile information for debugging
    imagePainter.setPen( Qt::red );
    imagePainter.setBackgroundMode( Qt::OpaqueMode );
    QFont font = imagePainter.font();
    font.setPixelSize( std::max( 30, mMap.mapTileResolution() / 6 ) );
    imagePainter.setFont( font );
    imagePainter.drawRect( 0, 0, initialImage.width() - 1, initialImage.height() - 1 );
    imagePainter.drawText( initialImage.rect(), jobData.debugText, QTextOption( Qt::AlignCenter ) );
    imagePainter.end();
  }

  return clippedImage;
}

/// @endcond
