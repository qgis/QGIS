/***************************************************************************
                          qgselevationprofilecanvas.cpp
                          -----------------
    begin                : March 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************/


/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgselevationprofilecanvas.h"
#include "qgsmaplayerlistutils_p.h"
#include "qgsplotcanvasitem.h"
#include "qgsprofilerequest.h"
#include "qgsabstractprofilesource.h"
#include "qgscurve.h"
#include "qgsprojectelevationproperties.h"
#include "qgsterrainprovider.h"
#include "qgsabstractprofilegenerator.h"
#include "qgsprofilerenderer.h"
#include "qgsplot.h"
#include "qgspoint.h"
#include "qgsgeos.h"
#include <QWheelEvent>

///@cond PRIVATE
class QgsElevationProfilePlotItem : public Qgs2DPlot, public QgsPlotCanvasItem
{
  public:

    QgsElevationProfilePlotItem( QgsElevationProfileCanvas *canvas )
      : QgsPlotCanvasItem( canvas )
    {
      setYMinimum( 0 );
      setYMaximum( 100 );
    }

    void setRenderer( QgsProfilePlotRenderer *renderer )
    {
      mRenderer = renderer;
    }

    void updateRect()
    {
      mRect = mCanvas->rect();
      setSize( mRect.size() );

      prepareGeometryChange();
      setPos( mRect.topLeft() );

      mImage = QImage();
      mPlotArea = QRectF();
      update();
    }

    void updatePlot()
    {
      mImage = QImage();
      mPlotArea = QRectF();
      update();
    }

    QRectF boundingRect() const override
    {
      return mRect;
    }

    QRectF plotArea()
    {
      if ( !mPlotArea.isNull() )
        return mPlotArea;

      // force immediate recalculation of plot area
      QgsRenderContext context;
      calculateOptimisedIntervals( context );
      mPlotArea = interiorPlotArea( context );
      return mPlotArea;
    }

    void renderContent( QgsRenderContext &rc, const QRectF &plotArea ) override
    {
      mPlotArea = plotArea;

      if ( !mRenderer )
        return;

      const QImage plot = mRenderer->renderToImage( plotArea.width(), plotArea.height(), xMinimum(), xMaximum(), yMinimum(), yMaximum() );
      rc.painter()->drawImage( plotArea.left(), plotArea.top(), plot );
    }

    void paint( QPainter *painter ) override
    {
      // cache rendering to an image, so we don't need to redraw the plot
      if ( !mImage.isNull() )
      {
        painter->drawImage( 0, 0, mImage );
      }
      else
      {
        mImage = QImage( mRect.width(), mRect.height(), QImage::Format_ARGB32_Premultiplied );
        mImage.fill( Qt::transparent );

        QPainter imagePainter( &mImage );
        imagePainter.setRenderHint( QPainter::Antialiasing, true );
        QgsRenderContext rc = QgsRenderContext::fromQPainter( &imagePainter );
        calculateOptimisedIntervals( rc );
        render( rc );
        imagePainter.end();

        painter->drawImage( 0, 0, mImage );
      }
    }

  private:

    QImage mImage;
    QRectF mRect;
    QRectF mPlotArea;
    QgsProfilePlotRenderer *mRenderer = nullptr;
};
///@endcond PRIVATE


QgsElevationProfileCanvas::QgsElevationProfileCanvas( QWidget *parent )
  : QgsPlotCanvas( parent )
{
  mPlotItem = new QgsElevationProfilePlotItem( this );
}

QgsElevationProfileCanvas::~QgsElevationProfileCanvas()
{
  if ( mCurrentJob )
  {
    mPlotItem->setRenderer( nullptr );
    mCurrentJob->deleteLater();
    mCurrentJob = nullptr;
  }
}

void QgsElevationProfileCanvas::cancelJobs()
{
  if ( mCurrentJob )
  {
    mPlotItem->setRenderer( nullptr );
    disconnect( mCurrentJob, &QgsProfilePlotRenderer::generationFinished, this, &QgsElevationProfileCanvas::generationFinished );
    mCurrentJob->cancelGeneration();
    mCurrentJob->deleteLater();
    mCurrentJob = nullptr;
  }
}

void QgsElevationProfileCanvas::panContentsBy( double dx, double dy )
{
  const double dxPercent = dx / mPlotItem->plotArea().width();
  const double dyPercent = dy / mPlotItem->plotArea().height();

  // these look backwards, but we are dragging the paper, not the view!
  const double dxPlot = - dxPercent * ( mPlotItem->xMaximum() - mPlotItem->xMinimum() );
  const double dyPlot = dyPercent * ( mPlotItem->yMaximum() - mPlotItem->yMinimum() );

  mPlotItem->setXMinimum( mPlotItem->xMinimum() + dxPlot );
  mPlotItem->setXMaximum( mPlotItem->xMaximum() + dxPlot );
  mPlotItem->setYMinimum( mPlotItem->yMinimum() + dyPlot );
  mPlotItem->setYMaximum( mPlotItem->yMaximum() + dyPlot );

  mPlotItem->updatePlot();
}

