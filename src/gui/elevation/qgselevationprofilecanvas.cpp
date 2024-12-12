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

#include "qgsapplication.h"
#include "qgselevationprofilecanvas.h"
#include "moc_qgselevationprofilecanvas.cpp"
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
#include "qgsnumericformat.h"
#include "qgsexpressioncontextutils.h"
#include "qgsprofilesnapping.h"
#include "qgsmaplayerelevationproperties.h"
#include "qgsscreenhelper.h"
#include "qgsfillsymbol.h"
#include "qgslinesymbol.h"
#include "qgsprofilesourceregistry.h"

#include <QWheelEvent>
#include <QTimer>
#include <QPalette>

///@cond PRIVATE
class QgsElevationProfilePlotItem : public Qgs2DPlot, public QgsPlotCanvasItem
{
  public:
    QgsElevationProfilePlotItem( QgsElevationProfileCanvas *canvas )
      : QgsPlotCanvasItem( canvas )
    {
      setYMinimum( 0 );
      setYMaximum( 100 );

      xAxis().setLabelSuffixPlacement( Qgis::PlotAxisSuffixPlacement::FirstAndLastLabels );
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

    QString distanceSuffix() const
    {
      switch ( mDistanceUnit )
      {
        case Qgis::DistanceUnit::Meters:
        case Qgis::DistanceUnit::Kilometers:
        case Qgis::DistanceUnit::Feet:
        case Qgis::DistanceUnit::NauticalMiles:
        case Qgis::DistanceUnit::Yards:
        case Qgis::DistanceUnit::Miles:
        case Qgis::DistanceUnit::Centimeters:
        case Qgis::DistanceUnit::Millimeters:
        case Qgis::DistanceUnit::Inches:
        case Qgis::DistanceUnit::ChainsInternational:
        case Qgis::DistanceUnit::ChainsBritishBenoit1895A:
        case Qgis::DistanceUnit::ChainsBritishBenoit1895B:
        case Qgis::DistanceUnit::ChainsBritishSears1922Truncated:
        case Qgis::DistanceUnit::ChainsBritishSears1922:
        case Qgis::DistanceUnit::ChainsClarkes:
        case Qgis::DistanceUnit::ChainsUSSurvey:
        case Qgis::DistanceUnit::FeetBritish1865:
        case Qgis::DistanceUnit::FeetBritish1936:
        case Qgis::DistanceUnit::FeetBritishBenoit1895A:
        case Qgis::DistanceUnit::FeetBritishBenoit1895B:
        case Qgis::DistanceUnit::FeetBritishSears1922Truncated:
        case Qgis::DistanceUnit::FeetBritishSears1922:
        case Qgis::DistanceUnit::FeetClarkes:
        case Qgis::DistanceUnit::FeetGoldCoast:
        case Qgis::DistanceUnit::FeetIndian:
        case Qgis::DistanceUnit::FeetIndian1937:
        case Qgis::DistanceUnit::FeetIndian1962:
        case Qgis::DistanceUnit::FeetIndian1975:
        case Qgis::DistanceUnit::FeetUSSurvey:
        case Qgis::DistanceUnit::LinksInternational:
        case Qgis::DistanceUnit::LinksBritishBenoit1895A:
        case Qgis::DistanceUnit::LinksBritishBenoit1895B:
        case Qgis::DistanceUnit::LinksBritishSears1922Truncated:
        case Qgis::DistanceUnit::LinksBritishSears1922:
        case Qgis::DistanceUnit::LinksClarkes:
        case Qgis::DistanceUnit::LinksUSSurvey:
        case Qgis::DistanceUnit::YardsBritishBenoit1895A:
        case Qgis::DistanceUnit::YardsBritishBenoit1895B:
        case Qgis::DistanceUnit::YardsBritishSears1922Truncated:
        case Qgis::DistanceUnit::YardsBritishSears1922:
        case Qgis::DistanceUnit::YardsClarkes:
        case Qgis::DistanceUnit::YardsIndian:
        case Qgis::DistanceUnit::YardsIndian1937:
        case Qgis::DistanceUnit::YardsIndian1962:
        case Qgis::DistanceUnit::YardsIndian1975:
        case Qgis::DistanceUnit::MilesUSSurvey:
        case Qgis::DistanceUnit::Fathoms:
        case Qgis::DistanceUnit::MetersGermanLegal:
          return QStringLiteral( " %1" ).arg( QgsUnitTypes::toAbbreviatedString( mDistanceUnit ) );

        case Qgis::DistanceUnit::Degrees:
          return QObject::tr( "°" );
        case Qgis::DistanceUnit::Unknown:
          return QString();
      }
      BUILTIN_UNREACHABLE
    }

    void setXAxisUnits( Qgis::DistanceUnit unit )
    {
      mDistanceUnit = unit;
      xAxis().setLabelSuffix( distanceSuffix() );
      update();
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

      const double distance = ( point.x() - area.left() ) / area.width() * ( xMaximum() - xMinimum() ) * mXScaleFactor + xMinimum() * mXScaleFactor;
      const double elevation = ( area.bottom() - point.y() ) / area.height() * ( yMaximum() - yMinimum() ) + yMinimum();
      return QgsProfilePoint( distance, elevation );
    }

    QgsPointXY plotPointToCanvasPoint( const QgsProfilePoint &point )
    {
      if ( point.distance() < xMinimum() * mXScaleFactor || point.distance() > xMaximum() * mXScaleFactor || point.elevation() < yMinimum() || point.elevation() > yMaximum() )
        return QgsPointXY();

      const QRectF area = plotArea();

      const double x = ( point.distance() - xMinimum() * mXScaleFactor ) / ( ( xMaximum() - xMinimum() ) * mXScaleFactor ) * ( area.width() ) + area.left();
      const double y = area.bottom() - ( point.elevation() - yMinimum() ) / ( yMaximum() - yMinimum() ) * ( area.height() );
      return QgsPointXY( x, y );
    }

    void renderContent( QgsRenderContext &rc, const QRectF &plotArea ) override
    {
      mPlotArea = plotArea;

      if ( !mRenderer )
        return;

      const double pixelRatio = !scene()->views().empty() ? scene()->views().at( 0 )->devicePixelRatioF() : 1;

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
          plot = mRenderer->renderToImage( plotArea.width() * pixelRatio, plotArea.height() * pixelRatio, xMinimum() * mXScaleFactor, xMaximum() * mXScaleFactor, yMinimum(), yMaximum(), source, pixelRatio );
          plot.setDevicePixelRatio( pixelRatio );
          mCachedImages.insert( source, plot );
        }
        rc.painter()->drawImage( QPointF( plotArea.left(), plotArea.top() ), plot );
      }
    }

    void paint( QPainter *painter ) override
    {
      // cache rendering to an image, so we don't need to redraw the plot
      if ( !mImage.isNull() )
      {
        painter->drawImage( QPointF( 0, 0 ), mImage );
      }
      else
      {
        const double pixelRatio = !scene()->views().empty() ? scene()->views().at( 0 )->devicePixelRatioF() : 1;
        mImage = QImage( mRect.width() * pixelRatio, mRect.height() * pixelRatio, QImage::Format_ARGB32_Premultiplied );
        mImage.setDevicePixelRatio( pixelRatio );
        mImage.fill( Qt::transparent );

        QPainter imagePainter( &mImage );
        imagePainter.setRenderHint( QPainter::Antialiasing, true );
        QgsRenderContext rc = QgsRenderContext::fromQPainter( &imagePainter );
        rc.setDevicePixelRatio( pixelRatio );

        const double mapUnitsPerPixel = ( xMaximum() - xMinimum() ) * mXScaleFactor / plotArea().width();
        rc.setMapToPixel( QgsMapToPixel( mapUnitsPerPixel ) );

        rc.expressionContext().appendScope( QgsExpressionContextUtils::globalScope() );
        rc.expressionContext().appendScope( QgsExpressionContextUtils::projectScope( mProject ) );

        calculateOptimisedIntervals( rc );
        render( rc );
        imagePainter.end();

        painter->drawImage( QPointF( 0, 0 ), mImage );
      }
    }

    QgsProject *mProject = nullptr;
    double mXScaleFactor = 1.0;

    Qgis::DistanceUnit mDistanceUnit = Qgis::DistanceUnit::Unknown;

  private:
    QImage mImage;

    QMap<QString, QImage> mCachedImages;

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
      const QgsPointXY crossHairPlotPoint = mPlotItem->plotPointToCanvasPoint( mPoint );
      if ( crossHairPlotPoint.isEmpty() )
        return;

      painter->save();
      painter->setBrush( Qt::NoBrush );
      QPen crossHairPen;
      crossHairPen.setCosmetic( true );
      crossHairPen.setWidthF( 1 );
      crossHairPen.setStyle( Qt::DashLine );
      crossHairPen.setCapStyle( Qt::FlatCap );
      const QPalette scenePalette = mPlotItem->scene()->palette();
      QColor penColor = scenePalette.color( QPalette::ColorGroup::Active, QPalette::Text );
      penColor.setAlpha( 150 );
      crossHairPen.setColor( penColor );
      painter->setPen( crossHairPen );
      painter->drawLine( QPointF( mPlotItem->plotArea().left(), crossHairPlotPoint.y() ), QPointF( mPlotItem->plotArea().right(), crossHairPlotPoint.y() ) );
      painter->drawLine( QPointF( crossHairPlotPoint.x(), mPlotItem->plotArea().top() ), QPointF( crossHairPlotPoint.x(), mPlotItem->plotArea().bottom() ) );

      // also render current point text
      QgsNumericFormatContext numericContext;

      const QString xCoordinateText = mPlotItem->xAxis().numericFormat()->formatDouble( mPoint.distance() / mPlotItem->mXScaleFactor, numericContext )
                                      + mPlotItem->distanceSuffix();

      const QString yCoordinateText = mPlotItem->yAxis().numericFormat()->formatDouble( mPoint.elevation(), numericContext );

      QFont font;
      const QFontMetrics fm( font );
      const double height = fm.capHeight();
      const double xWidth = fm.horizontalAdvance( xCoordinateText );
      const double yWidth = fm.horizontalAdvance( yCoordinateText );
      const double textAxisMargin = fm.horizontalAdvance( ' ' );

      QPointF xCoordOrigin;
      QPointF yCoordOrigin;

      if ( mPoint.distance() < ( mPlotItem->xMaximum() + mPlotItem->xMinimum() ) * 0.5 * mPlotItem->mXScaleFactor )
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

      // semi opaque background color brush
      QColor backgroundColor = mPlotItem->chartBackgroundSymbol()->color();
      backgroundColor.setAlpha( 220 );
      painter->setBrush( QBrush( backgroundColor ) );
      painter->setPen( Qt::NoPen );
      painter->drawRect( QRectF( xCoordOrigin.x() - textAxisMargin + 1, xCoordOrigin.y() - textAxisMargin - height + 1, xWidth + 2 * textAxisMargin - 2, height + 2 * textAxisMargin - 2 ) );
      painter->drawRect( QRectF( yCoordOrigin.x() - textAxisMargin + 1, yCoordOrigin.y() - textAxisMargin - height + 1, yWidth + 2 * textAxisMargin - 2, height + 2 * textAxisMargin - 2 ) );

      painter->setBrush( Qt::NoBrush );
      painter->setPen( scenePalette.color( QPalette::ColorGroup::Active, QPalette::Text ) );

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
  mScreenHelper = new QgsScreenHelper( this );

  mPlotItem = new QgsElevationProfilePlotItem( this );

  // follow system color scheme by default
  setBackgroundColor( QColor() );

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
  const double dxPlot = -dxPercent * ( mPlotItem->xMaximum() - mPlotItem->xMinimum() );
  const double dyPlot = dyPercent * ( mPlotItem->yMaximum() - mPlotItem->yMinimum() );

  // no need to handle axis scale lock here, we aren't changing scales
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

  // no need to handle axis scale lock here, we aren't changing scales
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
  const double xToleranceInPlotUnits = ( mPlotItem->xMaximum() - mPlotItem->xMinimum() ) * mPlotItem->mXScaleFactor / ( mPlotItem->plotArea().width() ) * toleranceInPixels;
  const double yToleranceInPlotUnits = ( mPlotItem->yMaximum() - mPlotItem->yMinimum() ) / ( mPlotItem->plotArea().height() ) * toleranceInPixels;

  QgsProfileSnapContext context;
  context.maximumSurfaceDistanceDelta = 2 * xToleranceInPlotUnits;
  context.maximumSurfaceElevationDelta = 10 * yToleranceInPlotUnits;
  context.maximumPointDistanceDelta = 4 * xToleranceInPlotUnits;
  context.maximumPointElevationDelta = 4 * yToleranceInPlotUnits;
  context.displayRatioElevationVsDistance = ( ( mPlotItem->yMaximum() - mPlotItem->yMinimum() ) / ( mPlotItem->plotArea().height() ) )
                                            / ( ( mPlotItem->xMaximum() - mPlotItem->xMinimum() ) * mPlotItem->mXScaleFactor / ( mPlotItem->plotArea().width() ) );

  return context;
}

