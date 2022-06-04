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
#include "qgsnumericformat.h"
#include "qgsexpressioncontextutils.h"
#include "qgsprofilesnapping.h"
#include "qgsmaplayerelevationproperties.h"
#include "qgsapplication.h"

#include <QWheelEvent>
#include <QTimer>
#include <QDesktopWidget>

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
      mCachedImages.clear();
      mPlotArea = QRectF();
      update();
    }

    void updatePlot()
    {
      mImage = QImage();
      mCachedImages.clear();
      mPlotArea = QRectF();
      update();
    }

    bool redrawResults( const QString &sourceId )
    {
      auto it = mCachedImages.find( sourceId );
      if ( it == mCachedImages.end() )
        return false;

      mCachedImages.erase( it );
      mImage = QImage();
      return true;
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
      if ( !scene()->views().isEmpty() )
        context.setScaleFactor( scene()->views().at( 0 )->logicalDpiX() / 25.4 );

      calculateOptimisedIntervals( context );
      mPlotArea = interiorPlotArea( context );
      return mPlotArea;
    }

    QgsProfilePoint canvasPointToPlotPoint( QPointF point )
    {
      const QRectF area = plotArea();
      if ( !area.contains( point.x(), point.y() ) )
        return QgsProfilePoint();

      const double distance = ( point.x() - area.left() ) / area.width() * ( xMaximum() - xMinimum() ) + xMinimum();
      const double elevation = ( area.bottom() - point.y() ) / area.height() * ( yMaximum() - yMinimum() ) + yMinimum();
      return QgsProfilePoint( distance, elevation );
    }

    QgsPointXY plotPointToCanvasPoint( const QgsProfilePoint &point )
    {
      if ( point.distance() < xMinimum() || point.distance() > xMaximum() || point.elevation() < yMinimum() || point.elevation() > yMaximum() )
        return QgsPointXY();

      const QRectF area = plotArea();

      const double x = ( point.distance() - xMinimum() ) / ( xMaximum() - xMinimum() ) * ( area.width() ) + area.left();
      const double y = area.bottom() - ( point.elevation() - yMinimum() ) / ( yMaximum() - yMinimum() ) * ( area.height() );
      return QgsPointXY( x, y );
    }

    void renderContent( QgsRenderContext &rc, const QRectF &plotArea ) override
    {
      mPlotArea = plotArea;

      if ( !mRenderer )
        return;

      const QStringList sourceIds = mRenderer->sourceIds();
      for ( const QString &source : sourceIds )
      {
        QImage plot;
        auto it = mCachedImages.constFind( source );
        if ( it != mCachedImages.constEnd() )
        {
          plot = it.value();
        }
        else
        {
          plot = mRenderer->renderToImage( plotArea.width(), plotArea.height(), xMinimum(), xMaximum(), yMinimum(), yMaximum(), source );
          mCachedImages.insert( source, plot );
        }
        rc.painter()->drawImage( plotArea.left(), plotArea.top(), plot );
      }
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

        rc.expressionContext().appendScope( QgsExpressionContextUtils::globalScope() );
        rc.expressionContext().appendScope( QgsExpressionContextUtils::projectScope( mProject ) );

        calculateOptimisedIntervals( rc );
        render( rc );
        imagePainter.end();

        painter->drawImage( 0, 0, mImage );
      }
    }

    QgsProject *mProject = nullptr;

  private:

    QImage mImage;

    QMap< QString, QImage > mCachedImages;

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

    void setPoint( const QgsProfilePoint &point )
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
      crossHairPen.setWidthF( 1 );
      crossHairPen.setStyle( Qt::DashLine );
      crossHairPen.setCapStyle( Qt::FlatCap );
      crossHairPen.setColor( QColor( 0, 0, 0, 150 ) );
      painter->setPen( crossHairPen );
      painter->drawLine( QPointF( mPlotItem->plotArea().left(), crossHairPlotPoint.y() ), QPointF( mPlotItem->plotArea().right(), crossHairPlotPoint.y() ) );
      painter->drawLine( QPointF( crossHairPlotPoint.x(), mPlotItem->plotArea().top() ), QPointF( crossHairPlotPoint.x(), mPlotItem->plotArea().bottom() ) );

      // also render current point text
      QgsNumericFormatContext numericContext;

      const QString xCoordinateText = mPlotItem->xAxis().numericFormat()->formatDouble( mPoint.distance(), numericContext );
      const QString yCoordinateText = mPlotItem->yAxis().numericFormat()->formatDouble( mPoint.elevation(), numericContext );

      QFont font;
      const QFontMetrics fm( font );
      const double height = fm.capHeight();
      const double xWidth = fm.horizontalAdvance( xCoordinateText );
      const double yWidth = fm.horizontalAdvance( yCoordinateText );
      const double textAxisMargin = fm.horizontalAdvance( ' ' );

      QPointF xCoordOrigin;
      QPointF yCoordOrigin;

      if ( mPoint.distance() < ( mPlotItem->xMaximum() + mPlotItem->xMinimum() ) * 0.5 )
      {
        if ( mPoint.elevation() < ( mPlotItem->yMaximum() + mPlotItem->yMinimum() ) * 0.5 )
        {
          // render x coordinate on right top (left top align)
          xCoordOrigin = QPointF( crossHairPlotPoint.x() + textAxisMargin, mPlotItem->plotArea().top() + height + textAxisMargin );
          // render y coordinate on right top (right bottom align)
          yCoordOrigin = QPointF( mPlotItem->plotArea().right() - yWidth - textAxisMargin, crossHairPlotPoint.y() - textAxisMargin );
        }
        else
        {
          // render x coordinate on right bottom (left bottom align)
          xCoordOrigin = QPointF( crossHairPlotPoint.x() + textAxisMargin, mPlotItem->plotArea().bottom() - textAxisMargin );
          // render y coordinate on right bottom (right top align)
          yCoordOrigin = QPointF( mPlotItem->plotArea().right() - yWidth - textAxisMargin, crossHairPlotPoint.y() + height + textAxisMargin );
        }
      }
      else
      {
        if ( mPoint.elevation() < ( mPlotItem->yMaximum() + mPlotItem->yMinimum() ) * 0.5 )
        {
          // render x coordinate on left top (right top align)
          xCoordOrigin = QPointF( crossHairPlotPoint.x() - xWidth - textAxisMargin, mPlotItem->plotArea().top() + height + textAxisMargin );
          // render y coordinate on left top (left bottom align)
          yCoordOrigin = QPointF( mPlotItem->plotArea().left() + textAxisMargin, crossHairPlotPoint.y() - textAxisMargin );
        }
        else
        {
          // render x coordinate on left bottom (right bottom align)
          xCoordOrigin = QPointF( crossHairPlotPoint.x() - xWidth - textAxisMargin, mPlotItem->plotArea().bottom() - textAxisMargin );
          // render y coordinate on left bottom (left top align)
          yCoordOrigin = QPointF( mPlotItem->plotArea().left() + textAxisMargin, crossHairPlotPoint.y() + height + textAxisMargin );
        }
      }

      // semi opaque white background
      painter->setBrush( QBrush( QColor( 255, 255, 255, 220 ) ) );
      painter->setPen( Qt::NoPen );
      painter->drawRect( QRectF( xCoordOrigin.x() - textAxisMargin + 1, xCoordOrigin.y() - textAxisMargin - height + 1, xWidth + 2 * textAxisMargin - 2, height + 2 * textAxisMargin - 2 ) );
      painter->drawRect( QRectF( yCoordOrigin.x() - textAxisMargin + 1, yCoordOrigin.y() - textAxisMargin - height + 1, yWidth + 2 * textAxisMargin - 2, height + 2 * textAxisMargin - 2 ) );

      painter->setBrush( Qt::NoBrush );
      painter->setPen( Qt::black );

      painter->drawText( xCoordOrigin, xCoordinateText );
      painter->drawText( yCoordOrigin, yCoordinateText );
      painter->restore();
    }

  private:

    QRectF mRect;
    QgsProfilePoint mPoint;
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

  // updating the profile plot is deferred on a timer, so that we don't trigger it too often
  mDeferredRegenerationTimer = new QTimer( this );
  mDeferredRegenerationTimer->setSingleShot( true );
  mDeferredRegenerationTimer->stop();
  connect( mDeferredRegenerationTimer, &QTimer::timeout, this, &QgsElevationProfileCanvas::startDeferredRegeneration );

  mDeferredRedrawTimer = new QTimer( this );
  mDeferredRedrawTimer->setSingleShot( true );
  mDeferredRedrawTimer->stop();
  connect( mDeferredRedrawTimer, &QTimer::timeout, this, &QgsElevationProfileCanvas::startDeferredRedraw );

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

  refineResults();

  mPlotItem->updatePlot();
  emit plotAreaChanged();
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

  refineResults();

  mPlotItem->updatePlot();
  emit plotAreaChanged();
}

