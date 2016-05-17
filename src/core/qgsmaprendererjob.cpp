/***************************************************************************
  qgsmaprendererjob.cpp
  --------------------------------------
  Date                 : December 2013
  Copyright            : (C) 2013 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaprendererjob.h"

#include <QPainter>
#include <QTime>
#include <QTimer>
#include <QtConcurrentMap>
#include <QSettings>

#include "qgscrscache.h"
#include "qgslogger.h"
#include "qgsrendercontext.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaplayerrenderer.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsmaprenderercache.h"
#include "qgsmessagelog.h"
#include "qgspallabeling.h"
#include "qgsvectorlayerrenderer.h"
#include "qgsvectorlayer.h"

QgsMapRendererJob::QgsMapRendererJob( const QgsMapSettings& settings )
    : mSettings( settings )
    , mCache( nullptr )
    , mRenderingTime( 0 )
{
}


QgsMapRendererQImageJob::QgsMapRendererQImageJob( const QgsMapSettings& settings )
    : QgsMapRendererJob( settings )
{
}


QgsMapRendererJob::Errors QgsMapRendererJob::errors() const
{
  return mErrors;
}

void QgsMapRendererJob::setCache( QgsMapRendererCache* cache )
{
  mCache = cache;
}

const QgsMapSettings& QgsMapRendererJob::mapSettings() const
{
  return mSettings;
}


bool QgsMapRendererJob::reprojectToLayerExtent( const QgsMapLayer *ml, const QgsCoordinateTransform *ct, QgsRectangle &extent, QgsRectangle &r2 )
{
  bool split = false;

  try
  {
#ifdef QGISDEBUG
    // QgsLogger::debug<QgsRectangle>("Getting extent of canvas in layers CS. Canvas is ", extent, __FILE__, __FUNCTION__, __LINE__);
#endif
    // Split the extent into two if the source CRS is
    // geographic and the extent crosses the split in
    // geographic coordinates (usually +/- 180 degrees,
    // and is assumed to be so here), and draw each
    // extent separately.
    static const double splitCoord = 180.0;

    if ( ml->crs().geographicFlag() )
    {
      if ( ml->type() == QgsMapLayer::VectorLayer && !ct->destCRS().geographicFlag() )
      {
        // if we transform from a projected coordinate system check
        // check if transforming back roughly returns the input
        // extend - otherwise render the world.
        QgsRectangle extent1 = ct->transformBoundingBox( extent, QgsCoordinateTransform::ReverseTransform );
        QgsRectangle extent2 = ct->transformBoundingBox( extent1, QgsCoordinateTransform::ForwardTransform );

        QgsDebugMsg( QString( "\n0:%1 %2x%3\n1:%4\n2:%5 %6x%7 (w:%8 h:%9)" )
                     .arg( extent.toString() ).arg( extent.width() ).arg( extent.height() )
                     .arg( extent1.toString(), extent2.toString() ).arg( extent2.width() ).arg( extent2.height() )
                     .arg( fabs( 1.0 - extent2.width() / extent.width() ) )
                     .arg( fabs( 1.0 - extent2.height() / extent.height() ) )
                   );

        if ( fabs( 1.0 - extent2.width() / extent.width() ) < 0.5 &&
             fabs( 1.0 - extent2.height() / extent.height() ) < 0.5 )
        {
          extent = extent1;
        }
        else
        {
          extent = QgsRectangle( -180.0, -90.0, 180.0, 90.0 );
        }
      }
      else
      {
        // Note: ll = lower left point
        QgsPoint ll = ct->transform( extent.xMinimum(), extent.yMinimum(),
                                     QgsCoordinateTransform::ReverseTransform );

        //   and ur = upper right point
        QgsPoint ur = ct->transform( extent.xMaximum(), extent.yMaximum(),
                                     QgsCoordinateTransform::ReverseTransform );

        QgsDebugMsg( QString( "in:%1 (ll:%2 ur:%3)" ).arg( extent.toString(), ll.toString(), ur.toString() ) );

        extent = ct->transformBoundingBox( extent, QgsCoordinateTransform::ReverseTransform );

        QgsDebugMsg( QString( "out:%1 (w:%2 h:%3)" ).arg( extent.toString() ).arg( extent.width() ).arg( extent.height() ) );

        if ( ll.x() > ur.x() )
        {
          // the coordinates projected in reverse order than what one would expect.
          // we are probably looking at an area that includes longitude of 180 degrees.
          // we need to take into account coordinates from two intervals: (-180,x1) and (x2,180)
          // so let's use (-180,180). This hopefully does not add too much overhead. It is
          // more straightforward than rendering with two separate extents and more consistent
          // for rendering, labeling and caching as everything is rendered just in one go
          extent.setXMinimum( -splitCoord );
          extent.setXMaximum( splitCoord );
        }
      }

      // TODO: the above rule still does not help if using a projection that covers the whole
      // world. E.g. with EPSG:3857 the longitude spectrum -180 to +180 is mapped to approx.
      // -2e7 to +2e7. Converting extent from -5e7 to +5e7 is transformed as -90 to +90,
      // but in fact the extent should cover the whole world.
    }
    else // can't cross 180
    {
      if ( ct->destCRS().geographicFlag() &&
           ( extent.xMinimum() <= -180 || extent.xMaximum() >= 180 ||
             extent.yMinimum() <=  -90 || extent.yMaximum() >=  90 ) )
        // Use unlimited rectangle because otherwise we may end up transforming wrong coordinates.
        // E.g. longitude -200 to +160 would be understood as +40 to +160 due to periodicity.
        // We could try to clamp coords to (-180,180) for lon resp. (-90,90) for lat,
        // but this seems like a safer choice.
        extent = QgsRectangle( -DBL_MAX, -DBL_MAX, DBL_MAX, DBL_MAX );
      else
        extent = ct->transformBoundingBox( extent, QgsCoordinateTransform::ReverseTransform );
    }
  }
  catch ( QgsCsException &cse )
  {
    Q_UNUSED( cse );
    QgsDebugMsg( "Transform error caught" );
    extent = QgsRectangle( -DBL_MAX, -DBL_MAX, DBL_MAX, DBL_MAX );
    r2     = QgsRectangle( -DBL_MAX, -DBL_MAX, DBL_MAX, DBL_MAX );
  }

  return split;
}



LayerRenderJobs QgsMapRendererJob::prepareJobs( QPainter* painter, QgsPalLabeling* labelingEngine, QgsLabelingEngineV2* labelingEngine2 )
{
  LayerRenderJobs layerJobs;

  // render all layers in the stack, starting at the base
  QListIterator<QString> li( mSettings.layers() );
  li.toBack();

  if ( mCache )
  {
    bool cacheValid = mCache->init( mSettings.visibleExtent(), mSettings.scale() );
    QgsDebugMsg( QString( "CACHE VALID: %1" ).arg( cacheValid ) );
    Q_UNUSED( cacheValid );
  }

  mGeometryCaches.clear();

  while ( li.hasPrevious() )
  {
    QString layerId = li.previous();

    QgsDebugMsg( "Rendering at layer item " + layerId );

    QgsMapLayer *ml = QgsMapLayerRegistry::instance()->mapLayer( layerId );

    if ( !ml )
    {
      mErrors.append( Error( layerId, tr( "Layer not found in registry." ) ) );
      continue;
    }

    QgsDebugMsg( QString( "layer %1:  minscale:%2  maxscale:%3  scaledepvis:%4  blendmode:%5" )
                 .arg( ml->name() )
                 .arg( ml->minimumScale() )
                 .arg( ml->maximumScale() )
                 .arg( ml->hasScaleBasedVisibility() )
                 .arg( ml->blendMode() )
               );

    if ( !ml->isInScaleRange( mSettings.scale() ) ) //|| mOverview )
    {
      QgsDebugMsg( "Layer not rendered because it is not within the defined visibility scale range" );
      continue;
    }

    QgsRectangle r1 = mSettings.visibleExtent(), r2;
    const QgsCoordinateTransform* ct = nullptr;

    if ( mSettings.hasCrsTransformEnabled() )
    {
      ct = mSettings.layerTransform( ml );
      if ( ct )
      {
        reprojectToLayerExtent( ml, ct, r1, r2 );
      }
      QgsDebugMsg( "extent: " + r1.toString() );
      if ( !r1.isFinite() || !r2.isFinite() )
      {
        mErrors.append( Error( layerId, tr( "There was a problem transforming the layer's extent. Layer skipped." ) ) );
        continue;
      }
    }

    // Force render of layers that are being edited
    // or if there's a labeling engine that needs the layer to register features
    if ( mCache && ml->type() == QgsMapLayer::VectorLayer )
    {
      QgsVectorLayer* vl = qobject_cast<QgsVectorLayer *>( ml );
      if ( vl->isEditable() || (( labelingEngine || labelingEngine2 ) && QgsPalLabeling::staticWillUseLayer( vl ) ) )
        mCache->clearCacheImage( ml->id() );
    }

    layerJobs.append( LayerRenderJob() );
    LayerRenderJob& job = layerJobs.last();
    job.cached = false;
    job.img = nullptr;
    job.blendMode = ml->blendMode();
    job.layerId = ml->id();
    job.renderingTime = -1;

    job.context = QgsRenderContext::fromMapSettings( mSettings );
    job.context.setPainter( painter );
    job.context.setLabelingEngine( labelingEngine );
    job.context.setLabelingEngineV2( labelingEngine2 );
    job.context.setCoordinateTransform( ct );
    job.context.setExtent( r1 );

    // if we can use the cache, let's do it and avoid rendering!
    if ( mCache && !mCache->cacheImage( ml->id() ).isNull() )
    {
      job.cached = true;
      job.img = new QImage( mCache->cacheImage( ml->id() ) );
      job.renderer = nullptr;
      job.context.setPainter( nullptr );
      continue;
    }

    // If we are drawing with an alternative blending mode then we need to render to a separate image
    // before compositing this on the map. This effectively flattens the layer and prevents
    // blending occurring between objects on the layer
    if ( mCache || !painter || needTemporaryImage( ml ) )
    {
      // Flattened image for drawing when a blending mode is set
      QImage * mypFlattenedImage = nullptr;
      mypFlattenedImage = new QImage( mSettings.outputSize().width(),
                                      mSettings.outputSize().height(),
                                      mSettings.outputImageFormat() );
      if ( mypFlattenedImage->isNull() )
      {
        mErrors.append( Error( layerId, tr( "Insufficient memory for image %1x%2" ).arg( mSettings.outputSize().width() ).arg( mSettings.outputSize().height() ) ) );
        delete mypFlattenedImage;
        layerJobs.removeLast();
        continue;
      }
      mypFlattenedImage->fill( 0 );

      job.img = mypFlattenedImage;
      QPainter* mypPainter = new QPainter( job.img );
      mypPainter->setRenderHint( QPainter::Antialiasing, mSettings.testFlag( QgsMapSettings::Antialiasing ) );
      job.context.setPainter( mypPainter );
    }

    bool hasStyleOverride = mSettings.layerStyleOverrides().contains( ml->id() );
    if ( hasStyleOverride )
      ml->styleManager()->setOverrideStyle( mSettings.layerStyleOverrides().value( ml->id() ) );

    job.renderer = ml->createMapRenderer( job.context );

    if ( hasStyleOverride )
      ml->styleManager()->restoreOverrideStyle();

    if ( mRequestedGeomCacheForLayers.contains( ml->id() ) )
    {
      if ( QgsVectorLayerRenderer* vlr = dynamic_cast<QgsVectorLayerRenderer*>( job.renderer ) )
      {
        vlr->setGeometryCachePointer( &mGeometryCaches[ ml->id()] );
      }
    }

  } // while (li.hasPrevious())

  return layerJobs;
}


void QgsMapRendererJob::cleanupJobs( LayerRenderJobs& jobs )
{
  for ( LayerRenderJobs::iterator it = jobs.begin(); it != jobs.end(); ++it )
  {
    LayerRenderJob& job = *it;
    if ( job.img )
    {
      delete job.context.painter();
      job.context.setPainter( nullptr );

      if ( mCache && !job.cached && !job.context.renderingStopped() )
      {
        QgsDebugMsg( "caching image for " + job.layerId );
        mCache->setCacheImage( job.layerId, *job.img );
      }

      delete job.img;
      job.img = nullptr;
    }

    if ( job.renderer )
    {
      Q_FOREACH ( const QString& message, job.renderer->errors() )
        mErrors.append( Error( job.renderer->layerID(), message ) );

      delete job.renderer;
      job.renderer = nullptr;
    }
  }

  jobs.clear();

  updateLayerGeometryCaches();
}


QImage QgsMapRendererJob::composeImage( const QgsMapSettings& settings, const LayerRenderJobs& jobs )
{
  QImage image( settings.outputSize(), settings.outputImageFormat() );
  image.fill( settings.backgroundColor().rgba() );

  QPainter painter( &image );

  for ( LayerRenderJobs::const_iterator it = jobs.constBegin(); it != jobs.constEnd(); ++it )
  {
    const LayerRenderJob& job = *it;

    painter.setCompositionMode( job.blendMode );

    Q_ASSERT( job.img );
    painter.drawImage( 0, 0, *job.img );
  }

  painter.end();
  return image;
}

void QgsMapRendererJob::logRenderingTime( const LayerRenderJobs& jobs )
{
  QSettings settings;
  if ( !settings.value( "/Map/logCanvasRefreshEvent", false ).toBool() )
    return;

  QMultiMap<int, QString> elapsed;
  Q_FOREACH ( const LayerRenderJob& job, jobs )
    elapsed.insert( job.renderingTime, job.layerId );

  QList<int> tt( elapsed.uniqueKeys() );
  qSort( tt.begin(), tt.end(), qGreater<int>() );
  Q_FOREACH ( int t, tt )
  {
    QgsMessageLog::logMessage( tr( "%1 ms: %2" ).arg( t ).arg( QStringList( elapsed.values( t ) ).join( ", " ) ), tr( "Rendering" ) );
  }
  QgsMessageLog::logMessage( "---", tr( "Rendering" ) );
}
