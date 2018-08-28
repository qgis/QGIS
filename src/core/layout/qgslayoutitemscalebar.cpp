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
#include "qgslayoutitemregistry.h"
#include "qgslayoutitemmap.h"
#include "qgslayout.h"
#include "qgslayoututils.h"
#include "qgsdistancearea.h"
#include "qgsscalebarrenderer.h"
#include "qgsdoubleboxscalebarrenderer.h"
#include "qgsmapsettings.h"
#include "qgsnumericscalebarrenderer.h"
#include "qgssingleboxscalebarrenderer.h"
#include "qgsticksscalebarrenderer.h"
#include "qgsrectangle.h"
#include "qgsproject.h"
#include "qgssymbollayerutils.h"
#include "qgsfontutils.h"
#include "qgsunittypes.h"
#include "qgssettings.h"

#include <QDomDocument>
#include <QDomElement>
#include <QFontMetricsF>
#include <QPainter>

#include <cmath>

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
  return QgsApplication::getThemeIcon( QStringLiteral( "/mLayoutItemScaleBar.svg" ) );
}

QgsLayoutItemScaleBar *QgsLayoutItemScaleBar::create( QgsLayout *layout )
{
  return new QgsLayoutItemScaleBar( layout );
}

QgsLayoutSize QgsLayoutItemScaleBar::minimumSize() const
{
  return QgsLayoutSize( mStyle->calculateBoxSize( mSettings, createScaleContext() ), QgsUnitTypes::LayoutMillimeters );
}

