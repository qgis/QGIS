
#include "qgsmaprendererv2.h"

#include "qgsscalecalculator.h"
#include "qgsmaprendererjob.h"
#include "qgsmaptopixel.h"
#include "qgslogger.h"

/*

usage in QgsMapCanvas - upon pan / zoom - in QgsMapCanvasMap:
- stop rendering if active
- update QgsMapRendererV2 settings
- start rendering
- start update timer
- [on timeout/finished] show rendered image

usage in QgsComposer
- create QgsMapRendererV2
- setup, start with QPainter
- wait until it finishes

*/




QgsMapRendererV2::QgsMapRendererV2()
  : mActiveJob(0)
  , mScaleCalculator(new QgsScaleCalculator())
{
}

QgsMapRendererV2::~QgsMapRendererV2()
{
  // make sure there is no job running
  cancel();

}

bool QgsMapRendererV2::start(bool parallel)
{
  if (mActiveJob)
    return false;

  if (parallel)
  {
    Q_ASSERT(false && "parallel job not implemented yet");
    return false;
  }
  else
  {
    mActiveJob = new QgsMapRendererSequentialJob(mSettings);
  }

  connect(mActiveJob, SIGNAL(finished()), this, SLOT(onJobFinished()));

  mActiveJob->start();
  return true;
}


bool QgsMapRendererV2::startWithCustomPainter(QPainter *painter)
{
  if (mActiveJob)
    return false;

  mActiveJob = new QgsMapRendererCustomPainterJob(mSettings, painter);

  connect(mActiveJob, SIGNAL(finished()), this, SLOT(onJobFinished()));

  mActiveJob->start();
  return true;
}

bool QgsMapRendererV2::cancel()
{
  if (!mActiveJob)
    return false;

  mActiveJob->cancel();
  return true;
}


void QgsMapRendererV2::onJobFinished()
{
  qDebug("onJobFinished");
  Q_ASSERT(mActiveJob);

  emit finished();

  mActiveJob->deleteLater();
  mActiveJob = 0;
}






QgsRectangle QgsMapRendererV2::extent() const
{
  return mSettings.extent;
}

void QgsMapRendererV2::setExtent(const QgsRectangle& extent)
{
  mSettings.extent = extent;
}


