
#include "qgsmaprendererjob.h"

#include <QPainter>

#include "qgscrscache.h"
#include "qgslogger.h"
#include "qgsrendercontext.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"
#include "qgspallabeling.h"


QgsMapRendererQImageJob::QgsMapRendererQImageJob(QgsMapRendererJob::Type type, const QgsMapSettings& settings)
  : QgsMapRendererJob(type, settings)
{
}


QgsMapRendererSequentialJob::QgsMapRendererSequentialJob(const QgsMapSettings& settings)
  : QgsMapRendererQImageJob(SequentialJob, settings)
  , mInternalJob(0)
  , mLabelingResults( 0 )
{
  qDebug("SEQUENTIAL construct");

  mImage = QImage(mSettings.outputSize(), QImage::Format_ARGB32_Premultiplied);

  mPainter = new QPainter(&mImage);

  mInternalJob = new QgsMapRendererCustomPainterJob(mSettings, mPainter);

  connect(mInternalJob, SIGNAL(finished()), SLOT(internalFinished()));
}

QgsMapRendererSequentialJob::~QgsMapRendererSequentialJob()
{
  qDebug("SEQUENTIAL destruct");
  //delete mInternalJob;
  Q_ASSERT(mInternalJob == 0);

  delete mLabelingResults;
  mLabelingResults = 0;
}


void QgsMapRendererSequentialJob::start()
{
  qDebug("SEQUENTIAL START");
  qDebug("%d,%d", mSettings.outputSize().width(), mSettings.outputSize().height());

  mInternalJob->start();
}


void QgsMapRendererSequentialJob::cancel()
{
  if (mInternalJob)
  {
    mInternalJob->cancel();
    delete mInternalJob;
    mInternalJob = 0;
  }
}

void QgsMapRendererSequentialJob::waitForFinished()
{
  if (mInternalJob)
    mInternalJob->cancel();
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
  : QgsMapRendererJob(CustomPainterJob, settings)
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

  connect(&mFutureWatcher, SIGNAL(finished()), SLOT(futureFinished()));

  mFuture = QtConcurrent::run(staticRender, this);
  mFutureWatcher.setFuture(mFuture);
}

#include <QApplication>

