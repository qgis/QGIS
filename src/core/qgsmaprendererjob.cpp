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

#include "qgslogger.h"
#include "qgsrendercontext.h"
#include "qgsmaplayer.h"
#include "qgsproject.h"
#include "qgsmaplayerrenderer.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsmaprenderercache.h"
#include "qgsmessagelog.h"
#include "qgspallabeling.h"
#include "qgsvectorlayerrenderer.h"
#include "qgsvectorlayer.h"
#include "qgsexception.h"
#include "qgslabelingengine.h"
#include "qgsmaplayerlistutils.h"
#include "qgsvectorlayerlabeling.h"
#include "qgssettings.h"
#include "qgsexpressioncontextutils.h"

///@cond PRIVATE

const QString QgsMapRendererJob::LABEL_CACHE_ID = QStringLiteral( "_labels_" );

QgsMapRendererJob::QgsMapRendererJob( const QgsMapSettings &settings )
  : mSettings( settings )

{
}


QgsMapRendererQImageJob::QgsMapRendererQImageJob( const QgsMapSettings &settings )
  : QgsMapRendererJob( settings )
{
}


QgsMapRendererJob::Errors QgsMapRendererJob::errors() const
{
  return mErrors;
}

void QgsMapRendererJob::setCache( QgsMapRendererCache *cache )
{
  mCache = cache;
}

QHash<QgsMapLayer *, int> QgsMapRendererJob::perLayerRenderingTime() const
{
  QHash<QgsMapLayer *, int> result;
  for ( auto it = mPerLayerRenderingTime.constBegin(); it != mPerLayerRenderingTime.constEnd(); ++it )
  {
    if ( it.key() )
      result.insert( it.key(), it.value() );
  }
  return result;
}

const QgsMapSettings &QgsMapRendererJob::mapSettings() const
{
  return mSettings;
}

bool QgsMapRendererJob::prepareLabelCache() const
{
  bool canCache = mCache;

  // calculate which layers will be labeled
  QSet< QgsMapLayer * > labeledLayers;
  const QList<QgsMapLayer *> layers = mSettings.layers();
  for ( const QgsMapLayer *ml : layers )
  {
    QgsVectorLayer *vl = const_cast< QgsVectorLayer * >( qobject_cast<const QgsVectorLayer *>( ml ) );
    if ( vl && QgsPalLabeling::staticWillUseLayer( vl ) )
      labeledLayers << vl;
    if ( vl && vl->labelsEnabled() && vl->labeling()->requiresAdvancedEffects() )
    {
      canCache = false;
      break;
    }
  }

  if ( mCache && mCache->hasCacheImage( LABEL_CACHE_ID ) )
  {
    // we may need to clear label cache and re-register labeled features - check for that here

    // can we reuse the cached label solution?
    bool canUseCache = canCache && mCache->dependentLayers( LABEL_CACHE_ID ).toSet() == labeledLayers;
    if ( !canUseCache )
    {
      // no - participating layers have changed
      mCache->clearCacheImage( LABEL_CACHE_ID );
    }
  }
  return canCache;
}


