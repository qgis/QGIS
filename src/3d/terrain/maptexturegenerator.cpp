#include "maptexturegenerator.h"

#include <qgsmaprenderercustompainterjob.h>
#include <qgsmaprenderersequentialjob.h>
#include <qgsmapsettings.h>
#include <qgsproject.h>

#include "map3d.h"

MapTextureGenerator::MapTextureGenerator( const Map3D &map )
  : map( map )
  , lastJobId( 0 )
{
}

int MapTextureGenerator::render( const QgsRectangle &extent, const QString &debugText )
{
  QgsMapSettings mapSettings( baseMapSettings() );
  mapSettings.setExtent( extent );

  QgsMapRendererSequentialJob *job = new QgsMapRendererSequentialJob( mapSettings );
  connect( job, &QgsMapRendererJob::finished, this, &MapTextureGenerator::onRenderingFinished );
  job->start();

  JobData jobData;
  jobData.jobId = ++lastJobId;
  jobData.job = job;
  jobData.extent = extent;
  jobData.debugText = debugText;

  jobs.insert( job, jobData );
  //qDebug() << "added job: " << jobData.jobId << "  .... in queue: " << jobs.count();
  return jobData.jobId;
}

void MapTextureGenerator::cancelJob( int jobId )
{
  Q_FOREACH ( const JobData &jd, jobs )
  {
    if ( jd.jobId == jobId )
    {
      //qDebug() << "cancelling job " << jobId;
      jd.job->cancelWithoutBlocking();
      disconnect( jd.job, &QgsMapRendererJob::finished, this, &MapTextureGenerator::onRenderingFinished );
      jd.job->deleteLater();
      jobs.remove( jd.job );
      return;
    }
  }
  Q_ASSERT( false && "requested job ID does not exist!" );
}

QImage MapTextureGenerator::renderSynchronously( const QgsRectangle &extent, const QString &debugText )
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

  if ( map.showTerrainTilesInfo() )
  {
    // extra tile information for debugging
    p.setPen( Qt::white );
    p.drawRect( 0, 0, img.width() - 1, img.height() - 1 );
    p.drawText( img.rect(), debugText, QTextOption( Qt::AlignCenter ) );
  }

  p.end();

  return img;
}


void MapTextureGenerator::onRenderingFinished()
{
  QgsMapRendererSequentialJob *mapJob = static_cast<QgsMapRendererSequentialJob *>( sender() );

  Q_ASSERT( jobs.contains( mapJob ) );
  JobData jobData = jobs.value( mapJob );

  QImage img = mapJob->renderedImage();

  if ( map.showTerrainTilesInfo() )
  {
    // extra tile information for debugging
    QPainter p( &img );
    p.setPen( Qt::white );
    p.drawRect( 0, 0, img.width() - 1, img.height() - 1 );
    p.drawText( img.rect(), jobData.debugText, QTextOption( Qt::AlignCenter ) );
    p.end();
  }

  mapJob->deleteLater();
  jobs.remove( mapJob );

  //qDebug() << "finished job " << jobData.jobId << "  ... in queue: " << jobs.count();

  // pass QImage further
  emit tileReady( jobData.jobId, img );
}

QgsMapSettings MapTextureGenerator::baseMapSettings()
{
  QgsMapSettings mapSettings;
  mapSettings.setLayers( map.layers() );
  mapSettings.setOutputSize( QSize( map.mapTileResolution(), map.mapTileResolution() ) );
  mapSettings.setDestinationCrs( map.crs );
  mapSettings.setBackgroundColor( map.backgroundColor() );
  return mapSettings;
}
