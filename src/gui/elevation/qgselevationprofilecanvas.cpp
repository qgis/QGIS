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
#include "qgspoint.h"
#include "qgsgeos.h"
#include "qgsplot.h"
#include "qgsguiutils.h"
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

    QgsPointXY canvasPointToPlotPoint( const QPointF &point )
    {
      if ( !mPlotArea.contains( point.x(), point.y() ) )
        return QgsPointXY();

      const double x = ( point.x() - mPlotArea.left() ) / mPlotArea.width() * ( xMaximum() - xMinimum() ) + xMinimum();
      const double y = ( mPlotArea.bottom() - point.y() ) / mPlotArea.height() * ( yMaximum() - yMinimum() ) + yMinimum();
      return QgsPointXY( x, y );
    }

    QgsPointXY plotPointToCanvasPoint( const QgsPointXY &point )
    {
      if ( point.x() < xMinimum() || point.x() > xMaximum() || point.y() < yMinimum() || point.y() > yMaximum() )
        return QgsPointXY();

      const double x = ( point.x() - xMinimum() ) / ( xMaximum() - xMinimum() ) * ( mPlotArea.width() ) + mPlotArea.left();
      const double y = mPlotArea.bottom() - ( point.y() - yMinimum() ) / ( yMaximum() - yMinimum() ) * ( mPlotArea.height() );
      return QgsPointXY( x, y );
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

class QgsElevationProfileCrossHairsItem : public QgsPlotCanvasItem
{
  public:

    QgsElevationProfileCrossHairsItem( QgsElevationProfileCanvas *canvas, QgsElevationProfilePlotItem *plotItem )
      : QgsPlotCanvasItem( canvas )
      , mPlotItem( plotItem )
    {
    }

    void updateRect()
    {
      mRect = mCanvas->rect();

      prepareGeometryChange();
      setPos( mRect.topLeft() );
      update();
    }

    void setPoint( const QgsPointXY &point )
    {
      mPoint = point;
      update();
    }

    QRectF boundingRect() const override
    {
      return mRect;
    }

    void paint( QPainter *painter ) override
    {
      const QgsPointXY crossHairPlotPoint  = mPlotItem->plotPointToCanvasPoint( mPoint );
      if ( crossHairPlotPoint.isEmpty() )
        return;

      painter->save();
      painter->setBrush( Qt::NoBrush );
      QPen crossHairPen;
      crossHairPen.setCosmetic( true );
      crossHairPen.setWidthF( QgsGuiUtils::scaleIconSize( 2 ) );
      crossHairPen.setStyle( Qt::DashLine );
      crossHairPen.setCapStyle( Qt::FlatCap );
      crossHairPen.setColor( QColor( 0, 0, 0, 150 ) );
      painter->setPen( crossHairPen );
      painter->drawLine( QPointF( mPlotItem->plotArea().left(), crossHairPlotPoint.y() ), QPointF( mPlotItem->plotArea().right(), crossHairPlotPoint.y() ) );
      painter->drawLine( QPointF( crossHairPlotPoint.x(), mPlotItem->plotArea().top() ), QPointF( crossHairPlotPoint.x(), mPlotItem->plotArea().bottom() ) );
      painter->restore();
    }

  private:

    QRectF mRect;
    QgsPointXY mPoint;
    QgsElevationProfilePlotItem *mPlotItem = nullptr;
};
///@endcond PRIVATE


QgsElevationProfileCanvas::QgsElevationProfileCanvas( QWidget *parent )
  : QgsPlotCanvas( parent )
{
  mPlotItem = new QgsElevationProfilePlotItem( this );
  mCrossHairsItem = new QgsElevationProfileCrossHairsItem( this, mPlotItem );
  mCrossHairsItem->setZValue( 100 );
  mCrossHairsItem->hide();
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
  const double newCenterY = mPlotItem->yMinimum() + ( mPlotItem->plotArea().bottom() - y ) / mPlotItem->plotArea().height() * ( mPlotItem->yMaximum() - mPlotItem->yMinimum() );

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
  scalePlot( factor, factor );
}

void QgsElevationProfileCanvas::scalePlot( double xFactor, double yFactor )
{
  const double currentWidth = mPlotItem->xMaximum() - mPlotItem->xMinimum();
  const double currentHeight = mPlotItem->yMaximum() - mPlotItem->yMinimum();

  const double newWidth = currentWidth / xFactor;
  const double newHeight = currentHeight / yFactor;

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

void QgsElevationProfileCanvas::mouseMoveEvent( QMouseEvent *e )
{
  QgsPlotCanvas::mouseMoveEvent( e );
  if ( e->isAccepted() )
  {
    mCrossHairsItem->hide();
    return;
  }

  const QgsPointXY plotPoint = canvasPointToPlotPoint( e->pos() );
  if ( plotPoint.isEmpty() )
  {
    mCrossHairsItem->hide();
  }
  else
  {
    mCrossHairsItem->setPoint( plotPoint );
    mCrossHairsItem->show();
  }
}

QRectF QgsElevationProfileCanvas::plotArea() const
{
  return mPlotItem->plotArea();
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

QgsPointXY QgsElevationProfileCanvas::canvasPointToPlotPoint( const QPointF &point ) const
{
  if ( !mPlotItem->plotArea().contains( point.x(), point.y() ) )
    return QgsPointXY();

  return mPlotItem->canvasPointToPlotPoint( point );
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
  mCrossHairsItem->updateRect();
}

void QgsElevationProfileCanvas::paintEvent( QPaintEvent *event )
{
  QgsPlotCanvas::paintEvent( event );

  if ( !mFirstDrawOccurred )
  {
    // on first show we need to update the visible rect of the plot. (Not sure why this doesn't work in showEvent, but it doesn't).
    mFirstDrawOccurred = true;
    mPlotItem->updateRect();
    mCrossHairsItem->updateRect();
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

  if ( zRange.upper() < zRange.lower() )
  {
    // invalid range, e.g. no features found in plot!
    mPlotItem->setYMinimum( 0 );
    mPlotItem->setYMaximum( 10 );
  }
  else
  {
    // add 5% margin to height range
    const double margin = ( zRange.upper() - zRange.lower() ) * 0.05;
    mPlotItem->setYMinimum( zRange.lower() - margin );
    mPlotItem->setYMaximum( zRange.upper() + margin );
  }

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

const Qgs2DPlot &QgsElevationProfileCanvas::plot() const
{
  return *mPlotItem;
}

///@cond PRIVATE
class QgsElevationProfilePlot : public Qgs2DPlot
{
  public:

    QgsElevationProfilePlot( QgsProfilePlotRenderer *renderer )
      : mRenderer( renderer )
    {
    }

    void renderContent( QgsRenderContext &rc, const QRectF &plotArea ) override
    {
      if ( !mRenderer )
        return;

      rc.painter()->translate( plotArea.left(), plotArea.top() );
      mRenderer->render( rc, plotArea.width(), plotArea.height(), xMinimum(), xMaximum(), yMinimum(), yMaximum() );
      rc.painter()->translate( -plotArea.left(), -plotArea.top() );
    }

  private:

    QgsProfilePlotRenderer *mRenderer = nullptr;
};
///@endcond PRIVATE

void QgsElevationProfileCanvas::render( QgsRenderContext &context, double width, double height, const Qgs2DPlot &plotSettings )
{
  if ( !mCurrentJob )
    return;

  QgsElevationProfilePlot profilePlot( mCurrentJob );

  // quick and nasty way to transfer settings from another plot class -- in future we probably want to improve this, but let's let the API settle first...
  QDomDocument doc;
  QDomElement elem = doc.createElement( QStringLiteral( "plot" ) );
  QgsReadWriteContext rwContext;
  plotSettings.writeXml( elem, doc, rwContext );
  profilePlot.readXml( elem, rwContext );

  profilePlot.setSize( QSizeF( width, height ) );
  profilePlot.render( context );
}

void QgsElevationProfileCanvas::clear()
{
  setProfileCurve( nullptr );
  mPlotItem->setRenderer( nullptr );
  mPlotItem->updatePlot();
}
