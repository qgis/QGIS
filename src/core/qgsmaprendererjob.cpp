
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
#include "qgsmaprenderercache.h"
#include "qgspallabeling.h"
#include "qgsvectorlayerrenderer.h"


QgsMapRendererJob::QgsMapRendererJob( const QgsMapSettings& settings )
    : mSettings( settings )
    , mCache( 0 )
    , mRenderingTime( 0 )
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


QgsMapRendererQImageJob::QgsMapRendererQImageJob( const QgsMapSettings& settings )
    : QgsMapRendererJob( settings )
{
}


QgsMapRendererSequentialJob::QgsMapRendererSequentialJob( const QgsMapSettings& settings )
    : QgsMapRendererQImageJob( settings )
    , mInternalJob( 0 )
    , mPainter( 0 )
    , mLabelingResults( 0 )
{
  QgsDebugMsg( "SEQUENTIAL construct" );

  mImage = QImage( mSettings.outputSize(), QImage::Format_ARGB32_Premultiplied );
}

QgsMapRendererSequentialJob::~QgsMapRendererSequentialJob()
{
  QgsDebugMsg( "SEQUENTIAL destruct" );
  if ( isActive() )
  {
    // still running!
    QgsDebugMsg( "SEQUENTIAL destruct -- still running! (cancelling)" );
    cancel();
  }

  Q_ASSERT( mInternalJob == 0 && mPainter == 0 );

  delete mLabelingResults;
  mLabelingResults = 0;
}


void QgsMapRendererSequentialJob::start()
{
  if ( isActive() )
    return; // do nothing if we are already running

  mRenderingStart.start();

  mErrors.clear();

  QgsDebugMsg( "SEQUENTIAL START" );

  Q_ASSERT( mInternalJob == 0  && mPainter == 0 );

  mPainter = new QPainter( &mImage );

  mInternalJob = new QgsMapRendererCustomPainterJob( mSettings, mPainter );
  mInternalJob->setCache( mCache );

  connect( mInternalJob, SIGNAL( finished() ), SLOT( internalFinished() ) );

  mInternalJob->start();
}


void QgsMapRendererSequentialJob::cancel()
{
  if ( !isActive() )
    return;

  QgsDebugMsg( "sequential - cancel internal" );
  mInternalJob->cancel();

  Q_ASSERT( mInternalJob == 0 && mPainter == 0 );
}

void QgsMapRendererSequentialJob::waitForFinished()
{
  if ( !isActive() )
    return;

  mInternalJob->waitForFinished();
}

bool QgsMapRendererSequentialJob::isActive() const
{
  return mInternalJob != 0;
}

QgsLabelingResults* QgsMapRendererSequentialJob::takeLabelingResults()
{
  QgsLabelingResults* tmp = mLabelingResults;
  mLabelingResults = 0;
  return tmp;
}


QImage QgsMapRendererSequentialJob::renderedImage()
{
  if ( isActive() && mCache )
    // this will allow immediate display of cached layers and at the same time updates of the layer being rendered
    return composeImage( mSettings, mInternalJob->jobs() );
  else
    return mImage;
}


void QgsMapRendererSequentialJob::internalFinished()
{
  QgsDebugMsg( "SEQUENTIAL finished" );

  mPainter->end();
  delete mPainter;
  mPainter = 0;

  mLabelingResults = mInternalJob->takeLabelingResults();

  mErrors = mInternalJob->errors();

  // now we are in a slot called from mInternalJob - do not delete it immediately
  // so the class is still valid when the execution returns to the class
  mInternalJob->deleteLater();
  mInternalJob = 0;

  mRenderingTime = mRenderingStart.elapsed();

  emit finished();
}


////////



QgsMapRendererCustomPainterJob::QgsMapRendererCustomPainterJob( const QgsMapSettings& settings, QPainter* painter )
    : QgsMapRendererJob( settings )
    , mPainter( painter )
    , mLabelingEngine( 0 )
    , mActive( false )
{
  QgsDebugMsg( "QPAINTER construct" );
}

QgsMapRendererCustomPainterJob::~QgsMapRendererCustomPainterJob()
{
  QgsDebugMsg( "QPAINTER destruct" );
  Q_ASSERT( !mFutureWatcher.isRunning() );
  //cancel();

  delete mLabelingEngine;
  mLabelingEngine = 0;
}