void QgsElevationProfileCanvas::scalePlot( double factor )
{
  scalePlot( factor, factor );
  emit plotAreaChanged();
}

QgsProfileSnapContext QgsElevationProfileCanvas::snapContext() const
{
  const double toleranceInPixels = QFontMetrics( font() ).horizontalAdvance( ' ' );
  const double xToleranceInPlotUnits = ( mPlotItem->xMaximum() - mPlotItem->xMinimum() ) / ( mPlotItem->plotArea().width() ) * toleranceInPixels;
  const double yToleranceInPlotUnits = ( mPlotItem->yMaximum() - mPlotItem->yMinimum() ) / ( mPlotItem->plotArea().height() ) * toleranceInPixels;

  QgsProfileSnapContext context;
  context.maximumSurfaceDistanceDelta = 2 * xToleranceInPlotUnits;
  context.maximumSurfaceElevationDelta = 10 * yToleranceInPlotUnits;
  context.maximumPointDistanceDelta = 4 * xToleranceInPlotUnits;
  context.maximumPointElevationDelta = 4 * yToleranceInPlotUnits;
  context.displayRatioElevationVsDistance = ( ( mPlotItem->yMaximum() - mPlotItem->yMinimum() ) / ( mPlotItem->plotArea().height() ) )
      / ( ( mPlotItem->xMaximum() - mPlotItem->xMinimum() ) / ( mPlotItem->plotArea().width() ) );

  return context;
}