QgsProfileIdentifyContext QgsElevationProfileCanvas::identifyContext() const
{
  const double toleranceInPixels = QFontMetrics( font() ).horizontalAdvance( ' ' );
  const double xToleranceInPlotUnits = ( mPlotItem->xMaximum() - mPlotItem->xMinimum() ) * mPlotItem->mXScaleFactor / ( mPlotItem->plotArea().width() ) * toleranceInPixels;
  const double yToleranceInPlotUnits = ( mPlotItem->yMaximum() - mPlotItem->yMinimum() ) / ( mPlotItem->plotArea().height() ) * toleranceInPixels;

  QgsProfileIdentifyContext context;
  context.maximumSurfaceDistanceDelta = 2 * xToleranceInPlotUnits;
  context.maximumSurfaceElevationDelta = 10 * yToleranceInPlotUnits;
  context.maximumPointDistanceDelta = 4 * xToleranceInPlotUnits;
  context.maximumPointElevationDelta = 4 * yToleranceInPlotUnits;
  context.displayRatioElevationVsDistance = ( ( mPlotItem->yMaximum() - mPlotItem->yMinimum() ) / ( mPlotItem->plotArea().height() ) )
                                            / ( ( mPlotItem->xMaximum() - mPlotItem->xMinimum() ) * mPlotItem->mXScaleFactor / ( mPlotItem->plotArea().width() ) );

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
    case Qgis::LayerType::Vector:
    {
      QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer );
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
    case Qgis::LayerType::Raster:
    case Qgis::LayerType::Plugin:
    case Qgis::LayerType::Mesh:
    case Qgis::LayerType::VectorTile:
    case Qgis::LayerType::Annotation:
    case Qgis::LayerType::PointCloud:
    case Qgis::LayerType::Group:
    case Qgis::LayerType::TiledScene:
      break;
  }
}

