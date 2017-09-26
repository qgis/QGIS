/***************************************************************************
                           qgscomposerscalebar.cpp
                             -------------------
    begin                : March 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposerscalebar.h"
#include "qgscomposermap.h"
#include "qgscomposition.h"
#include "qgscomposerutils.h"
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

QgsComposerScaleBar::QgsComposerScaleBar( QgsComposition *composition )
  : QgsComposerItem( composition )
  , mSegmentMillimeters( 0.0 )
{
  applyDefaultSettings();
  applyDefaultSize();
}

QgsComposerScaleBar::~QgsComposerScaleBar()
{
  delete mStyle;
}

void QgsComposerScaleBar::paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget )
{
  Q_UNUSED( itemStyle );
  Q_UNUSED( pWidget );
  if ( !mStyle || !painter )
  {
    return;
  }
  if ( !shouldDrawItem() )
  {
    return;
  }

  drawBackground( painter );

  QgsRenderContext c = QgsComposerUtils::createRenderContextForMap( mComposerMap, painter );

  mStyle->draw( c, mSettings, createScaleContext() );

  //draw frame and selection boxes if necessary
  drawFrame( painter );
  if ( isSelected() )
  {
    drawSelectionBoxes( painter );
  }
}

void QgsComposerScaleBar::setNumSegments( int nSegments )
{
  if ( !mStyle )
  {
    mSettings.setNumberOfSegments( nSegments );
    return;
  }
  double width = mStyle->calculateBoxSize( mSettings, createScaleContext() ).width();
  mSettings.setNumberOfSegments( nSegments );
  double widthAfter = mStyle->calculateBoxSize( mSettings, createScaleContext() ).width();
  correctXPositionAlignment( width, widthAfter );
  emit itemChanged();
}

void QgsComposerScaleBar::setNumUnitsPerSegment( double units )
{
  if ( !mStyle )
  {
    mSettings.setUnitsPerSegment( units );
    return;
  }
  double width = mStyle->calculateBoxSize( mSettings, createScaleContext() ).width();
  mSettings.setUnitsPerSegment( units );
  refreshSegmentMillimeters();
  double widthAfter = mStyle->calculateBoxSize( mSettings, createScaleContext() ).width();
  correctXPositionAlignment( width, widthAfter );
  emit itemChanged();
}

void QgsComposerScaleBar::setSegmentSizeMode( QgsScaleBarSettings::SegmentSizeMode mode )
{
  if ( !mStyle )
  {
    mSettings.setSegmentSizeMode( mode );
    return;
  }
  double width = mStyle->calculateBoxSize( mSettings, createScaleContext() ).width();
  mSettings.setSegmentSizeMode( mode );
  refreshSegmentMillimeters();
  double widthAfter = mStyle->calculateBoxSize( mSettings, createScaleContext() ).width();
  correctXPositionAlignment( width, widthAfter );
  emit itemChanged();
}

void QgsComposerScaleBar::setMinBarWidth( double minWidth )
{
  if ( !mStyle )
  {
    mSettings.setMinimumBarWidth( minWidth );
    return;
  }
  double width = mStyle->calculateBoxSize( mSettings, createScaleContext() ).width();
  mSettings.setMinimumBarWidth( minWidth );
  refreshSegmentMillimeters();
  double widthAfter = mStyle->calculateBoxSize( mSettings, createScaleContext() ).width();
  correctXPositionAlignment( width, widthAfter );
  emit itemChanged();
}

void QgsComposerScaleBar::setMaxBarWidth( double maxWidth )
{
  if ( !mStyle )
  {
    mSettings.setMaximumBarWidth( maxWidth );
    return;
  }
  double width = mStyle->calculateBoxSize( mSettings, createScaleContext() ).width();
  mSettings.setMaximumBarWidth( maxWidth );
  refreshSegmentMillimeters();
  double widthAfter = mStyle->calculateBoxSize( mSettings, createScaleContext() ).width();
  correctXPositionAlignment( width, widthAfter );
  emit itemChanged();
}

void QgsComposerScaleBar::setNumSegmentsLeft( int nSegmentsLeft )
{
  if ( !mStyle )
  {
    mSettings.setNumberOfSegmentsLeft( nSegmentsLeft );
    return;
  }
  double width = mStyle->calculateBoxSize( mSettings, createScaleContext() ).width();
  mSettings.setNumberOfSegmentsLeft( nSegmentsLeft );
  double widthAfter = mStyle->calculateBoxSize( mSettings, createScaleContext() ).width();
  correctXPositionAlignment( width, widthAfter );
  emit itemChanged();
}

void QgsComposerScaleBar::setBoxContentSpace( double space )
{
  if ( !mStyle )
  {
    mSettings.setBoxContentSpace( space );
    return;
  }
  double width = mStyle->calculateBoxSize( mSettings, createScaleContext() ).width();
  mSettings.setBoxContentSpace( space );
  double widthAfter = mStyle->calculateBoxSize( mSettings, createScaleContext() ).width();
  correctXPositionAlignment( width, widthAfter );
  emit itemChanged();
}

void QgsComposerScaleBar::setComposerMap( QgsComposerMap *map )
{
  if ( mComposerMap )
  {
    disconnect( mComposerMap, &QgsComposerMap::extentChanged, this, &QgsComposerScaleBar::updateSegmentSize );
    disconnect( mComposerMap, &QObject::destroyed, this, &QgsComposerScaleBar::invalidateCurrentMap );
  }
  mComposerMap = map;

  if ( !map )
  {
    return;
  }

  connect( mComposerMap, &QgsComposerMap::extentChanged, this, &QgsComposerScaleBar::updateSegmentSize );
  connect( mComposerMap, &QObject::destroyed, this, &QgsComposerScaleBar::invalidateCurrentMap );

  refreshSegmentMillimeters();
  emit itemChanged();
}

void QgsComposerScaleBar::invalidateCurrentMap()
{
  if ( !mComposerMap )
  {
    return;
  }

  disconnect( mComposerMap, &QgsComposerMap::extentChanged, this, &QgsComposerScaleBar::updateSegmentSize );
  disconnect( mComposerMap, &QObject::destroyed, this, &QgsComposerScaleBar::invalidateCurrentMap );
  mComposerMap = nullptr;
}

void QgsComposerScaleBar::refreshDataDefinedProperty( const QgsComposerObject::DataDefinedProperty property, const QgsExpressionContext *context )
{
  QgsExpressionContext scopedContext = createExpressionContext();
  const QgsExpressionContext *evalContext = context ? context : &scopedContext;

  bool forceUpdate = false;
  //updates data defined properties and redraws item to match
  if ( property == QgsComposerObject::ScalebarFillColor || property == QgsComposerObject::AllProperties )
  {
    QBrush b = mSettings.brush();
    b.setColor( mDataDefinedProperties.valueAsColor( QgsComposerObject::ScalebarFillColor, *evalContext, mSettings.fillColor() ) );
    mSettings.setBrush( b );
    forceUpdate = true;
  }
  if ( property == QgsComposerObject::ScalebarFillColor2 || property == QgsComposerObject::AllProperties )
  {
    QBrush b = mSettings.brush2();
    b.setColor( mDataDefinedProperties.valueAsColor( QgsComposerObject::ScalebarFillColor2, *evalContext, mSettings.fillColor2() ) );
    mSettings.setBrush2( b );
    forceUpdate = true;
  }
  if ( property == QgsComposerObject::ScalebarLineColor || property == QgsComposerObject::AllProperties )
  {
    QPen p = mSettings.pen();
    p.setColor( mDataDefinedProperties.valueAsColor( QgsComposerObject::ScalebarLineColor, *evalContext, mSettings.lineColor() ) );
    mSettings.setPen( p );
    forceUpdate = true;
  }
  if ( property == QgsComposerObject::ScalebarLineWidth || property == QgsComposerObject::AllProperties )
  {
    QPen p = mSettings.pen();
    p.setWidthF( mDataDefinedProperties.valueAsDouble( QgsComposerObject::ScalebarLineWidth, *evalContext, mSettings.lineWidth() ) );
    mSettings.setPen( p );
    forceUpdate = true;
  }
  if ( forceUpdate )
  {
    update();
  }

  QgsComposerObject::refreshDataDefinedProperty( property, context );
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

void QgsComposerScaleBar::refreshSegmentMillimeters()
{
  if ( mComposerMap )
  {
    //get mm dimension of composer map
    QRectF composerItemRect = mComposerMap->rect();

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
        // lowerNiceUnitsPerSeg nor upperNiceUnitsPerSeg are are in
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

double QgsComposerScaleBar::mapWidth() const
{
  if ( !mComposerMap )
  {
    return 0.0;
  }

  QgsRectangle composerMapRect = *( mComposerMap->currentMapExtent() );
  if ( mSettings.units() == QgsUnitTypes::DistanceUnknownUnit )
  {
    return composerMapRect.width();
  }
  else
  {
    QgsDistanceArea da;
    da.setSourceCrs( mComposerMap->crs() );
    da.setEllipsoid( mComposition->project()->ellipsoid() );

    QgsUnitTypes::DistanceUnit units = da.lengthUnits();
    double measure = da.measureLine( QgsPointXY( composerMapRect.xMinimum(), composerMapRect.yMinimum() ),
                                     QgsPointXY( composerMapRect.xMaximum(), composerMapRect.yMinimum() ) );
    measure /= QgsUnitTypes::fromUnitToUnitFactor( mSettings.units(), units );
    return measure;
  }
}

QgsScaleBarRenderer::ScaleBarContext QgsComposerScaleBar::createScaleContext() const
{
  QgsScaleBarRenderer::ScaleBarContext scaleContext;
  scaleContext.size = rect().size();
  scaleContext.segmentWidth = mSegmentMillimeters;
  scaleContext.scale = mComposerMap ? mComposerMap->scale() : 1.0;
  return scaleContext;
}

void QgsComposerScaleBar::setAlignment( QgsScaleBarSettings::Alignment a )
{
  mSettings.setAlignment( a );
  update();
  emit itemChanged();
}

void QgsComposerScaleBar::setUnits( QgsUnitTypes::DistanceUnit u )
{
  mSettings.setUnits( u );
  refreshSegmentMillimeters();
  emit itemChanged();
}

void QgsComposerScaleBar::setLineJoinStyle( Qt::PenJoinStyle style )
{
  if ( mSettings.lineJoinStyle() == style )
  {
    //no change
    return;
  }
  mSettings.setLineJoinStyle( style );
  update();
  emit itemChanged();
}

void QgsComposerScaleBar::setLineCapStyle( Qt::PenCapStyle style )
{
  if ( mSettings.lineCapStyle() == style )
  {
    //no change
    return;
  }
  mSettings.setLineCapStyle( style );
  update();
  emit itemChanged();
}

void QgsComposerScaleBar::applyDefaultSettings()
{
  //style
  delete mStyle;
  mStyle = new QgsSingleBoxScaleBarRenderer();

  //default to no background
  setBackgroundEnabled( false );

  //get default composer font from settings
  QgsSettings settings;
  QString defaultFontString = settings.value( QStringLiteral( "Composer/defaultFont" ) ).toString();
  QFont f;
  if ( !defaultFontString.isEmpty() )
  {
    f.setFamily( defaultFontString );
  }
  f.setPointSizeF( 12.0 );
  mSettings.setFont( f );

  mSettings.setUnits( QgsUnitTypes::DistanceUnknownUnit );

  emit itemChanged();
}

void QgsComposerScaleBar::applyDefaultSize( QgsUnitTypes::DistanceUnit u )
{
  if ( mComposerMap )
  {
    setUnits( u );
    double upperMagnitudeMultiplier = 1.0;
    double widthInSelectedUnits = mapWidth();
    double initialUnitsPerSegment = widthInSelectedUnits / 10.0; //default scalebar width equals half the map width
    setNumUnitsPerSegment( initialUnitsPerSegment );

    switch ( u )
    {
      case QgsUnitTypes::DistanceUnknownUnit:
      {
        upperMagnitudeMultiplier = 1.0;
        setUnitLabeling( tr( "units" ) );
        break;
      }
      case QgsUnitTypes::DistanceMeters:
      {
        if ( initialUnitsPerSegment > 1000.0 )
        {
          upperMagnitudeMultiplier = 1000.0;
          setUnitLabeling( tr( "km" ) );
        }
        else
        {
          upperMagnitudeMultiplier = 1.0;
          setUnitLabeling( tr( "m" ) );
        }
        break;
      }
      case QgsUnitTypes::DistanceFeet:
      {
        if ( initialUnitsPerSegment > 5419.95 )
        {
          upperMagnitudeMultiplier = 5419.95;
          setUnitLabeling( tr( "miles" ) );
        }
        else
        {
          upperMagnitudeMultiplier = 1.0;
          setUnitLabeling( tr( "ft" ) );
        }
        break;
      }

      default:
        setUnitLabeling( QgsUnitTypes::toAbbreviatedString( u ) );
        upperMagnitudeMultiplier = 1;
        break;
    }

    double segmentWidth = initialUnitsPerSegment / upperMagnitudeMultiplier;
    int segmentMagnitude = std::floor( std::log10( segmentWidth ) );
    double unitsPerSegment = upperMagnitudeMultiplier * ( std::pow( 10.0, segmentMagnitude ) );
    double multiplier = std::floor( ( widthInSelectedUnits / ( unitsPerSegment * 10.0 ) ) / 2.5 ) * 2.5;

    if ( multiplier > 0 )
    {
      unitsPerSegment = unitsPerSegment * multiplier;
    }
    setNumUnitsPerSegment( unitsPerSegment );
    setNumMapUnitsPerScaleBarUnit( upperMagnitudeMultiplier );

    setNumSegments( 4 );
    setNumSegmentsLeft( 2 );
  }

  refreshSegmentMillimeters();
  adjustBoxSize();
  emit itemChanged();
}

void QgsComposerScaleBar::adjustBoxSize()
{
  if ( !mStyle )
  {
    return;
  }

  QRectF box = QRectF( pos(), mStyle->calculateBoxSize( mSettings, createScaleContext() ) );
  if ( rect().height() > box.height() )
  {
    //keep user specified item height if higher than minimum scale bar height
    box.setHeight( rect().height() );
  }

  //update rect for data defined size and position
  QRectF newRect = evalItemRect( box, true );

  //scale bars have a minimum size, respect that regardless of data defined settings
  if ( newRect.width() < box.width() )
  {
    newRect.setWidth( box.width() );
  }
  if ( newRect.height() < box.height() )
  {
    newRect.setHeight( box.height() );
  }

  QgsComposerItem::setSceneRect( newRect );
}

void QgsComposerScaleBar::setSceneRect( const QRectF &rectangle )
{
  QRectF box = QRectF( pos(), mStyle->calculateBoxSize( mSettings, createScaleContext() ) );
  if ( rectangle.height() > box.height() )
  {
    //keep user specified item height if higher than minimum scale bar height
    box.setHeight( rectangle.height() );
  }
  box.moveTopLeft( rectangle.topLeft() );

  //update rect for data defined size and position
  QRectF newRect = evalItemRect( rectangle );

  //scale bars have a minimum size, respect that regardless of data defined settings
  if ( newRect.width() < box.width() )
  {
    newRect.setWidth( box.width() );
  }
  if ( newRect.height() < box.height() )
  {
    newRect.setHeight( box.height() );
  }

  QgsComposerItem::setSceneRect( newRect );
}

void QgsComposerScaleBar::update()
{
  //Don't adjust box size for numeric scale bars:
  if ( mStyle && mStyle->name() != QLatin1String( "Numeric" ) )
  {
    adjustBoxSize();
  }
  QgsComposerItem::update();
}

void QgsComposerScaleBar::updateSegmentSize()
{
  if ( !mStyle )
  {
    return;
  }
  double width = mStyle->calculateBoxSize( mSettings, createScaleContext() ).width();
  refreshSegmentMillimeters();
  double widthAfter = mStyle->calculateBoxSize( mSettings, createScaleContext() ).width();
  correctXPositionAlignment( width, widthAfter );
  update();
  emit itemChanged();
}

void QgsComposerScaleBar::setStyle( const QString &styleName )
{
  delete mStyle;
  mStyle = nullptr;

  //switch depending on style name
  if ( styleName == QLatin1String( "Single Box" ) )
  {
    mStyle = new QgsSingleBoxScaleBarRenderer();
  }
  else if ( styleName == QLatin1String( "Double Box" ) )
  {
    mStyle = new QgsDoubleBoxScaleBarRenderer();
  }
  else if ( styleName == QLatin1String( "Line Ticks Middle" )  || styleName == QLatin1String( "Line Ticks Down" ) || styleName == QLatin1String( "Line Ticks Up" ) )
  {
    QgsTicksScaleBarRenderer *tickStyle = new QgsTicksScaleBarRenderer();
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
    mStyle = tickStyle;
  }
  else if ( styleName == QLatin1String( "Numeric" ) )
  {
    mStyle = new QgsNumericScaleBarRenderer();
  }
  emit itemChanged();
}

QString QgsComposerScaleBar::style() const
{
  if ( mStyle )
  {
    return mStyle->name();
  }
  else
  {
    return QLatin1String( "" );
  }
}

QFont QgsComposerScaleBar::font() const
{
  return mSettings.font();
}

void QgsComposerScaleBar::setFont( const QFont &font )
{
  mSettings.setFont( font );
  update();
  emit itemChanged();
}

bool QgsComposerScaleBar::writeXml( QDomElement &elem, QDomDocument &doc ) const
{
  if ( elem.isNull() )
  {
    return false;
  }

  QDomElement composerScaleBarElem = doc.createElement( QStringLiteral( "ComposerScaleBar" ) );
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
  composerScaleBarElem.appendChild( QgsFontUtils::toXmlElement( mSettings.font(), doc, QStringLiteral( "scaleBarFont" ) ) );
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
  if ( mComposerMap )
  {
    composerScaleBarElem.setAttribute( QStringLiteral( "mapId" ), mComposerMap->id() );
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

  //font color
  QDomElement fontColorElem = doc.createElement( QStringLiteral( "textColor" ) );
  fontColorElem.setAttribute( QStringLiteral( "red" ), QString::number( mSettings.fontColor().red() ) );
  fontColorElem.setAttribute( QStringLiteral( "green" ), QString::number( mSettings.fontColor().green() ) );
  fontColorElem.setAttribute( QStringLiteral( "blue" ), QString::number( mSettings.fontColor().blue() ) );
  fontColorElem.setAttribute( QStringLiteral( "alpha" ), QString::number( mSettings.fontColor().alpha() ) );
  composerScaleBarElem.appendChild( fontColorElem );

  //alignment
  composerScaleBarElem.setAttribute( QStringLiteral( "alignment" ), QString::number( static_cast< int >( mSettings.alignment() ) ) );

  elem.appendChild( composerScaleBarElem );
  return _writeXml( composerScaleBarElem, doc );
}

bool QgsComposerScaleBar::readXml( const QDomElement &itemElem, const QDomDocument &doc )
{
  if ( itemElem.isNull() )
  {
    return false;
  }

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
  QFont f;
  if ( !QgsFontUtils::setFromXmlChildNode( f, itemElem, QStringLiteral( "scaleBarFont" ) ) )
  {
    f.fromString( itemElem.attribute( QStringLiteral( "font" ), QLatin1String( "" ) ) );
  }
  mSettings.setFont( f );

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
      mSettings.setFontColor( QColor( textRed, textGreen, textBlue, textAlpha ) );
    }
  }
  else
  {
    QColor c;
    c.setNamedColor( itemElem.attribute( QStringLiteral( "fontColor" ), QStringLiteral( "#000000" ) ) );
    mSettings.setFontColor( c );
  }

  //style
  delete mStyle;
  mStyle = nullptr;
  QString styleString = itemElem.attribute( QStringLiteral( "style" ), QLatin1String( "" ) );
  setStyle( tr( styleString.toLocal8Bit().data() ) );

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
  int mapId = itemElem.attribute( QStringLiteral( "mapId" ), QStringLiteral( "-1" ) ).toInt();
  if ( mapId >= 0 )
  {
    const QgsComposerMap *composerMap = mComposition->getComposerMapById( mapId );
    mComposerMap = const_cast< QgsComposerMap *>( composerMap );
    if ( mComposerMap )
    {
      connect( mComposerMap, &QgsComposerMap::extentChanged, this, &QgsComposerScaleBar::updateSegmentSize );
      connect( mComposerMap, &QObject::destroyed, this, &QgsComposerScaleBar::invalidateCurrentMap );
    }
  }

  updateSegmentSize();

  //restore general composer item properties
  QDomNodeList composerItemList = itemElem.elementsByTagName( QStringLiteral( "ComposerItem" ) );
  if ( !composerItemList.isEmpty() )
  {
    QDomElement composerItemElem = composerItemList.at( 0 ).toElement();
    _readXml( composerItemElem, doc );
  }

  return true;
}

void QgsComposerScaleBar::correctXPositionAlignment( double width, double widthAfter )
{
  //Don't adjust position for numeric scale bars:
  if ( mStyle->name() == QLatin1String( "Numeric" ) )
  {
    return;
  }

  if ( mSettings.alignment() == QgsScaleBarSettings::AlignMiddle )
  {
    move( -( widthAfter - width ) / 2.0, 0 );
  }
  else if ( mSettings.alignment() == QgsScaleBarSettings::AlignRight )
  {
    move( -( widthAfter - width ), 0 );
  }
}