QgsProfileIdentifyContext QgsElevationProfileCanvas::identifyContext() const
{
  const double toleranceInPixels = QFontMetrics( font() ).horizontalAdvance( ' ' );
  const double xToleranceInPlotUnits = ( mPlotItem->xMaximum() - mPlotItem->xMinimum() ) / ( mPlotItem->plotArea().width() ) * toleranceInPixels;
  const double yToleranceInPlotUnits = ( mPlotItem->yMaximum() - mPlotItem->yMinimum() ) / ( mPlotItem->plotArea().height() ) * toleranceInPixels;

  QgsProfileIdentifyContext context;
  context.maximumSurfaceDistanceDelta = 2 * xToleranceInPlotUnits;
  context.maximumSurfaceElevationDelta = 10 * yToleranceInPlotUnits;
  context.maximumPointDistanceDelta = 4 * xToleranceInPlotUnits;
  context.maximumPointElevationDelta = 4 * yToleranceInPlotUnits;
  context.displayRatioElevationVsDistance = ( ( mPlotItem->yMaximum() - mPlotItem->yMinimum() ) / ( mPlotItem->plotArea().height() ) )
      / ( ( mPlotItem->xMaximum() - mPlotItem->xMinimum() ) / ( mPlotItem->plotArea().width() ) );

  context.project = mProject;

  return context;
}