void QgsMapRendererCustomPainterJob::start()
{
  if ( isActive() )
    return;

  mRenderingStart.start();

  mActive = true;

  mErrors.clear();

  QgsDebugMsg( "QPAINTER run!" );

  QgsDebugMsg( "Preparing list of layer jobs for rendering" );
  QTime prepareTime;
  prepareTime.start();

  // clear the background
  mPainter->fillRect( 0, 0, mSettings.outputSize().width(), mSettings.outputSize().height(), mSettings.backgroundColor() );

  mPainter->setRenderHint( QPainter::Antialiasing, mSettings.testFlag( QgsMapSettings::Antialiasing ) );

  QPaintDevice* thePaintDevice = mPainter->device();

  QString errMsg = QString( "pre-set DPI not equal to painter's DPI (%1 vs %2)" ).arg( thePaintDevice->logicalDpiX() ).arg( mSettings.outputDpi() );
  Q_ASSERT_X( thePaintDevice->logicalDpiX() == mSettings.outputDpi(), "Job::startRender()", errMsg.toAscii().data() );

  delete mLabelingEngine;
  mLabelingEngine = 0;

  if ( mSettings.testFlag( QgsMapSettings::DrawLabeling ) )
  {
    mLabelingEngine = new QgsPalLabeling;
    mLabelingEngine->loadEngineSettings();
    mLabelingEngine->init( mSettings );
  }

  mLayerJobs = prepareJobs( mPainter, mLabelingEngine );

  QgsDebugMsg( "Rendering prepared in (seconds): " + QString( "%1" ).arg( prepareTime.elapsed() / 1000.0 ) );

  // now we are ready to start rendering!
  if ( !mLayerJobs.isEmpty() )
  {
    connect( &mFutureWatcher, SIGNAL( finished() ), SLOT( futureFinished() ) );

    mFuture = QtConcurrent::run( staticRender, this );
    mFutureWatcher.setFuture( mFuture );
  }
  else
  {
    // just make sure we will clean up and emit finished() signal
    QTimer::singleShot( 0, this, SLOT( futureFinished() ) );
  }
}


void QgsMapRendererCustomPainterJob::cancel()
{
  if ( !isActive() )
  {
    QgsDebugMsg( "QPAINTER not running!" );
    return;
  }

  QgsDebugMsg( "QPAINTER cancelling" );
  disconnect( &mFutureWatcher, SIGNAL( finished() ), this, SLOT( futureFinished() ) );

  mLabelingRenderContext.setRenderingStopped( true );
  for ( LayerRenderJobs::iterator it = mLayerJobs.begin(); it != mLayerJobs.end(); ++it )
  {
    it->context.setRenderingStopped( true );
  }

  QTime t;
  t.start();

  mFutureWatcher.waitForFinished();

  QgsDebugMsg( QString( "QPAINER cancel waited %1 ms" ).arg( t.elapsed() / 1000.0 ) );

  futureFinished();

  QgsDebugMsg( "QPAINTER cancelled" );
}

void QgsMapRendererCustomPainterJob::waitForFinished()
{
  if ( !isActive() )
    return;

  disconnect( &mFutureWatcher, SIGNAL( finished() ), this, SLOT( futureFinished() ) );

  QTime t;
  t.start();

  mFutureWatcher.waitForFinished();

  QgsDebugMsg( QString( "waitForFinished: %1 ms" ).arg( t.elapsed() / 1000.0 ) );

  futureFinished();
}

bool QgsMapRendererCustomPainterJob::isActive() const
{
  return mActive;
}


QgsLabelingResults* QgsMapRendererCustomPainterJob::takeLabelingResults()
{
  return mLabelingEngine ? mLabelingEngine->takeResults() : 0;
}


void QgsMapRendererCustomPainterJob::futureFinished()
{
  mActive = false;
  mRenderingTime = mRenderingStart.elapsed();
  QgsDebugMsg( "QPAINTER futureFinished" );

  // final cleanup
  cleanupJobs( mLayerJobs );

  emit finished();
}


void QgsMapRendererCustomPainterJob::staticRender( QgsMapRendererCustomPainterJob* self )
{
  self->doRender();
}