bool QgsMapRendererJob::reprojectToLayerExtent( const QgsMapLayer *ml, const QgsCoordinateTransform &ct, QgsRectangle &extent, QgsRectangle &r2 )
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
    static const double SPLIT_COORD = 180.0;

    if ( ml->crs().isGeographic() )
    {
      if ( ml->type() == QgsMapLayer::VectorLayer && !ct.destinationCrs().isGeographic() )
      {
        // if we transform from a projected coordinate system check
        // check if transforming back roughly returns the input
        // extend - otherwise render the world.
        QgsRectangle extent1 = ct.transformBoundingBox( extent, QgsCoordinateTransform::ReverseTransform );
        QgsRectangle extent2 = ct.transformBoundingBox( extent1, QgsCoordinateTransform::ForwardTransform );

        QgsDebugMsgLevel( QStringLiteral( "\n0:%1 %2x%3\n1:%4\n2:%5 %6x%7 (w:%8 h:%9)" )
                          .arg( extent.toString() ).arg( extent.width() ).arg( extent.height() )
                          .arg( extent1.toString(), extent2.toString() ).arg( extent2.width() ).arg( extent2.height() )
                          .arg( std::fabs( 1.0 - extent2.width() / extent.width() ) )
                          .arg( std::fabs( 1.0 - extent2.height() / extent.height() ) )
                          , 3 );

        if ( std::fabs( 1.0 - extent2.width() / extent.width() ) < 0.5 &&
             std::fabs( 1.0 - extent2.height() / extent.height() ) < 0.5 )
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
        QgsPointXY ll = ct.transform( extent.xMinimum(), extent.yMinimum(),
                                      QgsCoordinateTransform::ReverseTransform );

        //   and ur = upper right point
        QgsPointXY ur = ct.transform( extent.xMaximum(), extent.yMaximum(),
                                      QgsCoordinateTransform::ReverseTransform );

        QgsDebugMsgLevel( QStringLiteral( "in:%1 (ll:%2 ur:%3)" ).arg( extent.toString(), ll.toString(), ur.toString() ), 4 );

        extent = ct.transformBoundingBox( extent, QgsCoordinateTransform::ReverseTransform );

        QgsDebugMsgLevel( QStringLiteral( "out:%1 (w:%2 h:%3)" ).arg( extent.toString() ).arg( extent.width() ).arg( extent.height() ), 4 );

        if ( ll.x() > ur.x() )
        {
          // the coordinates projected in reverse order than what one would expect.
          // we are probably looking at an area that includes longitude of 180 degrees.
          // we need to take into account coordinates from two intervals: (-180,x1) and (x2,180)
          // so let's use (-180,180). This hopefully does not add too much overhead. It is
          // more straightforward than rendering with two separate extents and more consistent
          // for rendering, labeling and caching as everything is rendered just in one go
          extent.setXMinimum( -SPLIT_COORD );
          extent.setXMaximum( SPLIT_COORD );
        }
      }

      // TODO: the above rule still does not help if using a projection that covers the whole
      // world. E.g. with EPSG:3857 the longitude spectrum -180 to +180 is mapped to approx.
      // -2e7 to +2e7. Converting extent from -5e7 to +5e7 is transformed as -90 to +90,
      // but in fact the extent should cover the whole world.
    }
    else // can't cross 180
    {
      if ( ct.destinationCrs().isGeographic() &&
           ( extent.xMinimum() <= -180 || extent.xMaximum() >= 180 ||
             extent.yMinimum() <= -90 || extent.yMaximum() >= 90 ) )
        // Use unlimited rectangle because otherwise we may end up transforming wrong coordinates.
        // E.g. longitude -200 to +160 would be understood as +40 to +160 due to periodicity.
        // We could try to clamp coords to (-180,180) for lon resp. (-90,90) for lat,
        // but this seems like a safer choice.
        extent = QgsRectangle( std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max() );
      else
        extent = ct.transformBoundingBox( extent, QgsCoordinateTransform::ReverseTransform );
    }
  }
  catch ( QgsCsException &cse )
  {
    Q_UNUSED( cse );
    QgsDebugMsg( QStringLiteral( "Transform error caught" ) );
    extent = QgsRectangle( std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max() );
    r2     = QgsRectangle( std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max() );
  }

  return split;
}