void QgsElevationProfileCanvas::setupLayerConnections( QgsMapLayer *layer, bool isDisconnect )
{
  if ( isDisconnect )
  {
    disconnect( layer->elevationProperties(), &QgsMapLayerElevationProperties::profileGenerationPropertyChanged, this, &QgsElevationProfileCanvas::onLayerProfileGenerationPropertyChanged );
    disconnect( layer->elevationProperties(), &QgsMapLayerElevationProperties::profileRenderingPropertyChanged, this, &QgsElevationProfileCanvas::onLayerProfileRendererPropertyChanged );
    disconnect( layer, &QgsMapLayer::dataChanged, this, &QgsElevationProfileCanvas::regenerateResultsForLayer );
  }
  else
  {
    connect( layer->elevationProperties(), &QgsMapLayerElevationProperties::profileGenerationPropertyChanged, this, &QgsElevationProfileCanvas::onLayerProfileGenerationPropertyChanged );
    connect( layer->elevationProperties(), &QgsMapLayerElevationProperties::profileRenderingPropertyChanged, this, &QgsElevationProfileCanvas::onLayerProfileRendererPropertyChanged );
    connect( layer, &QgsMapLayer::dataChanged, this, &QgsElevationProfileCanvas::regenerateResultsForLayer );
  }

  switch ( layer->type() )
  {
    case QgsMapLayerType::VectorLayer:
    {
      QgsVectorLayer *vl = qobject_cast< QgsVectorLayer * >( layer );
      if ( isDisconnect )
      {
        disconnect( vl, &QgsVectorLayer::featureAdded, this, &QgsElevationProfileCanvas::regenerateResultsForLayer );
        disconnect( vl, &QgsVectorLayer::featureDeleted, this, &QgsElevationProfileCanvas::regenerateResultsForLayer );
        disconnect( vl, &QgsVectorLayer::geometryChanged, this, &QgsElevationProfileCanvas::regenerateResultsForLayer );
        disconnect( vl, &QgsVectorLayer::attributeValueChanged, this, &QgsElevationProfileCanvas::regenerateResultsForLayer );
      }
      else
      {
        connect( vl, &QgsVectorLayer::featureAdded, this, &QgsElevationProfileCanvas::regenerateResultsForLayer );
        connect( vl, &QgsVectorLayer::featureDeleted, this, &QgsElevationProfileCanvas::regenerateResultsForLayer );
        connect( vl, &QgsVectorLayer::geometryChanged, this, &QgsElevationProfileCanvas::regenerateResultsForLayer );
        connect( vl, &QgsVectorLayer::attributeValueChanged, this, &QgsElevationProfileCanvas::regenerateResultsForLayer );
      }
      break;
    }
    case QgsMapLayerType::RasterLayer:
    case QgsMapLayerType::PluginLayer:
    case QgsMapLayerType::MeshLayer:
    case QgsMapLayerType::VectorTileLayer:
    case QgsMapLayerType::AnnotationLayer:
    case QgsMapLayerType::PointCloudLayer:
    case QgsMapLayerType::GroupLayer:
      break;
  }
}

QgsPointXY QgsElevationProfileCanvas::snapToPlot( QPoint point )
{
  if ( !mCurrentJob || !mSnappingEnabled )
    return QgsPointXY();

  const QgsProfilePoint plotPoint = canvasPointToPlotPoint( point );

  const QgsProfileSnapResult snappedPoint = mCurrentJob->snapPoint( plotPoint, snapContext() );
  if ( !snappedPoint.isValid() )
    return QgsPointXY();

  return plotPointToCanvasPoint( snappedPoint.snappedPoint );
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

  refineResults();
  mPlotItem->updatePlot();
  emit plotAreaChanged();
}

void QgsElevationProfileCanvas::zoomToRect( const QRectF &rect )
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

  refineResults();
  mPlotItem->updatePlot();
  emit plotAreaChanged();
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
  emit plotAreaChanged();
}