void QgsElevationProfileCanvas::adjustRangeForAxisScaleLock( double &xMinimum, double &xMaximum, double &yMinimum, double &yMaximum ) const
{
  // ensures that we always "zoom out" to match horizontal/vertical scales
  const double horizontalScale = ( xMaximum - xMinimum ) / mPlotItem->plotArea().width();
  const double verticalScale = ( yMaximum - yMinimum ) / mPlotItem->plotArea().height();
  if ( horizontalScale > verticalScale )
  {
    const double height = horizontalScale * mPlotItem->plotArea().height();
    const double deltaHeight = ( yMaximum - yMinimum ) - height;
    yMinimum += deltaHeight / 2;
    yMaximum -= deltaHeight / 2;
  }
  else
  {
    const double width = verticalScale * mPlotItem->plotArea().width();
    const double deltaWidth = ( ( xMaximum - xMinimum ) - width );
    xMinimum += deltaWidth / 2;
    xMaximum -= deltaWidth / 2;
  }
}

Qgis::DistanceUnit QgsElevationProfileCanvas::distanceUnit() const
{
  return mDistanceUnit;
}

void QgsElevationProfileCanvas::setDistanceUnit( Qgis::DistanceUnit unit )
{
  mDistanceUnit = unit;
  const double oldMin = mPlotItem->xMinimum() * mPlotItem->mXScaleFactor;
  const double oldMax = mPlotItem->xMaximum() * mPlotItem->mXScaleFactor;
  mPlotItem->mXScaleFactor = QgsUnitTypes::fromUnitToUnitFactor( mDistanceUnit, mCrs.mapUnits() );
  mPlotItem->setXAxisUnits( mDistanceUnit );
  mPlotItem->setXMinimum( oldMin / mPlotItem->mXScaleFactor );
  mPlotItem->setXMaximum( oldMax / mPlotItem->mXScaleFactor );
  mPlotItem->updatePlot();
}