LayerRenderJobs QgsMapRendererJob::prepareJobs( QPainter *painter, QgsLabelingEngine *labelingEngine2 )
{
  LayerRenderJobs layerJobs;

  // render all layers in the stack, starting at the base
  QListIterator<QgsMapLayer *> li( mSettings.layers() );
  li.toBack();

  if ( mCache )
  {
    bool cacheValid = mCache->init( mSettings.visibleExtent(), mSettings.scale() );
    Q_UNUSED( cacheValid );
    QgsDebugMsgLevel( QStringLiteral( "CACHE VALID: %1" ).arg( cacheValid ), 4 );
  }

  bool requiresLabelRedraw = !( mCache && mCache->hasCacheImage( LABEL_CACHE_ID ) );

  while ( li.hasPrevious() )
  {
    QgsMapLayer *ml = li.previous();

    QgsDebugMsgLevel( QStringLiteral( "layer %1:  minscale:%2  maxscale:%3  scaledepvis:%4  blendmode:%5" )
                      .arg( ml->name() )
                      .arg( ml->minimumScale() )
                      .arg( ml->maximumScale() )
                      .arg( ml->hasScaleBasedVisibility() )
                      .arg( ml->blendMode() )
                      , 3 );

    if ( !ml->isInScaleRange( mSettings.scale() ) ) //|| mOverview )
    {
      QgsDebugMsgLevel( QStringLiteral( "Layer not rendered because it is not within the defined visibility scale range" ), 3 );
      continue;
    }

    QgsRectangle r1 = mSettings.visibleExtent(), r2;
    QgsCoordinateTransform ct;

    ct = mSettings.layerTransform( ml );
    if ( ct.isValid() )
    {
      reprojectToLayerExtent( ml, ct, r1, r2 );
    }
    QgsDebugMsgLevel( "extent: " + r1.toString(), 3 );
    if ( !r1.isFinite() || !r2.isFinite() )
    {
      mErrors.append( Error( ml->id(), tr( "There was a problem transforming the layer's extent. Layer skipped." ) ) );
      continue;
    }

    // Force render of layers that are being edited
    // or if there's a labeling engine that needs the layer to register features
    if ( mCache && ml->type() == QgsMapLayer::VectorLayer )
    {
      QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( ml );
      bool requiresLabeling = false;
      requiresLabeling = ( labelingEngine2 && QgsPalLabeling::staticWillUseLayer( vl ) ) && requiresLabelRedraw;
      if ( vl->isEditable() || requiresLabeling )
      {
        mCache->clearCacheImage( ml->id() );
      }
    }

    layerJobs.append( LayerRenderJob() );
    LayerRenderJob &job = layerJobs.last();
    job.cached = false;
    job.img = nullptr;
    job.layer = ml;
    job.renderingTime = -1;

    job.context = QgsRenderContext::fromMapSettings( mSettings );
    job.context.expressionContext().appendScope( QgsExpressionContextUtils::layerScope( ml ) );
    job.context.setPainter( painter );
    job.context.setLabelingEngine( labelingEngine2 );
    job.context.setCoordinateTransform( ct );
    job.context.setExtent( r1 );

    if ( mFeatureFilterProvider )
      job.context.setFeatureFilterProvider( mFeatureFilterProvider );

    QgsMapLayerStyleOverride styleOverride( ml );
    if ( mSettings.layerStyleOverrides().contains( ml->id() ) )
      styleOverride.setOverrideStyle( mSettings.layerStyleOverrides().value( ml->id() ) );

    job.blendMode = ml->blendMode();
    job.opacity = 1.0;
    if ( QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( ml ) )
    {
      job.opacity = vl->opacity();
    }

    // if we can use the cache, let's do it and avoid rendering!
    if ( mCache && mCache->hasCacheImage( ml->id() ) )
    {
      job.cached = true;
      job.imageInitialized = true;
      job.img = new QImage( mCache->cacheImage( ml->id() ) );
      job.img->setDevicePixelRatio( static_cast<qreal>( mSettings.devicePixelRatio() ) );
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
      QImage *mypFlattenedImage = new QImage( mSettings.deviceOutputSize(),
                                              mSettings.outputImageFormat() );
      mypFlattenedImage->setDevicePixelRatio( static_cast<qreal>( mSettings.devicePixelRatio() ) );
      if ( mypFlattenedImage->isNull() )
      {
        mErrors.append( Error( ml->id(), tr( "Insufficient memory for image %1x%2" ).arg( mSettings.outputSize().width() ).arg( mSettings.outputSize().height() ) ) );
        delete mypFlattenedImage;
        layerJobs.removeLast();
        continue;
      }

      job.img = mypFlattenedImage;
      QPainter *mypPainter = new QPainter( job.img );
      mypPainter->setRenderHint( QPainter::Antialiasing, mSettings.testFlag( QgsMapSettings::Antialiasing ) );
      job.context.setPainter( mypPainter );
    }

    QTime layerTime;
    layerTime.start();
    job.renderer = ml->createMapRenderer( job.context );
    job.renderingTime = layerTime.elapsed(); // include job preparation time in layer rendering time
  } // while (li.hasPrevious())

  return layerJobs;
}

LabelRenderJob QgsMapRendererJob::prepareLabelingJob( QPainter *painter, QgsLabelingEngine *labelingEngine2, bool canUseLabelCache )
{
  LabelRenderJob job;
  job.context = QgsRenderContext::fromMapSettings( mSettings );
  job.context.setPainter( painter );
  job.context.setLabelingEngine( labelingEngine2 );
  job.context.setExtent( mSettings.visibleExtent() );
  job.context.setFeatureFilterProvider( mFeatureFilterProvider );

  // if we can use the cache, let's do it and avoid rendering!
  bool hasCache = canUseLabelCache && mCache && mCache->hasCacheImage( LABEL_CACHE_ID );
  if ( hasCache )
  {
    job.cached = true;
    job.complete = true;
    job.img = new QImage( mCache->cacheImage( LABEL_CACHE_ID ) );
    Q_ASSERT( job.img->devicePixelRatio() == mSettings.devicePixelRatio() );
    job.context.setPainter( nullptr );
  }
  else
  {
    if ( canUseLabelCache && ( mCache || !painter ) )
    {
      // Flattened image for drawing labels
      QImage *mypFlattenedImage = nullptr;
      mypFlattenedImage = new QImage( mSettings.deviceOutputSize(),
                                      mSettings.outputImageFormat() );
      mypFlattenedImage->setDevicePixelRatio( mSettings.devicePixelRatio() );
      if ( mypFlattenedImage->isNull() )
      {
        mErrors.append( Error( QStringLiteral( "labels" ), tr( "Insufficient memory for label image %1x%2" ).arg( mSettings.outputSize().width() ).arg( mSettings.outputSize().height() ) ) );
        delete mypFlattenedImage;
      }
      else
      {
        job.img = mypFlattenedImage;
      }
    }
  }

  return job;
}