void QgsElevationProfileCanvas::mouseMoveEvent( QMouseEvent *e )
{
  QgsPlotCanvas::mouseMoveEvent( e );
  if ( e->isAccepted() )
  {
    mCrossHairsItem->hide();
    return;
  }

  QgsProfilePoint plotPoint = canvasPointToPlotPoint( e->pos() );
  if ( mCurrentJob && mSnappingEnabled && !plotPoint.isEmpty() )
  {
    const QgsProfileSnapResult snapResult = mCurrentJob->snapPoint( plotPoint, snapContext() );
    if ( snapResult.isValid() )
      plotPoint = snapResult.snappedPoint;
  }

  if ( plotPoint.isEmpty() )
  {
    mCrossHairsItem->hide();
  }
  else
  {
    mCrossHairsItem->setPoint( plotPoint );
    mCrossHairsItem->show();
  }
  emit canvasPointHovered( e->pos(), plotPoint );
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
  request.setTolerance( mTolerance );
  request.setTransformContext( mProject->transformContext() );
  request.setTerrainProvider( mProject->elevationProperties()->terrainProvider() ? mProject->elevationProperties()->terrainProvider()->clone() : nullptr );
  QgsExpressionContext context;
  context.appendScope( QgsExpressionContextUtils::globalScope() );
  context.appendScope( QgsExpressionContextUtils::projectScope( mProject ) );
  request.setExpressionContext( context );

  const QList< QgsMapLayer * > layersToGenerate = layers();
  QList< QgsAbstractProfileSource * > sources;
  sources.reserve( layersToGenerate .size() );
  for ( QgsMapLayer *layer : layersToGenerate )
  {
    if ( QgsAbstractProfileSource *source = dynamic_cast< QgsAbstractProfileSource * >( layer ) )
      sources.append( source );
  }

  mCurrentJob = new QgsProfilePlotRenderer( sources, request );
  connect( mCurrentJob, &QgsProfilePlotRenderer::generationFinished, this, &QgsElevationProfileCanvas::generationFinished );

  QgsProfileGenerationContext generationContext;
  generationContext.setDpi( QgsApplication::desktop()->logicalDpiX() );
  generationContext.setMaximumErrorMapUnits( MAX_ERROR_PIXELS * ( mProfileCurve->length() ) / mPlotItem->plotArea().width() );
  generationContext.setMapUnitsPerDistancePixel( mProfileCurve->length() / mPlotItem->plotArea().width() );
  mCurrentJob->setContext( generationContext );

  mCurrentJob->startGeneration();
  mPlotItem->setRenderer( mCurrentJob );

  emit activeJobCountChanged( 1 );
}

void QgsElevationProfileCanvas::invalidateCurrentPlotExtent()
{
  mZoomFullWhenJobFinished = true;
}

void QgsElevationProfileCanvas::generationFinished()
{
  if ( !mCurrentJob )
    return;

  emit activeJobCountChanged( 0 );

  if ( mZoomFullWhenJobFinished )
  {
    // we only zoom full for the initial generation
    mZoomFullWhenJobFinished = false;
    zoomFull();
  }
  else
  {
    // here we should invalidate cached results only for the layers which have been refined

    // and if no layers are being refeined, don't invalidate anything

    mPlotItem->updatePlot();
  }

  if ( mForceRegenerationAfterCurrentJobCompletes )
  {
    mForceRegenerationAfterCurrentJobCompletes = false;
    mCurrentJob->invalidateAllRefinableSources();
    scheduleDeferredRegeneration();
  }
}

void QgsElevationProfileCanvas::onLayerProfileGenerationPropertyChanged()
{
  // TODO -- handle nicely when existing job is in progress
  if ( !mCurrentJob || mCurrentJob->isActive() )
    return;

  QgsMapLayerElevationProperties *properties = qobject_cast< QgsMapLayerElevationProperties * >( sender() );
  if ( !properties )
    return;

  if ( QgsMapLayer *layer = qobject_cast< QgsMapLayer * >( properties->parent() ) )
  {
    if ( QgsAbstractProfileSource *source = dynamic_cast< QgsAbstractProfileSource * >( layer ) )
    {
      if ( mCurrentJob->invalidateResults( source ) )
        scheduleDeferredRegeneration();
    }
  }
}

void QgsElevationProfileCanvas::onLayerProfileRendererPropertyChanged()
{
  // TODO -- handle nicely when existing job is in progress
  if ( !mCurrentJob || mCurrentJob->isActive() )
    return;

  QgsMapLayerElevationProperties *properties = qobject_cast< QgsMapLayerElevationProperties * >( sender() );
  if ( !properties )
    return;

  if ( QgsMapLayer *layer = qobject_cast< QgsMapLayer * >( properties->parent() ) )
  {
    if ( QgsAbstractProfileSource *source = dynamic_cast< QgsAbstractProfileSource * >( layer ) )
    {
      mCurrentJob->replaceSource( source );
    }
    if ( mPlotItem->redrawResults( layer->id() ) )
      scheduleDeferredRedraw();
  }
}