void QgsLayoutItemScaleBar::draw( QgsLayoutItemRenderContext &context )
{
  if ( !mStyle )
    return;

  mStyle->draw( context.renderContext(), mSettings, createScaleContext() );
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

void QgsLayoutItemScaleBar::setSegmentSizeMode( QgsScaleBarSettings::SegmentSizeMode mode )
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

void QgsLayoutItemScaleBar::refreshDataDefinedProperty( const QgsLayoutObject::DataDefinedProperty property )
{
  QgsExpressionContext context = createExpressionContext();

  bool forceUpdate = false;
  //updates data defined properties and redraws item to match
  if ( property == QgsLayoutObject::ScalebarFillColor || property == QgsLayoutObject::AllProperties )
  {
    QBrush b = mSettings.brush();
    b.setColor( mDataDefinedProperties.valueAsColor( QgsLayoutObject::ScalebarFillColor, context, mSettings.fillColor() ) );
    mSettings.setBrush( b );
    forceUpdate = true;
  }
  if ( property == QgsLayoutObject::ScalebarFillColor2 || property == QgsLayoutObject::AllProperties )
  {
    QBrush b = mSettings.brush2();
    b.setColor( mDataDefinedProperties.valueAsColor( QgsLayoutObject::ScalebarFillColor2, context, mSettings.fillColor2() ) );
    mSettings.setBrush2( b );
    forceUpdate = true;
  }
  if ( property == QgsLayoutObject::ScalebarLineColor || property == QgsLayoutObject::AllProperties )
  {
    QPen p = mSettings.pen();
    p.setColor( mDataDefinedProperties.valueAsColor( QgsLayoutObject::ScalebarLineColor, context, mSettings.lineColor() ) );
    mSettings.setPen( p );
    forceUpdate = true;
  }
  if ( property == QgsLayoutObject::ScalebarLineWidth || property == QgsLayoutObject::AllProperties )
  {
    QPen p = mSettings.pen();
    p.setWidthF( mDataDefinedProperties.valueAsDouble( QgsLayoutObject::ScalebarLineWidth, context, mSettings.lineWidth() ) );
    mSettings.setPen( p );
    forceUpdate = true;
  }
  if ( forceUpdate )
  {
    refreshItemSize();
    update();
  }

  QgsLayoutItem::refreshDataDefinedProperty( property );
}

// nextNiceNumber(4573.23, d) = 5000 (d=1) -> 4600 (d=10) -> 4580 (d=100) -> 4574 (d=1000) -> etc
inline double nextNiceNumber( double a, double d = 1 )
{
  double s = std::pow( 10.0, std::floor( std::log10( a ) ) ) / d;
  return std::ceil( a / s ) * s;
}

// prevNiceNumber(4573.23, d) = 4000 (d=1) -> 4500 (d=10) -> 4570 (d=100) -> 4573 (d=1000) -> etc
inline double prevNiceNumber( double a, double d = 1 )
{
  double s = std::pow( 10.0, std::floor( std::log10( a ) ) ) / d;
  return std::floor( a / s ) * s;
}

void QgsLayoutItemScaleBar::refreshSegmentMillimeters()
{
  if ( mMap )
  {
    //get mm dimension of composer map
    QRectF composerItemRect = mMap->rect();

    if ( mSettings.segmentSizeMode() == QgsScaleBarSettings::SegmentSizeFixed )
    {
      //calculate size depending on mNumUnitsPerSegment
      mSegmentMillimeters = composerItemRect.width() / mapWidth() * mSettings.unitsPerSegment();
    }
    else /*if(mSegmentSizeMode == SegmentSizeFitWidth)*/
    {
      if ( mSettings.maximumBarWidth() < mSettings.minimumBarWidth() )
      {
        mSegmentMillimeters = 0;
      }
      else
      {
        double nSegments = ( mSettings.numberOfSegmentsLeft() != 0 ) + mSettings.numberOfSegments();
        // unitsPerSegments which fit minBarWidth resp. maxBarWidth
        double minUnitsPerSeg = ( mSettings.minimumBarWidth() * mapWidth() ) / ( nSegments * composerItemRect.width() );
        double maxUnitsPerSeg = ( mSettings.maximumBarWidth() * mapWidth() ) / ( nSegments * composerItemRect.width() );

        // Start with coarsest "nice" number closest to minUnitsPerSeg resp
        // maxUnitsPerSeg, then proceed to finer numbers as long as neither
        // lowerNiceUnitsPerSeg nor upperNiceUnitsPerSeg are in
        // [minUnitsPerSeg, maxUnitsPerSeg]
        double lowerNiceUnitsPerSeg = nextNiceNumber( minUnitsPerSeg );
        double upperNiceUnitsPerSeg = prevNiceNumber( maxUnitsPerSeg );

        double d = 1;
        while ( lowerNiceUnitsPerSeg > maxUnitsPerSeg && upperNiceUnitsPerSeg < minUnitsPerSeg )
        {
          d *= 10;
          lowerNiceUnitsPerSeg = nextNiceNumber( minUnitsPerSeg, d );
          upperNiceUnitsPerSeg = prevNiceNumber( maxUnitsPerSeg, d );
        }

        // Pick mNumUnitsPerSegment from {lowerNiceUnitsPerSeg, upperNiceUnitsPerSeg}, use the larger if possible
        mSettings.setUnitsPerSegment( upperNiceUnitsPerSeg < minUnitsPerSeg ? lowerNiceUnitsPerSeg : upperNiceUnitsPerSeg );
        mSegmentMillimeters = composerItemRect.width() / mapWidth() * mSettings.unitsPerSegment();
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

  QgsRectangle mapExtent = mMap->extent();
  if ( mSettings.units() == QgsUnitTypes::DistanceUnknownUnit )
  {
    return mapExtent.width();
  }
  else
  {
    QgsDistanceArea da;
    da.setSourceCrs( mMap->crs(), mLayout->project()->transformContext() );
    da.setEllipsoid( mLayout->project()->ellipsoid() );

    QgsUnitTypes::DistanceUnit units = da.lengthUnits();
    double measure = da.measureLine( QgsPointXY( mapExtent.xMinimum(), mapExtent.yMinimum() ),
                                     QgsPointXY( mapExtent.xMaximum(), mapExtent.yMinimum() ) );
    measure /= QgsUnitTypes::fromUnitToUnitFactor( mSettings.units(), units );
    return measure;
  }
}

QgsScaleBarRenderer::ScaleBarContext QgsLayoutItemScaleBar::createScaleContext() const
{
  QgsScaleBarRenderer::ScaleBarContext scaleContext;
  scaleContext.size = rect().size();
  scaleContext.segmentWidth = mSegmentMillimeters;
  scaleContext.scale = mMap ? mMap->scale() : 1.0;
  return scaleContext;
}

void QgsLayoutItemScaleBar::setAlignment( QgsScaleBarSettings::Alignment a )
{
  mSettings.setAlignment( a );
  refreshItemSize();
  emit changed();
}

void QgsLayoutItemScaleBar::setUnits( QgsUnitTypes::DistanceUnit u )
{
  mSettings.setUnits( u );
  refreshSegmentMillimeters();
  refreshItemSize();
  emit changed();
}

void QgsLayoutItemScaleBar::setLineJoinStyle( Qt::PenJoinStyle style )
{
  if ( mSettings.lineJoinStyle() == style )
  {
    //no change
    return;
  }
  mSettings.setLineJoinStyle( style );
  update();
  emit changed();
}

void QgsLayoutItemScaleBar::setLineCapStyle( Qt::PenCapStyle style )
{
  if ( mSettings.lineCapStyle() == style )
  {
    //no change
    return;
  }
  mSettings.setLineCapStyle( style );
  update();
  emit changed();
}

void QgsLayoutItemScaleBar::applyDefaultSettings()
{
  //style
  mStyle = qgis::make_unique< QgsSingleBoxScaleBarRenderer >();

  //default to no background
  setBackgroundEnabled( false );

  //get default composer font from settings
  QgsSettings settings;
  QString defaultFontString = settings.value( QStringLiteral( "LayoutDesigner/defaultFont" ), QVariant(), QgsSettings::Gui ).toString();
  QgsTextFormat format;
  QFont f;
  if ( !defaultFontString.isEmpty() )
  {
    f.setFamily( defaultFontString );
  }
  format.setFont( f );
  format.setSize( 12.0 );
  format.setSizeUnit( QgsUnitTypes::RenderPoints );

  mSettings.setTextFormat( format );

  mSettings.setUnits( QgsUnitTypes::DistanceUnknownUnit );
  refreshItemSize();

  emit changed();
}

QgsUnitTypes::DistanceUnit QgsLayoutItemScaleBar::guessUnits() const
{
  if ( !mMap )
    return QgsUnitTypes::DistanceMeters;

  QgsCoordinateReferenceSystem crs = mMap->crs();
  // start with crs units
  QgsUnitTypes::DistanceUnit unit = crs.mapUnits();
  if ( unit == QgsUnitTypes::DistanceDegrees || unit == QgsUnitTypes::DistanceUnknownUnit )
  {
    // geographic CRS, use metric units
    unit = QgsUnitTypes::DistanceMeters;
  }

  // try to pick reasonable choice between metric / imperial units
  double widthInSelectedUnits = mapWidth();
  double initialUnitsPerSegment = widthInSelectedUnits / 10.0; //default scalebar width equals half the map width
  switch ( unit )
  {
    case QgsUnitTypes::DistanceMeters:
    {
      if ( initialUnitsPerSegment > 1000.0 )
      {
        unit = QgsUnitTypes::DistanceKilometers;
      }
      break;
    }
    case QgsUnitTypes::DistanceFeet:
    {
      if ( initialUnitsPerSegment > 5419.95 )
      {
        unit = QgsUnitTypes::DistanceMiles;
      }
      break;
    }
    default:
      break;
  }

  return unit;
}

void QgsLayoutItemScaleBar::applyDefaultSize( QgsUnitTypes::DistanceUnit units )
{
  mSettings.setUnits( units );
  if ( mMap )
  {
    double upperMagnitudeMultiplier = 1.0;
    double widthInSelectedUnits = mapWidth();
    double initialUnitsPerSegment = widthInSelectedUnits / 10.0; //default scalebar width equals half the map width
    mSettings.setUnitsPerSegment( initialUnitsPerSegment );

    setUnitLabel( QgsUnitTypes::toAbbreviatedString( units ) );
    upperMagnitudeMultiplier = 1;

    double segmentWidth = initialUnitsPerSegment / upperMagnitudeMultiplier;
    int segmentMagnitude = std::floor( std::log10( segmentWidth ) );
    double unitsPerSegment = upperMagnitudeMultiplier * ( std::pow( 10.0, segmentMagnitude ) );
    double multiplier = std::floor( ( widthInSelectedUnits / ( unitsPerSegment * 10.0 ) ) / 2.5 ) * 2.5;

    if ( multiplier > 0 )
    {
      unitsPerSegment = unitsPerSegment * multiplier;
    }
    mSettings.setUnitsPerSegment( unitsPerSegment );
    mSettings.setMapUnitsPerScaleBarUnit( upperMagnitudeMultiplier );

    mSettings.setNumberOfSegments( 2 );
    mSettings.setNumberOfSegmentsLeft( 0 );
  }

  refreshSegmentMillimeters();
  resizeToMinimumWidth();
  emit changed();
}

void QgsLayoutItemScaleBar::resizeToMinimumWidth()
{
  if ( !mStyle )
    return;

  double widthMM = mStyle->calculateBoxSize( mSettings, createScaleContext() ).width();
  QgsLayoutSize currentSize = sizeWithUnits();
  currentSize.setWidth( mLayout->renderContext().measurementConverter().convert( QgsLayoutMeasurement( widthMM, QgsUnitTypes::LayoutMillimeters ), currentSize.units() ).length() );
  attemptResize( currentSize );
  update();
  emit changed();
}

void QgsLayoutItemScaleBar::update()
{
  //Don't adjust box size for numeric scale bars:
  if ( mStyle && mStyle->name() != QLatin1String( "Numeric" ) )
  {
    refreshItemSize();
  }
  QgsLayoutItem::update();
}

void QgsLayoutItemScaleBar::updateScale()
{
  refreshSegmentMillimeters();
  resizeToMinimumWidth();
  update();
}

void QgsLayoutItemScaleBar::setStyle( const QString &styleName )
{
  //switch depending on style name
  if ( styleName == QLatin1String( "Single Box" ) )
  {
    mStyle = qgis::make_unique< QgsSingleBoxScaleBarRenderer >();
  }
  else if ( styleName == QLatin1String( "Double Box" ) )
  {
    mStyle = qgis::make_unique< QgsDoubleBoxScaleBarRenderer >();
  }
  else if ( styleName == QLatin1String( "Line Ticks Middle" )  || styleName == QLatin1String( "Line Ticks Down" ) || styleName == QLatin1String( "Line Ticks Up" ) )
  {
    std::unique_ptr< QgsTicksScaleBarRenderer > tickStyle = qgis::make_unique< QgsTicksScaleBarRenderer >();
    if ( styleName == QLatin1String( "Line Ticks Middle" ) )
    {
      tickStyle->setTickPosition( QgsTicksScaleBarRenderer::TicksMiddle );
    }
    else if ( styleName == QLatin1String( "Line Ticks Down" ) )
    {
      tickStyle->setTickPosition( QgsTicksScaleBarRenderer::TicksDown );
    }
    else if ( styleName == QLatin1String( "Line Ticks Up" ) )
    {
      tickStyle->setTickPosition( QgsTicksScaleBarRenderer::TicksUp );
    }
    mStyle = std::move( tickStyle );
  }
  else if ( styleName == QLatin1String( "Numeric" ) )
  {
    mStyle = qgis::make_unique< QgsNumericScaleBarRenderer >();
  }
  refreshItemSize();
  emit changed();
}

QString QgsLayoutItemScaleBar::style() const
{
  if ( mStyle )
  {
    return mStyle->name();
  }
  else
  {
    return QString();
  }
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

bool QgsLayoutItemScaleBar::writePropertiesToElement( QDomElement &composerScaleBarElem, QDomDocument &doc, const QgsReadWriteContext &rwContext ) const
{
  composerScaleBarElem.setAttribute( QStringLiteral( "height" ), QString::number( mSettings.height() ) );
  composerScaleBarElem.setAttribute( QStringLiteral( "labelBarSpace" ), QString::number( mSettings.labelBarSpace() ) );
  composerScaleBarElem.setAttribute( QStringLiteral( "boxContentSpace" ), QString::number( mSettings.boxContentSpace() ) );
  composerScaleBarElem.setAttribute( QStringLiteral( "numSegments" ), mSettings.numberOfSegments() );
  composerScaleBarElem.setAttribute( QStringLiteral( "numSegmentsLeft" ), mSettings.numberOfSegmentsLeft() );
  composerScaleBarElem.setAttribute( QStringLiteral( "numUnitsPerSegment" ), QString::number( mSettings.unitsPerSegment() ) );
  composerScaleBarElem.setAttribute( QStringLiteral( "segmentSizeMode" ), mSettings.segmentSizeMode() );
  composerScaleBarElem.setAttribute( QStringLiteral( "minBarWidth" ), mSettings.minimumBarWidth() );
  composerScaleBarElem.setAttribute( QStringLiteral( "maxBarWidth" ), mSettings.maximumBarWidth() );
  composerScaleBarElem.setAttribute( QStringLiteral( "segmentMillimeters" ), QString::number( mSegmentMillimeters ) );
  composerScaleBarElem.setAttribute( QStringLiteral( "numMapUnitsPerScaleBarUnit" ), QString::number( mSettings.mapUnitsPerScaleBarUnit() ) );

  QDomElement textElem = mSettings.textFormat().writeXml( doc, rwContext );
  composerScaleBarElem.appendChild( textElem );

  composerScaleBarElem.setAttribute( QStringLiteral( "outlineWidth" ), QString::number( mSettings.lineWidth() ) );
  composerScaleBarElem.setAttribute( QStringLiteral( "unitLabel" ), mSettings.unitLabel() );
  composerScaleBarElem.setAttribute( QStringLiteral( "unitType" ), QgsUnitTypes::encodeUnit( mSettings.units() ) );
  composerScaleBarElem.setAttribute( QStringLiteral( "lineJoinStyle" ), QgsSymbolLayerUtils::encodePenJoinStyle( mSettings.lineJoinStyle() ) );
  composerScaleBarElem.setAttribute( QStringLiteral( "lineCapStyle" ), QgsSymbolLayerUtils::encodePenCapStyle( mSettings.lineCapStyle() ) );

  //style
  if ( mStyle )
  {
    composerScaleBarElem.setAttribute( QStringLiteral( "style" ), mStyle->name() );
  }

  //map id
  if ( mMap )
  {
    composerScaleBarElem.setAttribute( QStringLiteral( "mapUuid" ), mMap->uuid() );
  }

  //colors

  //fill color
  QDomElement fillColorElem = doc.createElement( QStringLiteral( "fillColor" ) );
  fillColorElem.setAttribute( QStringLiteral( "red" ), QString::number( mSettings.fillColor().red() ) );
  fillColorElem.setAttribute( QStringLiteral( "green" ), QString::number( mSettings.fillColor().green() ) );
  fillColorElem.setAttribute( QStringLiteral( "blue" ), QString::number( mSettings.fillColor().blue() ) );
  fillColorElem.setAttribute( QStringLiteral( "alpha" ), QString::number( mSettings.fillColor().alpha() ) );
  composerScaleBarElem.appendChild( fillColorElem );

  //fill color 2
  QDomElement fillColor2Elem = doc.createElement( QStringLiteral( "fillColor2" ) );
  fillColor2Elem.setAttribute( QStringLiteral( "red" ), QString::number( mSettings.fillColor2().red() ) );
  fillColor2Elem.setAttribute( QStringLiteral( "green" ), QString::number( mSettings.fillColor2().green() ) );
  fillColor2Elem.setAttribute( QStringLiteral( "blue" ), QString::number( mSettings.fillColor2().blue() ) );
  fillColor2Elem.setAttribute( QStringLiteral( "alpha" ), QString::number( mSettings.fillColor2().alpha() ) );
  composerScaleBarElem.appendChild( fillColor2Elem );

  //pen color
  QDomElement strokeColorElem = doc.createElement( QStringLiteral( "strokeColor" ) );
  strokeColorElem.setAttribute( QStringLiteral( "red" ), QString::number( mSettings.lineColor().red() ) );
  strokeColorElem.setAttribute( QStringLiteral( "green" ), QString::number( mSettings.lineColor().green() ) );
  strokeColorElem.setAttribute( QStringLiteral( "blue" ), QString::number( mSettings.lineColor().blue() ) );
  strokeColorElem.setAttribute( QStringLiteral( "alpha" ), QString::number( mSettings.lineColor().alpha() ) );
  composerScaleBarElem.appendChild( strokeColorElem );

  //alignment
  composerScaleBarElem.setAttribute( QStringLiteral( "alignment" ), QString::number( static_cast< int >( mSettings.alignment() ) ) );

  return true;
}

bool QgsLayoutItemScaleBar::readPropertiesFromElement( const QDomElement &itemElem, const QDomDocument &, const QgsReadWriteContext &context )
{
  mSettings.setHeight( itemElem.attribute( QStringLiteral( "height" ), QStringLiteral( "5.0" ) ).toDouble() );
  mSettings.setLabelBarSpace( itemElem.attribute( QStringLiteral( "labelBarSpace" ), QStringLiteral( "3.0" ) ).toDouble() );
  mSettings.setBoxContentSpace( itemElem.attribute( QStringLiteral( "boxContentSpace" ), QStringLiteral( "1.0" ) ).toDouble() );
  mSettings.setNumberOfSegments( itemElem.attribute( QStringLiteral( "numSegments" ), QStringLiteral( "2" ) ).toInt() );
  mSettings.setNumberOfSegmentsLeft( itemElem.attribute( QStringLiteral( "numSegmentsLeft" ), QStringLiteral( "0" ) ).toInt() );
  mSettings.setUnitsPerSegment( itemElem.attribute( QStringLiteral( "numUnitsPerSegment" ), QStringLiteral( "1.0" ) ).toDouble() );
  mSettings.setSegmentSizeMode( static_cast<QgsScaleBarSettings::SegmentSizeMode>( itemElem.attribute( QStringLiteral( "segmentSizeMode" ), QStringLiteral( "0" ) ).toInt() ) );
  mSettings.setMinimumBarWidth( itemElem.attribute( QStringLiteral( "minBarWidth" ), QStringLiteral( "50" ) ).toDouble() );
  mSettings.setMaximumBarWidth( itemElem.attribute( QStringLiteral( "maxBarWidth" ), QStringLiteral( "150" ) ).toDouble() );
  mSegmentMillimeters = itemElem.attribute( QStringLiteral( "segmentMillimeters" ), QStringLiteral( "0.0" ) ).toDouble();
  mSettings.setMapUnitsPerScaleBarUnit( itemElem.attribute( QStringLiteral( "numMapUnitsPerScaleBarUnit" ), QStringLiteral( "1.0" ) ).toDouble() );
  mSettings.setLineWidth( itemElem.attribute( QStringLiteral( "outlineWidth" ), QStringLiteral( "0.3" ) ).toDouble() );
  mSettings.setUnitLabel( itemElem.attribute( QStringLiteral( "unitLabel" ) ) );
  mSettings.setLineJoinStyle( QgsSymbolLayerUtils::decodePenJoinStyle( itemElem.attribute( QStringLiteral( "lineJoinStyle" ), QStringLiteral( "miter" ) ) ) );
  mSettings.setLineCapStyle( QgsSymbolLayerUtils::decodePenCapStyle( itemElem.attribute( QStringLiteral( "lineCapStyle" ), QStringLiteral( "square" ) ) ) );

  QDomNodeList textFormatNodeList = itemElem.elementsByTagName( QStringLiteral( "text-style" ) );
  if ( !textFormatNodeList.isEmpty() )
  {
    QDomElement textFormatElem = textFormatNodeList.at( 0 ).toElement();
    mSettings.textFormat().readXml( textFormatElem, context );
  }
  else
  {
    QFont f;
    if ( !QgsFontUtils::setFromXmlChildNode( f, itemElem, QStringLiteral( "scaleBarFont" ) ) )
    {
      f.fromString( itemElem.attribute( QStringLiteral( "font" ), QString() ) );
    }
    mSettings.textFormat().setFont( f );
    if ( f.pointSizeF() > 0 )
    {
      mSettings.textFormat().setSize( f.pointSizeF() );
      mSettings.textFormat().setSizeUnit( QgsUnitTypes::RenderPoints );
    }
    else if ( f.pixelSize() > 0 )
    {
      mSettings.textFormat().setSize( f.pixelSize() );
      mSettings.textFormat().setSizeUnit( QgsUnitTypes::RenderPixels );
    }
  }

  //colors
  //fill color
  QDomNodeList fillColorList = itemElem.elementsByTagName( QStringLiteral( "fillColor" ) );
  if ( !fillColorList.isEmpty() )
  {
    QDomElement fillColorElem = fillColorList.at( 0 ).toElement();
    bool redOk, greenOk, blueOk, alphaOk;
    int fillRed, fillGreen, fillBlue, fillAlpha;

    fillRed = fillColorElem.attribute( QStringLiteral( "red" ) ).toDouble( &redOk );
    fillGreen = fillColorElem.attribute( QStringLiteral( "green" ) ).toDouble( &greenOk );
    fillBlue = fillColorElem.attribute( QStringLiteral( "blue" ) ).toDouble( &blueOk );
    fillAlpha = fillColorElem.attribute( QStringLiteral( "alpha" ) ).toDouble( &alphaOk );

    if ( redOk && greenOk && blueOk && alphaOk )
    {
      mSettings.setFillColor( QColor( fillRed, fillGreen, fillBlue, fillAlpha ) );
    }
  }
  else
  {
    mSettings.setFillColor( QColor( itemElem.attribute( QStringLiteral( "brushColor" ), QStringLiteral( "#000000" ) ) ) );
  }

  //fill color 2
  QDomNodeList fillColor2List = itemElem.elementsByTagName( QStringLiteral( "fillColor2" ) );
  if ( !fillColor2List.isEmpty() )
  {
    QDomElement fillColor2Elem = fillColor2List.at( 0 ).toElement();
    bool redOk, greenOk, blueOk, alphaOk;
    int fillRed, fillGreen, fillBlue, fillAlpha;

    fillRed = fillColor2Elem.attribute( QStringLiteral( "red" ) ).toDouble( &redOk );
    fillGreen = fillColor2Elem.attribute( QStringLiteral( "green" ) ).toDouble( &greenOk );
    fillBlue = fillColor2Elem.attribute( QStringLiteral( "blue" ) ).toDouble( &blueOk );
    fillAlpha = fillColor2Elem.attribute( QStringLiteral( "alpha" ) ).toDouble( &alphaOk );

    if ( redOk && greenOk && blueOk && alphaOk )
    {
      mSettings.setFillColor2( QColor( fillRed, fillGreen, fillBlue, fillAlpha ) );
    }
  }
  else
  {
    mSettings.setFillColor2( QColor( itemElem.attribute( QStringLiteral( "brush2Color" ), QStringLiteral( "#ffffff" ) ) ) );
  }

  //stroke color
  QDomNodeList strokeColorList = itemElem.elementsByTagName( QStringLiteral( "strokeColor" ) );
  if ( !strokeColorList.isEmpty() )
  {
    QDomElement strokeColorElem = strokeColorList.at( 0 ).toElement();
    bool redOk, greenOk, blueOk, alphaOk;
    int strokeRed, strokeGreen, strokeBlue, strokeAlpha;

    strokeRed = strokeColorElem.attribute( QStringLiteral( "red" ) ).toDouble( &redOk );
    strokeGreen = strokeColorElem.attribute( QStringLiteral( "green" ) ).toDouble( &greenOk );
    strokeBlue = strokeColorElem.attribute( QStringLiteral( "blue" ) ).toDouble( &blueOk );
    strokeAlpha = strokeColorElem.attribute( QStringLiteral( "alpha" ) ).toDouble( &alphaOk );

    if ( redOk && greenOk && blueOk && alphaOk )
    {
      mSettings.setLineColor( QColor( strokeRed, strokeGreen, strokeBlue, strokeAlpha ) );
      QPen p = mSettings.pen();
      p.setColor( mSettings.lineColor() );
      mSettings.setPen( p );
    }
  }
  else
  {
    mSettings.setLineColor( QColor( itemElem.attribute( QStringLiteral( "penColor" ), QStringLiteral( "#000000" ) ) ) );
    QPen p = mSettings.pen();
    p.setColor( mSettings.lineColor() );
    mSettings.setPen( p );
  }

  //font color
  QDomNodeList textColorList = itemElem.elementsByTagName( QStringLiteral( "textColor" ) );
  if ( !textColorList.isEmpty() )
  {
    QDomElement textColorElem = textColorList.at( 0 ).toElement();
    bool redOk, greenOk, blueOk, alphaOk;
    int textRed, textGreen, textBlue, textAlpha;

    textRed = textColorElem.attribute( QStringLiteral( "red" ) ).toDouble( &redOk );
    textGreen = textColorElem.attribute( QStringLiteral( "green" ) ).toDouble( &greenOk );
    textBlue = textColorElem.attribute( QStringLiteral( "blue" ) ).toDouble( &blueOk );
    textAlpha = textColorElem.attribute( QStringLiteral( "alpha" ) ).toDouble( &alphaOk );

    if ( redOk && greenOk && blueOk && alphaOk )
    {
      mSettings.textFormat().setColor( QColor( textRed, textGreen, textBlue, textAlpha ) );
    }
  }
  else if ( itemElem.hasAttribute( QStringLiteral( "fontColor" ) ) )
  {
    QColor c;
    c.setNamedColor( itemElem.attribute( QStringLiteral( "fontColor" ), QStringLiteral( "#000000" ) ) );
    mSettings.textFormat().setColor( c );
  }

  //style
  QString styleString = itemElem.attribute( QStringLiteral( "style" ), QString() );
  setStyle( styleString.toLocal8Bit().data() );

  if ( itemElem.attribute( QStringLiteral( "unitType" ) ).isEmpty() )
  {
    QgsUnitTypes::DistanceUnit u = QgsUnitTypes::DistanceUnknownUnit;
    switch ( itemElem.attribute( QStringLiteral( "units" ) ).toInt() )
    {
      case 0:
        u = QgsUnitTypes::DistanceUnknownUnit;
        break;
      case 1:
        u = QgsUnitTypes::DistanceMeters;
        break;
      case 2:
        u = QgsUnitTypes::DistanceFeet;
        break;
      case 3:
        u = QgsUnitTypes::DistanceNauticalMiles;
        break;
    }
    mSettings.setUnits( u );
  }
  else
  {
    mSettings.setUnits( QgsUnitTypes::decodeDistanceUnit( itemElem.attribute( QStringLiteral( "unitType" ) ) ) );
  }
  mSettings.setAlignment( static_cast< QgsScaleBarSettings::Alignment >( itemElem.attribute( QStringLiteral( "alignment" ), QStringLiteral( "0" ) ).toInt() ) );

  //map
  disconnectCurrentMap();
  mMap = nullptr;
  mMapUuid = itemElem.attribute( QStringLiteral( "mapUuid" ) );
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
