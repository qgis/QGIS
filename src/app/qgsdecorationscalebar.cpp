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
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsmaptopixel.h"
#include "qgspointxy.h"
#include "qgsproject.h"
#include "qgsunittypes.h"
#include "qgssettings.h"
#include "qgssymbollayerutils.h"

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
}


void QgsDecorationScaleBar::run()
{
  QgsDecorationScaleBarDialog dlg( *this, QgisApp::instance()->mapCanvas()->mapUnits(), QgisApp::instance() );
  dlg.exec();
}


void QgsDecorationScaleBar::render( const QgsMapSettings &mapSettings, QgsRenderContext &context )
{
  int myBufferSize = 1; //softcode this later

  //Get canvas dimensions
  int myCanvasHeight = context.painter()->device()->height();
  int myCanvasWidth = context.painter()->device()->width();

  //Get map units per pixel. This can be negative at times (to do with
  //projections) and that just confuses the rest of the code in this
  //function, so force to a positive number.
  double myMapUnitsPerPixelDouble = std::fabs( context.mapToPixel().mapUnitsPerPixel() );
  double myActualSize = mPreferredSize;

  // Exit if the canvas width is 0 or layercount is 0 or QGIS will freeze
  int myLayerCount = mapSettings.layers().count();
  if ( !myLayerCount || !myCanvasWidth || !myMapUnitsPerPixelDouble )
    return;

  //Large if statement which determines whether to render the scale bar
  if ( enabled() )
  {
    // Hard coded sizes
    int myMajorTickSize = 8;
    int myTextOffsetX = 3;

    QgsSettings settings;
    bool ok = false;
    QgsUnitTypes::DistanceUnit myPreferredUnits = QgsUnitTypes::decodeDistanceUnit( settings.value( QStringLiteral( "qgis/measure/displayunits" ) ).toString(), &ok );
    if ( !ok )
      myPreferredUnits = QgsUnitTypes::DistanceMeters;
    QgsUnitTypes::DistanceUnit myMapUnits = mapSettings.mapUnits();

    // Adjust units meter/feet/... or vice versa
    myMapUnitsPerPixelDouble *= QgsUnitTypes::fromUnitToUnitFactor( myMapUnits, myPreferredUnits );
    myMapUnits = myPreferredUnits;

    //Calculate size of scale bar for preferred number of map units
    double myScaleBarWidth = mPreferredSize / myMapUnitsPerPixelDouble;

    //If scale bar is very small reset to 1/4 of the canvas wide
    if ( myScaleBarWidth < 30 )
    {
      myScaleBarWidth = myCanvasWidth / 4.0; // pixels
      myActualSize = myScaleBarWidth * myMapUnitsPerPixelDouble; // map units
    }

    //if scale bar is more than half the canvas wide keep halving until not
    while ( myScaleBarWidth > myCanvasWidth / 3.0 )
    {
      myScaleBarWidth = myScaleBarWidth / 3;
    }
    myActualSize = myScaleBarWidth * myMapUnitsPerPixelDouble;

    // Work out the exponent for the number - e.g, 1234 will give 3,
    // and .001234 will give -3
    double myPowerOf10 = std::floor( std::log10( myActualSize ) );

    // snap to integer < 10 times power of 10
    if ( mSnapping )
    {
      double scaler = std::pow( 10.0, myPowerOf10 );
      myActualSize = std::round( myActualSize / scaler ) * scaler;
      myScaleBarWidth = myActualSize / myMapUnitsPerPixelDouble;
    }

    //Get type of map units and set scale bar unit label text
    QString myScaleBarUnitLabel;
    switch ( myMapUnits )
    {
      case QgsUnitTypes::DistanceMeters:
        if ( myActualSize > 1000.0 )
        {
          myScaleBarUnitLabel = tr( " km" );
          myActualSize = myActualSize / 1000;
        }
        else if ( myActualSize < 0.01 )
        {
          myScaleBarUnitLabel = tr( " mm" );
          myActualSize = myActualSize * 1000;
        }
        else if ( myActualSize < 0.1 )
        {
          myScaleBarUnitLabel = tr( " cm" );
          myActualSize = myActualSize * 100;
        }
        else
          myScaleBarUnitLabel = tr( " m" );
        break;
      case QgsUnitTypes::DistanceFeet:
        if ( myActualSize > 5280.0 ) //5280 feet to the mile
        {
          myScaleBarUnitLabel = tr( " miles" );
          // Adjust scale bar width to get even numbers
          myActualSize = myActualSize / 5000;
          myScaleBarWidth = ( myScaleBarWidth * 5280 ) / 5000;
        }
        else if ( myActualSize == 5280.0 ) //5280 feet to the mile
        {
          myScaleBarUnitLabel = tr( " mile" );
          // Adjust scale bar width to get even numbers
          myActualSize = myActualSize / 5000;
          myScaleBarWidth = ( myScaleBarWidth * 5280 ) / 5000;
        }
        else if ( myActualSize < 1 )
        {
          myScaleBarUnitLabel = tr( " inches" );
          myActualSize = myActualSize * 10;
          myScaleBarWidth = ( myScaleBarWidth * 10 ) / 12;
        }
        else if ( myActualSize == 1.0 )
        {
          myScaleBarUnitLabel = tr( " foot" );
        }
        else
        {
          myScaleBarUnitLabel = tr( " feet" );
        }
        break;
      case QgsUnitTypes::DistanceDegrees:
        if ( myActualSize == 1.0 )
          myScaleBarUnitLabel = tr( " degree" );
        else
          myScaleBarUnitLabel = tr( " degrees" );
        break;
      case QgsUnitTypes::DistanceUnknownUnit:
        myScaleBarUnitLabel = tr( " unknown" );
        //intentional fall-through
        FALLTHROUGH;
      default:
        QgsDebugMsg( QString( "Error: not picked up map units - actual value = %1" ).arg( myMapUnits ) );
    }

    //Set font and calculate width of unit label
    int myFontSize = 10; //we use this later for buffering
    QFont myFont( QStringLiteral( "helvetica" ), myFontSize );
    context.painter()->setFont( myFont );
    QFontMetrics myFontMetrics( myFont );
    double myFontWidth = myFontMetrics.width( myScaleBarUnitLabel );
    double myFontHeight = myFontMetrics.height();

    //Set the maximum label
    QString myScaleBarMaxLabel = QLocale::system().toString( myActualSize );

    //Calculate total width of scale bar and label
    double myTotalScaleBarWidth = myScaleBarWidth + myFontWidth;

    int myMarginW = 10;
    int myMarginH = 20;
    int myOriginX = 0;
    int myOriginY = 0;

    // Set  margin according to selected units
    switch ( mMarginUnit )
    {
      case QgsUnitTypes::RenderMillimeters:
      {
        int myPixelsInchX = context.painter()->device()->logicalDpiX();
        int myPixelsInchY = context.painter()->device()->logicalDpiY();
        myOriginX = myPixelsInchX * INCHES_TO_MM * mMarginHorizontal;
        myOriginY = myPixelsInchY * INCHES_TO_MM * mMarginVertical;
        break;
      }

      case QgsUnitTypes::RenderPixels:
        myOriginX = mMarginHorizontal - 5.; // Minus 5 to shift tight into corner
        myOriginY = mMarginVertical - 5.;
        break;

      case QgsUnitTypes::RenderPercentage:
      {
        float myMarginDoubledW = myMarginW * 2.0;
        float myMarginDoubledH = myMarginH * 2.0;
        myOriginX = ( ( myCanvasWidth - myMarginDoubledW - myTotalScaleBarWidth ) / 100. ) * mMarginHorizontal;
        myOriginY = ( ( myCanvasHeight - myMarginDoubledH ) / 100. ) * mMarginVertical;
        break;
      }

      default:  // Use default of top left
        break;
    }

    //Determine the origin of scale bar depending on placement selected
    switch ( mPlacement )
    {
      case BottomLeft:
        myOriginX += myMarginW;
        myOriginY = myCanvasHeight - myOriginY - myMarginH;
        break;
      case TopLeft:
        myOriginX += myMarginW;
        myOriginY += myMarginH;
        break;
      case TopRight:
        myOriginX = myCanvasWidth - myOriginX - myMarginW - ( static_cast< int >( myTotalScaleBarWidth ) );
        myOriginY += myMarginH;
        break;
      case BottomRight:
        myOriginX = myCanvasWidth - myOriginX - myMarginW - ( static_cast< int >( myTotalScaleBarWidth ) );
        myOriginY = myCanvasHeight - myOriginY - myMarginH;
        break;
      default:
        QgsDebugMsg( "Unable to determine where to put scale bar so defaulting to top left" );
    }

    //Set pen to draw with
    QPen myForegroundPen( mColor, 2 );
    QPen myBackgroundPen( mOutlineColor, 4 );

    //Cast myScaleBarWidth to int for drawing
    int myScaleBarWidthInt = static_cast< int >( myScaleBarWidth );

    //Create array of vertices for scale bar depending on style
    switch ( mStyleIndex )
    {
      case 0: // Tick Down
      {
        QPolygon myTickDownArray( 4 );
        //draw a buffer first so bar shows up on dark images
        context.painter()->setPen( myBackgroundPen );
        myTickDownArray.putPoints( 0, 4,
                                   myOriginX,                      myOriginY + myMajorTickSize,
                                   myOriginX,                      myOriginY,
                                   myScaleBarWidthInt + myOriginX, myOriginY,
                                   myScaleBarWidthInt + myOriginX, myOriginY + myMajorTickSize
                                 );
        context.painter()->drawPolyline( myTickDownArray );
        //now draw the bar itself in user selected color
        context.painter()->setPen( myForegroundPen );
        myTickDownArray.putPoints( 0, 4,
                                   myOriginX,                      myOriginY + myMajorTickSize,
                                   myOriginX,                      myOriginY,
                                   myScaleBarWidthInt + myOriginX, myOriginY,
                                   myScaleBarWidthInt + myOriginX, myOriginY + myMajorTickSize
                                 );
        context.painter()->drawPolyline( myTickDownArray );
        break;
      }
      case 1: // tick up
      {
        QPolygon myTickUpArray( 4 );
        //draw a buffer first so bar shows up on dark images
        context.painter()->setPen( myBackgroundPen );
        myTickUpArray.putPoints( 0, 4,
                                 myOriginX,                      myOriginY,
                                 myOriginX,                      myOriginY + myMajorTickSize,
                                 myScaleBarWidthInt + myOriginX, myOriginY + myMajorTickSize,
                                 myScaleBarWidthInt + myOriginX, myOriginY
                               );
        context.painter()->drawPolyline( myTickUpArray );
        //now draw the bar itself in user selected color
        context.painter()->setPen( myForegroundPen );
        myTickUpArray.putPoints( 0, 4,
                                 myOriginX,                      myOriginY,
                                 myOriginX,                      myOriginY + myMajorTickSize,
                                 myScaleBarWidthInt + myOriginX, myOriginY + myMajorTickSize,
                                 myScaleBarWidthInt + myOriginX, myOriginY
                               );
        context.painter()->drawPolyline( myTickUpArray );
        break;
      }
      case 2: // Bar
      {
        QPolygon myBarArray( 2 );
        //draw a buffer first so bar shows up on dark images
        context.painter()->setPen( myBackgroundPen );
        myBarArray.putPoints( 0, 2,
                              myOriginX,                      myOriginY + ( myMajorTickSize / 2 ),
                              myScaleBarWidthInt + myOriginX, myOriginY + ( myMajorTickSize / 2 )
                            );
        context.painter()->drawPolyline( myBarArray );
        //now draw the bar itself in user selected color
        context.painter()->setPen( myForegroundPen );
        myBarArray.putPoints( 0, 2,
                              myOriginX,                      myOriginY + ( myMajorTickSize / 2 ),
                              myScaleBarWidthInt + myOriginX, myOriginY + ( myMajorTickSize / 2 )
                            );
        context.painter()->drawPolyline( myBarArray );
        break;
      }
      case 3: // box
      {
        // Want square corners for a box
        myBackgroundPen.setJoinStyle( Qt::MiterJoin );
        myForegroundPen.setJoinStyle( Qt::MiterJoin );
        QPolygon myBoxArray( 5 );
        //draw a buffer first so bar shows up on dark images
        context.painter()->setPen( myBackgroundPen );
        myBoxArray.putPoints( 0, 5,
                              myOriginX,                      myOriginY,
                              myScaleBarWidthInt + myOriginX, myOriginY,
                              myScaleBarWidthInt + myOriginX, myOriginY + myMajorTickSize,
                              myOriginX,                      myOriginY + myMajorTickSize,
                              myOriginX,                      myOriginY
                            );
        context.painter()->drawPolyline( myBoxArray );
        //now draw the bar itself in user selected color
        context.painter()->setPen( myForegroundPen );
        context.painter()->setBrush( QBrush( mColor, Qt::SolidPattern ) );
        int midPointX = myScaleBarWidthInt / 2 + myOriginX;
        myBoxArray.putPoints( 0, 5,
                              myOriginX, myOriginY,
                              midPointX, myOriginY,
                              midPointX, myOriginY + myMajorTickSize,
                              myOriginX, myOriginY + myMajorTickSize,
                              myOriginX, myOriginY
                            );
        context.painter()->drawPolygon( myBoxArray );

        context.painter()->setBrush( Qt::NoBrush );
        myBoxArray.putPoints( 0, 5,
                              midPointX,                      myOriginY,
                              myScaleBarWidthInt + myOriginX, myOriginY,
                              myScaleBarWidthInt + myOriginX, myOriginY + myMajorTickSize,
                              midPointX,                      myOriginY + myMajorTickSize,
                              midPointX,                      myOriginY
                            );
        context.painter()->drawPolygon( myBoxArray );
        break;
      }
      default:
        QgsDebugMsg( "Unknown style" );
    }

    //Do actual drawing of scale bar

    //
    //Do drawing of scale bar text
    //

    QColor myBackColor = mOutlineColor;
    QColor myForeColor = mColor;

    //Draw the minimum label buffer
    context.painter()->setPen( myBackColor );
    myFontWidth = myFontMetrics.width( QStringLiteral( "0" ) );
    myFontHeight = myFontMetrics.height();

    for ( int i = 0 - myBufferSize; i <= myBufferSize; i++ )
    {
      for ( int j = 0 - myBufferSize; j <= myBufferSize; j++ )
      {
        context.painter()->drawText( int( i + ( myOriginX - ( myFontWidth / 2 ) ) ),
                                     int( j + ( myOriginY - ( myFontHeight / 4 ) ) ),
                                     QStringLiteral( "0" ) );
      }
    }

    //Draw minimum label
    context.painter()->setPen( myForeColor );

    context.painter()->drawText(
      int( myOriginX - ( myFontWidth / 2 ) ),
      int( myOriginY - ( myFontHeight / 4 ) ),
      QStringLiteral( "0" )
    );

    //
    //Draw maximum label
    //
    context.painter()->setPen( myBackColor );
    myFontWidth = myFontMetrics.width( myScaleBarMaxLabel );
    myFontHeight = myFontMetrics.height();
    //first the buffer
    for ( int i = 0 - myBufferSize; i <= myBufferSize; i++ )
    {
      for ( int j = 0 - myBufferSize; j <= myBufferSize; j++ )
      {
        context.painter()->drawText( int( i + ( myOriginX + myScaleBarWidthInt - ( myFontWidth / 2 ) ) ),
                                     int( j + ( myOriginY - ( myFontHeight / 4 ) ) ),
                                     myScaleBarMaxLabel );
      }
    }
    //then the text itself
    context.painter()->setPen( myForeColor );
    context.painter()->drawText(
      int( myOriginX + myScaleBarWidthInt - ( myFontWidth / 2 ) ),
      int( myOriginY - ( myFontHeight / 4 ) ),
      myScaleBarMaxLabel
    );

    //
    //Draw unit label
    //
    context.painter()->setPen( myBackColor );
    //first the buffer
    for ( int i = 0 - myBufferSize; i <= myBufferSize; i++ )
    {
      for ( int j = 0 - myBufferSize; j <= myBufferSize; j++ )
      {
        context.painter()->drawText( i + ( myOriginX + myScaleBarWidthInt + myTextOffsetX ),
                                     j + ( myOriginY + myMajorTickSize ),
                                     myScaleBarUnitLabel );
      }
    }
    //then the text itself
    context.painter()->setPen( myForeColor );
    context.painter()->drawText(
      ( myOriginX + myScaleBarWidthInt + myTextOffsetX ), ( myOriginY + myMajorTickSize ),
      myScaleBarUnitLabel
    );
  }
}