void QgsElevationProfileCanvas::regenerateResultsForLayer()
{
  if ( QgsMapLayer *layer = qobject_cast< QgsMapLayer * >( sender() ) )
  {
    if ( QgsAbstractProfileSource *source = dynamic_cast< QgsAbstractProfileSource * >( layer ) )
    {
      if ( mCurrentJob->invalidateResults( source ) )
        scheduleDeferredRegeneration();
    }
  }
}

void QgsElevationProfileCanvas::scheduleDeferredRegeneration()
{
  if ( !mDeferredRegenerationScheduled )
  {
    mDeferredRegenerationTimer->start( 1 );
    mDeferredRegenerationScheduled = true;
  }
}

void QgsElevationProfileCanvas::scheduleDeferredRedraw()
{
  if ( !mDeferredRedrawScheduled )
  {
    mDeferredRedrawTimer->start( 1 );
    mDeferredRedrawScheduled = true;
  }
}

void QgsElevationProfileCanvas::startDeferredRegeneration()
{
  if ( mCurrentJob && !mCurrentJob->isActive() )
  {
    emit activeJobCountChanged( 1 );
    mCurrentJob->regenerateInvalidatedResults();
  }
  else if ( mCurrentJob )
  {
    mForceRegenerationAfterCurrentJobCompletes = true;
  }

  mDeferredRegenerationScheduled = false;
}

void QgsElevationProfileCanvas::startDeferredRedraw()
{
  mPlotItem->update();
  mDeferredRedrawScheduled = false;
}

void QgsElevationProfileCanvas::refineResults()
{
  if ( mCurrentJob )
  {
    QgsProfileGenerationContext context;
    context.setDpi( QgsApplication::desktop()->logicalDpiX() );
    const double plotDistanceRange = mPlotItem->xMaximum() - mPlotItem->xMinimum();
    const double plotElevationRange = mPlotItem->yMaximum() - mPlotItem->yMinimum();
    const double plotDistanceUnitsPerPixel = plotDistanceRange / mPlotItem->plotArea().width();

    // we round the actual desired map error down to just one significant figure, to avoid tiny differences
    // as the plot is panned
    const double targetMaxErrorInMapUnits = MAX_ERROR_PIXELS * plotDistanceUnitsPerPixel;
    const double factor = std::pow( 10.0, 1 - std::ceil( std::log10( std::fabs( targetMaxErrorInMapUnits ) ) ) );
    const double roundedErrorInMapUnits = std::floor( targetMaxErrorInMapUnits * factor ) / factor;
    context.setMaximumErrorMapUnits( roundedErrorInMapUnits );

    context.setMapUnitsPerDistancePixel( plotDistanceUnitsPerPixel );

    // for similar reasons we round the minimum distance off to multiples of the maximum error in map units
    const double distanceMin = std::floor( ( mPlotItem->xMinimum() - plotDistanceRange * 0.05 ) / context.maximumErrorMapUnits() ) * context.maximumErrorMapUnits();
    context.setDistanceRange( QgsDoubleRange( std::max( 0.0, distanceMin ),
                              mPlotItem->xMaximum() + plotDistanceRange * 0.05 ) );

    context.setElevationRange( QgsDoubleRange( mPlotItem->yMinimum() - plotElevationRange * 0.05,
                               mPlotItem->yMaximum() + plotElevationRange * 0.05 ) );
    mCurrentJob->setContext( context );
  }
  scheduleDeferredRegeneration();
}

QgsProfilePoint QgsElevationProfileCanvas::canvasPointToPlotPoint( QPointF point ) const
{
  if ( !mPlotItem->plotArea().contains( point.x(), point.y() ) )
    return QgsProfilePoint();

  return mPlotItem->canvasPointToPlotPoint( point );
}

QgsPointXY QgsElevationProfileCanvas::plotPointToCanvasPoint( const QgsProfilePoint &point ) const
{
  return mPlotItem->plotPointToCanvasPoint( point );
}