void QgsElevationProfileCanvas::centerPlotOn( double x, double y )
{
  if ( !mPlotItem->plotArea().contains( x, y ) )
    return;

  const double newCenterX = mPlotItem->xMinimum() + ( x - mPlotItem->plotArea().left() ) / mPlotItem->plotArea().width() * ( mPlotItem->xMaximum() - mPlotItem->xMinimum() );
  const double newCenterY = mPlotItem->yMinimum() + ( mPlotItem->plotArea().height() - y ) / mPlotItem->plotArea().height() * ( mPlotItem->yMaximum() - mPlotItem->yMinimum() );

  const double dxPlot = newCenterX - ( mPlotItem->xMaximum() + mPlotItem->xMinimum() ) * 0.5;
  const double dyPlot = newCenterY - ( mPlotItem->yMaximum() + mPlotItem->yMinimum() ) * 0.5;

  mPlotItem->setXMinimum( mPlotItem->xMinimum() + dxPlot );
  mPlotItem->setXMaximum( mPlotItem->xMaximum() + dxPlot );
  mPlotItem->setYMinimum( mPlotItem->yMinimum() + dyPlot );
  mPlotItem->setYMaximum( mPlotItem->yMaximum() + dyPlot );

  mPlotItem->updatePlot();
}

void QgsElevationProfileCanvas::scalePlot( double factor )
{
  const double currentWidth = mPlotItem->xMaximum() - mPlotItem->xMinimum();
  const double currentHeight = mPlotItem->yMaximum() - mPlotItem->yMinimum();

  const double newWidth = currentWidth / factor;
  const double newHeight = currentHeight / factor;

  const double currentCenterX = ( mPlotItem->xMinimum() + mPlotItem->xMaximum() ) * 0.5;
  const double currentCenterY = ( mPlotItem->yMinimum() + mPlotItem->yMaximum() ) * 0.5;

  mPlotItem->setXMinimum( currentCenterX - newWidth * 0.5 );
  mPlotItem->setXMaximum( currentCenterX + newWidth * 0.5 );
  mPlotItem->setYMinimum( currentCenterY - newHeight * 0.5 );
  mPlotItem->setYMaximum( currentCenterY + newHeight * 0.5 );

  mPlotItem->updatePlot();
}

void QgsElevationProfileCanvas::zoomToRect( const QRectF rect )
{
  const QRectF intersected = rect.intersected( mPlotItem->plotArea() );

  const double minX = ( intersected.left() - mPlotItem->plotArea().left() ) / mPlotItem->plotArea().width() * ( mPlotItem->xMaximum() - mPlotItem->xMinimum() ) + mPlotItem->xMinimum();
  const double maxX = ( intersected.right() - mPlotItem->plotArea().left() ) / mPlotItem->plotArea().width() * ( mPlotItem->xMaximum() - mPlotItem->xMinimum() ) + mPlotItem->xMinimum();
  const double minY = ( mPlotItem->plotArea().bottom() - intersected.bottom() ) / mPlotItem->plotArea().height() * ( mPlotItem->yMaximum() - mPlotItem->yMinimum() ) + mPlotItem->yMinimum();
  const double maxY = ( mPlotItem->plotArea().bottom() - intersected.top() ) / mPlotItem->plotArea().height() * ( mPlotItem->yMaximum() - mPlotItem->yMinimum() ) + mPlotItem->yMinimum();

  mPlotItem->setXMinimum( minX );
  mPlotItem->setXMaximum( maxX );
  mPlotItem->setYMinimum( minY );
  mPlotItem->setYMaximum( maxY );

  mPlotItem->updatePlot();
}

