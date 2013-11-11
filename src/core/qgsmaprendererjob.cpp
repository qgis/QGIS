
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

void QgsMapRendererSequentialJob::setLabelingEngine( QgsPalLabeling *labeling )
{
  mInternalJob->setLabelingEngine( labeling );
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


void QgsMapRendererCustomPainterJob::setLabelingEngine( QgsPalLabeling *labeling )
{
  mLabelingEngine = labeling;
}


void QgsMapRendererCustomPainterJob::futureFinished()
{
  qDebug("QPAINTER futureFinished");
  emit finished();
}


void QgsMapRendererCustomPainterJob::staticRender(QgsMapRendererCustomPainterJob* self)
{
  self->startRender();
}


void QgsMapRendererCustomPainterJob::startRender()
{
  qDebug("QPAINTER startRender");

  // clear the background
  mPainter->fillRect( 0, 0, mSettings.outputSize().width(), mSettings.outputSize().height(), mSettings.backgroundColor() );

  mPainter->setRenderHint( QPainter::Antialiasing, mSettings.isAntiAliasingEnabled() );

  QPaintDevice* thePaintDevice = mPainter->device();

#ifdef QGISDEBUG
  QgsDebugMsg( "Starting to render layer stack." );
  QTime renderTime;
  renderTime.start();
#endif

  mRenderContext.setMapToPixel( mSettings.mapToPixel() ); //  QgsMapToPixel( mSettings.mapUnitsPerPixel(), mSettings.outputSize().height(), mSettings.visibleExtent().yMinimum(), mSettings.visibleExtent().xMinimum() ) );
  mRenderContext.setExtent( mSettings.visibleExtent() );

  mRenderContext.setDrawEditingInformation( false );
  mRenderContext.setPainter( mPainter );
  mRenderContext.setCoordinateTransform( 0 );
  //this flag is only for stopping during the current rendering progress,
  //so must be false at every new render operation
  mRenderContext.setRenderingStopped( false );

  // set selection color
  mRenderContext.setSelectionColor( mSettings.selectionColor() );

  //calculate scale factor
  //use the specified dpi and not those from the paint device
  //because sometimes QPainter units are in a local coord sys (e.g. in case of QGraphicsScene)
  double* forceWidthScale = 0; // TODO: may point to a value (composer)
  double sceneDpi = mSettings.outputDpi();
  double scaleFactor = 1.0;
  if ( mSettings.outputUnits() == QgsMapSettings::Millimeters )
  {
    if ( forceWidthScale )
    {
      scaleFactor = *forceWidthScale;
    }
    else
    {
      scaleFactor = sceneDpi / 25.4;
    }
  }
  double rasterScaleFactor = ( thePaintDevice->logicalDpiX() + thePaintDevice->logicalDpiY() ) / 2.0 / sceneDpi;
  mRenderContext.setRasterScaleFactor( rasterScaleFactor );
  mRenderContext.setScaleFactor( scaleFactor );
  mRenderContext.setRendererScale( mSettings.scale() );

  mRenderContext.setLabelingEngine( mLabelingEngine );
  if ( mLabelingEngine )
    mLabelingEngine->init( mSettings );

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
    //QPainter * mypContextPainter = mRenderContext.painter();
    // Flattened image for drawing when a blending mode is set
    //QImage * mypFlattenedImage = 0;

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

    /*if ( mRenderContext.useAdvancedEffects() )
    {
      // Set the QPainter composition mode so that this layer is rendered using
      // the desired blending mode
      mypContextPainter->setCompositionMode( ml->blendMode() );
    }*/

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

      //decide if we have to scale the raster
      //this is necessary in case QGraphicsScene is used
      bool scaleRaster = false;
      QgsMapToPixel rasterMapToPixel;
      QgsMapToPixel bk_mapToPixel;

      if ( ml->type() == QgsMapLayer::RasterLayer && qAbs( rasterScaleFactor - 1.0 ) > 0.000001 )
      {
        scaleRaster = true;
      }

      /*QSettings mySettings;
      bool useRenderCaching = false;
      if ( ! split )//render caching does not yet cater for split extents
      {
        if ( mySettings.value( "/qgis/enable_render_caching", false ).toBool() )
        {
          useRenderCaching = true;
          if ( !mySameAsLastFlag || ml->cacheImage() == 0 )
          {
            QgsDebugMsg( "Caching enabled but layer redraw forced by extent change or empty cache" );
            QImage * mypImage = new QImage( mRenderContext.painter()->device()->width(),
                                            mRenderContext.painter()->device()->height(), QImage::Format_ARGB32 );
            if ( mypImage->isNull() )
            {
              QgsDebugMsg( "insufficient memory for image " + QString::number( mRenderContext.painter()->device()->width() ) + "x" + QString::number( mRenderContext.painter()->device()->height() ) );
              emit drawError( ml );
              painter->end(); // drawError is not caught by anyone, so we end painting to notify caller
              return;
            }
            mypImage->fill( 0 );
            ml->setCacheImage( mypImage ); //no need to delete the old one, maplayer does it for you
            QPainter * mypPainter = new QPainter( ml->cacheImage() );
            // Changed to enable anti aliasing by default in QGIS 1.7
            if ( mySettings.value( "/qgis/enable_anti_aliasing", true ).toBool() )
            {
              mypPainter->setRenderHint( QPainter::Antialiasing );
            }
            mRenderContext.setPainter( mypPainter );
          }
          else if ( mySameAsLastFlag )
          {
            //draw from cached image
            QgsDebugMsg( "Caching enabled --- drawing layer from cached image" );
            mypContextPainter->drawImage( 0, 0, *( ml->cacheImage() ) );
            //short circuit as there is nothing else to do...
            continue;
          }
        }
      }*/

      // If we are drawing with an alternative blending mode then we need to render to a separate image
      // before compositing this on the map. This effectively flattens the layer and prevents
      // blending occuring between objects on the layer
      // (this is not required for raster layers or when layer caching is enabled, since that has the same effect)
      /*bool flattenedLayer = false;
      if (( mRenderContext.useAdvancedEffects() ) && ( ml->type() == QgsMapLayer::VectorLayer ) )
      {
        QgsVectorLayer* vl = qobject_cast<QgsVectorLayer *>( ml );
        if (( !useRenderCaching )
            && (( vl->blendMode() != QPainter::CompositionMode_SourceOver )
                || ( vl->featureBlendMode() != QPainter::CompositionMode_SourceOver )
                || ( vl->layerTransparency() != 0 ) ) )
        {
          flattenedLayer = true;
          mypFlattenedImage = new QImage( mRenderContext.painter()->device()->width(),
                                          mRenderContext.painter()->device()->height(), QImage::Format_ARGB32 );
          if ( mypFlattenedImage->isNull() )
          {
            QgsDebugMsg( "insufficient memory for image " + QString::number( mRenderContext.painter()->device()->width() ) + "x" + QString::number( mRenderContext.painter()->device()->height() ) );
            emit drawError( ml );
            painter->end(); // drawError is not caught by anyone, so we end painting to notify caller
            return;
          }
          mypFlattenedImage->fill( 0 );
          QPainter * mypPainter = new QPainter( mypFlattenedImage );
          if ( mySettings.value( "/qgis/enable_anti_aliasing", true ).toBool() )
          {
            mypPainter->setRenderHint( QPainter::Antialiasing );
          }
          mypPainter->scale( rasterScaleFactor,  rasterScaleFactor );
          mRenderContext.setPainter( mypPainter );
        }
      }*/

      // Per feature blending mode
      /*if (( mRenderContext.useAdvancedEffects() ) && ( ml->type() == QgsMapLayer::VectorLayer ) )
      {
        QgsVectorLayer* vl = qobject_cast<QgsVectorLayer *>( ml );
        if ( vl->featureBlendMode() != QPainter::CompositionMode_SourceOver )
        {
          // set the painter to the feature blend mode, so that features drawn
          // on this layer will interact and blend with each other
          mRenderContext.painter()->setCompositionMode( vl->featureBlendMode() );
        }
      }*/

      if ( scaleRaster )
      {
        bk_mapToPixel = mRenderContext.mapToPixel();
        rasterMapToPixel = mRenderContext.mapToPixel();
        rasterMapToPixel.setMapUnitsPerPixel( mRenderContext.mapToPixel().mapUnitsPerPixel() / rasterScaleFactor );
        rasterMapToPixel.setYMaximum( mSettings.outputSize().height() * rasterScaleFactor );
        mRenderContext.setMapToPixel( rasterMapToPixel );
        mRenderContext.painter()->save();
        mRenderContext.painter()->scale( 1.0 / rasterScaleFactor, 1.0 / rasterScaleFactor );
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

      if ( scaleRaster )
      {
        mRenderContext.setMapToPixel( bk_mapToPixel );
        mRenderContext.painter()->restore();
      }

      //apply layer transparency for vector layers
      /*if (( mRenderContext.useAdvancedEffects() ) && ( ml->type() == QgsMapLayer::VectorLayer ) )
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
      }*/

      /*if ( useRenderCaching )
      {
        // composite the cached image into our view and then clean up from caching
        // by reinstating the painter as it was swapped out for caching renders
        delete mRenderContext.painter();
        mRenderContext.setPainter( mypContextPainter );
        //draw from cached image that we created further up
        if ( ml->cacheImage() )
          mypContextPainter->drawImage( 0, 0, *( ml->cacheImage() ) );
      }
      else if ( flattenedLayer )
      {
        // If we flattened this layer for alternate blend modes, composite it now
        delete mRenderContext.painter();
        mRenderContext.setPainter( mypContextPainter );
        mypContextPainter->save();
        mypContextPainter->scale( 1.0 / rasterScaleFactor, 1.0 / rasterScaleFactor );
        mypContextPainter->drawImage( 0, 0, *( mypFlattenedImage ) );
        mypContextPainter->restore();
        delete mypFlattenedImage;
        mypFlattenedImage = 0;
      }*/

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

  /*if ( !mOverview )
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

      if ( ml && ( ml->type() != QgsMapLayer::RasterLayer ) )
      {
        // only make labels if the layer is visible
        // after scale dep viewing settings are checked
        if ( !ml->hasScaleBasedVisibility() || ( ml->minimumScale() < mScale && mScale < ml->maximumScale() ) )
        {
          bool split = false;

          if ( hasCrsTransformEnabled() )
          {
            QgsRectangle r1 = mExtent;
            split = splitLayersExtent( ml, r1, r2 );
            ct = new QgsCoordinateTransform( ml->crs(), *mDestCRS );
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
    }
  } // if (!mOverview)*/

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
