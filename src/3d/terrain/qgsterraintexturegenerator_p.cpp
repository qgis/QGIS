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

///@cond PRIVATE

QgsTerrainTextureGenerator::QgsTerrainTextureGenerator( const Qgs3DMapSettings &map )
  : mMap( map )
  , mLastJobId( 0 )
{
}

int QgsTerrainTextureGenerator::render( const QgsRectangle &extent, const QString &debugText )
{
  QgsMapSettings mapSettings( baseMapSettings() );
  mapSettings.setExtent( extent );

  QgsMapRendererSequentialJob *job = new QgsMapRendererSequentialJob( mapSettings );
  connect( job, &QgsMapRendererJob::finished, this, &QgsTerrainTextureGenerator::onRenderingFinished );
  job->start();

  JobData jobData;
  jobData.jobId = ++mLastJobId;
  jobData.job = job;
  jobData.extent = extent;
  jobData.debugText = debugText;

  mJobs.insert( job, jobData );
  //qDebug() << "added job: " << jobData.jobId << "  .... in queue: " << jobs.count();
  return jobData.jobId;
}

void QgsTerrainTextureGenerator::cancelJob( int jobId )
{
  Q_FOREACH ( const JobData &jd, mJobs )
  {
    if ( jd.jobId == jobId )
    {
      //qDebug() << "canceling job " << jobId;
      jd.job->cancelWithoutBlocking();
      disconnect( jd.job, &QgsMapRendererJob::finished, this, &QgsTerrainTextureGenerator::onRenderingFinished );
      jd.job->deleteLater();
      mJobs.remove( jd.job );
      return;
    }
  }
  Q_ASSERT( false && "requested job ID does not exist!" );
}

QImage QgsTerrainTextureGenerator::renderSynchronously( const QgsRectangle &extent, const QString &debugText )
{
  QgsMapSettings mapSettings( baseMapSettings() );
  mapSettings.setExtent( extent );

  QImage img = QImage( mapSettings.outputSize(), mapSettings.outputImageFormat() );
  img.setDotsPerMeterX( 1000 * mapSettings.outputDpi() / 25.4 );
  img.setDotsPerMeterY( 1000 * mapSettings.outputDpi() / 25.4 );
  img.fill( Qt::transparent );

  QPainter p( &img );

  QgsMapRendererCustomPainterJob job( mapSettings, &p );
  job.renderSynchronously();

  if ( mMap.showTerrainTilesInfo() )
  {
    // extra tile information for debugging
    p.setPen( Qt::white );
    p.drawRect( 0, 0, img.width() - 1, img.height() - 1 );
    p.drawText( img.rect(), debugText, QTextOption( Qt::AlignCenter ) );
  }

  p.end();

  return img;
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

  //qDebug() << "finished job " << jobData.jobId << "  ... in queue: " << jobs.count();

  // pass QImage further
  emit tileReady( jobData.jobId, img );
}

QgsMapSettings QgsTerrainTextureGenerator::baseMapSettings()
{
  QgsMapSettings mapSettings;

  mapSettings.setOutputSize( QSize( mMap.mapTileResolution(), mMap.mapTileResolution() ) );
  mapSettings.setDestinationCrs( mMap.crs() );
  mapSettings.setBackgroundColor( mMap.backgroundColor() );
  mapSettings.setFlag( QgsMapSettings::DrawLabeling, mMap.showLabels() );
  mapSettings.setTransformContext( mMap.transformContext() );
  mapSettings.setPathResolver( mMap.pathResolver() );

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