void QgsElevationProfileCanvas::wheelZoom( QWheelEvent *event )
{
  //get mouse wheel zoom behavior settings
  QgsSettings settings;
  double zoomFactor = settings.value( QStringLiteral( "qgis/zoom_factor" ), 2 ).toDouble();

  // "Normal" mouse have an angle delta of 120, precision mouses provide data faster, in smaller steps
  zoomFactor = 1.0 + ( zoomFactor - 1.0 ) / 120.0 * std::fabs( event->angleDelta().y() );

  if ( event->modifiers() & Qt::ControlModifier )
  {
    //holding ctrl while wheel zooming results in a finer zoom
    zoomFactor = 1.0 + ( zoomFactor - 1.0 ) / 20.0;
  }

  //calculate zoom scale factor
  bool zoomIn = event->angleDelta().y() > 0;
  double scaleFactor = ( zoomIn ? 1 / zoomFactor : zoomFactor );

  QRectF viewportRect = mPlotItem->plotArea();
  if ( viewportRect.contains( event->pos() ) )
  {
    //adjust view center
    const double oldCenterX = 0.5 * ( mPlotItem->xMaximum() + mPlotItem->xMinimum() );
    const double oldCenterY = 0.5 * ( mPlotItem->yMaximum() + mPlotItem->yMinimum() );

    const double eventPosX = ( event->pos().x() - viewportRect.left() ) / viewportRect.width() * ( mPlotItem->xMaximum() - mPlotItem->xMinimum() ) + mPlotItem->xMinimum();
    const double eventPosY = ( viewportRect.bottom() - event->pos().y() ) / viewportRect.height() * ( mPlotItem->yMaximum() - mPlotItem->yMinimum() ) + mPlotItem->yMinimum();

    const double newCenterX = eventPosX + ( ( oldCenterX - eventPosX ) * scaleFactor );
    const double newCenterY = eventPosY + ( ( oldCenterY - eventPosY ) * scaleFactor );

    const double dxPlot = newCenterX - ( mPlotItem->xMaximum() + mPlotItem->xMinimum() ) * 0.5;
    const double dyPlot = newCenterY - ( mPlotItem->yMaximum() + mPlotItem->yMinimum() ) * 0.5;

    mPlotItem->setXMinimum( mPlotItem->xMinimum() + dxPlot );
    mPlotItem->setXMaximum( mPlotItem->xMaximum() + dxPlot );
    mPlotItem->setYMinimum( mPlotItem->yMinimum() + dyPlot );
    mPlotItem->setYMaximum( mPlotItem->yMaximum() + dyPlot );
  }

  //zoom plot
  if ( zoomIn )
  {
    scalePlot( zoomFactor );
  }
  else
  {
    scalePlot( 1 / zoomFactor );
  }
}

void QgsElevationProfileCanvas::refresh()
{
  if ( !mProject || !profileCurve() )
    return;

  if ( mCurrentJob )
  {
    mPlotItem->setRenderer( nullptr );
    disconnect( mCurrentJob, &QgsProfilePlotRenderer::generationFinished, this, &QgsElevationProfileCanvas::generationFinished );
    mCurrentJob->deleteLater();
    mCurrentJob = nullptr;
  }

  QgsProfileRequest request( profileCurve()->clone() );
  request.setCrs( mCrs );
  request.setTransformContext( mProject->transformContext() );
  request.setTerrainProvider( mProject->elevationProperties()->terrainProvider() ? mProject->elevationProperties()->terrainProvider()->clone() : nullptr );

  const QList< QgsMapLayer * > layersToUpdate = layers();
  QList< QgsAbstractProfileSource * > sources;
  sources.reserve( layersToUpdate.size() );
  for ( QgsMapLayer *layer : layersToUpdate )
  {
    if ( QgsAbstractProfileSource *source = dynamic_cast< QgsAbstractProfileSource * >( layer ) )
      sources.append( source );
  }

  mCurrentJob = new QgsProfilePlotRenderer( sources, request );
  connect( mCurrentJob, &QgsProfilePlotRenderer::generationFinished, this, &QgsElevationProfileCanvas::generationFinished );
  mCurrentJob->startGeneration();
  mPlotItem->setRenderer( mCurrentJob );

  emit activeJobCountChanged( 1 );
}

void QgsElevationProfileCanvas::generationFinished()
{
  if ( !mCurrentJob )
    return;

  emit activeJobCountChanged( 0 );

  zoomFull();
}

void QgsElevationProfileCanvas::setProject( QgsProject *project )
{
  mProject = project;
}

void QgsElevationProfileCanvas::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  mCrs = crs;
}

void QgsElevationProfileCanvas::setProfileCurve( QgsCurve *curve )
{
  mProfileCurve.reset( curve );
}

QgsCurve *QgsElevationProfileCanvas::profileCurve() const
{
  return mProfileCurve.get();
}

QgsCoordinateReferenceSystem QgsElevationProfileCanvas::crs() const
{
  return mCrs;
}

void QgsElevationProfileCanvas::setLayers( const QList<QgsMapLayer *> &layers )
{
  // filter list, removing null layers and invalid layers
  auto filteredList = layers;
  filteredList.erase( std::remove_if( filteredList.begin(), filteredList.end(),
                                      []( QgsMapLayer * layer )
  {
    return !layer || !layer->isValid();
  } ), filteredList.end() );

  mLayers = _qgis_listRawToQPointer( filteredList );
}