void QgsMapRendererJob::cleanupJobs( LayerRenderJobs &jobs )
{
  for ( LayerRenderJobs::iterator it = jobs.begin(); it != jobs.end(); ++it )
  {
    LayerRenderJob &job = *it;
    if ( job.img )
    {
      delete job.context.painter();
      job.context.setPainter( nullptr );

      if ( mCache && !job.cached && !job.context.renderingStopped() && job.layer )
      {
        QgsDebugMsgLevel( "caching image for " + ( job.layer ? job.layer->id() : QString() ), 2 );
        mCache->setCacheImage( job.layer->id(), *job.img, QList< QgsMapLayer * >() << job.layer );
      }

      delete job.img;
      job.img = nullptr;
    }

    if ( job.renderer )
    {
      Q_FOREACH ( const QString &message, job.renderer->errors() )
        mErrors.append( Error( job.renderer->layerId(), message ) );

      delete job.renderer;
      job.renderer = nullptr;
    }

    if ( job.layer )
      mPerLayerRenderingTime.insert( job.layer, job.renderingTime );
  }

  jobs.clear();
}

void QgsMapRendererJob::cleanupLabelJob( LabelRenderJob &job )
{
  if ( job.img )
  {
    if ( mCache && !job.cached && !job.context.renderingStopped() )
    {
      QgsDebugMsg( QStringLiteral( "caching label result image" ) );
      mCache->setCacheImage( LABEL_CACHE_ID, *job.img, _qgis_listQPointerToRaw( job.participatingLayers ) );
    }

    delete job.img;
    job.img = nullptr;
  }
}


QImage QgsMapRendererJob::composeImage( const QgsMapSettings &settings, const LayerRenderJobs &jobs, const LabelRenderJob &labelJob )
{
  QImage image( settings.deviceOutputSize(), settings.outputImageFormat() );
  image.setDevicePixelRatio( settings.devicePixelRatio() );
  image.fill( settings.backgroundColor().rgba() );

  QPainter painter( &image );


  for ( LayerRenderJobs::const_iterator it = jobs.constBegin(); it != jobs.constEnd(); ++it )
  {
    const LayerRenderJob &job = *it;

    if ( job.layer && job.layer->customProperty( QStringLiteral( "rendering/renderAboveLabels" ) ).toBool() )
      continue; // skip layer for now, it will be rendered after labels

    if ( !job.imageInitialized )
      continue; // img not safe to compose

    painter.setCompositionMode( job.blendMode );
    painter.setOpacity( job.opacity );

    Q_ASSERT( job.img );

    painter.drawImage( 0, 0, *job.img );
  }

  // IMPORTANT - don't draw labelJob img before the label job is complete,
  // as the image is uninitialized and full of garbage before the label job
  // commences
  if ( labelJob.img && labelJob.complete )
  {
    painter.setCompositionMode( QPainter::CompositionMode_SourceOver );
    painter.setOpacity( 1.0 );
    painter.drawImage( 0, 0, *labelJob.img );
  }

  // render any layers with the renderAboveLabels flag now
  for ( LayerRenderJobs::const_iterator it = jobs.constBegin(); it != jobs.constEnd(); ++it )
  {
    const LayerRenderJob &job = *it;

    if ( !job.layer || !job.layer->customProperty( QStringLiteral( "rendering/renderAboveLabels" ) ).toBool() )
      continue;

    if ( !job.imageInitialized )
      continue; // img not safe to compose

    painter.setCompositionMode( job.blendMode );
    painter.setOpacity( job.opacity );

    Q_ASSERT( job.img );

    painter.drawImage( 0, 0, *job.img );
  }

  painter.end();
  return image;
}

void QgsMapRendererJob::logRenderingTime( const LayerRenderJobs &jobs, const LabelRenderJob &labelJob )
{
  QgsSettings settings;
  if ( !settings.value( QStringLiteral( "Map/logCanvasRefreshEvent" ), false ).toBool() )
    return;

  QMultiMap<int, QString> elapsed;
  Q_FOREACH ( const LayerRenderJob &job, jobs )
    elapsed.insert( job.renderingTime, job.layer ? job.layer->id() : QString() );

  elapsed.insert( labelJob.renderingTime, tr( "Labeling" ) );

  QList<int> tt( elapsed.uniqueKeys() );
  std::sort( tt.begin(), tt.end(), std::greater<int>() );
  Q_FOREACH ( int t, tt )
  {
    QgsMessageLog::logMessage( tr( "%1 ms: %2" ).arg( t ).arg( QStringList( elapsed.values( t ) ).join( QStringLiteral( ", " ) ) ), tr( "Rendering" ) );
  }
  QgsMessageLog::logMessage( QStringLiteral( "---" ), tr( "Rendering" ) );
}

///@endcond PRIVATE