void QgsMapRendererCustomPainterJob::doRender()
{
  QgsDebugMsg( "Starting to render layer stack." );
  QTime renderTime;
  renderTime.start();

  for ( LayerRenderJobs::iterator it = mLayerJobs.begin(); it != mLayerJobs.end(); ++it )
  {
    LayerRenderJob& job = *it;

    if ( job.context.renderingStopped() )
      break;

    if ( job.context.useAdvancedEffects() )
    {
      // Set the QPainter composition mode so that this layer is rendered using
      // the desired blending mode
      mPainter->setCompositionMode( job.blendMode );
    }

    if ( !job.cached )
      job.renderer->render();

    if ( job.img )
    {
      // If we flattened this layer for alternate blend modes, composite it now
      mPainter->drawImage( 0, 0, *job.img );
    }

  }

  QgsDebugMsg( "Done rendering map layers" );

  if ( mSettings.testFlag( QgsMapSettings::DrawLabeling ) && !mLabelingRenderContext.renderingStopped() )
    drawLabeling( mSettings, mLabelingRenderContext, mLabelingEngine, mPainter );

  QgsDebugMsg( "Rendering completed in (seconds): " + QString( "%1" ).arg( renderTime.elapsed() / 1000.0 ) );
}


void QgsMapRendererJob::drawLabeling( const QgsMapSettings& settings, QgsRenderContext& renderContext, QgsPalLabeling* labelingEngine, QPainter* painter )
{
  QgsDebugMsg( "Draw labeling start" );

  QTime t;
  t.start();

  // Reset the composition mode before rendering the labels
  painter->setCompositionMode( QPainter::CompositionMode_SourceOver );

  // TODO: this is not ideal - we could override rendering stopped flag that has been set in meanwhile
  renderContext = QgsRenderContext::fromMapSettings( settings );
  renderContext.setPainter( painter );
  renderContext.setLabelingEngine( labelingEngine );

  // old labeling - to be removed at some point...
  drawOldLabeling( settings, renderContext );

  drawNewLabeling( settings, renderContext, labelingEngine );

  QgsDebugMsg( QString( "Draw labeling took (seconds): %1" ).arg( t.elapsed() / 1000. ) );
}


void QgsMapRendererJob::drawOldLabeling( const QgsMapSettings& settings, QgsRenderContext& renderContext )
{
  // render all labels for vector layers in the stack, starting at the base
  QListIterator<QString> li( settings.layers() );
  li.toBack();
  while ( li.hasPrevious() )
  {
    if ( renderContext.renderingStopped() )
    {
      break;
    }

    QString layerId = li.previous();

    QgsMapLayer *ml = QgsMapLayerRegistry::instance()->mapLayer( layerId );

    if ( !ml || ( ml->type() != QgsMapLayer::VectorLayer ) )
      continue;

    // only make labels if the layer is visible
    // after scale dep viewing settings are checked
    if ( ml->hasScaleBasedVisibility() && ( settings.scale() < ml->minimumScale() || settings.scale() > ml->maximumScale() ) )
      continue;

    const QgsCoordinateTransform* ct = 0;
    QgsRectangle r1 = settings.visibleExtent(), r2;

    if ( settings.hasCrsTransformEnabled() )
    {
      ct = QgsCoordinateTransformCache::instance()->transform( ml->crs().authid(), settings.destinationCrs().authid() );
      reprojectToLayerExtent( ct, ml->crs().geographicFlag(), r1, r2 );
    }

    renderContext.setCoordinateTransform( ct );
    renderContext.setExtent( r1 );

    ml->drawLabels( renderContext );
  }
}


void QgsMapRendererJob::drawNewLabeling( const QgsMapSettings& settings, QgsRenderContext& renderContext, QgsPalLabeling* labelingEngine )
{
  if ( labelingEngine && !renderContext.renderingStopped() )
  {
    // set correct extent
    renderContext.setExtent( settings.visibleExtent() );
    renderContext.setCoordinateTransform( NULL );

    labelingEngine->drawLabeling( renderContext );
    labelingEngine->exit();
  }
}

void QgsMapRendererJob::updateLayerGeometryCaches()
{
  foreach ( QString id, mGeometryCaches.keys() )
  {
    const QgsGeometryCache& cache = mGeometryCaches[id];
    if ( QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( id ) ) )
      * vl->cache() = cache;
  }
  mGeometryCaches.clear();
}