QList<QgsMapLayer *> QgsElevationProfileCanvas::layers() const
{
  return _qgis_listQPointerToRaw( mLayers );
}

void QgsElevationProfileCanvas::resizeEvent( QResizeEvent *event )
{
  QgsPlotCanvas::resizeEvent( event );
  mPlotItem->updateRect();
}

void QgsElevationProfileCanvas::paintEvent( QPaintEvent *event )
{
  QgsPlotCanvas::paintEvent( event );

  if ( !mFirstDrawOccurred )
  {
    // on first show we need to update the visible rect of the plot. (Not sure why this doesn't work in showEvent, but it doesn't).
    mFirstDrawOccurred = true;
    mPlotItem->updateRect();
  }
}

QgsPoint QgsElevationProfileCanvas::toMapCoordinates( const QgsPointXY &point ) const
{
  if ( !mPlotItem->plotArea().contains( point.x(), point.y() ) )
    return QgsPoint();

  if ( !mProfileCurve )
    return QgsPoint();

  const double dx = point.x() - mPlotItem->plotArea().left();

  const double distanceAlongPlotPercent = dx / mPlotItem->plotArea().width();
  double distanceAlongCurveLength = distanceAlongPlotPercent * ( mPlotItem->xMaximum() - mPlotItem->xMinimum() ) + mPlotItem->xMinimum();

  std::unique_ptr< QgsPoint > mapXyPoint( mProfileCurve->interpolatePoint( distanceAlongCurveLength ) );
  if ( !mapXyPoint )
    return QgsPoint();

  const double mapZ = ( mPlotItem->yMaximum() - mPlotItem->yMinimum() ) / ( mPlotItem->plotArea().height() ) * ( mPlotItem->plotArea().bottom() - point.y() ) + mPlotItem->yMinimum();

  return QgsPoint( mapXyPoint->x(), mapXyPoint->y(), mapZ );
}

QgsPointXY QgsElevationProfileCanvas::toCanvasCoordinates( const QgsPoint &point ) const
{
  if ( !mProfileCurve )
    return QgsPointXY();

  QgsGeos geos( mProfileCurve.get() );
  QString error;
  const double distanceAlongCurve = geos.lineLocatePoint( point, &error );

  const double distanceAlongCurveOnPlot = distanceAlongCurve - mPlotItem->xMinimum();
  const double distanceAlongCurvePercent = distanceAlongCurveOnPlot / ( mPlotItem->xMaximum() - mPlotItem->xMinimum() );
  const double distanceAlongPlotRect = distanceAlongCurvePercent * mPlotItem->plotArea().width();

  const double canvasX = mPlotItem->plotArea().left() + distanceAlongPlotRect;

  double canvasY = 0;
  if ( std::isnan( point.z() ) || point.z() < mPlotItem->yMinimum() )
  {
    canvasY = mPlotItem->plotArea().top();
  }
  else if ( point.z() > mPlotItem->yMaximum() )
  {
    canvasY = mPlotItem->plotArea().bottom();
  }
  else
  {
    const double yPercent = ( point.z() - mPlotItem->yMinimum() ) / ( mPlotItem->yMaximum() - mPlotItem->yMinimum() );
    canvasY = mPlotItem->plotArea().bottom() - mPlotItem->plotArea().height() * yPercent;
  }

  return QgsPointXY( canvasX, canvasY );
}

void QgsElevationProfileCanvas::zoomFull()
{
  if ( !mCurrentJob )
    return;

  const QgsDoubleRange zRange = mCurrentJob->zRange();
  // add 5% margin to height range
  const double margin = ( zRange.upper() - zRange.lower() ) * 0.05;
  mPlotItem->setYMinimum( zRange.lower() - margin );
  mPlotItem->setYMaximum( zRange.upper() + margin );

  const double profileLength = profileCurve()->length();
  mPlotItem->setXMinimum( 0 );
  // just 2% margin to max distance -- any more is overkill and wasted space
  mPlotItem->setXMaximum( profileLength  * 1.02 );

  mPlotItem->updatePlot();
}

void QgsElevationProfileCanvas::setVisiblePlotRange( double minimumDistance, double maximumDistance, double minimumElevation, double maximumElevation )
{
  mPlotItem->setYMinimum( minimumElevation );
  mPlotItem->setYMaximum( maximumElevation );
  mPlotItem->setXMinimum( minimumDistance );
  mPlotItem->setXMaximum( maximumDistance );
  mPlotItem->updatePlot();
}

void QgsElevationProfileCanvas::clear()
{
  setProfileCurve( nullptr );
  mPlotItem->setRenderer( nullptr );
  mPlotItem->updatePlot();
}