void QgsMapRendererCustomPainterJob::cancel()
{
  if (mFuture.isRunning())
  {
    qDebug("QPAINTER cancelling");
    disconnect(&mFutureWatcher, SIGNAL(finished()), this, SLOT(futureFinished()));

    mRenderContext.setRenderingStopped(true);

    QTime t;
    t.start();

    mFutureWatcher.waitForFinished();

    qDebug("QPAINER cancel waited %f ms", t.elapsed() / 1000.0);

    futureFinished();

    qDebug("QPAINTER cancelled");
  }
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
  qDebug("QPAINTER doRender");

  // clear the background
  mPainter->fillRect( 0, 0, mSettings.outputSize().width(), mSettings.outputSize().height(), mSettings.backgroundColor() );

  mPainter->setRenderHint( QPainter::Antialiasing, mSettings.testFlag( QgsMapSettings::Antialiasing ) );

  QPaintDevice* thePaintDevice = mPainter->device();

  Q_ASSERT_X( thePaintDevice->logicalDpiX() == mSettings.outputDpi(), "Job::startRender()", "pre-set DPI not equal to painter's DPI" );

#ifdef QGISDEBUG
  QgsDebugMsg( "Starting to render layer stack." );
  QTime renderTime;
  renderTime.start();
#endif

  delete mLabelingEngine;
  mLabelingEngine = 0;

  if ( mSettings.testFlag( QgsMapSettings::DrawLabeling ) )
  {
    mLabelingEngine = new QgsPalLabeling;
    mLabelingEngine->loadEngineSettings();
    mLabelingEngine->init( mSettings );
  }

  mRenderContext = QgsRenderContext::fromMapSettings( mSettings );
  mRenderContext.setPainter( mPainter );
  mRenderContext.setLabelingEngine( mLabelingEngine );

  // render all layers in the stack, starting at the base
  QListIterator<QString> li( mSettings.layers() );
  li.toBack();

  QgsRectangle r1, r2;
  const QgsCoordinateTransform* ct;

  while ( li.hasPrevious() )
  {
    if ( mRenderContext.renderingStopped() )
    {
      break;
    }

    // Store the painter in case we need to swap it out for the
    // cache painter
    QPainter * mypContextPainter = mRenderContext.painter();
    // Flattened image for drawing when a blending mode is set
    QImage * mypFlattenedImage = 0;

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

    if ( mRenderContext.useAdvancedEffects() )
    {
      // Set the QPainter composition mode so that this layer is rendered using
      // the desired blending mode
      mypContextPainter->setCompositionMode( ml->blendMode() );
    }

    if ( !ml->hasScaleBasedVisibility() || ( ml->minimumScale() <= mSettings.scale() && mSettings.scale() < ml->maximumScale() ) ) //|| mOverview )
    {
      //
      // Now do the call to the layer that actually does
      // the rendering work!
      //

      bool split = false;

      if ( mSettings.hasCrsTransformEnabled() )
      {
        r1 = mSettings.visibleExtent();
        ct = QgsCoordinateTransformCache::instance()->transform( ml->crs().authid(), mSettings.destinationCrs().authid() );
        split = reprojectToLayerExtent( ct, ml->crs().geographicFlag(), r1, r2 );
        mRenderContext.setExtent( r1 );
        QgsDebugMsg( "  extent 1: " + r1.toString() );
        QgsDebugMsg( "  extent 2: " + r2.toString() );
        if ( !r1.isFinite() || !r2.isFinite() ) //there was a problem transforming the extent. Skip the layer
        {
          continue;
        }
      }
      else
      {
        ct = NULL;
      }

      mRenderContext.setCoordinateTransform( ct );

      // If we are drawing with an alternative blending mode then we need to render to a separate image
      // before compositing this on the map. This effectively flattens the layer and prevents
      // blending occuring between objects on the layer
      bool flattenedLayer = false;
      if (( mRenderContext.useAdvancedEffects() ) && ( ml->type() == QgsMapLayer::VectorLayer ) )
      {
        QgsVectorLayer* vl = qobject_cast<QgsVectorLayer *>( ml );
        if ( (( vl->blendMode() != QPainter::CompositionMode_SourceOver )
                || ( vl->featureBlendMode() != QPainter::CompositionMode_SourceOver )
                || ( vl->layerTransparency() != 0 ) ) )
        {
          flattenedLayer = true;
          mypFlattenedImage = new QImage( mRenderContext.painter()->device()->width(),
                                          mRenderContext.painter()->device()->height(), QImage::Format_ARGB32 );
          if ( mypFlattenedImage->isNull() )
          {
            QgsDebugMsg( "insufficient memory for image " + QString::number( mRenderContext.painter()->device()->width() ) + "x" + QString::number( mRenderContext.painter()->device()->height() ) );
            // TODO emit drawError( ml );
            mPainter->end(); // drawError is not caught by anyone, so we end painting to notify caller
            return;
          }
          mypFlattenedImage->fill( 0 );
          QPainter * mypPainter = new QPainter( mypFlattenedImage );
          if ( mSettings.testFlag( QgsMapSettings::Antialiasing ) )
          {
            mypPainter->setRenderHint( QPainter::Antialiasing );
          }
          mRenderContext.setPainter( mypPainter );
        }
      }

      // Per feature blending mode
      if (( mRenderContext.useAdvancedEffects() ) && ( ml->type() == QgsMapLayer::VectorLayer ) )
      {
        QgsVectorLayer* vl = qobject_cast<QgsVectorLayer *>( ml );
        if ( vl->featureBlendMode() != QPainter::CompositionMode_SourceOver )
        {
          // set the painter to the feature blend mode, so that features drawn
          // on this layer will interact and blend with each other
          mRenderContext.painter()->setCompositionMode( vl->featureBlendMode() );
        }
      }

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
      }

      //apply layer transparency for vector layers
      if (( mRenderContext.useAdvancedEffects() ) && ( ml->type() == QgsMapLayer::VectorLayer ) )
      {
        QgsVectorLayer* vl = qobject_cast<QgsVectorLayer *>( ml );
        if ( vl->layerTransparency() != 0 )
        {
          // a layer transparency has been set, so update the alpha for the flattened layer
          // by combining it with the layer transparency
          QColor transparentFillColor = QColor( 0, 0, 0, 255 - ( 255 * vl->layerTransparency() / 100 ) );
          // use destination in composition mode to merge source's alpha with destination
          mRenderContext.painter()->setCompositionMode( QPainter::CompositionMode_DestinationIn );
          mRenderContext.painter()->fillRect( 0, 0, mRenderContext.painter()->device()->width(),
                                              mRenderContext.painter()->device()->height(), transparentFillColor );
        }
      }

      if ( flattenedLayer )
      {
        // If we flattened this layer for alternate blend modes, composite it now
        delete mRenderContext.painter();
        mRenderContext.setPainter( mypContextPainter );
        mypContextPainter->drawImage( 0, 0, *( mypFlattenedImage ) );
        delete mypFlattenedImage;
        mypFlattenedImage = 0;
      }

    }
    else // layer not visible due to scale
    {
      QgsDebugMsg( "Layer not rendered because it is not within the defined "
                   "visibility scale range" );
    }

  } // while (li.hasPrevious())

  QgsDebugMsg( "Done rendering map layers" );

  // Reset the composition mode before rendering the labels
  mRenderContext.painter()->setCompositionMode( QPainter::CompositionMode_SourceOver );

  // old labeling - to be removed at some point...
  if ( mSettings.testFlag( QgsMapSettings::DrawLabeling ) )
  {
    // render all labels for vector layers in the stack, starting at the base
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

      if ( mSettings.hasCrsTransformEnabled() )
      {
        QgsRectangle r1 = mSettings.visibleExtent();
        ct = QgsCoordinateTransformCache::instance()->transform( ml->crs().authid(), mSettings.destinationCrs().authid() );
        split = reprojectToLayerExtent( ct, ml->crs().geographicFlag(), r1, r2 );
        mRenderContext.setExtent( r1 );
      }
      else
      {
        ct = NULL;
      }

      mRenderContext.setCoordinateTransform( ct );

      ml->drawLabels( mRenderContext );
      if ( split )
      {
        mRenderContext.setExtent( r2 );
        ml->drawLabels( mRenderContext );
      }
    }
  }

  if ( mLabelingEngine )
  {
    // set correct extent
    mRenderContext.setExtent( mSettings.visibleExtent() );
    mRenderContext.setCoordinateTransform( NULL );

    mLabelingEngine->drawLabeling( mRenderContext );
    mLabelingEngine->exit();
  }

  QgsDebugMsg( "Rendering completed in (seconds): " + QString( "%1" ).arg( renderTime.elapsed() / 1000.0 ) );

}




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