void QgsElevationProfileCanvas::setBackgroundColor( const QColor &color )
{
  if ( !color.isValid() )
  {
    QPalette customPalette = qApp->palette();
    const QColor baseColor = qApp->palette().color( QPalette::ColorRole::Base );
    const QColor windowColor = qApp->palette().color( QPalette::ColorRole::Window );
    customPalette.setColor( QPalette::ColorRole::Base, windowColor );
    customPalette.setColor( QPalette::ColorRole::Window, baseColor );
    setPalette( customPalette );
    scene()->setPalette( customPalette );
  }
  else
  {
    // build custom palette
    const bool isDarkTheme = color.lightnessF() < 0.5;
    QPalette customPalette = qApp->palette();
    customPalette.setColor( QPalette::ColorRole::Window, color );
    if ( isDarkTheme )
    {
      customPalette.setColor( QPalette::ColorRole::Text, QColor( 255, 255, 255 ) );
      customPalette.setColor( QPalette::ColorRole::Base, color.lighter( 120 ) );
    }
    else
    {
      customPalette.setColor( QPalette::ColorRole::Text, QColor( 0, 0, 0 ) );
      customPalette.setColor( QPalette::ColorRole::Base, color.darker( 120 ) );
    }

    setPalette( customPalette );
    scene()->setPalette( customPalette );
  }

  updateChartFromPalette();
}