void QgsMapRendererV2::updateDerived()
{
  QgsRectangle extent = mSettings.extent;

  if ( extent.isEmpty() )
  {
    mSettings.valid = false;
    return;
  }

  // Don't allow zooms where the current extent is so small that it
  // can't be accurately represented using a double (which is what
  // currentExtent uses). Excluding 0 avoids a divide by zero and an
  // infinite loop when rendering to a new canvas. Excluding extents
  // greater than 1 avoids doing unnecessary calculations.

  // The scheme is to compare the width against the mean x coordinate
  // (and height against mean y coordinate) and only allow zooms where
  // the ratio indicates that there is more than about 12 significant
  // figures (there are about 16 significant figures in a double).

  if ( extent.width()  > 0 &&
       extent.height() > 0 &&
       extent.width()  < 1 &&
       extent.height() < 1 )
  {
    // Use abs() on the extent to avoid the case where the extent is
    // symmetrical about 0.
    double xMean = ( qAbs( extent.xMinimum() ) + qAbs( extent.xMaximum() ) ) * 0.5;
    double yMean = ( qAbs( extent.yMinimum() ) + qAbs( extent.yMaximum() ) ) * 0.5;

    double xRange = extent.width() / xMean;
    double yRange = extent.height() / yMean;

    static const double minProportion = 1e-12;
    if ( xRange < minProportion || yRange < minProportion )
    {
      mSettings.valid = false;
      return;
    }
  }

  double myHeight = mSettings.size.height();
  double myWidth = mSettings.size.width();

  if ( !myWidth || !myHeight )
  {
    mSettings.valid = false;
    return;
  }

  // calculate the translation and scaling parameters
  double mapUnitsPerPixelY = mSettings.extent.height() / myHeight;
  double mapUnitsPerPixelX = mSettings.extent.width() / myWidth;
  mSettings.mapUnitsPerPixel = mapUnitsPerPixelY > mapUnitsPerPixelX ? mapUnitsPerPixelY : mapUnitsPerPixelX;

  // calculate the actual extent of the mapCanvas
  double dxmin = mSettings.extent.xMinimum(), dxmax = mSettings.extent.xMaximum(),
         dymin = mSettings.extent.yMinimum(), dymax = mSettings.extent.yMaximum(), whitespace;

  if ( mapUnitsPerPixelY > mapUnitsPerPixelX )
  {
    whitespace = (( myWidth * mSettings.mapUnitsPerPixel ) - mSettings.extent.width() ) * 0.5;
    dxmin -= whitespace;
    dxmax += whitespace;
  }
  else
  {
    whitespace = (( myHeight * mSettings.mapUnitsPerPixel ) - mSettings.extent.height() ) * 0.5;
    dymin -= whitespace;
    dymax += whitespace;
  }

  mSettings.visibleExtent.set( dxmin, dymin, dxmax, dymax );

  // update the scale
  mSettings.scale = mScaleCalculator->calculate( mSettings.extent, mSettings.size.width() );

  QgsDebugMsg( QString( "Map units per pixel (x,y) : %1, %2" ).arg( qgsDoubleToString( mapUnitsPerPixelX ) ).arg( qgsDoubleToString( mapUnitsPerPixelY ) ) );
  QgsDebugMsg( QString( "Pixmap dimensions (x,y) : %1, %2" ).arg( qgsDoubleToString( myWidth ) ).arg( qgsDoubleToString( myHeight ) ) );
  QgsDebugMsg( QString( "Extent dimensions (x,y) : %1, %2" ).arg( qgsDoubleToString( mSettings.extent.width() ) ).arg( qgsDoubleToString( mSettings.extent.height() ) ) );
  QgsDebugMsg( mSettings.extent.toString() );
  QgsDebugMsg( QString( "Adjusted map units per pixel (x,y) : %1, %2" ).arg( qgsDoubleToString( mSettings.visibleExtent.width() / myWidth ) ).arg( qgsDoubleToString( mSettings.visibleExtent.height() / myHeight ) ) );
  QgsDebugMsg( QString( "Recalced pixmap dimensions (x,y) : %1, %2" ).arg( qgsDoubleToString( mSettings.visibleExtent.width() / mSettings.mapUnitsPerPixel ) ).arg( qgsDoubleToString( mSettings.visibleExtent.height() / mSettings.mapUnitsPerPixel ) ) );
  QgsDebugMsg( QString( "Scale (assuming meters as map units) = 1:%1" ).arg( qgsDoubleToString( mSettings.scale ) ) );

}


QSize QgsMapRendererV2::outputSize() const
{
  return mSettings.size;
}

void QgsMapRendererV2::setOutputSize(const QSize& size)
{
  mSettings.size = size;
}

double QgsMapRendererV2::outputDpi() const
{
  return mSettings.dpi;
}

void QgsMapRendererV2::setOutputDpi(double dpi)
{
  mSettings.dpi = dpi;
}


QStringList QgsMapRendererV2::layers() const
{
  return mSettings.layers;
}

void QgsMapRendererV2::setLayers(const QStringList& layers)
{
  mSettings.layers = layers;
}


bool QgsMapRendererV2::hasValidSettings() const
{
  return mSettings.valid;
}

QgsRectangle QgsMapRendererV2::visibleExtent() const
{
  return mSettings.visibleExtent;
}

double QgsMapRendererV2::mapUnitsPerPixel() const
{
  return mSettings.mapUnitsPerPixel;
}

double QgsMapRendererV2::scale() const
{
  return mSettings.scale;
}
