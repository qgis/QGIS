/***************************************************************************
  plugin.cpp
  Plugin to draw scale bar on map
Functions:

-------------------
begin                : Jun 1, 2004
copyright            : (C) 2004 by Peter Brewer
email                : sbr00pwb@users.sourceforge.net

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdecorationscalebar.h"

#include "qgsdecorationscalebardialog.h"

#include "qgis.h"
#include "qgisapp.h"
#include "qgsfontutils.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsmaptopixel.h"
#include "qgspointxy.h"
#include "qgsproject.h"
#include "qgsunittypes.h"
#include "qgssettings.h"
#include "qgssymbollayerutils.h"

#include "qgsdoubleboxscalebarrenderer.h"
#include "qgsnumericscalebarrenderer.h"
#include "qgssingleboxscalebarrenderer.h"
#include "qgsticksscalebarrenderer.h"

#include <QPainter>
#include <QAction>
#include <QPen>
#include <QPolygon>
#include <QString>
#include <QFontMetrics>
#include <QFont>
#include <QColor>
#include <QMenu>
#include <QFile>
#include <QLocale>

//non qt includes
#include <cmath>


QgsDecorationScaleBar::QgsDecorationScaleBar( QObject *parent )
  : QgsDecorationItem( parent )
{
  mPlacement = TopLeft;
  mMarginUnit = QgsUnitTypes::RenderMillimeters;
  mStyleLabels << tr( "Tick Down" ) << tr( "Tick Up" )
               << tr( "Bar" ) << tr( "Box" );

  setName( "Scale Bar" );
  projectRead();

  mSettings.setNumberOfSegments( 1 );
  mSettings.setNumberOfSegmentsLeft( 0 );
}

void QgsDecorationScaleBar::projectRead()
{
  QgsDecorationItem::projectRead();
  mPreferredSize = QgsProject::instance()->readNumEntry( mNameConfig, QStringLiteral( "/PreferredSize" ), 30 );
  mStyleIndex = QgsProject::instance()->readNumEntry( mNameConfig, QStringLiteral( "/Style" ), 0 );
  mSnapping = QgsProject::instance()->readBoolEntry( mNameConfig, QStringLiteral( "/Snapping" ), true );
  mColor = QgsSymbolLayerUtils::decodeColor( QgsProject::instance()->readEntry( mNameConfig, QStringLiteral( "/Color" ), QStringLiteral( "#000000" ) ) );
  mOutlineColor = QgsSymbolLayerUtils::decodeColor( QgsProject::instance()->readEntry( mNameConfig, QStringLiteral( "/OutlineColor" ), QStringLiteral( "#FFFFFF" ) ) );
  mMarginHorizontal = QgsProject::instance()->readNumEntry( mNameConfig, QStringLiteral( "/MarginH" ), 0 );
  mMarginVertical = QgsProject::instance()->readNumEntry( mNameConfig, QStringLiteral( "/MarginV" ), 0 );

  QDomDocument doc;
  QDomElement elem;
  QString textFormatXml = QgsProject::instance()->readEntry( mNameConfig, QStringLiteral( "/TextFormat" ) );
  if ( !textFormatXml.isEmpty() )
  {
    doc.setContent( textFormatXml );
    elem = doc.documentElement();
    QgsReadWriteContext context;
    context.setPathResolver( QgsProject::instance()->pathResolver() );
    mTextFormat.readXml( elem, context );
  }
  else
  {
    QString fontXml = QgsProject::instance()->readEntry( mNameConfig, QStringLiteral( "/Font" ) );
    if ( !fontXml.isEmpty() )
    {
      doc.setContent( fontXml );
      elem = doc.documentElement();
      QFont font;
      QgsFontUtils::setFromXmlElement( font, elem );
      mTextFormat = QgsTextFormat::fromQFont( font );
      mTextFormat.setColor( mColor );
    }
    else
    {
      mTextFormat = QgsTextFormat();
    }
  }

  setupScaleBar();
}

void QgsDecorationScaleBar::saveToProject()
{
  QgsDecorationItem::saveToProject();
  QgsProject::instance()->writeEntry( mNameConfig, QStringLiteral( "/PreferredSize" ), mPreferredSize );
  QgsProject::instance()->writeEntry( mNameConfig, QStringLiteral( "/Snapping" ), mSnapping );
  QgsProject::instance()->writeEntry( mNameConfig, QStringLiteral( "/Style" ), mStyleIndex );
  QgsProject::instance()->writeEntry( mNameConfig, QStringLiteral( "/Color" ), QgsSymbolLayerUtils::encodeColor( mColor ) );
  QgsProject::instance()->writeEntry( mNameConfig, QStringLiteral( "/OutlineColor" ), QgsSymbolLayerUtils::encodeColor( mOutlineColor ) );
  QgsProject::instance()->writeEntry( mNameConfig, QStringLiteral( "/MarginH" ), mMarginHorizontal );
  QgsProject::instance()->writeEntry( mNameConfig, QStringLiteral( "/MarginV" ), mMarginVertical );

  QDomDocument fontDoc;
  QgsReadWriteContext context;
  context.setPathResolver( QgsProject::instance()->pathResolver() );
  QDomElement textElem = mTextFormat.writeXml( fontDoc, context );
  fontDoc.appendChild( textElem );

  QgsProject::instance()->writeEntry( mNameConfig, QStringLiteral( "/TextFormat" ), fontDoc.toString() );
}


void QgsDecorationScaleBar::run()
{
  QgsDecorationScaleBarDialog dlg( *this, QgisApp::instance()->mapCanvas()->mapUnits(), QgisApp::instance() );
  dlg.exec();
}

void QgsDecorationScaleBar::setupScaleBar()
{
  mSettings.setTextFormat( mTextFormat );
  switch ( mStyleIndex )
  {
    case 0:
    case 1:
    {
      std::unique_ptr< QgsTicksScaleBarRenderer > tickStyle = qgis::make_unique< QgsTicksScaleBarRenderer >();
      tickStyle->setTickPosition( mStyleIndex == 0 ? QgsTicksScaleBarRenderer::TicksDown : QgsTicksScaleBarRenderer::TicksUp );
      mStyle = std::move( tickStyle );
      mSettings.setFillColor( mColor );
      mSettings.setLineColor( mColor ); // Compatibility with pre 3.2 configuration
      mSettings.setHeight( 2.2 );
      mSettings.setLineWidth( 0.3 );
      break;
    }
    case 2:
    case 3:
      mStyle = qgis::make_unique< QgsSingleBoxScaleBarRenderer >();
      mSettings.setFillColor( mColor );
      mSettings.setFillColor2( QColor( "transparent" ) );
      mSettings.setLineColor( mOutlineColor );
      mSettings.setHeight( mStyleIndex == 2 ? 1 : 3 );
      mSettings.setLineWidth( mStyleIndex == 2 ? 0.2 : 0.3 );
      break;
  }
  mSettings.setLabelBarSpace( 1.8 );
}
void QgsDecorationScaleBar::render( const QgsMapSettings &mapSettings, QgsRenderContext &context )
{
  if ( !enabled() )
    return;

  //Get canvas dimensions
  QPaintDevice *device = context.painter()->device();
  int deviceHeight = device->height() / device->devicePixelRatioF();
  int deviceWidth = device->width() / device->devicePixelRatioF();

  //Get map units per pixel. This can be negative at times (to do with
  //projections) and that just confuses the rest of the code in this
  //function, so force to a positive number.
  double scaleBarUnitsPerPixel = std::fabs( context.mapToPixel().mapUnitsPerPixel() );

  // Exit if the canvas width is 0 or layercount is 0 or QGIS will freeze
  if ( mapSettings.layers().isEmpty() || !deviceWidth || !scaleBarUnitsPerPixel )
    return;

  double unitsPerSegment = mPreferredSize;

  QgsSettings settings;
  bool ok = false;
  QgsUnitTypes::DistanceUnit preferredUnits = QgsUnitTypes::decodeDistanceUnit( settings.value( QStringLiteral( "qgis/measure/displayunits" ) ).toString(), &ok );
  if ( !ok )
    preferredUnits = QgsUnitTypes::DistanceMeters;

  QgsUnitTypes::DistanceUnit scaleBarUnits = mapSettings.mapUnits();

  // Adjust units meter/feet/... or vice versa
  scaleBarUnitsPerPixel *= QgsUnitTypes::fromUnitToUnitFactor( scaleBarUnits, preferredUnits );
  scaleBarUnits = preferredUnits;

  //Calculate size of scale bar for preferred number of map units
  double scaleBarWidth = mPreferredSize / scaleBarUnitsPerPixel;

  //If scale bar is very small reset to 1/4 of the canvas wide
  if ( scaleBarWidth < 30 )
  {
    scaleBarWidth = deviceWidth / 4.0; // value in pixels
    unitsPerSegment = scaleBarWidth * scaleBarUnitsPerPixel; // value in map units
  }

  //if scale bar is more than half the canvas wide keep halving until not
  while ( scaleBarWidth > deviceWidth / 3.0 )
  {
    scaleBarWidth = scaleBarWidth / 3;
  }
  unitsPerSegment = scaleBarWidth * scaleBarUnitsPerPixel;

  // Work out the exponent for the number - e.g, 1234 will give 3,
  // and .001234 will give -3
  double powerOf10 = std::floor( std::log10( unitsPerSegment ) );

  // snap to integer < 10 times power of 10
  if ( mSnapping )
  {
    double scaler = std::pow( 10.0, powerOf10 );
    unitsPerSegment = std::round( unitsPerSegment / scaler ) * scaler;
    scaleBarWidth = unitsPerSegment / scaleBarUnitsPerPixel;
  }

  //Get type of map units and set scale bar unit label text
  double mapSize = unitsPerSegment * QgsUnitTypes::fromUnitToUnitFactor( scaleBarUnits, mapSettings.mapUnits() );
  double segmentSize = context.convertFromMapUnits( mapSize, QgsUnitTypes::RenderMillimeters );

  QString scaleBarUnitLabel;
  switch ( scaleBarUnits )
  {
    case QgsUnitTypes::DistanceMeters:
      if ( unitsPerSegment > 1000.0 )
      {
        scaleBarUnitLabel = tr( "km" );
        unitsPerSegment = unitsPerSegment / 1000;
      }
      else if ( unitsPerSegment < 0.01 )
      {
        scaleBarUnitLabel = tr( "mm" );
        unitsPerSegment = unitsPerSegment * 1000;
      }
      else if ( unitsPerSegment < 0.1 )
      {
        scaleBarUnitLabel = tr( "cm" );
        unitsPerSegment = unitsPerSegment * 100;
      }
      else
        scaleBarUnitLabel = tr( "m" );
      break;
    case QgsUnitTypes::DistanceFeet:
      if ( unitsPerSegment > 5280.0 ) //5280 feet to the mile
      {
        scaleBarUnitLabel = tr( "miles" );
        // Adjust scale bar width to get even numbers
        unitsPerSegment = unitsPerSegment / 5000;
        //scaleBarWidth = ( scaleBarWidth * 5280 ) / 5000;
      }
      else if ( unitsPerSegment == 5280.0 ) //5280 feet to the mile
      {
        scaleBarUnitLabel = tr( "mile" );
        // Adjust scale bar width to get even numbers
        unitsPerSegment = unitsPerSegment / 5000;
        //scaleBarWidth = ( scaleBarWidth * 5280 ) / 5000;
      }
      else if ( unitsPerSegment < 1 )
      {
        scaleBarUnitLabel = tr( "inches" );
        unitsPerSegment = unitsPerSegment * 10;
        //scaleBarWidth = ( scaleBarWidth * 10 ) / 12;
      }
      else if ( unitsPerSegment == 1.0 )
      {
        scaleBarUnitLabel = tr( "foot" );
      }
      else
      {
        scaleBarUnitLabel = tr( "feet" );
      }
      break;
    case QgsUnitTypes::DistanceDegrees:
      if ( unitsPerSegment == 1.0 )
        scaleBarUnitLabel = tr( "degree" );
      else
        scaleBarUnitLabel = tr( "degrees" );
      break;
    case QgsUnitTypes::DistanceKilometers:
    case QgsUnitTypes::DistanceNauticalMiles:
    case QgsUnitTypes::DistanceYards:
    case QgsUnitTypes::DistanceMiles:
    case QgsUnitTypes::DistanceCentimeters:
    case QgsUnitTypes::DistanceMillimeters:
    case QgsUnitTypes::DistanceUnknownUnit:
      scaleBarUnitLabel = QgsUnitTypes::toAbbreviatedString( scaleBarUnits );
      break;
  }

  mSettings.setUnits( scaleBarUnits );
  mSettings.setNumberOfSegments( mStyleIndex == 3 ? 2 : 1 );
  mSettings.setUnitsPerSegment( mStyleIndex == 3 ? unitsPerSegment / 2 : unitsPerSegment );
  mSettings.setUnitLabel( scaleBarUnitLabel );
  if ( mPlacement == TopCenter || mPlacement == BottomCenter )
  {
    mSettings.setLabelHorizontalPlacement( QgsScaleBarSettings::LabelCenteredSegment );
  }
  QgsScaleBarRenderer::ScaleBarContext scaleContext;
  scaleContext.segmentWidth = mStyleIndex == 3 ? segmentSize / 2 : segmentSize;
  scaleContext.scale = mapSettings.scale();

  //Calculate total width of scale bar and label
  QSizeF size = mStyle->calculateBoxSize( mSettings, scaleContext );
  size.setWidth( context.convertToPainterUnits( size.width(), QgsUnitTypes::RenderMillimeters ) );
  size.setHeight( context.convertToPainterUnits( size.height(), QgsUnitTypes::RenderMillimeters ) );

  int originX = 0;
  int originY = 0;

  // Set  margin according to selected units
  switch ( mMarginUnit )
  {
    case QgsUnitTypes::RenderMillimeters:
    {
      int pixelsInchX = context.painter()->device()->logicalDpiX();
      int pixelsInchY = context.painter()->device()->logicalDpiY();
      originX = pixelsInchX * INCHES_TO_MM * mMarginHorizontal;
      originY = pixelsInchY * INCHES_TO_MM * mMarginVertical;
      break;
    }

    case QgsUnitTypes::RenderPixels:
      originX = mMarginHorizontal - 5.; // Minus 5 to shift tight into corner
      originY = mMarginVertical - 5.;
      break;

    case QgsUnitTypes::RenderPercentage:
    {
      originX = ( ( deviceWidth - size.width() ) / 100. ) * mMarginHorizontal;
      originY = ( ( deviceHeight ) / 100. ) * mMarginVertical;
      break;
    }
    case QgsUnitTypes::RenderMapUnits:
    case QgsUnitTypes::RenderPoints:
    case QgsUnitTypes::RenderInches:
    case QgsUnitTypes::RenderUnknownUnit:
    case QgsUnitTypes::RenderMetersInMapUnits:
      break;
  }

  //Determine the origin of scale bar depending on placement selected
  switch ( mPlacement )
  {
    case TopLeft:
      break;
    case TopRight:
      originX = deviceWidth - originX - size.width();
      break;
    case BottomLeft:
      originY = deviceHeight - originY - size.height();
      break;
    case BottomRight:
      originX = deviceWidth - originX - size.width();
      originY = deviceHeight - originY - size.height();
      break;
    case TopCenter:
      originX = deviceWidth / 2 - size.width() / 2 + originX;
      break;
    case BottomCenter:
      originX = deviceWidth / 2 - size.width() / 2 + originX;
      originY = deviceHeight - originY - size.height();
      break;
    default:
      QgsDebugMsg( QStringLiteral( "Unsupported placement index of %1" ).arg( static_cast<int>( mPlacement ) ) );
  }

  context.painter()->save();
  context.painter()->translate( originX, originY );
  mStyle->draw( context, mSettings, scaleContext );
  context.painter()->restore();
}