bool QgsElevationProfileCanvas::lockAxisScales() const
{
  return mLockAxisScales;
}

void QgsElevationProfileCanvas::setLockAxisScales( bool lock )
{
  mLockAxisScales = lock;
  if ( mLockAxisScales )
  {
    double xMinimum = mPlotItem->xMinimum() * mPlotItem->mXScaleFactor;
    double xMaximum = mPlotItem->xMaximum() * mPlotItem->mXScaleFactor;
    double yMinimum = mPlotItem->yMinimum();
    double yMaximum = mPlotItem->yMaximum();
    adjustRangeForAxisScaleLock( xMinimum, xMaximum, yMinimum, yMaximum );
    mPlotItem->setXMinimum( xMinimum / mPlotItem->mXScaleFactor );
    mPlotItem->setXMaximum( xMaximum / mPlotItem->mXScaleFactor );
    mPlotItem->setYMinimum( yMinimum );
    mPlotItem->setYMaximum( yMaximum );

    refineResults();
    mPlotItem->updatePlot();
    emit plotAreaChanged();
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
  if ( mLockAxisScales )
    yFactor = xFactor;

  const double currentWidth = ( mPlotItem->xMaximum() - mPlotItem->xMinimum() ) * mPlotItem->mXScaleFactor;
  const double currentHeight = mPlotItem->yMaximum() - mPlotItem->yMinimum();

  const double newWidth = currentWidth / xFactor;
  const double newHeight = currentHeight / yFactor;

  const double currentCenterX = ( mPlotItem->xMinimum() + mPlotItem->xMaximum() ) * 0.5 * mPlotItem->mXScaleFactor;
  const double currentCenterY = ( mPlotItem->yMinimum() + mPlotItem->yMaximum() ) * 0.5;

  double xMinimum = currentCenterX - newWidth * 0.5;
  double xMaximum = currentCenterX + newWidth * 0.5;
  double yMinimum = currentCenterY - newHeight * 0.5;
  double yMaximum = currentCenterY + newHeight * 0.5;
  if ( mLockAxisScales )
  {
    adjustRangeForAxisScaleLock( xMinimum, xMaximum, yMinimum, yMaximum );
  }

  mPlotItem->setXMinimum( xMinimum / mPlotItem->mXScaleFactor );
  mPlotItem->setXMaximum( xMaximum / mPlotItem->mXScaleFactor );
  mPlotItem->setYMinimum( yMinimum );
  mPlotItem->setYMaximum( yMaximum );

  refineResults();
  mPlotItem->updatePlot();
  emit plotAreaChanged();
}

void QgsElevationProfileCanvas::zoomToRect( const QRectF &rect )
{
  const QRectF intersected = rect.intersected( mPlotItem->plotArea() );

  double minX = ( intersected.left() - mPlotItem->plotArea().left() ) / mPlotItem->plotArea().width() * ( mPlotItem->xMaximum() - mPlotItem->xMinimum() ) * mPlotItem->mXScaleFactor + mPlotItem->xMinimum() * mPlotItem->mXScaleFactor;
  double maxX = ( intersected.right() - mPlotItem->plotArea().left() ) / mPlotItem->plotArea().width() * ( mPlotItem->xMaximum() - mPlotItem->xMinimum() ) * mPlotItem->mXScaleFactor + mPlotItem->xMinimum() * mPlotItem->mXScaleFactor;
  double minY = ( mPlotItem->plotArea().bottom() - intersected.bottom() ) / mPlotItem->plotArea().height() * ( mPlotItem->yMaximum() - mPlotItem->yMinimum() ) + mPlotItem->yMinimum();
  double maxY = ( mPlotItem->plotArea().bottom() - intersected.top() ) / mPlotItem->plotArea().height() * ( mPlotItem->yMaximum() - mPlotItem->yMinimum() ) + mPlotItem->yMinimum();

  if ( mLockAxisScales )
  {
    adjustRangeForAxisScaleLock( minX, maxX, minY, maxY );
  }

  mPlotItem->setXMinimum( minX / mPlotItem->mXScaleFactor );
  mPlotItem->setXMaximum( maxX / mPlotItem->mXScaleFactor );
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
  bool reverseZoom = settings.value( QStringLiteral( "qgis/reverse_wheel_zoom" ), false ).toBool();
  bool zoomIn = reverseZoom ? event->angleDelta().y() < 0 : event->angleDelta().y() > 0;

  // "Normal" mouse have an angle delta of 120, precision mouses provide data faster, in smaller steps
  zoomFactor = 1.0 + ( zoomFactor - 1.0 ) / 120.0 * std::fabs( event->angleDelta().y() );

  if ( event->modifiers() & Qt::ControlModifier )
  {
    //holding ctrl while wheel zooming results in a finer zoom
    zoomFactor = 1.0 + ( zoomFactor - 1.0 ) / 20.0;
  }

  //calculate zoom scale factor
  double scaleFactor = ( zoomIn ? 1 / zoomFactor : zoomFactor );

  QRectF viewportRect = mPlotItem->plotArea();

  if ( viewportRect.contains( event->position() ) )
  {
    //adjust view center
    const double oldCenterX = 0.5 * ( mPlotItem->xMaximum() + mPlotItem->xMinimum() );
    const double oldCenterY = 0.5 * ( mPlotItem->yMaximum() + mPlotItem->yMinimum() );

    const double eventPosX = ( event->position().x() - viewportRect.left() ) / viewportRect.width() * ( mPlotItem->xMaximum() - mPlotItem->xMinimum() ) + mPlotItem->xMinimum();
    const double eventPosY = ( viewportRect.bottom() - event->position().y() ) / viewportRect.height() * ( mPlotItem->yMaximum() - mPlotItem->yMinimum() ) + mPlotItem->yMinimum();

    const double newCenterX = eventPosX + ( ( oldCenterX - eventPosX ) * scaleFactor );
    const double newCenterY = eventPosY + ( ( oldCenterY - eventPosY ) * scaleFactor );

    const double dxPlot = newCenterX - ( mPlotItem->xMaximum() + mPlotItem->xMinimum() ) * 0.5;
    const double dyPlot = newCenterY - ( mPlotItem->yMaximum() + mPlotItem->yMinimum() ) * 0.5;

    // don't need to handle axis scale lock here, we are always changing axis by the same scale
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

  cancelJobs();

  QgsProfileRequest request( profileCurve()->clone() );
  request.setCrs( mCrs );
  request.setTolerance( mTolerance );
  request.setTransformContext( mProject->transformContext() );
  request.setTerrainProvider( mProject->elevationProperties()->terrainProvider() ? mProject->elevationProperties()->terrainProvider()->clone() : nullptr );
  QgsExpressionContext context;
  context.appendScope( QgsExpressionContextUtils::globalScope() );
  context.appendScope( QgsExpressionContextUtils::projectScope( mProject ) );
  request.setExpressionContext( context );

  const QList<QgsMapLayer *> layersToGenerate = layers();
  QList<QgsAbstractProfileSource *> sources;
  const QList<QgsAbstractProfileSource *> registrySources = QgsApplication::profileSourceRegistry()->profileSources();
  sources.reserve( layersToGenerate.size() + registrySources.size() );

  sources << registrySources;
  for ( QgsMapLayer *layer : layersToGenerate )
  {
    if ( QgsAbstractProfileSource *source = dynamic_cast<QgsAbstractProfileSource *>( layer ) )
      sources.append( source );
  }

  mCurrentJob = new QgsProfilePlotRenderer( sources, request );
  connect( mCurrentJob, &QgsProfilePlotRenderer::generationFinished, this, &QgsElevationProfileCanvas::generationFinished );

  QgsProfileGenerationContext generationContext;
  generationContext.setDpi( mScreenHelper->screenDpi() );
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

  QgsMapLayerElevationProperties *properties = qobject_cast<QgsMapLayerElevationProperties *>( sender() );
  if ( !properties )
    return;

  if ( QgsMapLayer *layer = qobject_cast<QgsMapLayer *>( properties->parent() ) )
  {
    if ( QgsAbstractProfileSource *source = dynamic_cast<QgsAbstractProfileSource *>( layer ) )
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

  QgsMapLayerElevationProperties *properties = qobject_cast<QgsMapLayerElevationProperties *>( sender() );
  if ( !properties )
    return;

  if ( QgsMapLayer *layer = qobject_cast<QgsMapLayer *>( properties->parent() ) )
  {
    if ( QgsAbstractProfileSource *source = dynamic_cast<QgsAbstractProfileSource *>( layer ) )
    {
      mCurrentJob->replaceSource( source );
    }
    if ( mPlotItem->redrawResults( layer->id() ) )
      scheduleDeferredRedraw();
  }
}

void QgsElevationProfileCanvas::regenerateResultsForLayer()
{
  if ( !mCurrentJob )
    return;

  if ( QgsMapLayer *layer = qobject_cast<QgsMapLayer *>( sender() ) )
  {
    if ( QgsAbstractProfileSource *source = dynamic_cast<QgsAbstractProfileSource *>( layer ) )
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
    context.setDpi( mScreenHelper->screenDpi() );
    const double plotDistanceRange = ( mPlotItem->xMaximum() - mPlotItem->xMinimum() ) * mPlotItem->mXScaleFactor;
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
    const double distanceMin = std::floor( ( mPlotItem->xMinimum() * mPlotItem->mXScaleFactor - plotDistanceRange * 0.05 ) / context.maximumErrorMapUnits() ) * context.maximumErrorMapUnits();
    context.setDistanceRange( QgsDoubleRange( std::max( 0.0, distanceMin ), mPlotItem->xMaximum() * mPlotItem->mXScaleFactor + plotDistanceRange * 0.05 ) );

    context.setElevationRange( QgsDoubleRange( mPlotItem->yMinimum() - plotElevationRange * 0.05, mPlotItem->yMaximum() + plotElevationRange * 0.05 ) );
    mCurrentJob->setContext( context );
  }
  scheduleDeferredRegeneration();
}

void QgsElevationProfileCanvas::updateChartFromPalette()
{
  const QPalette chartPalette = palette();
  setBackgroundBrush( QBrush( chartPalette.color( QPalette::ColorRole::Base ) ) );
  {
    QgsTextFormat textFormat = mPlotItem->xAxis().textFormat();
    textFormat.setColor( chartPalette.color( QPalette::ColorGroup::Active, QPalette::Text ) );
    mPlotItem->xAxis().setTextFormat( textFormat );
    mPlotItem->yAxis().setTextFormat( textFormat );
  }
  {
    std::unique_ptr<QgsFillSymbol> chartFill( mPlotItem->chartBackgroundSymbol()->clone() );
    chartFill->setColor( chartPalette.color( QPalette::ColorGroup::Active, QPalette::ColorRole::Window ) );
    mPlotItem->setChartBackgroundSymbol( chartFill.release() );
  }
  {
    std::unique_ptr<QgsFillSymbol> chartBorder( mPlotItem->chartBorderSymbol()->clone() );
    chartBorder->setColor( chartPalette.color( QPalette::ColorGroup::Active, QPalette::ColorRole::Text ) );
    mPlotItem->setChartBorderSymbol( chartBorder.release() );
  }
  {
    std::unique_ptr<QgsLineSymbol> chartMajorSymbol( mPlotItem->xAxis().gridMajorSymbol()->clone() );
    QColor c = chartPalette.color( QPalette::ColorGroup::Active, QPalette::ColorRole::Text );
    c.setAlpha( 150 );
    chartMajorSymbol->setColor( c );
    mPlotItem->xAxis().setGridMajorSymbol( chartMajorSymbol->clone() );
    mPlotItem->yAxis().setGridMajorSymbol( chartMajorSymbol.release() );
  }
  {
    std::unique_ptr<QgsLineSymbol> chartMinorSymbol( mPlotItem->xAxis().gridMinorSymbol()->clone() );
    QColor c = chartPalette.color( QPalette::ColorGroup::Active, QPalette::ColorRole::Text );
    c.setAlpha( 50 );
    chartMinorSymbol->setColor( c );
    mPlotItem->xAxis().setGridMinorSymbol( chartMinorSymbol->clone() );
    mPlotItem->yAxis().setGridMinorSymbol( chartMinorSymbol.release() );
  }
  mPlotItem->updatePlot();
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
  filteredList.erase( std::remove_if( filteredList.begin(), filteredList.end(), []( QgsMapLayer *layer ) {
                        return !layer || !layer->isValid();
                      } ),
                      filteredList.end() );

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

  if ( mLockAxisScales )
  {
    double xMinimum = mPlotItem->xMinimum();
    double xMaximum = mPlotItem->xMaximum();
    double yMinimum = mPlotItem->yMinimum();
    double yMaximum = mPlotItem->yMaximum();
    adjustRangeForAxisScaleLock( xMinimum, xMaximum, yMinimum, yMaximum );
    mPlotItem->setXMinimum( xMinimum );
    mPlotItem->setXMaximum( xMaximum );
    mPlotItem->setYMinimum( yMinimum );
    mPlotItem->setYMaximum( yMaximum );
  }

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
  double distanceAlongCurveLength = distanceAlongPlotPercent * ( mPlotItem->xMaximum() - mPlotItem->xMinimum() ) * mPlotItem->mXScaleFactor + mPlotItem->xMinimum() * mPlotItem->mXScaleFactor;

  std::unique_ptr<QgsPoint> mapXyPoint( mProfileCurve->interpolatePoint( distanceAlongCurveLength ) );
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

  const double distanceAlongCurveOnPlot = distanceAlongCurve - mPlotItem->xMinimum() * mPlotItem->mXScaleFactor;
  const double distanceAlongCurvePercent = distanceAlongCurveOnPlot / ( ( mPlotItem->xMaximum() - mPlotItem->xMinimum() ) * mPlotItem->mXScaleFactor );
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

  double yMinimum = 0;
  double yMaximum = 0;

  if ( zRange.upper() < zRange.lower() )
  {
    // invalid range, e.g. no features found in plot!
    yMinimum = 0;
    yMaximum = 10;
  }
  else if ( qgsDoubleNear( zRange.lower(), zRange.upper(), 0.0000001 ) )
  {
    // corner case ... a zero height plot! Just pick an arbitrary +/- 5 height range.
    yMinimum = zRange.lower() - 5;
    yMaximum = zRange.lower() + 5;
  }
  else
  {
    // add 5% margin to height range
    const double margin = ( zRange.upper() - zRange.lower() ) * 0.05;
    yMinimum = zRange.lower() - margin;
    yMaximum = zRange.upper() + margin;
  }

  const double profileLength = profileCurve()->length();
  double xMinimum = 0;
  // just 2% margin to max distance -- any more is overkill and wasted space
  double xMaximum = profileLength * 1.02;

  if ( mLockAxisScales )
  {
    adjustRangeForAxisScaleLock( xMinimum, xMaximum, yMinimum, yMaximum );
  }

  mPlotItem->setXMinimum( xMinimum / mPlotItem->mXScaleFactor );
  mPlotItem->setXMaximum( xMaximum / mPlotItem->mXScaleFactor );
  mPlotItem->setYMinimum( yMinimum );
  mPlotItem->setYMaximum( yMaximum );

  refineResults();
  mPlotItem->updatePlot();
  emit plotAreaChanged();
}

void QgsElevationProfileCanvas::setVisiblePlotRange( double minimumDistance, double maximumDistance, double minimumElevation, double maximumElevation )
{
  if ( mLockAxisScales )
  {
    adjustRangeForAxisScaleLock( minimumDistance, maximumDistance, minimumElevation, maximumElevation );
  }

  mPlotItem->setYMinimum( minimumElevation );
  mPlotItem->setYMaximum( maximumElevation );
  mPlotItem->setXMinimum( minimumDistance / mPlotItem->mXScaleFactor );
  mPlotItem->setXMaximum( maximumDistance / mPlotItem->mXScaleFactor );
  refineResults();
  mPlotItem->updatePlot();
  emit plotAreaChanged();
}

QgsDoubleRange QgsElevationProfileCanvas::visibleDistanceRange() const
{
  return QgsDoubleRange( mPlotItem->xMinimum() * mPlotItem->mXScaleFactor, mPlotItem->xMaximum() * mPlotItem->mXScaleFactor );
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
      mRenderer->render( rc, plotArea.width(), plotArea.height(), xMinimum() * mXScale, xMaximum() * mXScale, yMinimum(), yMaximum() );
      rc.painter()->translate( -plotArea.left(), -plotArea.top() );
    }

    double mXScale = 1;

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

  profilePlot.mXScale = mPlotItem->mXScaleFactor;
  profilePlot.xAxis().setLabelSuffix( mPlotItem->xAxis().labelSuffix() );
  profilePlot.xAxis().setLabelSuffixPlacement( mPlotItem->xAxis().labelSuffixPlacement() );

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
  cancelJobs();
  mPlotItem->updatePlot();
}

void QgsElevationProfileCanvas::setSnappingEnabled( bool enabled )
{
  mSnappingEnabled = enabled;
}
