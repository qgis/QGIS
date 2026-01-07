/***************************************************************************
                            qgslayoutitemscalebar.cpp
                            -------------------------
    begin                : November 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgslayoutitemscalebar.h"

#include <cmath>

#include "qgsdistancearea.h"
#include "qgsfillsymbol.h"
#include "qgsfillsymbollayer.h"
#include "qgsfontutils.h"
#include "qgslayout.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutitemregistry.h"
#include "qgslayoutrendercontext.h"
#include "qgslayoututils.h"
#include "qgslinesymbol.h"
#include "qgslinesymbollayer.h"
#include "qgsmessagelog.h"
#include "qgsnumericformat.h"
#include "qgsnumericformatregistry.h"
#include "qgsproject.h"
#include "qgsrectangle.h"
#include "qgsscalebarrenderer.h"
#include "qgsscalebarrendererregistry.h"
#include "qgssettings.h"
#include "qgssingleboxscalebarrenderer.h"
#include "qgsstyleentityvisitor.h"
#include "qgssymbollayerutils.h"
#include "qgsunittypes.h"

#include <QAbstractTextDocumentLayout>
#include <QDomDocument>
#include <QDomElement>
#include <QFontMetricsF>
#include <QPainter>
#include <QTextDocument>

#include "moc_qgslayoutitemscalebar.cpp"

QgsLayoutItemScaleBar::QgsLayoutItemScaleBar( QgsLayout *layout )
  : QgsLayoutItem( layout )
{
  applyDefaultSettings();
  applyDefaultSize();
}

int QgsLayoutItemScaleBar::type() const
{
  return QgsLayoutItemRegistry::LayoutScaleBar;
}

QIcon QgsLayoutItemScaleBar::icon() const
{
  return QgsApplication::getThemeIcon( u"/mLayoutItemScaleBar.svg"_s );
}

QgsLayoutItemScaleBar *QgsLayoutItemScaleBar::create( QgsLayout *layout )
{
  return new QgsLayoutItemScaleBar( layout );
}

QgsLayoutSize QgsLayoutItemScaleBar::minimumSize() const
{
  QgsRenderContext context = QgsLayoutUtils::createRenderContextForLayout( mLayout, nullptr );

  const QgsScaleBarRenderer::ScaleBarContext scaleContext = createScaleContext();
  if ( !scaleContext.isValid() )
    return QgsLayoutSize();

  return QgsLayoutSize( mStyle->calculateBoxSize( context, mSettings, scaleContext ), Qgis::LayoutUnit::Millimeters );
}

void QgsLayoutItemScaleBar::draw( QgsLayoutItemRenderContext &context )
{
  if ( !mStyle )
    return;

  const QgsScaleBarRenderer::ScaleBarContext scaleContext = createScaleContext();
  if ( !scaleContext.isValid() )
  {
    if ( mLayout->renderContext().isPreviewRender() )
    {
      // No initial render available - so draw some preview text alerting user
      QPainter *painter = context.renderContext().painter();

      const double scale = context.renderContext().convertToPainterUnits( 1, Qgis::RenderUnit::Millimeters );
      const QRectF thisPaintRect = QRectF( 0, 0, rect().width() * scale, rect().height() * scale );

      painter->setBrush( QBrush( QColor( 255, 125, 125, 125 ) ) );
      painter->setPen( Qt::NoPen );
      painter->drawRect( thisPaintRect );
      painter->setBrush( Qt::NoBrush );

      painter->setPen( QColor( 200, 0, 0, 255 ) );
      QTextDocument td;
      td.setTextWidth( thisPaintRect.width() );
      td.setHtml( u"<span style=\"color: rgb(200,0,0);\"><b>%1</b><br>%2</span>"_s.arg(
                    tr( "Invalid scale!" ),
                    tr( "The scale bar cannot be rendered due to invalid settings or an incompatible linked map extent." ) ) );
      painter->setClipRect( thisPaintRect );
      QAbstractTextDocumentLayout::PaintContext ctx;
      td.documentLayout()->draw( painter, ctx );
    }
    return;
  }

  if ( dataDefinedProperties().isActive( QgsLayoutObject::DataDefinedProperty::ScalebarLineColor ) || dataDefinedProperties().isActive( QgsLayoutObject::DataDefinedProperty::ScalebarLineWidth ) )
  {
    // compatibility code - ScalebarLineColor and ScalebarLineWidth are deprecated
    const QgsExpressionContext expContext = createExpressionContext();
    std::unique_ptr< QgsLineSymbol > sym( mSettings.lineSymbol()->clone() );
    Q_NOWARN_DEPRECATED_PUSH
    if ( dataDefinedProperties().isActive( QgsLayoutObject::DataDefinedProperty::ScalebarLineWidth ) )
      sym->setWidth( mDataDefinedProperties.valueAsDouble( QgsLayoutObject::DataDefinedProperty::ScalebarLineWidth, expContext, mSettings.lineWidth() ) );
    if ( dataDefinedProperties().isActive( QgsLayoutObject::DataDefinedProperty::ScalebarLineColor ) )
      sym->setColor( mDataDefinedProperties.valueAsColor( QgsLayoutObject::DataDefinedProperty::ScalebarLineColor, expContext, mSettings.lineColor() ) );
    Q_NOWARN_DEPRECATED_POP
    mSettings.setLineSymbol( sym.release() );
  }
  if ( dataDefinedProperties().isActive( QgsLayoutObject::DataDefinedProperty::ScalebarFillColor ) )
  {
    // compatibility code - ScalebarLineColor and ScalebarLineWidth are deprecated
    const QgsExpressionContext expContext = createExpressionContext();
    std::unique_ptr< QgsFillSymbol > sym( mSettings.fillSymbol()->clone() );
    Q_NOWARN_DEPRECATED_PUSH
    sym->setColor( mDataDefinedProperties.valueAsColor( QgsLayoutObject::DataDefinedProperty::ScalebarFillColor, expContext, mSettings.fillColor() ) );
    Q_NOWARN_DEPRECATED_POP
    mSettings.setFillSymbol( sym.release() );
  }
  if ( dataDefinedProperties().isActive( QgsLayoutObject::DataDefinedProperty::ScalebarFillColor2 ) )
  {
    // compatibility code - ScalebarLineColor and ScalebarLineWidth are deprecated
    const QgsExpressionContext expContext = createExpressionContext();
    std::unique_ptr< QgsFillSymbol > sym( mSettings.alternateFillSymbol()->clone() );
    Q_NOWARN_DEPRECATED_PUSH
    sym->setColor( mDataDefinedProperties.valueAsColor( QgsLayoutObject::DataDefinedProperty::ScalebarFillColor2, expContext, mSettings.fillColor2() ) );
    Q_NOWARN_DEPRECATED_POP
    mSettings.setAlternateFillSymbol( sym.release() );
  }

  mStyle->draw( context.renderContext(), mSettings, scaleContext );
}

void QgsLayoutItemScaleBar::setNumberOfSegments( int nSegments )
{
  if ( !mStyle )
  {
    mSettings.setNumberOfSegments( nSegments );
    return;
  }
  mSettings.setNumberOfSegments( nSegments );
  resizeToMinimumWidth();
}

void QgsLayoutItemScaleBar::setUnitsPerSegment( double units )
{
  if ( !mStyle )
  {
    mSettings.setUnitsPerSegment( units );
    return;
  }
  mSettings.setUnitsPerSegment( units );
  refreshSegmentMillimeters();
  resizeToMinimumWidth();
}


void QgsLayoutItemScaleBar::setSegmentSizeMode( Qgis::ScaleBarSegmentSizeMode mode )
{
  if ( !mStyle )
  {
    mSettings.setSegmentSizeMode( mode );
    return;
  }
  mSettings.setSegmentSizeMode( mode );
  refreshSegmentMillimeters();
  resizeToMinimumWidth();
}

void QgsLayoutItemScaleBar::setMinimumBarWidth( double minWidth )
{
  if ( !mStyle )
  {
    mSettings.setMinimumBarWidth( minWidth );
    return;
  }
  mSettings.setMinimumBarWidth( minWidth );
  refreshSegmentMillimeters();
  resizeToMinimumWidth();
}

void QgsLayoutItemScaleBar::setMaximumBarWidth( double maxWidth )
{
  if ( !mStyle )
  {
    mSettings.setMaximumBarWidth( maxWidth );
    return;
  }
  mSettings.setMaximumBarWidth( maxWidth );
  refreshSegmentMillimeters();
  resizeToMinimumWidth();
}

QgsTextFormat QgsLayoutItemScaleBar::textFormat() const
{
  return mSettings.textFormat();
}

void QgsLayoutItemScaleBar::setTextFormat( const QgsTextFormat &format )
{
  mSettings.setTextFormat( format );
  refreshItemSize();
  emit changed();
}

QgsLineSymbol *QgsLayoutItemScaleBar::lineSymbol() const
{
  return mSettings.lineSymbol();
}

void QgsLayoutItemScaleBar::setLineSymbol( QgsLineSymbol *symbol )
{
  mSettings.setLineSymbol( symbol );
}

QgsLineSymbol *QgsLayoutItemScaleBar::divisionLineSymbol() const
{
  return mSettings.divisionLineSymbol();
}

void QgsLayoutItemScaleBar::setDivisionLineSymbol( QgsLineSymbol *symbol )
{
  mSettings.setDivisionLineSymbol( symbol );
}

QgsLineSymbol *QgsLayoutItemScaleBar::subdivisionLineSymbol() const
{
  return mSettings.subdivisionLineSymbol();
}

void QgsLayoutItemScaleBar::setSubdivisionLineSymbol( QgsLineSymbol *symbol )
{
  mSettings.setSubdivisionLineSymbol( symbol );
}

QgsFillSymbol *QgsLayoutItemScaleBar::fillSymbol() const
{
  return mSettings.fillSymbol();
}

void QgsLayoutItemScaleBar::setFillSymbol( QgsFillSymbol *symbol )
{
  mSettings.setFillSymbol( symbol );
}

QgsFillSymbol *QgsLayoutItemScaleBar::alternateFillSymbol() const
{
  return mSettings.alternateFillSymbol();
}

void QgsLayoutItemScaleBar::setAlternateFillSymbol( QgsFillSymbol *symbol )
{
  mSettings.setAlternateFillSymbol( symbol );
}

void QgsLayoutItemScaleBar::setNumberOfSegmentsLeft( int nSegmentsLeft )
{
  if ( !mStyle )
  {
    mSettings.setNumberOfSegmentsLeft( nSegmentsLeft );
    return;
  }
  mSettings.setNumberOfSegmentsLeft( nSegmentsLeft );
  resizeToMinimumWidth();
}

void QgsLayoutItemScaleBar::setBoxContentSpace( double space )
{
  if ( !mStyle )
  {
    mSettings.setBoxContentSpace( space );
    return;
  }
  mSettings.setBoxContentSpace( space );
  refreshItemSize();
}

void QgsLayoutItemScaleBar::setLinkedMap( QgsLayoutItemMap *map )
{
  disconnectCurrentMap();

  mMap = map;

  if ( !map )
  {
    return;
  }

  connect( mMap, &QgsLayoutItemMap::extentChanged, this, &QgsLayoutItemScaleBar::updateScale );
  connect( mMap, &QObject::destroyed, this, &QgsLayoutItemScaleBar::disconnectCurrentMap );

  refreshSegmentMillimeters();
  emit changed();
}

void QgsLayoutItemScaleBar::disconnectCurrentMap()
{
  if ( !mMap )
  {
    return;
  }

  disconnect( mMap, &QgsLayoutItemMap::extentChanged, this, &QgsLayoutItemScaleBar::updateScale );
  disconnect( mMap, &QObject::destroyed, this, &QgsLayoutItemScaleBar::disconnectCurrentMap );
  mMap = nullptr;
}

Qgis::ScaleCalculationMethod QgsLayoutItemScaleBar::method() const
{
  return mMethod;
}

void QgsLayoutItemScaleBar::setMethod( Qgis::ScaleCalculationMethod method )
{
  if ( mMethod == method )
    return;

  mMethod = method;
  refreshSegmentMillimeters();
  resizeToMinimumWidth();
}

void QgsLayoutItemScaleBar::refreshUnitsPerSegment( const QgsExpressionContext *context )
{
  if ( mDataDefinedProperties.isActive( QgsLayoutObject::DataDefinedProperty::ScalebarSegmentWidth ) )
  {
    double unitsPerSegment = mSettings.unitsPerSegment();
    bool ok = false;
    unitsPerSegment = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::DataDefinedProperty::ScalebarSegmentWidth, *context, unitsPerSegment, &ok );

    if ( !ok )
    {
      QgsMessageLog::logMessage( tr( "Scalebar units per segment expression eval error" ) );
    }
    else
    {
      setUnitsPerSegment( unitsPerSegment );
    }
  }
}

void QgsLayoutItemScaleBar::refreshMinimumBarWidth( const QgsExpressionContext *context )
{
  if ( mDataDefinedProperties.isActive( QgsLayoutObject::DataDefinedProperty::ScalebarMinimumWidth ) )
  {
    double minimumBarWidth = mSettings.minimumBarWidth();

    bool ok = false;
    minimumBarWidth = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::DataDefinedProperty::ScalebarMinimumWidth, *context, minimumBarWidth, &ok );

    if ( !ok )
    {
      QgsMessageLog::logMessage( tr( "Scalebar minimum segment width expression eval error" ) );
    }
    else
    {
      setMinimumBarWidth( minimumBarWidth );
    }
  }
}

void QgsLayoutItemScaleBar::refreshMaximumBarWidth( const QgsExpressionContext *context )
{
  if ( mDataDefinedProperties.isActive( QgsLayoutObject::DataDefinedProperty::ScalebarMaximumWidth ) )
  {
    double maximumBarWidth = mSettings.maximumBarWidth();

    bool ok = false;
    maximumBarWidth = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::DataDefinedProperty::ScalebarMaximumWidth, *context, maximumBarWidth, &ok );

    if ( !ok )
    {
      QgsMessageLog::logMessage( tr( "Scalebar maximum segment width expression eval error" ) );
    }
    else
    {
      setMaximumBarWidth( maximumBarWidth );
    }
  }
}

void QgsLayoutItemScaleBar::refreshNumberOfSegmentsLeft( const QgsExpressionContext *context )
{
  if ( mDataDefinedProperties.isActive( QgsLayoutObject::DataDefinedProperty::ScalebarLeftSegments ) )
  {
    int leftSegments = mSettings.numberOfSegmentsLeft();

    bool ok = false;
    leftSegments = mDataDefinedProperties.valueAsInt( QgsLayoutObject::DataDefinedProperty::ScalebarLeftSegments, *context, leftSegments, &ok );

    if ( !ok )
    {
      QgsMessageLog::logMessage( tr( "Scalebar left segment count expression eval error" ) );
    }
    else
    {
      setNumberOfSegmentsLeft( leftSegments );
    }
  }
}

void QgsLayoutItemScaleBar::refreshNumberOfSegmentsRight( const QgsExpressionContext *context )
{
  if ( mDataDefinedProperties.isActive( QgsLayoutObject::DataDefinedProperty::ScalebarRightSegments ) )
  {
    int rightSegments = mSettings.numberOfSegments();

    bool ok = false;
    rightSegments = mDataDefinedProperties.valueAsInt( QgsLayoutObject::DataDefinedProperty::ScalebarRightSegments, *context, rightSegments, &ok );

    if ( !ok )
    {
      QgsMessageLog::logMessage( tr( "Scalebar left segment count expression eval error" ) );
    }
    else
    {
      setNumberOfSegments( rightSegments );
    }
  }
}

void QgsLayoutItemScaleBar::refreshDataDefinedProperty( const QgsLayoutObject::DataDefinedProperty property )
{
  const QgsExpressionContext context = createExpressionContext();

  bool forceUpdate = false;

  if ( ( property == QgsLayoutObject::DataDefinedProperty::ScalebarHeight || property == QgsLayoutObject::DataDefinedProperty::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::DataDefinedProperty::ScalebarHeight ) ) )
  {
    double height = mSettings.height();

    bool ok = false;
    height = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::DataDefinedProperty::ScalebarHeight, context, height, &ok );

    if ( !ok )
    {
      QgsMessageLog::logMessage( tr( "Scalebar height expression eval error" ) );
    }
    else
    {
      setHeight( height );
    }

    forceUpdate = true;
  }

  if ( ( property == QgsLayoutObject::DataDefinedProperty::ScalebarSubdivisionHeight || property == QgsLayoutObject::DataDefinedProperty::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::DataDefinedProperty::ScalebarSubdivisionHeight ) ) )
  {
    double height = mSettings.subdivisionsHeight();

    bool ok = false;
    height = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::DataDefinedProperty::ScalebarSubdivisionHeight, context, height, &ok );

    if ( !ok )
    {
      QgsMessageLog::logMessage( tr( "Scalebar subdivision height expression eval error" ) );
    }
    else
    {
      setSubdivisionsHeight( height );
    }

    forceUpdate = true;
  }

  if ( property ==  QgsLayoutObject::DataDefinedProperty::ScalebarLeftSegments || property == QgsLayoutObject::DataDefinedProperty::AllProperties )
  {
    refreshNumberOfSegmentsLeft( &context );
    forceUpdate = true;
  }

  if ( property == QgsLayoutObject::DataDefinedProperty::ScalebarRightSegments || property == QgsLayoutObject::DataDefinedProperty::AllProperties )
  {
    refreshNumberOfSegmentsRight( &context );
    forceUpdate = true;
  }

  if ( ( property == QgsLayoutObject::DataDefinedProperty::ScalebarRightSegmentSubdivisions || property == QgsLayoutObject::DataDefinedProperty::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::DataDefinedProperty::ScalebarRightSegmentSubdivisions ) ) )
  {
    int segments = mSettings.numberOfSubdivisions();

    bool ok = false;
    segments = mDataDefinedProperties.valueAsInt( QgsLayoutObject::DataDefinedProperty::ScalebarRightSegmentSubdivisions, context, segments, &ok );

    if ( !ok )
    {
      QgsMessageLog::logMessage( tr( "Scalebar number of subdivisions expression eval error" ) );
    }
    else
    {
      setNumberOfSubdivisions( segments );
    }

    forceUpdate = true;
  }


  if ( property == QgsLayoutObject::DataDefinedProperty::ScalebarSegmentWidth || property == QgsLayoutObject::DataDefinedProperty::AllProperties )
  {
    refreshUnitsPerSegment( &context );
    forceUpdate = true;
  }

  if ( property == QgsLayoutObject::DataDefinedProperty::ScalebarMinimumWidth || property == QgsLayoutObject::DataDefinedProperty::AllProperties )
  {
    refreshMinimumBarWidth( &context );
    forceUpdate = true;
  }

  if ( property == QgsLayoutObject::DataDefinedProperty::ScalebarMaximumWidth || property == QgsLayoutObject::DataDefinedProperty::AllProperties )
  {
    refreshMaximumBarWidth( &context );
    forceUpdate = true;
  }

  // updates data defined properties and redraws item to match
  // -- Deprecated --
  if ( property == QgsLayoutObject::DataDefinedProperty::ScalebarFillColor || property == QgsLayoutObject::DataDefinedProperty::AllProperties )
  {
    forceUpdate = true;
  }
  if ( property == QgsLayoutObject::DataDefinedProperty::ScalebarFillColor2 || property == QgsLayoutObject::DataDefinedProperty::AllProperties )
  {
    forceUpdate = true;
  }
  if ( property == QgsLayoutObject::DataDefinedProperty::ScalebarLineColor || property == QgsLayoutObject::DataDefinedProperty::AllProperties )
  {
    forceUpdate = true;
  }
  if ( property == QgsLayoutObject::DataDefinedProperty::ScalebarLineWidth || property == QgsLayoutObject::DataDefinedProperty::AllProperties )
  {
    forceUpdate = true;
  }

  if ( forceUpdate )
  {
    refreshItemSize();
    update();
  }

  QgsLayoutItem::refreshDataDefinedProperty( property );
}

void QgsLayoutItemScaleBar::refreshSegmentMillimeters()
{
  if ( mMap )
  {
    //get mm dimension of composer map
    const QRectF composerItemRect = mMap->rect();

    const double currentMapWidth = mapWidth();
    if ( qgsDoubleNear( currentMapWidth, 0 ) || std::isnan( currentMapWidth ) )
    {
      mSegmentMillimeters = std::numeric_limits< double >::quiet_NaN();
      return;
    }

    switch ( mSettings.segmentSizeMode() )
    {
      case Qgis::ScaleBarSegmentSizeMode::Fixed:
      {
        //calculate size depending on mNumUnitsPerSegment
        mSegmentMillimeters = composerItemRect.width() / currentMapWidth * mSettings.unitsPerSegment();
        break;
      }

      case Qgis::ScaleBarSegmentSizeMode::FitWidth:
      {
        if ( mSettings.maximumBarWidth() < mSettings.minimumBarWidth() )
        {
          mSegmentMillimeters = 0;
        }
        else
        {
          const double nSegments = ( mSettings.numberOfSegmentsLeft() != 0 ) + mSettings.numberOfSegments();
          // unitsPerSegments which fit minBarWidth resp. maxBarWidth
          const double minUnitsPerSeg = ( mSettings.minimumBarWidth() * currentMapWidth ) / ( nSegments * composerItemRect.width() );
          const double maxUnitsPerSeg = ( mSettings.maximumBarWidth() * currentMapWidth ) / ( nSegments * composerItemRect.width() );
          mSettings.setUnitsPerSegment( QgsLayoutUtils::calculatePrettySize( minUnitsPerSeg, maxUnitsPerSeg ) );
          mSegmentMillimeters = composerItemRect.width() / currentMapWidth * mSettings.unitsPerSegment();
        }
        break;
      }
    }
  }
}

double QgsLayoutItemScaleBar::mapWidth() const
{
  if ( !mMap )
  {
    return 0.0;
  }

  const QgsRectangle mapExtent = mMap->extent();
  if ( mSettings.units() == Qgis::DistanceUnit::Unknown )
  {
    return mapExtent.width();
  }
  else
  {
    QgsDistanceArea da;
    da.setSourceCrs( mMap->crs(), mLayout->project()->transformContext() );
    da.setEllipsoid( mLayout->project()->ellipsoid() );

    const Qgis::DistanceUnit units = da.lengthUnits();

    QList< double > yValues;
    switch ( mMethod )
    {
      case Qgis::ScaleCalculationMethod::HorizontalTop:
        yValues << mapExtent.yMaximum();
        break;

      case Qgis::ScaleCalculationMethod::HorizontalMiddle:
        yValues << 0.5 * ( mapExtent.yMaximum() + mapExtent.yMinimum() );
        break;


      case Qgis::ScaleCalculationMethod::HorizontalBottom:
        yValues << mapExtent.yMinimum();
        break;

      case Qgis::ScaleCalculationMethod::HorizontalAverage:
        yValues << mapExtent.yMaximum();
        yValues << 0.5 * ( mapExtent.yMaximum() + mapExtent.yMinimum() );
        yValues << mapExtent.yMinimum();
        break;

      case Qgis::ScaleCalculationMethod::AtEquator:
        if ( mMap->crs().mapUnits() == Qgis::DistanceUnit::Degrees )
        {
          yValues << 0;
        }
        else
        {
          // this method is only for degree based systems, so just fallback to default if not degree based
          yValues << 0.5 * ( mapExtent.yMaximum() + mapExtent.yMinimum() );
        }
        break;
    }

    double sumValidMeasures = 0;
    int validMeasureCount = 0;

    for ( const double y : std::as_const( yValues ) )
    {
      try
      {
        double measure = da.measureLine( QgsPointXY( mapExtent.xMinimum(), y ),
                                         QgsPointXY( mapExtent.xMaximum(), y ) );
        if ( std::isnan( measure ) )
        {
          // TODO report errors to user
          QgsDebugError( u"An error occurred while calculating length"_s );
          continue;
        }

        measure /= QgsUnitTypes::fromUnitToUnitFactor( mSettings.units(), units );
        sumValidMeasures += measure;
        validMeasureCount++;
      }
      catch ( QgsCsException & )
      {
        // TODO report errors to user
        QgsDebugError( u"An error occurred while calculating length"_s );
        continue;
      }
    }

    if ( validMeasureCount == 0 )
      return std::numeric_limits< double >::quiet_NaN();

    return sumValidMeasures / validMeasureCount;
  }
}

QgsScaleBarRenderer::ScaleBarContext QgsLayoutItemScaleBar::createScaleContext() const
{
  QgsScaleBarRenderer::ScaleBarContext scaleContext;
  scaleContext.size = rect().size();
  scaleContext.segmentWidth = mSegmentMillimeters;
  scaleContext.scale = mMap ? mMap->scale() : 1.0;
  scaleContext.flags = mStyle->flags();
  return scaleContext;
}

void QgsLayoutItemScaleBar::setLabelVerticalPlacement( Qgis::ScaleBarDistanceLabelVerticalPlacement placement )
{
  mSettings.setLabelVerticalPlacement( placement );
  refreshItemSize();
  emit changed();
}

void QgsLayoutItemScaleBar::setLabelHorizontalPlacement( Qgis::ScaleBarDistanceLabelHorizontalPlacement placement )
{
  mSettings.setLabelHorizontalPlacement( placement );
  refreshItemSize();
  emit changed();
}

void QgsLayoutItemScaleBar::setAlignment( Qgis::ScaleBarAlignment a )
{
  mSettings.setAlignment( a );
  refreshItemSize();
  emit changed();
}

void QgsLayoutItemScaleBar::setUnits( Qgis::DistanceUnit u )
{
  mSettings.setUnits( u );
  refreshSegmentMillimeters();
  refreshItemSize();
  emit changed();
}

Qt::PenJoinStyle QgsLayoutItemScaleBar::lineJoinStyle() const
{
  Q_NOWARN_DEPRECATED_PUSH
  return mSettings.lineJoinStyle();
  Q_NOWARN_DEPRECATED_POP
}

void QgsLayoutItemScaleBar::setLineJoinStyle( Qt::PenJoinStyle style )
{
  Q_NOWARN_DEPRECATED_PUSH
  if ( mSettings.lineJoinStyle() == style )
  {
    //no change
    return;
  }
  mSettings.setLineJoinStyle( style );
  Q_NOWARN_DEPRECATED_POP
  update();
  emit changed();
}

Qt::PenCapStyle QgsLayoutItemScaleBar::lineCapStyle() const
{
  Q_NOWARN_DEPRECATED_PUSH
  return mSettings.lineCapStyle();
  Q_NOWARN_DEPRECATED_POP
}

void QgsLayoutItemScaleBar::setLineCapStyle( Qt::PenCapStyle style )
{
  Q_NOWARN_DEPRECATED_PUSH
  if ( mSettings.lineCapStyle() == style )
  {
    //no change
    return;
  }
  mSettings.setLineCapStyle( style );
  Q_NOWARN_DEPRECATED_POP
  update();
  emit changed();
}

void QgsLayoutItemScaleBar::applyDefaultSettings()
{
  //style
  mStyle = std::make_unique< QgsSingleBoxScaleBarRenderer >();

  //default to no background
  setBackgroundEnabled( false );

  //get default composer font from settings
  const QgsSettings settings;
  const QString defaultFontString = settings.value( u"LayoutDesigner/defaultFont"_s, QVariant(), QgsSettings::Gui ).toString();
  QgsTextFormat format;
  QFont f;
  if ( !defaultFontString.isEmpty() )
  {
    QgsFontUtils::setFontFamily( f, defaultFontString );
  }
  format.setFont( f );
  format.setSize( 12.0 );
  format.setSizeUnit( Qgis::RenderUnit::Points );

  mSettings.setTextFormat( format );

  mSettings.setUnits( Qgis::DistanceUnit::Unknown );
  refreshItemSize();

  emit changed();
}

bool QgsLayoutItemScaleBar::applyDefaultRendererSettings( QgsScaleBarRenderer *renderer )
{
  return renderer->applyDefaultSettings( mSettings );
}

Qgis::DistanceUnit QgsLayoutItemScaleBar::guessUnits() const
{
  if ( !mMap )
    return Qgis::DistanceUnit::Meters;

  const QgsCoordinateReferenceSystem crs = mMap->crs();
  // start with crs units
  Qgis::DistanceUnit unit = crs.mapUnits();
  if ( unit == Qgis::DistanceUnit::Degrees || unit == Qgis::DistanceUnit::Unknown )
  {
    // geographic CRS, use metric units
    unit = Qgis::DistanceUnit::Meters;
  }

  // try to pick reasonable choice between metric / imperial units
  const double widthInSelectedUnits = mapWidth();
  if ( std::isnan( widthInSelectedUnits ) )
    return unit;

  const double initialUnitsPerSegment = widthInSelectedUnits / 10.0; //default scalebar width equals half the map width
  switch ( unit )
  {
    case Qgis::DistanceUnit::Meters:
    {
      if ( initialUnitsPerSegment > 1000.0 )
      {
        unit = Qgis::DistanceUnit::Kilometers;
      }
      break;
    }
    case Qgis::DistanceUnit::Feet:
    {
      if ( initialUnitsPerSegment > 5419.95 )
      {
        unit = Qgis::DistanceUnit::Miles;
      }
      break;
    }
    default:
      break;
  }

  return unit;
}

void QgsLayoutItemScaleBar::applyDefaultSize( Qgis::DistanceUnit units )
{
  mSettings.setUnits( units );
  if ( mMap )
  {
    double upperMagnitudeMultiplier = 1.0;
    const double widthInSelectedUnits = mapWidth();
    if ( !std::isnan( widthInSelectedUnits ) )
    {
      const double initialUnitsPerSegment = widthInSelectedUnits / 10.0; //default scalebar width equals half the map width
      mSettings.setUnitsPerSegment( initialUnitsPerSegment );

      setUnitLabel( QgsUnitTypes::toAbbreviatedString( units ) );
      upperMagnitudeMultiplier = 1;

      const double segmentWidth = initialUnitsPerSegment / upperMagnitudeMultiplier;
      const int segmentMagnitude = std::floor( std::log10( segmentWidth ) );
      double unitsPerSegment = upperMagnitudeMultiplier * ( std::pow( 10.0, segmentMagnitude ) );
      const double multiplier = std::floor( ( widthInSelectedUnits / ( unitsPerSegment * 10.0 ) ) / 2.5 ) * 2.5;

      if ( multiplier > 0 )
      {
        unitsPerSegment = unitsPerSegment * multiplier;
      }
      mSettings.setUnitsPerSegment( unitsPerSegment );
      mSettings.setMapUnitsPerScaleBarUnit( upperMagnitudeMultiplier );

      mSettings.setNumberOfSegments( 2 );
      mSettings.setNumberOfSegmentsLeft( 0 );
    }
  }

  refreshSegmentMillimeters();
  resizeToMinimumWidth();
  emit changed();
}

void QgsLayoutItemScaleBar::resizeToMinimumWidth()
{
  if ( !mStyle )
    return;

  QgsRenderContext context = QgsLayoutUtils::createRenderContextForLayout( mLayout, nullptr );

  const QgsScaleBarRenderer::ScaleBarContext scaleContext = createScaleContext();
  if ( !scaleContext.isValid() )
    return;

  const double widthMM = mStyle->calculateBoxSize( context, mSettings, scaleContext ).width();
  QgsLayoutSize currentSize = sizeWithUnits();
  currentSize.setWidth( mLayout->renderContext().measurementConverter().convert( QgsLayoutMeasurement( widthMM, Qgis::LayoutUnit::Millimeters ), currentSize.units() ).length() );
  attemptResize( currentSize );
  update();
  emit changed();
}

void QgsLayoutItemScaleBar::update()
{
  //Don't adjust box size for numeric scale bars:
  if ( mStyle && mStyle->id() != "Numeric"_L1 )
  {
    refreshItemSize();
  }
  QgsLayoutItem::update();
}

void QgsLayoutItemScaleBar::updateScale()
{
  refreshSegmentMillimeters();
  //Don't adjust box size for numeric scale bars:
  if ( mStyle && mStyle->id() != "Numeric"_L1 )
  {
    resizeToMinimumWidth();
  }
  update();
}

void QgsLayoutItemScaleBar::setStyle( const QString &styleName )
{
  //switch depending on style name
  std::unique_ptr< QgsScaleBarRenderer> renderer( QgsApplication::scaleBarRendererRegistry()->renderer( styleName ) );
  if ( renderer )
  {
    mStyle = std::move( renderer );
  }
  refreshItemSize();
  emit changed();
}

QString QgsLayoutItemScaleBar::style() const
{
  if ( mStyle )
  {
    return mStyle->id();
  }
  else
  {
    return QString();
  }
}

const QgsNumericFormat *QgsLayoutItemScaleBar::numericFormat() const
{
  return mSettings.numericFormat();
}

void QgsLayoutItemScaleBar::setNumericFormat( QgsNumericFormat *format )
{
  mSettings.setNumericFormat( format );
}

QFont QgsLayoutItemScaleBar::font() const
{
  return mSettings.textFormat().font();
}

void QgsLayoutItemScaleBar::setFont( const QFont &font )
{
  Q_NOWARN_DEPRECATED_PUSH
  mSettings.setFont( font );
  Q_NOWARN_DEPRECATED_POP
  refreshItemSize();
  emit changed();
}

QColor QgsLayoutItemScaleBar::fontColor() const
{
  QColor color = mSettings.textFormat().color();
  color.setAlphaF( mSettings.textFormat().opacity() );
  return color;
}

void QgsLayoutItemScaleBar::setFontColor( const QColor &color )
{
  mSettings.textFormat().setColor( color );
  mSettings.textFormat().setOpacity( color.alphaF() );
}

QColor QgsLayoutItemScaleBar::fillColor() const
{
  Q_NOWARN_DEPRECATED_PUSH
  return mSettings.fillColor();
  Q_NOWARN_DEPRECATED_POP
}

void QgsLayoutItemScaleBar::setFillColor( const QColor &color )
{
  Q_NOWARN_DEPRECATED_PUSH
  mSettings.setFillColor( color );
  Q_NOWARN_DEPRECATED_POP
}

QColor QgsLayoutItemScaleBar::fillColor2() const
{
  Q_NOWARN_DEPRECATED_PUSH
  return mSettings.fillColor2();
  Q_NOWARN_DEPRECATED_POP
}

void QgsLayoutItemScaleBar::setFillColor2( const QColor &color )
{
  Q_NOWARN_DEPRECATED_PUSH
  mSettings.setFillColor2( color );
  Q_NOWARN_DEPRECATED_POP
}

QColor QgsLayoutItemScaleBar::lineColor() const
{
  Q_NOWARN_DEPRECATED_PUSH
  return mSettings.lineColor();
  Q_NOWARN_DEPRECATED_POP
}

void QgsLayoutItemScaleBar::setLineColor( const QColor &color )
{
  Q_NOWARN_DEPRECATED_PUSH
  mSettings.setLineColor( color );
  Q_NOWARN_DEPRECATED_POP
}

double QgsLayoutItemScaleBar::lineWidth() const
{
  Q_NOWARN_DEPRECATED_PUSH
  return mSettings.lineWidth();
  Q_NOWARN_DEPRECATED_POP
}

void QgsLayoutItemScaleBar::setLineWidth( double width )
{
  Q_NOWARN_DEPRECATED_PUSH
  mSettings.setLineWidth( width );
  Q_NOWARN_DEPRECATED_POP
}

QPen QgsLayoutItemScaleBar::pen() const
{
  Q_NOWARN_DEPRECATED_PUSH
  return mSettings.pen();
  Q_NOWARN_DEPRECATED_POP
}

QBrush QgsLayoutItemScaleBar::brush() const
{
  Q_NOWARN_DEPRECATED_PUSH
  return mSettings.brush();
  Q_NOWARN_DEPRECATED_POP
}

QBrush QgsLayoutItemScaleBar::brush2() const
{
  Q_NOWARN_DEPRECATED_PUSH
  return mSettings.brush2();
  Q_NOWARN_DEPRECATED_POP
}

bool QgsLayoutItemScaleBar::writePropertiesToElement( QDomElement &composerScaleBarElem, QDomDocument &doc, const QgsReadWriteContext &rwContext ) const
{
  composerScaleBarElem.setAttribute( u"height"_s, QString::number( mSettings.height() ) );
  composerScaleBarElem.setAttribute( u"labelBarSpace"_s, QString::number( mSettings.labelBarSpace() ) );
  composerScaleBarElem.setAttribute( u"boxContentSpace"_s, QString::number( mSettings.boxContentSpace() ) );
  composerScaleBarElem.setAttribute( u"numSegments"_s, mSettings.numberOfSegments() );
  composerScaleBarElem.setAttribute( u"numSegmentsLeft"_s, mSettings.numberOfSegmentsLeft() );
  composerScaleBarElem.setAttribute( u"numSubdivisions"_s, mSettings.numberOfSubdivisions() );
  composerScaleBarElem.setAttribute( u"subdivisionsHeight"_s, mSettings.subdivisionsHeight() );
  composerScaleBarElem.setAttribute( u"numUnitsPerSegment"_s, QString::number( mSettings.unitsPerSegment() ) );
  composerScaleBarElem.setAttribute( u"segmentSizeMode"_s, static_cast< int >( mSettings.segmentSizeMode() ) );
  composerScaleBarElem.setAttribute( u"minBarWidth"_s, mSettings.minimumBarWidth() );
  composerScaleBarElem.setAttribute( u"maxBarWidth"_s, mSettings.maximumBarWidth() );
  composerScaleBarElem.setAttribute( u"segmentMillimeters"_s, QString::number( mSegmentMillimeters ) );
  composerScaleBarElem.setAttribute( u"numMapUnitsPerScaleBarUnit"_s, QString::number( mSettings.mapUnitsPerScaleBarUnit() ) );
  composerScaleBarElem.setAttribute( u"method"_s, qgsEnumValueToKey( mMethod ) );

  const QDomElement textElem = mSettings.textFormat().writeXml( doc, rwContext );
  composerScaleBarElem.appendChild( textElem );

  Q_NOWARN_DEPRECATED_PUSH
  // kept just for allowing projects to open in QGIS < 3.14, remove for 4.0
  composerScaleBarElem.setAttribute( u"outlineWidth"_s, QString::number( mSettings.lineWidth() ) );
  composerScaleBarElem.setAttribute( u"lineJoinStyle"_s, QgsSymbolLayerUtils::encodePenJoinStyle( mSettings.lineJoinStyle() ) );
  composerScaleBarElem.setAttribute( u"lineCapStyle"_s, QgsSymbolLayerUtils::encodePenCapStyle( mSettings.lineCapStyle() ) );
  //pen color
  QDomElement strokeColorElem = doc.createElement( u"strokeColor"_s );
  strokeColorElem.setAttribute( u"red"_s, QString::number( mSettings.lineColor().red() ) );
  strokeColorElem.setAttribute( u"green"_s, QString::number( mSettings.lineColor().green() ) );
  strokeColorElem.setAttribute( u"blue"_s, QString::number( mSettings.lineColor().blue() ) );
  strokeColorElem.setAttribute( u"alpha"_s, QString::number( mSettings.lineColor().alpha() ) );
  composerScaleBarElem.appendChild( strokeColorElem );
  Q_NOWARN_DEPRECATED_POP

  composerScaleBarElem.setAttribute( u"unitLabel"_s, mSettings.unitLabel() );
  composerScaleBarElem.setAttribute( u"unitType"_s, QgsUnitTypes::encodeUnit( mSettings.units() ) );

  QDomElement numericFormatElem = doc.createElement( u"numericFormat"_s );
  mSettings.numericFormat()->writeXml( numericFormatElem, doc, rwContext );
  composerScaleBarElem.appendChild( numericFormatElem );

  //style
  if ( mStyle )
  {
    composerScaleBarElem.setAttribute( u"style"_s, mStyle->id() );
  }

  //map id
  if ( mMap )
  {
    composerScaleBarElem.setAttribute( u"mapUuid"_s, mMap->uuid() );
  }

  //colors

  Q_NOWARN_DEPRECATED_PUSH
  // kept just for allowing projects to open in QGIS < 3.14, remove for 4.0

  //fill color
  QDomElement fillColorElem = doc.createElement( u"fillColor"_s );
  fillColorElem.setAttribute( u"red"_s, QString::number( mSettings.fillColor().red() ) );
  fillColorElem.setAttribute( u"green"_s, QString::number( mSettings.fillColor().green() ) );
  fillColorElem.setAttribute( u"blue"_s, QString::number( mSettings.fillColor().blue() ) );
  fillColorElem.setAttribute( u"alpha"_s, QString::number( mSettings.fillColor().alpha() ) );
  composerScaleBarElem.appendChild( fillColorElem );

  //fill color 2
  QDomElement fillColor2Elem = doc.createElement( u"fillColor2"_s );
  fillColor2Elem.setAttribute( u"red"_s, QString::number( mSettings.fillColor2().red() ) );
  fillColor2Elem.setAttribute( u"green"_s, QString::number( mSettings.fillColor2().green() ) );
  fillColor2Elem.setAttribute( u"blue"_s, QString::number( mSettings.fillColor2().blue() ) );
  fillColor2Elem.setAttribute( u"alpha"_s, QString::number( mSettings.fillColor2().alpha() ) );
  composerScaleBarElem.appendChild( fillColor2Elem );

  Q_NOWARN_DEPRECATED_POP

  //label vertical/horizontal placement
  composerScaleBarElem.setAttribute( u"labelVerticalPlacement"_s, QString::number( static_cast< int >( mSettings.labelVerticalPlacement() ) ) );
  composerScaleBarElem.setAttribute( u"labelHorizontalPlacement"_s, QString::number( static_cast< int >( mSettings.labelHorizontalPlacement() ) ) );

  //alignment
  composerScaleBarElem.setAttribute( u"alignment"_s, QString::number( static_cast< int >( mSettings.alignment() ) ) );

  QDomElement lineSymbol = doc.createElement( u"lineSymbol"_s );
  const QDomElement symbolElem = QgsSymbolLayerUtils::saveSymbol( QString(),
                                 mSettings.lineSymbol(),
                                 doc,
                                 rwContext );
  lineSymbol.appendChild( symbolElem );
  composerScaleBarElem.appendChild( lineSymbol );

  QDomElement divisionSymbol = doc.createElement( u"divisionLineSymbol"_s );
  const QDomElement divisionSymbolElem = QgsSymbolLayerUtils::saveSymbol( QString(),
                                         mSettings.divisionLineSymbol(),
                                         doc,
                                         rwContext );
  divisionSymbol.appendChild( divisionSymbolElem );
  composerScaleBarElem.appendChild( divisionSymbol );

  QDomElement subdivisionSymbol = doc.createElement( u"subdivisionLineSymbol"_s );
  const QDomElement subdivisionSymbolElem = QgsSymbolLayerUtils::saveSymbol( QString(),
      mSettings.subdivisionLineSymbol(),
      doc,
      rwContext );
  subdivisionSymbol.appendChild( subdivisionSymbolElem );
  composerScaleBarElem.appendChild( subdivisionSymbol );

  QDomElement fillSymbol1Elem = doc.createElement( u"fillSymbol1"_s );
  const QDomElement symbol1Elem = QgsSymbolLayerUtils::saveSymbol( QString(),
                                  mSettings.fillSymbol(),
                                  doc,
                                  rwContext );
  fillSymbol1Elem.appendChild( symbol1Elem );
  composerScaleBarElem.appendChild( fillSymbol1Elem );

  QDomElement fillSymbol2Elem = doc.createElement( u"fillSymbol2"_s );
  const QDomElement symbol2Elem = QgsSymbolLayerUtils::saveSymbol( QString(),
                                  mSettings.alternateFillSymbol(),
                                  doc,
                                  rwContext );
  fillSymbol2Elem.appendChild( symbol2Elem );
  composerScaleBarElem.appendChild( fillSymbol2Elem );

  return true;
}


bool QgsLayoutItemScaleBar::readPropertiesFromElement( const QDomElement &itemElem, const QDomDocument &, const QgsReadWriteContext &context )
{
  mSettings.setHeight( itemElem.attribute( u"height"_s, u"5.0"_s ).toDouble() );
  mSettings.setLabelBarSpace( itemElem.attribute( u"labelBarSpace"_s, u"3.0"_s ).toDouble() );
  mSettings.setBoxContentSpace( itemElem.attribute( u"boxContentSpace"_s, u"1.0"_s ).toDouble() );
  mSettings.setNumberOfSegments( itemElem.attribute( u"numSegments"_s, u"2"_s ).toInt() );
  mSettings.setNumberOfSegmentsLeft( itemElem.attribute( u"numSegmentsLeft"_s, u"0"_s ).toInt() );
  mSettings.setNumberOfSubdivisions( itemElem.attribute( u"numSubdivisions"_s, u"1"_s ).toInt() );
  mSettings.setSubdivisionsHeight( itemElem.attribute( u"subdivisionsHeight"_s, u"1.5"_s ).toDouble() );
  mSettings.setUnitsPerSegment( itemElem.attribute( u"numUnitsPerSegment"_s, u"1.0"_s ).toDouble() );
  mSettings.setSegmentSizeMode( static_cast<Qgis::ScaleBarSegmentSizeMode >( itemElem.attribute( u"segmentSizeMode"_s, u"0"_s ).toInt() ) );
  mSettings.setMinimumBarWidth( itemElem.attribute( u"minBarWidth"_s, u"50"_s ).toDouble() );
  mSettings.setMaximumBarWidth( itemElem.attribute( u"maxBarWidth"_s, u"150"_s ).toDouble() );
  mSegmentMillimeters = itemElem.attribute( u"segmentMillimeters"_s, u"0.0"_s ).toDouble();
  mSettings.setMapUnitsPerScaleBarUnit( itemElem.attribute( u"numMapUnitsPerScaleBarUnit"_s, u"1.0"_s ).toDouble() );

  // default to horizontal bottom to keep same behavior for older projects
  mMethod = qgsEnumKeyToValue( itemElem.attribute( u"method"_s ), Qgis::ScaleCalculationMethod::HorizontalBottom );

  const QDomElement lineSymbolElem = itemElem.firstChildElement( u"lineSymbol"_s );
  bool foundLineSymbol = false;
  if ( !lineSymbolElem.isNull() )
  {
    const QDomElement symbolElem = lineSymbolElem.firstChildElement( u"symbol"_s );
    std::unique_ptr< QgsLineSymbol > lineSymbol( QgsSymbolLayerUtils::loadSymbol<QgsLineSymbol>( symbolElem, context ) );
    if ( lineSymbol )
    {
      mSettings.setLineSymbol( lineSymbol.release() );
      foundLineSymbol = true;
    }
  }
  const QDomElement divisionSymbolElem = itemElem.firstChildElement( u"divisionLineSymbol"_s );
  if ( !divisionSymbolElem.isNull() )
  {
    const QDomElement symbolElem = divisionSymbolElem.firstChildElement( u"symbol"_s );
    std::unique_ptr< QgsLineSymbol > lineSymbol( QgsSymbolLayerUtils::loadSymbol<QgsLineSymbol>( symbolElem, context ) );
    if ( lineSymbol )
    {
      mSettings.setDivisionLineSymbol( lineSymbol.release() );
    }
  }
  else if ( foundLineSymbol )
  {
    mSettings.setDivisionLineSymbol( mSettings.lineSymbol()->clone() );
  }
  const QDomElement subdivisionSymbolElem = itemElem.firstChildElement( u"subdivisionLineSymbol"_s );
  if ( !subdivisionSymbolElem.isNull() )
  {
    const QDomElement symbolElem = subdivisionSymbolElem.firstChildElement( u"symbol"_s );
    std::unique_ptr< QgsLineSymbol > lineSymbol( QgsSymbolLayerUtils::loadSymbol<QgsLineSymbol>( symbolElem, context ) );
    if ( lineSymbol )
    {
      mSettings.setSubdivisionLineSymbol( lineSymbol.release() );
    }
  }
  else if ( foundLineSymbol )
  {
    mSettings.setSubdivisionLineSymbol( mSettings.lineSymbol()->clone() );
  }

  if ( !foundLineSymbol )
  {
    // old project compatibility
    auto lineSymbol = std::make_unique< QgsLineSymbol >();
    auto lineSymbolLayer = std::make_unique< QgsSimpleLineSymbolLayer >();
    lineSymbolLayer->setWidth( itemElem.attribute( u"outlineWidth"_s, u"0.3"_s ).toDouble() );
    lineSymbolLayer->setWidthUnit( Qgis::RenderUnit::Millimeters );
    lineSymbolLayer->setPenJoinStyle( QgsSymbolLayerUtils::decodePenJoinStyle( itemElem.attribute( u"lineJoinStyle"_s, u"miter"_s ) ) );
    lineSymbolLayer->setPenCapStyle( QgsSymbolLayerUtils::decodePenCapStyle( itemElem.attribute( u"lineCapStyle"_s, u"square"_s ) ) );

    //stroke color
    const QDomNodeList strokeColorList = itemElem.elementsByTagName( u"strokeColor"_s );
    if ( !strokeColorList.isEmpty() )
    {
      const QDomElement strokeColorElem = strokeColorList.at( 0 ).toElement();
      bool redOk, greenOk, blueOk, alphaOk;
      int strokeRed, strokeGreen, strokeBlue, strokeAlpha;

      strokeRed = strokeColorElem.attribute( u"red"_s ).toDouble( &redOk );
      strokeGreen = strokeColorElem.attribute( u"green"_s ).toDouble( &greenOk );
      strokeBlue = strokeColorElem.attribute( u"blue"_s ).toDouble( &blueOk );
      strokeAlpha = strokeColorElem.attribute( u"alpha"_s ).toDouble( &alphaOk );

      if ( redOk && greenOk && blueOk && alphaOk )
      {
        lineSymbolLayer->setColor( QColor( strokeRed, strokeGreen, strokeBlue, strokeAlpha ) );
      }
    }
    else
    {
      lineSymbolLayer->setColor( QColor( itemElem.attribute( u"penColor"_s, u"#000000"_s ) ) );
    }

    // need to translate the deprecated ScalebarLineWidth and ScalebarLineColor properties to symbol properties,
    // and then remove them from the scalebar so they don't interfere and apply to other compatibility workarounds
    lineSymbolLayer->setDataDefinedProperty( QgsSymbolLayer::Property::StrokeWidth, dataDefinedProperties().property( QgsLayoutObject::DataDefinedProperty::ScalebarLineWidth ) );
    dataDefinedProperties().setProperty( QgsLayoutObject::DataDefinedProperty::ScalebarLineWidth, QgsProperty() );
    lineSymbolLayer->setDataDefinedProperty( QgsSymbolLayer::Property::StrokeColor, dataDefinedProperties().property( QgsLayoutObject::DataDefinedProperty::ScalebarLineColor ) );
    dataDefinedProperties().setProperty( QgsLayoutObject::DataDefinedProperty::ScalebarLineColor, QgsProperty() );

    lineSymbol->changeSymbolLayer( 0, lineSymbolLayer.release() );
    mSettings.setLineSymbol( lineSymbol->clone() );
    mSettings.setDivisionLineSymbol( lineSymbol->clone() );
    mSettings.setSubdivisionLineSymbol( lineSymbol.release() );
  }

  mSettings.setUnitLabel( itemElem.attribute( u"unitLabel"_s ) );

  const QDomNodeList textFormatNodeList = itemElem.elementsByTagName( u"text-style"_s );
  if ( !textFormatNodeList.isEmpty() )
  {
    const QDomElement textFormatElem = textFormatNodeList.at( 0 ).toElement();
    mSettings.textFormat().readXml( textFormatElem, context );
  }
  else
  {
    QFont f;
    if ( !QgsFontUtils::setFromXmlChildNode( f, itemElem, u"scaleBarFont"_s ) )
    {
      f.fromString( itemElem.attribute( u"font"_s, QString() ) );
    }
    mSettings.textFormat().setFont( f );
    if ( f.pointSizeF() > 0 )
    {
      mSettings.textFormat().setSize( f.pointSizeF() );
      mSettings.textFormat().setSizeUnit( Qgis::RenderUnit::Points );
    }
    else if ( f.pixelSize() > 0 )
    {
      mSettings.textFormat().setSize( f.pixelSize() );
      mSettings.textFormat().setSizeUnit( Qgis::RenderUnit::Pixels );
    }
  }

  const QDomNodeList numericFormatNodeList = itemElem.elementsByTagName( u"numericFormat"_s );
  if ( !numericFormatNodeList.isEmpty() )
  {
    const QDomElement numericFormatElem = numericFormatNodeList.at( 0 ).toElement();
    mSettings.setNumericFormat( QgsApplication::numericFormatRegistry()->createFromXml( numericFormatElem, context ) );
  }

  const QDomElement fillSymbol1Elem = itemElem.firstChildElement( u"fillSymbol1"_s );
  bool foundFillSymbol1 = false;
  if ( !fillSymbol1Elem.isNull() )
  {
    const QDomElement symbolElem = fillSymbol1Elem.firstChildElement( u"symbol"_s );
    std::unique_ptr< QgsFillSymbol > fillSymbol( QgsSymbolLayerUtils::loadSymbol<QgsFillSymbol>( symbolElem, context ) );
    if ( fillSymbol )
    {
      mSettings.setFillSymbol( fillSymbol.release() );
      foundFillSymbol1 = true;
    }
  }
  if ( !foundFillSymbol1 )
  {
    // old project compatibility
    auto fillSymbol = std::make_unique< QgsFillSymbol >();
    auto fillSymbolLayer = std::make_unique< QgsSimpleFillSymbolLayer >();
    fillSymbolLayer->setStrokeStyle( Qt::NoPen );

    //fill color
    const QDomNodeList fillColorList = itemElem.elementsByTagName( u"fillColor"_s );
    if ( !fillColorList.isEmpty() )
    {
      const QDomElement fillColorElem = fillColorList.at( 0 ).toElement();
      bool redOk, greenOk, blueOk, alphaOk;
      int fillRed, fillGreen, fillBlue, fillAlpha;

      fillRed = fillColorElem.attribute( u"red"_s ).toDouble( &redOk );
      fillGreen = fillColorElem.attribute( u"green"_s ).toDouble( &greenOk );
      fillBlue = fillColorElem.attribute( u"blue"_s ).toDouble( &blueOk );
      fillAlpha = fillColorElem.attribute( u"alpha"_s ).toDouble( &alphaOk );

      if ( redOk && greenOk && blueOk && alphaOk )
      {
        fillSymbolLayer->setColor( QColor( fillRed, fillGreen, fillBlue, fillAlpha ) );
      }
    }
    else
    {
      fillSymbolLayer->setColor( QColor( itemElem.attribute( u"brushColor"_s, u"#000000"_s ) ) );
    }

    // need to translate the deprecated ScalebarFillColor property to symbol properties,
    // and then remove them from the scalebar so they don't interfere and apply to other compatibility workarounds
    fillSymbolLayer->setDataDefinedProperty( QgsSymbolLayer::Property::FillColor, dataDefinedProperties().property( QgsLayoutObject::DataDefinedProperty::ScalebarFillColor ) );
    dataDefinedProperties().setProperty( QgsLayoutObject::DataDefinedProperty::ScalebarFillColor, QgsProperty() );

    fillSymbol->changeSymbolLayer( 0, fillSymbolLayer.release() );
    mSettings.setFillSymbol( fillSymbol.release() );
  }

  const QDomElement fillSymbol2Elem = itemElem.firstChildElement( u"fillSymbol2"_s );
  bool foundFillSymbol2 = false;
  if ( !fillSymbol2Elem.isNull() )
  {
    const QDomElement symbolElem = fillSymbol2Elem.firstChildElement( u"symbol"_s );
    std::unique_ptr< QgsFillSymbol > fillSymbol( QgsSymbolLayerUtils::loadSymbol<QgsFillSymbol>( symbolElem, context ) );
    if ( fillSymbol )
    {
      mSettings.setAlternateFillSymbol( fillSymbol.release() );
      foundFillSymbol2 = true;
    }
  }
  if ( !foundFillSymbol2 )
  {
    // old project compatibility
    auto fillSymbol = std::make_unique< QgsFillSymbol >();
    auto fillSymbolLayer = std::make_unique< QgsSimpleFillSymbolLayer >();
    fillSymbolLayer->setStrokeStyle( Qt::NoPen );

    //fill color 2

    const QDomNodeList fillColor2List = itemElem.elementsByTagName( u"fillColor2"_s );
    if ( !fillColor2List.isEmpty() )
    {
      const QDomElement fillColor2Elem = fillColor2List.at( 0 ).toElement();
      bool redOk, greenOk, blueOk, alphaOk;
      int fillRed, fillGreen, fillBlue, fillAlpha;

      fillRed = fillColor2Elem.attribute( u"red"_s ).toDouble( &redOk );
      fillGreen = fillColor2Elem.attribute( u"green"_s ).toDouble( &greenOk );
      fillBlue = fillColor2Elem.attribute( u"blue"_s ).toDouble( &blueOk );
      fillAlpha = fillColor2Elem.attribute( u"alpha"_s ).toDouble( &alphaOk );

      if ( redOk && greenOk && blueOk && alphaOk )
      {
        fillSymbolLayer->setColor( QColor( fillRed, fillGreen, fillBlue, fillAlpha ) );
      }
    }
    else
    {
      fillSymbolLayer->setColor( QColor( itemElem.attribute( u"brush2Color"_s, u"#ffffff"_s ) ) );
    }

    // need to translate the deprecated ScalebarFillColor2 property to symbol properties,
    // and then remove them from the scalebar so they don't interfere and apply to other compatibility workarounds
    fillSymbolLayer->setDataDefinedProperty( QgsSymbolLayer::Property::FillColor, dataDefinedProperties().property( QgsLayoutObject::DataDefinedProperty::ScalebarFillColor2 ) );
    dataDefinedProperties().setProperty( QgsLayoutObject::DataDefinedProperty::ScalebarFillColor2, QgsProperty() );

    fillSymbol->changeSymbolLayer( 0, fillSymbolLayer.release() );
    mSettings.setAlternateFillSymbol( fillSymbol.release() );

  }

  //font color
  const QDomNodeList textColorList = itemElem.elementsByTagName( u"textColor"_s );
  if ( !textColorList.isEmpty() )
  {
    const QDomElement textColorElem = textColorList.at( 0 ).toElement();
    bool redOk, greenOk, blueOk, alphaOk;
    int textRed, textGreen, textBlue, textAlpha;

    textRed = textColorElem.attribute( u"red"_s ).toDouble( &redOk );
    textGreen = textColorElem.attribute( u"green"_s ).toDouble( &greenOk );
    textBlue = textColorElem.attribute( u"blue"_s ).toDouble( &blueOk );
    textAlpha = textColorElem.attribute( u"alpha"_s ).toDouble( &alphaOk );

    if ( redOk && greenOk && blueOk && alphaOk )
    {
      mSettings.textFormat().setColor( QColor( textRed, textGreen, textBlue, textAlpha ) );
    }
  }
  else if ( itemElem.hasAttribute( u"fontColor"_s ) )
  {
    QColor c;
    c.setNamedColor( itemElem.attribute( u"fontColor"_s, u"#000000"_s ) );
    mSettings.textFormat().setColor( c );
  }

  //style
  setStyle( itemElem.attribute( u"style"_s, QString() ) );

  //call attemptResize after setStyle to ensure the appropriate size limitations are applied
  attemptResize( QgsLayoutSize::decodeSize( itemElem.attribute( u"size"_s ) ) );

  if ( itemElem.attribute( u"unitType"_s ).isEmpty() )
  {
    Qgis::DistanceUnit u = Qgis::DistanceUnit::Unknown;
    switch ( itemElem.attribute( u"units"_s ).toInt() )
    {
      case 0:
        u = Qgis::DistanceUnit::Unknown;
        break;
      case 1:
        u = Qgis::DistanceUnit::Meters;
        break;
      case 2:
        u = Qgis::DistanceUnit::Feet;
        break;
      case 3:
        u = Qgis::DistanceUnit::NauticalMiles;
        break;
    }
    mSettings.setUnits( u );
  }
  else
  {
    mSettings.setUnits( QgsUnitTypes::decodeDistanceUnit( itemElem.attribute( u"unitType"_s ) ) );
  }

  mSettings.setLabelVerticalPlacement( static_cast< Qgis::ScaleBarDistanceLabelVerticalPlacement >( itemElem.attribute( u"labelVerticalPlacement"_s, u"0"_s ).toInt() ) );
  mSettings.setLabelHorizontalPlacement( static_cast< Qgis::ScaleBarDistanceLabelHorizontalPlacement >( itemElem.attribute( u"labelHorizontalPlacement"_s, u"0"_s ).toInt() ) );

  mSettings.setAlignment( static_cast< Qgis::ScaleBarAlignment >( itemElem.attribute( u"alignment"_s, u"0"_s ).toInt() ) );

  //map
  disconnectCurrentMap();
  mMap = nullptr;
  mMapUuid = itemElem.attribute( u"mapUuid"_s );
  return true;
}


void QgsLayoutItemScaleBar::finalizeRestoreFromXml()
{
  if ( mLayout && !mMapUuid.isEmpty() )
  {
    disconnectCurrentMap();
    mMap = qobject_cast< QgsLayoutItemMap * >( mLayout->itemByUuid( mMapUuid, true ) );
    if ( mMap )
    {
      connect( mMap, &QgsLayoutItemMap::extentChanged, this, &QgsLayoutItemScaleBar::updateScale );
      connect( mMap, &QObject::destroyed, this, &QgsLayoutItemScaleBar::disconnectCurrentMap );
    }
  }

  updateScale();
}

bool QgsLayoutItemScaleBar::accept( QgsStyleEntityVisitorInterface *visitor ) const
{
  QgsStyleTextFormatEntity entity( mSettings.textFormat() );
  if ( !visitor->visit( QgsStyleEntityVisitorInterface::StyleLeaf( &entity, uuid(), displayName() ) ) )
    return false;

  return true;
}

QgsLayoutItem::ExportLayerBehavior QgsLayoutItemScaleBar::exportLayerBehavior() const
{
  return CanGroupWithItemsOfSameType;
}
