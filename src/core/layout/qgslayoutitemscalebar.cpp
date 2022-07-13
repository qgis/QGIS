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
#include "qgsscalebarrendererregistry.h"
#include "qgslayoutitemmap.h"
#include "qgslayout.h"
#include "qgslayoututils.h"
#include "qgsdistancearea.h"
#include "qgssingleboxscalebarrenderer.h"
#include "qgsscalebarrenderer.h"
#include "qgsmapsettings.h"
#include "qgsmessagelog.h"
#include "qgsrectangle.h"
#include "qgsproject.h"
#include "qgssymbollayerutils.h"
#include "qgsfontutils.h"
#include "qgsunittypes.h"
#include "qgssettings.h"
#include "qgsstyleentityvisitor.h"
#include "qgsnumericformat.h"
#include "qgsnumericformatregistry.h"
#include "qgslinesymbollayer.h"
#include "qgsfillsymbollayer.h"
#include "qgslinesymbol.h"
#include "qgsfillsymbol.h"

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
  QgsRenderContext context = QgsLayoutUtils::createRenderContextForLayout( mLayout, nullptr );
  return QgsLayoutSize( mStyle->calculateBoxSize( context, mSettings, createScaleContext() ), QgsUnitTypes::LayoutMillimeters );
}

void QgsLayoutItemScaleBar::draw( QgsLayoutItemRenderContext &context )
{
  if ( !mStyle )
    return;

  if ( dataDefinedProperties().isActive( QgsLayoutObject::ScalebarLineColor ) || dataDefinedProperties().isActive( QgsLayoutObject::ScalebarLineWidth ) )
  {
    // compatibility code - ScalebarLineColor and ScalebarLineWidth are deprecated
    const QgsExpressionContext expContext = createExpressionContext();
    std::unique_ptr< QgsLineSymbol > sym( mSettings.lineSymbol()->clone() );
    Q_NOWARN_DEPRECATED_PUSH
    if ( dataDefinedProperties().isActive( QgsLayoutObject::ScalebarLineWidth ) )
      sym->setWidth( mDataDefinedProperties.valueAsDouble( QgsLayoutObject::ScalebarLineWidth, expContext, mSettings.lineWidth() ) );
    if ( dataDefinedProperties().isActive( QgsLayoutObject::ScalebarLineColor ) )
      sym->setColor( mDataDefinedProperties.valueAsColor( QgsLayoutObject::ScalebarLineColor, expContext, mSettings.lineColor() ) );
    Q_NOWARN_DEPRECATED_POP
    mSettings.setLineSymbol( sym.release() );
  }
  if ( dataDefinedProperties().isActive( QgsLayoutObject::ScalebarFillColor ) )
  {
    // compatibility code - ScalebarLineColor and ScalebarLineWidth are deprecated
    const QgsExpressionContext expContext = createExpressionContext();
    std::unique_ptr< QgsFillSymbol > sym( mSettings.fillSymbol()->clone() );
    Q_NOWARN_DEPRECATED_PUSH
    sym->setColor( mDataDefinedProperties.valueAsColor( QgsLayoutObject::ScalebarFillColor, expContext, mSettings.fillColor() ) );
    Q_NOWARN_DEPRECATED_POP
    mSettings.setFillSymbol( sym.release() );
  }
  if ( dataDefinedProperties().isActive( QgsLayoutObject::ScalebarFillColor2 ) )
  {
    // compatibility code - ScalebarLineColor and ScalebarLineWidth are deprecated
    const QgsExpressionContext expContext = createExpressionContext();
    std::unique_ptr< QgsFillSymbol > sym( mSettings.alternateFillSymbol()->clone() );
    Q_NOWARN_DEPRECATED_PUSH
    sym->setColor( mDataDefinedProperties.valueAsColor( QgsLayoutObject::ScalebarFillColor2, expContext, mSettings.fillColor2() ) );
    Q_NOWARN_DEPRECATED_POP
    mSettings.setAlternateFillSymbol( sym.release() );
  }

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

void QgsLayoutItemScaleBar::refreshUnitsPerSegment( const QgsExpressionContext *context )
{
  if ( mDataDefinedProperties.isActive( QgsLayoutObject::ScalebarSegmentWidth ) )
  {
    double unitsPerSegment = mSettings.unitsPerSegment();
    bool ok = false;
    unitsPerSegment = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::ScalebarSegmentWidth, *context, unitsPerSegment, &ok );

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
  if ( mDataDefinedProperties.isActive( QgsLayoutObject::ScalebarMinimumWidth ) )
  {
    double minimumBarWidth = mSettings.minimumBarWidth();

    bool ok = false;
    minimumBarWidth = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::ScalebarMinimumWidth, *context, minimumBarWidth, &ok );

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
  if ( mDataDefinedProperties.isActive( QgsLayoutObject::ScalebarMaximumWidth ) )
  {
    double maximumBarWidth = mSettings.maximumBarWidth();

    bool ok = false;
    maximumBarWidth = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::ScalebarMaximumWidth, *context, maximumBarWidth, &ok );

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
  if ( mDataDefinedProperties.isActive( QgsLayoutObject::ScalebarLeftSegments ) )
  {
    int leftSegments = mSettings.numberOfSegmentsLeft();

    bool ok = false;
    leftSegments = mDataDefinedProperties.valueAsInt( QgsLayoutObject::ScalebarLeftSegments, *context, leftSegments, &ok );

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
  if ( mDataDefinedProperties.isActive( QgsLayoutObject::ScalebarRightSegments ) )
  {
    int rightSegments = mSettings.numberOfSegments();

    bool ok = false;
    rightSegments = mDataDefinedProperties.valueAsInt( QgsLayoutObject::ScalebarRightSegments, *context, rightSegments, &ok );

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

  if ( ( property == QgsLayoutObject::ScalebarHeight || property == QgsLayoutObject::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::ScalebarHeight ) ) )
  {
    double height = mSettings.height();

    bool ok = false;
    height = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::ScalebarHeight, context, height, &ok );

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

  if ( ( property == QgsLayoutObject::ScalebarSubdivisionHeight || property == QgsLayoutObject::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::ScalebarSubdivisionHeight ) ) )
  {
    double height = mSettings.subdivisionsHeight();

    bool ok = false;
    height = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::ScalebarSubdivisionHeight, context, height, &ok );

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

  if ( property == QgsLayoutObject::ScalebarLeftSegments || property == QgsLayoutObject::AllProperties )
  {
    refreshNumberOfSegmentsLeft( &context );
    forceUpdate = true;
  }

  if ( property == QgsLayoutObject::ScalebarRightSegments || property == QgsLayoutObject::AllProperties )
  {
    refreshNumberOfSegmentsRight( &context );
    forceUpdate = true;
  }

  if ( ( property == QgsLayoutObject::ScalebarRightSegmentSubdivisions || property == QgsLayoutObject::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::ScalebarRightSegmentSubdivisions ) ) )
  {
    int segments = mSettings.numberOfSubdivisions();

    bool ok = false;
    segments = mDataDefinedProperties.valueAsInt( QgsLayoutObject::ScalebarRightSegmentSubdivisions, context, segments, &ok );

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


  if ( property == QgsLayoutObject::ScalebarSegmentWidth || property == QgsLayoutObject::AllProperties )
  {
    refreshUnitsPerSegment( &context );
    forceUpdate = true;
  }

  if ( property == QgsLayoutObject::ScalebarMinimumWidth || property == QgsLayoutObject::AllProperties )
  {
    refreshMinimumBarWidth( &context );
    forceUpdate = true;
  }

  if ( property == QgsLayoutObject::ScalebarMaximumWidth || property == QgsLayoutObject::AllProperties )
  {
    refreshMaximumBarWidth( &context );
    forceUpdate = true;
  }

  // updates data defined properties and redraws item to match
  // -- Deprecated --
  if ( property == QgsLayoutObject::ScalebarFillColor || property == QgsLayoutObject::AllProperties )
  {
    forceUpdate = true;
  }
  if ( property == QgsLayoutObject::ScalebarFillColor2 || property == QgsLayoutObject::AllProperties )
  {
    forceUpdate = true;
  }
  if ( property == QgsLayoutObject::ScalebarLineColor || property == QgsLayoutObject::AllProperties )
  {
    forceUpdate = true;
  }
  if ( property == QgsLayoutObject::ScalebarLineWidth || property == QgsLayoutObject::AllProperties )
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

    switch ( mSettings.segmentSizeMode() )
    {
      case QgsScaleBarSettings::SegmentSizeFixed:
      {
        //calculate size depending on mNumUnitsPerSegment
        mSegmentMillimeters = composerItemRect.width() / mapWidth() * mSettings.unitsPerSegment();
        break;
      }

      case QgsScaleBarSettings::SegmentSizeFitWidth:
      {
        if ( mSettings.maximumBarWidth() < mSettings.minimumBarWidth() )
        {
          mSegmentMillimeters = 0;
        }
        else
        {
          const double nSegments = ( mSettings.numberOfSegmentsLeft() != 0 ) + mSettings.numberOfSegments();
          // unitsPerSegments which fit minBarWidth resp. maxBarWidth
          const double minUnitsPerSeg = ( mSettings.minimumBarWidth() * mapWidth() ) / ( nSegments * composerItemRect.width() );
          const double maxUnitsPerSeg = ( mSettings.maximumBarWidth() * mapWidth() ) / ( nSegments * composerItemRect.width() );
          mSettings.setUnitsPerSegment( QgsLayoutUtils::calculatePrettySize( minUnitsPerSeg, maxUnitsPerSeg ) );
          mSegmentMillimeters = composerItemRect.width() / mapWidth() * mSettings.unitsPerSegment();
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
  if ( mSettings.units() == QgsUnitTypes::DistanceUnknownUnit )
  {
    return mapExtent.width();
  }
  else
  {
    QgsDistanceArea da;
    da.setSourceCrs( mMap->crs(), mLayout->project()->transformContext() );
    da.setEllipsoid( mLayout->project()->ellipsoid() );

    const QgsUnitTypes::DistanceUnit units = da.lengthUnits();
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
  scaleContext.flags = mStyle->flags();
  return scaleContext;
}

void QgsLayoutItemScaleBar::setLabelVerticalPlacement( QgsScaleBarSettings::LabelVerticalPlacement placement )
{
  mSettings.setLabelVerticalPlacement( placement );
  refreshItemSize();
  emit changed();
}

void QgsLayoutItemScaleBar::setLabelHorizontalPlacement( QgsScaleBarSettings::LabelHorizontalPlacement placement )
{
  mSettings.setLabelHorizontalPlacement( placement );
  refreshItemSize();
  emit changed();
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
  const QString defaultFontString = settings.value( QStringLiteral( "LayoutDesigner/defaultFont" ), QVariant(), QgsSettings::Gui ).toString();
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

bool QgsLayoutItemScaleBar::applyDefaultRendererSettings( QgsScaleBarRenderer *renderer )
{
  return renderer->applyDefaultSettings( mSettings );
}

QgsUnitTypes::DistanceUnit QgsLayoutItemScaleBar::guessUnits() const
{
  if ( !mMap )
    return QgsUnitTypes::DistanceMeters;

  const QgsCoordinateReferenceSystem crs = mMap->crs();
  // start with crs units
  QgsUnitTypes::DistanceUnit unit = crs.mapUnits();
  if ( unit == QgsUnitTypes::DistanceDegrees || unit == QgsUnitTypes::DistanceUnknownUnit )
  {
    // geographic CRS, use metric units
    unit = QgsUnitTypes::DistanceMeters;
  }

  // try to pick reasonable choice between metric / imperial units
  const double widthInSelectedUnits = mapWidth();
  const double initialUnitsPerSegment = widthInSelectedUnits / 10.0; //default scalebar width equals half the map width
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
    const double widthInSelectedUnits = mapWidth();
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

  refreshSegmentMillimeters();
  resizeToMinimumWidth();
  emit changed();
}

void QgsLayoutItemScaleBar::resizeToMinimumWidth()
{
  if ( !mStyle )
    return;

  QgsRenderContext context = QgsLayoutUtils::createRenderContextForLayout( mLayout, nullptr );
  const double widthMM = mStyle->calculateBoxSize( context, mSettings, createScaleContext() ).width();
  QgsLayoutSize currentSize = sizeWithUnits();
  currentSize.setWidth( mLayout->renderContext().measurementConverter().convert( QgsLayoutMeasurement( widthMM, QgsUnitTypes::LayoutMillimeters ), currentSize.units() ).length() );
  attemptResize( currentSize );
  update();
  emit changed();
}

void QgsLayoutItemScaleBar::update()
{
  //Don't adjust box size for numeric scale bars:
  if ( mStyle && mStyle->id() != QLatin1String( "Numeric" ) )
  {
    refreshItemSize();
  }
  QgsLayoutItem::update();
}

void QgsLayoutItemScaleBar::updateScale()
{
  refreshSegmentMillimeters();
  //Don't adjust box size for numeric scale bars:
  if ( mStyle && mStyle->id() != QLatin1String( "Numeric" ) )
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
  composerScaleBarElem.setAttribute( QStringLiteral( "height" ), QString::number( mSettings.height() ) );
  composerScaleBarElem.setAttribute( QStringLiteral( "labelBarSpace" ), QString::number( mSettings.labelBarSpace() ) );
  composerScaleBarElem.setAttribute( QStringLiteral( "boxContentSpace" ), QString::number( mSettings.boxContentSpace() ) );
  composerScaleBarElem.setAttribute( QStringLiteral( "numSegments" ), mSettings.numberOfSegments() );
  composerScaleBarElem.setAttribute( QStringLiteral( "numSegmentsLeft" ), mSettings.numberOfSegmentsLeft() );
  composerScaleBarElem.setAttribute( QStringLiteral( "numSubdivisions" ), mSettings.numberOfSubdivisions() );
  composerScaleBarElem.setAttribute( QStringLiteral( "subdivisionsHeight" ), mSettings.subdivisionsHeight() );
  composerScaleBarElem.setAttribute( QStringLiteral( "numUnitsPerSegment" ), QString::number( mSettings.unitsPerSegment() ) );
  composerScaleBarElem.setAttribute( QStringLiteral( "segmentSizeMode" ), mSettings.segmentSizeMode() );
  composerScaleBarElem.setAttribute( QStringLiteral( "minBarWidth" ), mSettings.minimumBarWidth() );
  composerScaleBarElem.setAttribute( QStringLiteral( "maxBarWidth" ), mSettings.maximumBarWidth() );
  composerScaleBarElem.setAttribute( QStringLiteral( "segmentMillimeters" ), QString::number( mSegmentMillimeters ) );
  composerScaleBarElem.setAttribute( QStringLiteral( "numMapUnitsPerScaleBarUnit" ), QString::number( mSettings.mapUnitsPerScaleBarUnit() ) );

  const QDomElement textElem = mSettings.textFormat().writeXml( doc, rwContext );
  composerScaleBarElem.appendChild( textElem );

  Q_NOWARN_DEPRECATED_PUSH
  // kept just for allowing projects to open in QGIS < 3.14, remove for 4.0
  composerScaleBarElem.setAttribute( QStringLiteral( "outlineWidth" ), QString::number( mSettings.lineWidth() ) );
  composerScaleBarElem.setAttribute( QStringLiteral( "lineJoinStyle" ), QgsSymbolLayerUtils::encodePenJoinStyle( mSettings.lineJoinStyle() ) );
  composerScaleBarElem.setAttribute( QStringLiteral( "lineCapStyle" ), QgsSymbolLayerUtils::encodePenCapStyle( mSettings.lineCapStyle() ) );
  //pen color
  QDomElement strokeColorElem = doc.createElement( QStringLiteral( "strokeColor" ) );
  strokeColorElem.setAttribute( QStringLiteral( "red" ), QString::number( mSettings.lineColor().red() ) );
  strokeColorElem.setAttribute( QStringLiteral( "green" ), QString::number( mSettings.lineColor().green() ) );
  strokeColorElem.setAttribute( QStringLiteral( "blue" ), QString::number( mSettings.lineColor().blue() ) );
  strokeColorElem.setAttribute( QStringLiteral( "alpha" ), QString::number( mSettings.lineColor().alpha() ) );
  composerScaleBarElem.appendChild( strokeColorElem );
  Q_NOWARN_DEPRECATED_POP

  composerScaleBarElem.setAttribute( QStringLiteral( "unitLabel" ), mSettings.unitLabel() );
  composerScaleBarElem.setAttribute( QStringLiteral( "unitType" ), QgsUnitTypes::encodeUnit( mSettings.units() ) );

  QDomElement numericFormatElem = doc.createElement( QStringLiteral( "numericFormat" ) );
  mSettings.numericFormat()->writeXml( numericFormatElem, doc, rwContext );
  composerScaleBarElem.appendChild( numericFormatElem );

  //style
  if ( mStyle )
  {
    composerScaleBarElem.setAttribute( QStringLiteral( "style" ), mStyle->id() );
  }

  //map id
  if ( mMap )
  {
    composerScaleBarElem.setAttribute( QStringLiteral( "mapUuid" ), mMap->uuid() );
  }

  //colors

  Q_NOWARN_DEPRECATED_PUSH
  // kept just for allowing projects to open in QGIS < 3.14, remove for 4.0

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

  Q_NOWARN_DEPRECATED_POP

  //label vertical/horizontal placement
  composerScaleBarElem.setAttribute( QStringLiteral( "labelVerticalPlacement" ), QString::number( static_cast< int >( mSettings.labelVerticalPlacement() ) ) );
  composerScaleBarElem.setAttribute( QStringLiteral( "labelHorizontalPlacement" ), QString::number( static_cast< int >( mSettings.labelHorizontalPlacement() ) ) );

  //alignment
  composerScaleBarElem.setAttribute( QStringLiteral( "alignment" ), QString::number( static_cast< int >( mSettings.alignment() ) ) );

  QDomElement lineSymbol = doc.createElement( QStringLiteral( "lineSymbol" ) );
  const QDomElement symbolElem = QgsSymbolLayerUtils::saveSymbol( QString(),
                                 mSettings.lineSymbol(),
                                 doc,
                                 rwContext );
  lineSymbol.appendChild( symbolElem );
  composerScaleBarElem.appendChild( lineSymbol );

  QDomElement divisionSymbol = doc.createElement( QStringLiteral( "divisionLineSymbol" ) );
  const QDomElement divisionSymbolElem = QgsSymbolLayerUtils::saveSymbol( QString(),
                                         mSettings.divisionLineSymbol(),
                                         doc,
                                         rwContext );
  divisionSymbol.appendChild( divisionSymbolElem );
  composerScaleBarElem.appendChild( divisionSymbol );

  QDomElement subdivisionSymbol = doc.createElement( QStringLiteral( "subdivisionLineSymbol" ) );
  const QDomElement subdivisionSymbolElem = QgsSymbolLayerUtils::saveSymbol( QString(),
      mSettings.subdivisionLineSymbol(),
      doc,
      rwContext );
  subdivisionSymbol.appendChild( subdivisionSymbolElem );
  composerScaleBarElem.appendChild( subdivisionSymbol );

  QDomElement fillSymbol1Elem = doc.createElement( QStringLiteral( "fillSymbol1" ) );
  const QDomElement symbol1Elem = QgsSymbolLayerUtils::saveSymbol( QString(),
                                  mSettings.fillSymbol(),
                                  doc,
                                  rwContext );
  fillSymbol1Elem.appendChild( symbol1Elem );
  composerScaleBarElem.appendChild( fillSymbol1Elem );

  QDomElement fillSymbol2Elem = doc.createElement( QStringLiteral( "fillSymbol2" ) );
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
  mSettings.setHeight( itemElem.attribute( QStringLiteral( "height" ), QStringLiteral( "5.0" ) ).toDouble() );
  mSettings.setLabelBarSpace( itemElem.attribute( QStringLiteral( "labelBarSpace" ), QStringLiteral( "3.0" ) ).toDouble() );
  mSettings.setBoxContentSpace( itemElem.attribute( QStringLiteral( "boxContentSpace" ), QStringLiteral( "1.0" ) ).toDouble() );
  mSettings.setNumberOfSegments( itemElem.attribute( QStringLiteral( "numSegments" ), QStringLiteral( "2" ) ).toInt() );
  mSettings.setNumberOfSegmentsLeft( itemElem.attribute( QStringLiteral( "numSegmentsLeft" ), QStringLiteral( "0" ) ).toInt() );
  mSettings.setNumberOfSubdivisions( itemElem.attribute( QStringLiteral( "numSubdivisions" ), QStringLiteral( "1" ) ).toInt() );
  mSettings.setSubdivisionsHeight( itemElem.attribute( QStringLiteral( "subdivisionsHeight" ), QStringLiteral( "1.5" ) ).toDouble() );
  mSettings.setUnitsPerSegment( itemElem.attribute( QStringLiteral( "numUnitsPerSegment" ), QStringLiteral( "1.0" ) ).toDouble() );
  mSettings.setSegmentSizeMode( static_cast<QgsScaleBarSettings::SegmentSizeMode>( itemElem.attribute( QStringLiteral( "segmentSizeMode" ), QStringLiteral( "0" ) ).toInt() ) );
  mSettings.setMinimumBarWidth( itemElem.attribute( QStringLiteral( "minBarWidth" ), QStringLiteral( "50" ) ).toDouble() );
  mSettings.setMaximumBarWidth( itemElem.attribute( QStringLiteral( "maxBarWidth" ), QStringLiteral( "150" ) ).toDouble() );
  mSegmentMillimeters = itemElem.attribute( QStringLiteral( "segmentMillimeters" ), QStringLiteral( "0.0" ) ).toDouble();
  mSettings.setMapUnitsPerScaleBarUnit( itemElem.attribute( QStringLiteral( "numMapUnitsPerScaleBarUnit" ), QStringLiteral( "1.0" ) ).toDouble() );

  const QDomElement lineSymbolElem = itemElem.firstChildElement( QStringLiteral( "lineSymbol" ) );
  bool foundLineSymbol = false;
  if ( !lineSymbolElem.isNull() )
  {
    const QDomElement symbolElem = lineSymbolElem.firstChildElement( QStringLiteral( "symbol" ) );
    std::unique_ptr< QgsLineSymbol > lineSymbol( QgsSymbolLayerUtils::loadSymbol<QgsLineSymbol>( symbolElem, context ) );
    if ( lineSymbol )
    {
      mSettings.setLineSymbol( lineSymbol.release() );
      foundLineSymbol = true;
    }
  }
  const QDomElement divisionSymbolElem = itemElem.firstChildElement( QStringLiteral( "divisionLineSymbol" ) );
  if ( !divisionSymbolElem.isNull() )
  {
    const QDomElement symbolElem = divisionSymbolElem.firstChildElement( QStringLiteral( "symbol" ) );
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
  const QDomElement subdivisionSymbolElem = itemElem.firstChildElement( QStringLiteral( "subdivisionLineSymbol" ) );
  if ( !subdivisionSymbolElem.isNull() )
  {
    const QDomElement symbolElem = subdivisionSymbolElem.firstChildElement( QStringLiteral( "symbol" ) );
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
    std::unique_ptr< QgsLineSymbol > lineSymbol = std::make_unique< QgsLineSymbol >();
    std::unique_ptr< QgsSimpleLineSymbolLayer > lineSymbolLayer = std::make_unique< QgsSimpleLineSymbolLayer >();
    lineSymbolLayer->setWidth( itemElem.attribute( QStringLiteral( "outlineWidth" ), QStringLiteral( "0.3" ) ).toDouble() );
    lineSymbolLayer->setWidthUnit( QgsUnitTypes::RenderMillimeters );
    lineSymbolLayer->setPenJoinStyle( QgsSymbolLayerUtils::decodePenJoinStyle( itemElem.attribute( QStringLiteral( "lineJoinStyle" ), QStringLiteral( "miter" ) ) ) );
    lineSymbolLayer->setPenCapStyle( QgsSymbolLayerUtils::decodePenCapStyle( itemElem.attribute( QStringLiteral( "lineCapStyle" ), QStringLiteral( "square" ) ) ) );

    //stroke color
    const QDomNodeList strokeColorList = itemElem.elementsByTagName( QStringLiteral( "strokeColor" ) );
    if ( !strokeColorList.isEmpty() )
    {
      const QDomElement strokeColorElem = strokeColorList.at( 0 ).toElement();
      bool redOk, greenOk, blueOk, alphaOk;
      int strokeRed, strokeGreen, strokeBlue, strokeAlpha;

      strokeRed = strokeColorElem.attribute( QStringLiteral( "red" ) ).toDouble( &redOk );
      strokeGreen = strokeColorElem.attribute( QStringLiteral( "green" ) ).toDouble( &greenOk );
      strokeBlue = strokeColorElem.attribute( QStringLiteral( "blue" ) ).toDouble( &blueOk );
      strokeAlpha = strokeColorElem.attribute( QStringLiteral( "alpha" ) ).toDouble( &alphaOk );

      if ( redOk && greenOk && blueOk && alphaOk )
      {
        lineSymbolLayer->setColor( QColor( strokeRed, strokeGreen, strokeBlue, strokeAlpha ) );
      }
    }
    else
    {
      lineSymbolLayer->setColor( QColor( itemElem.attribute( QStringLiteral( "penColor" ), QStringLiteral( "#000000" ) ) ) );
    }

    // need to translate the deprecated ScalebarLineWidth and ScalebarLineColor properties to symbol properties,
    // and then remove them from the scalebar so they don't interfere and apply to other compatibility workarounds
    lineSymbolLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyStrokeWidth, dataDefinedProperties().property( QgsLayoutObject::ScalebarLineWidth ) );
    dataDefinedProperties().setProperty( QgsLayoutObject::ScalebarLineWidth, QgsProperty() );
    lineSymbolLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyStrokeColor, dataDefinedProperties().property( QgsLayoutObject::ScalebarLineColor ) );
    dataDefinedProperties().setProperty( QgsLayoutObject::ScalebarLineColor, QgsProperty() );

    lineSymbol->changeSymbolLayer( 0, lineSymbolLayer.release() );
    mSettings.setLineSymbol( lineSymbol->clone() );
    mSettings.setDivisionLineSymbol( lineSymbol->clone() );
    mSettings.setSubdivisionLineSymbol( lineSymbol.release() );
  }

  mSettings.setUnitLabel( itemElem.attribute( QStringLiteral( "unitLabel" ) ) );

  const QDomNodeList textFormatNodeList = itemElem.elementsByTagName( QStringLiteral( "text-style" ) );
  if ( !textFormatNodeList.isEmpty() )
  {
    const QDomElement textFormatElem = textFormatNodeList.at( 0 ).toElement();
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

  const QDomNodeList numericFormatNodeList = itemElem.elementsByTagName( QStringLiteral( "numericFormat" ) );
  if ( !numericFormatNodeList.isEmpty() )
  {
    const QDomElement numericFormatElem = numericFormatNodeList.at( 0 ).toElement();
    mSettings.setNumericFormat( QgsApplication::numericFormatRegistry()->createFromXml( numericFormatElem, context ) );
  }

  const QDomElement fillSymbol1Elem = itemElem.firstChildElement( QStringLiteral( "fillSymbol1" ) );
  bool foundFillSymbol1 = false;
  if ( !fillSymbol1Elem.isNull() )
  {
    const QDomElement symbolElem = fillSymbol1Elem.firstChildElement( QStringLiteral( "symbol" ) );
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
    std::unique_ptr< QgsFillSymbol > fillSymbol = std::make_unique< QgsFillSymbol >();
    std::unique_ptr< QgsSimpleFillSymbolLayer > fillSymbolLayer = std::make_unique< QgsSimpleFillSymbolLayer >();
    fillSymbolLayer->setStrokeStyle( Qt::NoPen );

    //fill color
    const QDomNodeList fillColorList = itemElem.elementsByTagName( QStringLiteral( "fillColor" ) );
    if ( !fillColorList.isEmpty() )
    {
      const QDomElement fillColorElem = fillColorList.at( 0 ).toElement();
      bool redOk, greenOk, blueOk, alphaOk;
      int fillRed, fillGreen, fillBlue, fillAlpha;

      fillRed = fillColorElem.attribute( QStringLiteral( "red" ) ).toDouble( &redOk );
      fillGreen = fillColorElem.attribute( QStringLiteral( "green" ) ).toDouble( &greenOk );
      fillBlue = fillColorElem.attribute( QStringLiteral( "blue" ) ).toDouble( &blueOk );
      fillAlpha = fillColorElem.attribute( QStringLiteral( "alpha" ) ).toDouble( &alphaOk );

      if ( redOk && greenOk && blueOk && alphaOk )
      {
        fillSymbolLayer->setColor( QColor( fillRed, fillGreen, fillBlue, fillAlpha ) );
      }
    }
    else
    {
      fillSymbolLayer->setColor( QColor( itemElem.attribute( QStringLiteral( "brushColor" ), QStringLiteral( "#000000" ) ) ) );
    }

    // need to translate the deprecated ScalebarFillColor property to symbol properties,
    // and then remove them from the scalebar so they don't interfere and apply to other compatibility workarounds
    fillSymbolLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyFillColor, dataDefinedProperties().property( QgsLayoutObject::ScalebarFillColor ) );
    dataDefinedProperties().setProperty( QgsLayoutObject::ScalebarFillColor, QgsProperty() );

    fillSymbol->changeSymbolLayer( 0, fillSymbolLayer.release() );
    mSettings.setFillSymbol( fillSymbol.release() );
  }

  const QDomElement fillSymbol2Elem = itemElem.firstChildElement( QStringLiteral( "fillSymbol2" ) );
  bool foundFillSymbol2 = false;
  if ( !fillSymbol2Elem.isNull() )
  {
    const QDomElement symbolElem = fillSymbol2Elem.firstChildElement( QStringLiteral( "symbol" ) );
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
    std::unique_ptr< QgsFillSymbol > fillSymbol = std::make_unique< QgsFillSymbol >();
    std::unique_ptr< QgsSimpleFillSymbolLayer > fillSymbolLayer = std::make_unique< QgsSimpleFillSymbolLayer >();
    fillSymbolLayer->setStrokeStyle( Qt::NoPen );

    //fill color 2

    const QDomNodeList fillColor2List = itemElem.elementsByTagName( QStringLiteral( "fillColor2" ) );
    if ( !fillColor2List.isEmpty() )
    {
      const QDomElement fillColor2Elem = fillColor2List.at( 0 ).toElement();
      bool redOk, greenOk, blueOk, alphaOk;
      int fillRed, fillGreen, fillBlue, fillAlpha;

      fillRed = fillColor2Elem.attribute( QStringLiteral( "red" ) ).toDouble( &redOk );
      fillGreen = fillColor2Elem.attribute( QStringLiteral( "green" ) ).toDouble( &greenOk );
      fillBlue = fillColor2Elem.attribute( QStringLiteral( "blue" ) ).toDouble( &blueOk );
      fillAlpha = fillColor2Elem.attribute( QStringLiteral( "alpha" ) ).toDouble( &alphaOk );

      if ( redOk && greenOk && blueOk && alphaOk )
      {
        fillSymbolLayer->setColor( QColor( fillRed, fillGreen, fillBlue, fillAlpha ) );
      }
    }
    else
    {
      fillSymbolLayer->setColor( QColor( itemElem.attribute( QStringLiteral( "brush2Color" ), QStringLiteral( "#ffffff" ) ) ) );
    }

    // need to translate the deprecated ScalebarFillColor2 property to symbol properties,
    // and then remove them from the scalebar so they don't interfere and apply to other compatibility workarounds
    fillSymbolLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyFillColor, dataDefinedProperties().property( QgsLayoutObject::ScalebarFillColor2 ) );
    dataDefinedProperties().setProperty( QgsLayoutObject::ScalebarFillColor2, QgsProperty() );

    fillSymbol->changeSymbolLayer( 0, fillSymbolLayer.release() );
    mSettings.setAlternateFillSymbol( fillSymbol.release() );

  }

  //font color
  const QDomNodeList textColorList = itemElem.elementsByTagName( QStringLiteral( "textColor" ) );
  if ( !textColorList.isEmpty() )
  {
    const QDomElement textColorElem = textColorList.at( 0 ).toElement();
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
  setStyle( itemElem.attribute( QStringLiteral( "style" ), QString() ) );

  //call attemptResize after setStyle to ensure the appropriate size limitations are applied
  attemptResize( QgsLayoutSize::decodeSize( itemElem.attribute( QStringLiteral( "size" ) ) ) );

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

  mSettings.setLabelVerticalPlacement( static_cast< QgsScaleBarSettings::LabelVerticalPlacement >( itemElem.attribute( QStringLiteral( "labelVerticalPlacement" ), QStringLiteral( "0" ) ).toInt() ) );
  mSettings.setLabelHorizontalPlacement( static_cast< QgsScaleBarSettings::LabelHorizontalPlacement >( itemElem.attribute( QStringLiteral( "labelHorizontalPlacement" ), QStringLiteral( "0" ) ).toInt() ) );

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
