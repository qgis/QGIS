
#include "qgsmaprendererjob.h"

#include <QPainter>
#include <QTimer>
#include <QtConcurrentMap>

#include "qgscrscache.h"
#include "qgslogger.h"
#include "qgsrendercontext.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaplayerrenderer.h"
#include "qgspallabeling.h"


QgsMapRendererJob::QgsMapRendererJob( const QgsMapSettings& settings )
  : mSettings(settings)
{
}


QgsMapRendererQImageJob::QgsMapRendererQImageJob( const QgsMapSettings& settings )
  : QgsMapRendererJob( settings )
{
}


QgsMapRendererSequentialJob::QgsMapRendererSequentialJob(const QgsMapSettings& settings)
  : QgsMapRendererQImageJob( settings )
  , mInternalJob(0)
  , mPainter( 0 )
  , mLabelingResults( 0 )
{
  qDebug("SEQUENTIAL construct");

  mImage = QImage(mSettings.outputSize(), QImage::Format_ARGB32_Premultiplied);
}

QgsMapRendererSequentialJob::~QgsMapRendererSequentialJob()
{
  qDebug("SEQUENTIAL destruct");
  if ( isActive() )
  {
    // still running!
    qDebug("SEQUENTIAL destruct -- still running! (cancelling)");
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

  qDebug("SEQUENTIAL START");
  qDebug("%d,%d", mSettings.outputSize().width(), mSettings.outputSize().height());

  Q_ASSERT( mInternalJob == 0  && mPainter == 0 );

  mPainter = new QPainter(&mImage);

  mInternalJob = new QgsMapRendererCustomPainterJob(mSettings, mPainter);

  connect(mInternalJob, SIGNAL(finished()), SLOT(internalFinished()));

  mInternalJob->start();
}


void QgsMapRendererSequentialJob::cancel()
{
  if ( !isActive() )
    return;

  qDebug("sequential - cancel internal");
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
  return mImage;
}


void QgsMapRendererSequentialJob::internalFinished()
{
  qDebug("SEQUENTIAL finished");

  mPainter->end();
  delete mPainter;
  mPainter = 0;

  mLabelingResults = mInternalJob->takeLabelingResults();

  delete mInternalJob;
  mInternalJob = 0;

  emit finished();
}


////////



QgsMapRendererCustomPainterJob::QgsMapRendererCustomPainterJob(const QgsMapSettings& settings, QPainter* painter)
  : QgsMapRendererJob( settings )
  , mPainter(painter)
  , mLabelingEngine( 0 )
{
  qDebug("QPAINTER construct");
}

QgsMapRendererCustomPainterJob::~QgsMapRendererCustomPainterJob()
{
  qDebug("QPAINTER destruct");
  Q_ASSERT(!mFutureWatcher.isRunning());
  //cancel();

  delete mLabelingEngine;
  mLabelingEngine = 0;
}

void QgsMapRendererCustomPainterJob::start()
{
  qDebug("QPAINTER run!");

  QgsDebugMsg( "Preparing list of layer jobs for rendering" );
  QTime prepareTime;
  prepareTime.start();

  // clear the background
  mPainter->fillRect( 0, 0, mSettings.outputSize().width(), mSettings.outputSize().height(), mSettings.backgroundColor() );

  mPainter->setRenderHint( QPainter::Antialiasing, mSettings.testFlag( QgsMapSettings::Antialiasing ) );

  QPaintDevice* thePaintDevice = mPainter->device();

  Q_ASSERT_X( thePaintDevice->logicalDpiX() == mSettings.outputDpi(), "Job::startRender()", "pre-set DPI not equal to painter's DPI" );

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
    connect(&mFutureWatcher, SIGNAL(finished()), SLOT(futureFinished()));

    mFuture = QtConcurrent::run(staticRender, this);
    mFutureWatcher.setFuture(mFuture);
  }
  else
  {
    // just make sure we will clean up and emit finished() signal
    QTimer::singleShot( 0, this, SLOT(futureFinished()) );
  }
}