bool QgsMapRendererJob::needTemporaryImage( QgsMapLayer* ml )
{
  if ( mSettings.testFlag( QgsMapSettings::UseAdvancedEffects ) && ml->type() == QgsMapLayer::VectorLayer )
  {
    QgsVectorLayer* vl = qobject_cast<QgsVectorLayer *>( ml );
    if ((( vl->blendMode() != QPainter::CompositionMode_SourceOver )
         || ( vl->featureBlendMode() != QPainter::CompositionMode_SourceOver )
         || ( vl->layerTransparency() != 0 ) ) )
      return true;
  }

  return false;
}


////////////////


bool QgsMapRendererJob::reprojectToLayerExtent( const QgsCoordinateTransform* ct, bool layerCrsGeographic, QgsRectangle& extent, QgsRectangle& r2 )
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

    if ( layerCrsGeographic )
    {
      // Note: ll = lower left point
      //   and ur = upper right point
      QgsPoint ll = ct->transform( extent.xMinimum(), extent.yMinimum(),
                                   QgsCoordinateTransform::ReverseTransform );

      QgsPoint ur = ct->transform( extent.xMaximum(), extent.yMaximum(),
                                   QgsCoordinateTransform::ReverseTransform );

      extent = ct->transformBoundingBox( extent, QgsCoordinateTransform::ReverseTransform );

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
    else // can't cross 180
    {
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



LayerRenderJobs QgsMapRendererJob::prepareJobs( QPainter* painter, QgsPalLabeling* labelingEngine )
{
  LayerRenderJobs layerJobs;

  // render all layers in the stack, starting at the base
  QListIterator<QString> li( mSettings.layers() );
  li.toBack();

  if ( mCache )
  {
    bool cacheValid = mCache->init( mSettings.visibleExtent(), mSettings.scale() );
    QgsDebugMsg( QString( "CACHE VALID: %1" ).arg( cacheValid ) );
  }

  mGeometryCaches.clear();

  while ( li.hasPrevious() )
  {
    QString layerId = li.previous();

    QgsDebugMsg( "Rendering at layer item " + layerId );

    QgsMapLayer *ml = QgsMapLayerRegistry::instance()->mapLayer( layerId );

    if ( !ml )
    {
      mErrors.append( Error( layerId, "Layer not found in registry." ) );
      continue;
    }

    QgsDebugMsg( QString( "layer %1:  minscale:%2  maxscale:%3  scaledepvis:%4  extent:%5  blendmode:%6" )
                 .arg( ml->name() )
                 .arg( ml->minimumScale() )
                 .arg( ml->maximumScale() )
                 .arg( ml->hasScaleBasedVisibility() )
                 .arg( ml->extent().toString() )
                 .arg( ml->blendMode() )
               );

    if ( ml->hasScaleBasedVisibility() && ( mSettings.scale() < ml->minimumScale() || mSettings.scale() > ml->maximumScale() ) ) //|| mOverview )
    {
      QgsDebugMsg( "Layer not rendered because it is not within the defined visibility scale range" );
      continue;
    }

    QgsRectangle r1 = mSettings.visibleExtent(), r2;
    const QgsCoordinateTransform* ct = 0;

    if ( mSettings.hasCrsTransformEnabled() )
    {
      ct = QgsCoordinateTransformCache::instance()->transform( ml->crs().authid(), mSettings.destinationCrs().authid() );
      reprojectToLayerExtent( ct, ml->crs().geographicFlag(), r1, r2 );
      QgsDebugMsg( "extent: " + r1.toString() );
      if ( !r1.isFinite() || !r2.isFinite() )
      {
        mErrors.append( Error( layerId, "There was a problem transforming layer's' extent. Layer skipped." ) );
        continue;
      }
    }

    // Force render of layers that are being edited
    // or if there's a labeling engine that needs the layer to register features
    if ( mCache && ml->type() == QgsMapLayer::VectorLayer )
    {
      QgsVectorLayer* vl = qobject_cast<QgsVectorLayer *>( ml );
      if ( vl->isEditable() || ( labelingEngine && labelingEngine->willUseLayer( vl ) ) )
        mCache->clearCacheImage( ml->id() );
    }

    layerJobs.append( LayerRenderJob() );
    LayerRenderJob& job = layerJobs.last();
    job.cached = false;
    job.img = 0;
    job.blendMode = ml->blendMode();
    job.layerId = ml->id();

    job.context = QgsRenderContext::fromMapSettings( mSettings );
    job.context.setPainter( painter );
    job.context.setLabelingEngine( labelingEngine );
    job.context.setCoordinateTransform( ct );
    job.context.setExtent( r1 );

    // if we can use the cache, let's do it and avoid rendering!
    if ( mCache && !mCache->cacheImage( ml->id() ).isNull() )
    {
      job.cached = true;
      job.img = new QImage( mCache->cacheImage( ml->id() ) );
      job.renderer = 0;
      job.context.setPainter( 0 );
      continue;
    }

    // If we are drawing with an alternative blending mode then we need to render to a separate image
    // before compositing this on the map. This effectively flattens the layer and prevents
    // blending occuring between objects on the layer
    if ( mCache || !painter || needTemporaryImage( ml ) )
    {
      // Flattened image for drawing when a blending mode is set
      QImage * mypFlattenedImage = 0;
      mypFlattenedImage = new QImage( mSettings.outputSize().width(),
                                      mSettings.outputSize().height(), QImage::Format_ARGB32_Premultiplied );
      if ( mypFlattenedImage->isNull() )
      {
        mErrors.append( Error( layerId, "Insufficient memory for image " + QString::number( mSettings.outputSize().width() ) + "x" + QString::number( mSettings.outputSize().height() ) ) );
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

    job.renderer = ml->createMapRenderer( job.context );

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
      job.context.setPainter( 0 );

      if ( mCache && !job.cached && !job.context.renderingStopped() )
      {
        QgsDebugMsg( "caching image for " + job.layerId );
        mCache->setCacheImage( job.layerId, *job.img );
      }

      delete job.img;
      job.img = 0;
    }

    if ( job.renderer )
    {
      foreach ( QString message, job.renderer->errors() )
        mErrors.append( Error( job.renderer->layerID(), message ) );

      delete job.renderer;
      job.renderer = 0;
    }
  }

  jobs.clear();

  updateLayerGeometryCaches();
}

/////////////


QgsMapRendererParallelJob::QgsMapRendererParallelJob( const QgsMapSettings& settings )
    : QgsMapRendererQImageJob( settings )
    , mStatus( Idle )
    , mLabelingEngine( 0 )
{
}

QgsMapRendererParallelJob::~QgsMapRendererParallelJob()
{
  if ( isActive() )
  {
    cancel();
  }

  delete mLabelingEngine;
  mLabelingEngine = 0;
}

void QgsMapRendererParallelJob::start()
{
  if ( isActive() )
    return;

  mRenderingStart.start();

  mStatus = RenderingLayers;

  delete mLabelingEngine;
  mLabelingEngine = 0;

  if ( mSettings.testFlag( QgsMapSettings::DrawLabeling ) )
  {
    mLabelingEngine = new QgsPalLabeling;
    mLabelingEngine->loadEngineSettings();
    mLabelingEngine->init( mSettings );
  }


  mLayerJobs = prepareJobs( 0, mLabelingEngine );

  qDebug( "QThreadPool max thread count is %d", QThreadPool::globalInstance()->maxThreadCount() );

  // start async job

  connect( &mFutureWatcher, SIGNAL( finished() ), SLOT( renderLayersFinished() ) );

  mFuture = QtConcurrent::map( mLayerJobs, renderLayerStatic );
  mFutureWatcher.setFuture( mFuture );
}

void QgsMapRendererParallelJob::cancel()
{
  if ( !isActive() )
    return;

  QgsDebugMsg( QString( "PARALLEL cancel at status %1" ).arg( mStatus ) );

  mLabelingRenderContext.setRenderingStopped( true );
  for ( LayerRenderJobs::iterator it = mLayerJobs.begin(); it != mLayerJobs.end(); ++it )
  {
    it->context.setRenderingStopped( true );
  }

  if ( mStatus == RenderingLayers )
  {
    disconnect( &mFutureWatcher, SIGNAL( finished() ), this, SLOT( renderLayersFinished() ) );

    mFutureWatcher.waitForFinished();

    renderLayersFinished();
  }

  if ( mStatus == RenderingLabels )
  {
    disconnect( &mLabelingFutureWatcher, SIGNAL( finished() ), this, SLOT( renderingFinished() ) );

    mLabelingFutureWatcher.waitForFinished();

    renderingFinished();
  }

  Q_ASSERT( mStatus == Idle );
}

void QgsMapRendererParallelJob::waitForFinished()
{
  if ( !isActive() )
    return;

  if ( mStatus == RenderingLayers )
  {
    disconnect( &mFutureWatcher, SIGNAL( finished() ), this, SLOT( renderLayersFinished() ) );

    QTime t;
    t.start();

    mFutureWatcher.waitForFinished();

    QgsDebugMsg( QString( "waitForFinished (1): %1 ms" ).arg( t.elapsed() / 1000.0 ) );

    renderLayersFinished();
  }

  if ( mStatus == RenderingLabels )
  {
    disconnect( &mLabelingFutureWatcher, SIGNAL( finished() ), this, SLOT( renderingFinished() ) );

    QTime t;
    t.start();

    mLabelingFutureWatcher.waitForFinished();

    QgsDebugMsg( QString( "waitForFinished (2): %1 ms" ).arg( t.elapsed() / 1000.0 ) );

    renderingFinished();
  }

  Q_ASSERT( mStatus == Idle );
}

bool QgsMapRendererParallelJob::isActive() const
{
  return mStatus != Idle;
}

QgsLabelingResults* QgsMapRendererParallelJob::takeLabelingResults()
{
  return mLabelingEngine ? mLabelingEngine->takeResults() : 0;
}

QImage QgsMapRendererParallelJob::renderedImage()
{
  if ( mStatus == RenderingLayers )
    return composeImage( mSettings, mLayerJobs );
  else
    return mFinalImage; // when rendering labels or idle
}

void QgsMapRendererParallelJob::renderLayersFinished()
{
  Q_ASSERT( mStatus == RenderingLayers );

  // compose final image
  mFinalImage = composeImage( mSettings, mLayerJobs );

  cleanupJobs( mLayerJobs );

  QgsDebugMsg( "PARALLEL layers finished" );

  if ( mSettings.testFlag( QgsMapSettings::DrawLabeling ) && !mLabelingRenderContext.renderingStopped() )
  {
    mStatus = RenderingLabels;

    connect( &mLabelingFutureWatcher, SIGNAL( finished() ), this, SLOT( renderingFinished() ) );

    // now start rendering of labeling!
    mLabelingFuture = QtConcurrent::run( renderLabelsStatic, this );
    mLabelingFutureWatcher.setFuture( mLabelingFuture );
  }
  else
  {
    renderingFinished();
  }
}

void QgsMapRendererParallelJob::renderingFinished()
{
  QgsDebugMsg( "PARALLEL finished" );

  mStatus = Idle;

  mRenderingTime = mRenderingStart.elapsed();

  emit finished();
}

void QgsMapRendererParallelJob::renderLayerStatic( LayerRenderJob& job )
{
  if ( job.context.renderingStopped() )
    return;

  if ( job.cached )
    return;

  QTime t;
  t.start();
  QgsDebugMsg( QString( "job %1 start" ).arg(( ulong ) &job, 0, 16 ) );
  job.renderer->render();
  int tt = t.elapsed();
  QgsDebugMsg( QString( "job %1 end [%2 ms]" ).arg(( ulong ) &job, 0, 16 ).arg( tt ) );
}


void QgsMapRendererParallelJob::renderLabelsStatic( QgsMapRendererParallelJob* self )
{
  QPainter painter( &self->mFinalImage );

  drawLabeling( self->mSettings, self->mLabelingRenderContext, self->mLabelingEngine, &painter );

  painter.end();
}


QImage QgsMapRendererJob::composeImage( const QgsMapSettings& settings, const LayerRenderJobs& jobs )
{
  QImage image( settings.outputSize(), QImage::Format_ARGB32_Premultiplied );
  image.fill( settings.backgroundColor().rgb() );

  QPainter painter( &image );

  for ( LayerRenderJobs::const_iterator it = jobs.constBegin(); it != jobs.constEnd(); ++it )
  {
    const LayerRenderJob& job = *it;

    painter.setCompositionMode( job.blendMode );

    Q_ASSERT( job.img != 0 );
    painter.drawImage( 0, 0, *job.img );
  }

  painter.end();
  return image;
}
