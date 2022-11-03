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

  QList<QgsMapLayer *> layers = mMap.layers();
  QList<QgsMapLayer *> toBeRenderedLayers;
  for ( QgsMapLayer *l : layers )
  {
    if ( l->renderer3D() == nullptr )
      toBeRenderedLayers.push_back( l );
  }
  mapSettings.setLayers( toBeRenderedLayers );

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

    if ( mMap.showTerrainTilesInfo() )
    {
      // extra tile information for debugging
      QPainter p( &img );
      p.setPen( Qt::white );
      p.drawRect( 0, 0, img.width() - 1, img.height() - 1 );
      p.drawText( img.rect(), jobData.debugText, QTextOption( Qt::AlignCenter ) );
      p.end();
    }

    // pass QImage further
    emit tileReady( jobData.jobId, img );
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

  if ( mMap.showTerrainTilesInfo() )
  {
    // extra tile information for debugging
    QPainter p( &img );
    p.setPen( Qt::white );
    p.drawRect( 0, 0, img.width() - 1, img.height() - 1 );
    p.drawText( img.rect(), jobData.debugText, QTextOption( Qt::AlignCenter ) );
    p.end();
  }

  mapJob->deleteLater();
  mJobs.remove( mapJob );

  // QgsDebugMsgLevel( QStringLiteral("finished job %1 ... in queue: %2").arg( jobData.jobId).arg( jobs.count() ), 2 );

  QgsEventTracing::addEvent( QgsEventTracing::AsyncEnd, QStringLiteral( "3D" ), QStringLiteral( "Texture" ), jobData.tileId.text() );

  // pass QImage further
  emit tileReady( jobData.jobId, img );
}

QgsMapSettings QgsTerrainTextureGenerator::baseMapSettings()
{
  QgsMapSettings mapSettings;

  mapSettings.setOutputSize( mTextureSize );
  mapSettings.setDestinationCrs( mMap.crs() );
  mapSettings.setBackgroundColor( mMap.backgroundColor() );
  mapSettings.setFlag( Qgis::MapSettingsFlag::DrawLabeling, mMap.showLabels() );
  mapSettings.setFlag( Qgis::MapSettingsFlag::Render3DMap );
  mapSettings.setTransformContext( mMap.transformContext() );
  mapSettings.setPathResolver( mMap.pathResolver() );
  mapSettings.setRendererUsage( mMap.rendererUsage() );

  QgsMapThemeCollection *mapThemes = mMap.mapThemeCollection();
  QString mapThemeName = mMap.terrainMapTheme();
  if ( mapThemeName.isEmpty() || !mapThemes || !mapThemes->hasMapTheme( mapThemeName ) )
  {
    mapSettings.setLayers( mMap.layers() );
  }
  else
  {
    mapSettings.setLayers( mapThemes->mapThemeVisibleLayers( mapThemeName ) );
    mapSettings.setLayerStyleOverrides( mapThemes->mapThemeStyleOverrides( mapThemeName ) );
  }

  return mapSettings;
}

/// @endcond