void QgsMapRendererCustomPainterJob::cancel()
{
  // TODO: custom flag indicating that some rendering has started?
  //if (mFuture.isRunning())
  {
    qDebug("QPAINTER cancelling");
    disconnect(&mFutureWatcher, SIGNAL(finished()), this, SLOT(futureFinished()));

    mRenderContext.setRenderingStopped(true);
    for ( LayerRenderJobs::iterator it = mLayerJobs.begin(); it != mLayerJobs.end(); ++it )
    {
      it->context.setRenderingStopped( true );
    }

    QTime t;
    t.start();

    mFutureWatcher.waitForFinished();

    qDebug("QPAINER cancel waited %f ms", t.elapsed() / 1000.0);

    futureFinished();

    qDebug("QPAINTER cancelled");
  }
  //else
  //  qDebug("QPAINTER not running!");
}

void QgsMapRendererCustomPainterJob::waitForFinished()
{
  if (!mFuture.isRunning())
    return;

  disconnect(&mFutureWatcher, SIGNAL(finished()), this, SLOT(futureFinished()));

  QTime t;
  t.start();

  mFutureWatcher.waitForFinished();

  qDebug("waitForFinished: %f ms", t.elapsed() / 1000.0);

  futureFinished();
}

bool QgsMapRendererCustomPainterJob::isActive() const
{
  return mFutureWatcher.isRunning();
}


QgsLabelingResults* QgsMapRendererCustomPainterJob::takeLabelingResults()
{
  return mLabelingEngine ? mLabelingEngine->takeResults() : 0;
}


void QgsMapRendererCustomPainterJob::futureFinished()
{
  qDebug("QPAINTER futureFinished");
  emit finished();
}


void QgsMapRendererCustomPainterJob::staticRender(QgsMapRendererCustomPainterJob* self)
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

    job.renderer->render();

    if ( job.img )
    {
      // If we flattened this layer for alternate blend modes, composite it now
      mPainter->drawImage( 0, 0, *job.img );
    }

  }

  // final cleanup
  cleanupJobs( mLayerJobs );

  QgsDebugMsg( "Done rendering map layers" );

  // Reset the composition mode before rendering the labels
  mPainter->setCompositionMode( QPainter::CompositionMode_SourceOver );

  mRenderContext = QgsRenderContext::fromMapSettings( mSettings );
  mRenderContext.setPainter( mPainter );
  mRenderContext.setLabelingEngine( mLabelingEngine );

  // old labeling - to be removed at some point...
  drawOldLabeling();

  drawNewLabeling();

  QgsDebugMsg( "Rendering completed in (seconds): " + QString( "%1" ).arg( renderTime.elapsed() / 1000.0 ) );
}


void QgsMapRendererCustomPainterJob::drawOldLabeling()
{
  if ( mSettings.testFlag( QgsMapSettings::DrawLabeling ) )
  {
    // render all labels for vector layers in the stack, starting at the base
    QListIterator<QString> li( mSettings.layers() );
    li.toBack();
    while ( li.hasPrevious() )
    {
      if ( mRenderContext.renderingStopped() )
      {
        break;
      }

      QString layerId = li.previous();

      QgsMapLayer *ml = QgsMapLayerRegistry::instance()->mapLayer( layerId );

      if ( !ml || ( ml->type() != QgsMapLayer::VectorLayer ) )
        continue;

      // only make labels if the layer is visible
      // after scale dep viewing settings are checked
      if ( ml->hasScaleBasedVisibility() && ( mSettings.scale() < ml->minimumScale() || mSettings.scale() > ml->maximumScale() ) )
        continue;

      bool split = false;
      const QgsCoordinateTransform* ct = 0;
      QgsRectangle r1 = mSettings.visibleExtent(), r2;

      if ( mSettings.hasCrsTransformEnabled() )
      {
        ct = QgsCoordinateTransformCache::instance()->transform( ml->crs().authid(), mSettings.destinationCrs().authid() );
        split = reprojectToLayerExtent( ct, ml->crs().geographicFlag(), r1, r2 );
      }

      mRenderContext.setCoordinateTransform( ct );
      mRenderContext.setExtent( r1 );

      ml->drawLabels( mRenderContext );
      if ( split )
      {
        mRenderContext.setExtent( r2 );
        ml->drawLabels( mRenderContext );
      }
    }
  }
}