void QgsElevationProfileCanvas::setProject( QgsProject *project )
{
  mProject = project;
  mPlotItem->mProject = project;
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

void QgsElevationProfileCanvas::setTolerance( double tolerance )
{
  mTolerance = tolerance;
}

QgsCoordinateReferenceSystem QgsElevationProfileCanvas::crs() const
{
  return mCrs;
}

void QgsElevationProfileCanvas::setLayers( const QList<QgsMapLayer *> &layers )
{
  for ( QgsMapLayer *layer : std::as_const( mLayers ) )
  {
    setupLayerConnections( layer, true );
  }

  // filter list, removing null layers and invalid layers
  auto filteredList = layers;
  filteredList.erase( std::remove_if( filteredList.begin(), filteredList.end(),
                                      []( QgsMapLayer * layer )
  {
    return !layer || !layer->isValid();
  } ), filteredList.end() );

  mLayers = _qgis_listRawToQPointer( filteredList );
  for ( QgsMapLayer *layer : std::as_const( mLayers ) )
  {
    setupLayerConnections( layer, false );
  }
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
  else if ( qgsDoubleNear( zRange.lower(), zRange.upper(), 0.0000001 ) )
  {
    // corner case ... a zero height plot! Just pick an arbitrary +/- 5 height range.
    mPlotItem->setYMinimum( zRange.lower() - 5 );
    mPlotItem->setYMaximum( zRange.lower() + 5 );
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

  refineResults();
  mPlotItem->updatePlot();
  emit plotAreaChanged();
}

void QgsElevationProfileCanvas::setVisiblePlotRange( double minimumDistance, double maximumDistance, double minimumElevation, double maximumElevation )
{
  mPlotItem->setYMinimum( minimumElevation );
  mPlotItem->setYMaximum( maximumElevation );
  mPlotItem->setXMinimum( minimumDistance );
  mPlotItem->setXMaximum( maximumDistance );
  refineResults();
  mPlotItem->updatePlot();
  emit plotAreaChanged();
}

QgsDoubleRange QgsElevationProfileCanvas::visibleDistanceRange() const
{
  return QgsDoubleRange( mPlotItem->xMinimum(), mPlotItem->xMaximum() );
}

QgsDoubleRange QgsElevationProfileCanvas::visibleElevationRange() const
{
  return QgsDoubleRange( mPlotItem->yMinimum(), mPlotItem->yMaximum() );
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

  context.expressionContext().appendScope( QgsExpressionContextUtils::globalScope() );
  context.expressionContext().appendScope( QgsExpressionContextUtils::projectScope( mProject ) );

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

QVector<QgsProfileIdentifyResults> QgsElevationProfileCanvas::identify( QPointF point )
{
  if ( !mCurrentJob )
    return {};

  const QgsProfilePoint plotPoint = canvasPointToPlotPoint( point );

  return mCurrentJob->identify( plotPoint, identifyContext() );
}

QVector<QgsProfileIdentifyResults> QgsElevationProfileCanvas::identify( const QRectF &rect )
{
  if ( !mCurrentJob )
    return {};

  const QgsProfilePoint topLeftPlotPoint = canvasPointToPlotPoint( rect.topLeft() );
  const QgsProfilePoint bottomRightPlotPoint = canvasPointToPlotPoint( rect.bottomRight() );

  double distance1 = topLeftPlotPoint.distance();
  double distance2 = bottomRightPlotPoint.distance();
  if ( distance2 < distance1 )
    std::swap( distance1, distance2 );

  double elevation1 = topLeftPlotPoint.elevation();
  double elevation2 = bottomRightPlotPoint.elevation();
  if ( elevation2 < elevation1 )
    std::swap( elevation1, elevation2 );

  return mCurrentJob->identify( QgsDoubleRange( distance1, distance2 ), QgsDoubleRange( elevation1, elevation2 ), identifyContext() );
}

void QgsElevationProfileCanvas::clear()
{
  setProfileCurve( nullptr );
  mPlotItem->setRenderer( nullptr );
  mPlotItem->updatePlot();
}

void QgsElevationProfileCanvas::setSnappingEnabled( bool enabled )
{
  mSnappingEnabled = enabled;
}