void QgsMapRendererCustomPainterJob::drawNewLabeling()
{
  if ( mLabelingEngine && !mRenderContext.renderingStopped() )
  {
    // set correct extent
    mRenderContext.setExtent( mSettings.visibleExtent() );
    mRenderContext.setCoordinateTransform( NULL );

    mLabelingEngine->drawLabeling( mRenderContext );
    mLabelingEngine->exit();
  }
}


bool QgsMapRendererJob::needTemporaryImage( QgsMapLayer* ml )
{
  if ( mSettings.testFlag( QgsMapSettings::UseAdvancedEffects ) && ml->type() == QgsMapLayer::VectorLayer )
  {
    QgsVectorLayer* vl = qobject_cast<QgsVectorLayer *>( ml );
    if ( (( vl->blendMode() != QPainter::CompositionMode_SourceOver )
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
        r2 = extent;
        extent.setXMinimum( splitCoord );
        r2.setXMaximum( splitCoord );
        split = true;
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

  while ( li.hasPrevious() )
  {
    QString layerId = li.previous();

    QgsDebugMsg( "Rendering at layer item " + layerId );

    QgsMapLayer *ml = QgsMapLayerRegistry::instance()->mapLayer( layerId );

    if ( !ml )
    {
      QgsDebugMsg( "Layer not found in registry!" );
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

    bool split = false;
    QgsRectangle r1 = mSettings.visibleExtent(), r2;
    const QgsCoordinateTransform* ct = 0;

    if ( mSettings.hasCrsTransformEnabled() )
    {
      ct = QgsCoordinateTransformCache::instance()->transform( ml->crs().authid(), mSettings.destinationCrs().authid() );
      split = reprojectToLayerExtent( ct, ml->crs().geographicFlag(), r1, r2 );
      QgsDebugMsg( "  extent 1: " + r1.toString() );
      QgsDebugMsg( "  extent 2: " + r2.toString() );
      if ( !r1.isFinite() || !r2.isFinite() ) //there was a problem transforming the extent. Skip the layer
        continue;
    }

    // Flattened image for drawing when a blending mode is set
    QImage * mypFlattenedImage = 0;

    // If we are drawing with an alternative blending mode then we need to render to a separate image
    // before compositing this on the map. This effectively flattens the layer and prevents
    // blending occuring between objects on the layer
    if ( !painter || needTemporaryImage( ml ) )
    {
      mypFlattenedImage = new QImage( mSettings.outputSize().width(),
                                      mSettings.outputSize().height(), QImage::Format_ARGB32 );
      if ( mypFlattenedImage->isNull() )
      {
        QgsDebugMsg( "insufficient memory for image " + QString::number( mSettings.outputSize().width() ) + "x" + QString::number( mSettings.outputSize().height() ) );
        // TODO: add to the list of errors!
        delete mypFlattenedImage;
        continue;
      }
      mypFlattenedImage->fill( 0 );
    }

    layerJobs.append( LayerRenderJob() );
    LayerRenderJob& job = layerJobs.last();
    job.img = 0;
    job.blendMode = ml->blendMode();

    job.context = QgsRenderContext::fromMapSettings( mSettings );
    job.context.setPainter( painter );
    job.context.setLabelingEngine( labelingEngine );
    job.context.setCoordinateTransform( ct );
    job.context.setExtent( r1 );

    if ( mypFlattenedImage )
    {
      job.img = mypFlattenedImage;
      QPainter* mypPainter = new QPainter( job.img );
      mypPainter->setRenderHint( QPainter::Antialiasing, mSettings.testFlag( QgsMapSettings::Antialiasing ) );
      job.context.setPainter( mypPainter );
    }

    job.renderer = ml->createMapRenderer( job.context );

    /*
    // TODO: split extent
    if ( !ml->draw( mRenderContext ) )
    {
      // TODO emit drawError( ml );
    }
    else
    {
      QgsDebugMsg( "Layer rendered without issues" );
    }

    if ( split )
    {
      mRenderContext.setExtent( r2 );
      if ( !ml->draw( mRenderContext ) )
      {
        // TODO emit drawError( ml );
      }
    }*/

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
      delete job.img;
      job.img = 0;
    }

    delete job.renderer;
    job.renderer = 0;
  }

  jobs.clear();
}

/////////////


QgsMapRendererParallelJob::QgsMapRendererParallelJob(const QgsMapSettings& settings)
  : QgsMapRendererQImageJob( settings )
{
}

QgsMapRendererParallelJob::~QgsMapRendererParallelJob()
{
  if ( isActive() )
  {
    cancel();
  }
}

void QgsMapRendererParallelJob::start()
{
  if ( isActive() )
    return;

  // TODO: create labeling engine

  mLayerJobs = prepareJobs( 0, 0 );

  // start async job

  connect(&mFutureWatcher, SIGNAL(finished()), SLOT(renderLayersFinished()));

  mFuture = QtConcurrent::map( mLayerJobs, renderLayerStatic );
  mFutureWatcher.setFuture(mFuture);
}

void QgsMapRendererParallelJob::cancel()
{
  if ( !isActive() )
    return;

  disconnect(&mFutureWatcher, SIGNAL(finished()), this, SLOT(renderLayersFinished()));

  for ( LayerRenderJobs::iterator it = mLayerJobs.begin(); it != mLayerJobs.end(); ++it )
  {
    it->context.setRenderingStopped( true );
  }

  mFutureWatcher.waitForFinished();

  renderLayersFinished();
}

void QgsMapRendererParallelJob::waitForFinished()
{
  if ( !isActive() )
    return;

  disconnect(&mFutureWatcher, SIGNAL(finished()), this, SLOT(renderLayersFinished()));

  QTime t;
  t.start();

  mFutureWatcher.waitForFinished();

  qDebug("waitForFinished: %f ms", t.elapsed() / 1000.0);

  renderLayersFinished();
}

bool QgsMapRendererParallelJob::isActive() const
{
  return mFutureWatcher.isRunning();
}

QgsLabelingResults* QgsMapRendererParallelJob::takeLabelingResults()
{
  return 0;
}

QImage QgsMapRendererParallelJob::renderedImage()
{
  if ( isActive() )
    return composeImage();
  else
    return mFinalImage;
}

void QgsMapRendererParallelJob::renderLayersFinished()
{
  // TODO: now start rendering of labeling!

  // compose final image
  mFinalImage = composeImage();

  cleanupJobs( mLayerJobs );

  emit finished();
}

void QgsMapRendererParallelJob::renderLayerStatic(LayerRenderJob& job)
{
  if ( job.context.renderingStopped() )
    return;

  QTime t;
  t.start();
  QgsDebugMsg( QString("job %1 start").arg( (ulong) &job, 0, 16 ) );
  job.renderer->render();
  int tt = t.elapsed();
  QgsDebugMsg( QString("job %1 end [%2 ms]").arg( (ulong) &job, 0, 16 ).arg( tt ) );
}

QImage QgsMapRendererParallelJob::composeImage()
{
  QImage image( mSettings.outputSize(), QImage::Format_ARGB32_Premultiplied );
  image.fill( mSettings.backgroundColor() );

  QPainter painter(&image);

  for (LayerRenderJobs::const_iterator it = mLayerJobs.begin(); it != mLayerJobs.end(); ++it)
  {
    const LayerRenderJob& job = *it;

    painter.setCompositionMode( job.blendMode );

    Q_ASSERT( job.img != 0 );
    painter.drawImage( 0, 0, *job.img );
  }

  painter.end();
  return image;
}
