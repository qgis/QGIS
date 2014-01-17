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
#include "qgspoint.h"
#include "qgsproject.h"


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


QgsDecorationScaleBar::QgsDecorationScaleBar( QObject* parent )
    : QgsDecorationItem( parent )
{
  mPlacementLabels << tr( "Bottom Left" ) << tr( "Top Left" )
  << tr( "Top Right" ) << tr( "Bottom Right" );
  mPlacementIndex = 1;
  mStyleLabels << tr( "Tick Down" ) << tr( "Tick Up" )
  << tr( "Bar" ) << tr( "Box" );

  setName( "Scale Bar" );
  projectRead();
}

QgsDecorationScaleBar::~QgsDecorationScaleBar()
{

}

void QgsDecorationScaleBar::projectRead()
{
  QgsDecorationItem::projectRead();
  mPreferredSize = QgsProject::instance()->readNumEntry( mNameConfig, "/PreferredSize", 30 );
  mStyleIndex = QgsProject::instance()->readNumEntry( mNameConfig, "/Style", 0 );
  mPlacementIndex = QgsProject::instance()->readNumEntry( mNameConfig, "/Placement", 2 );
  // mEnabled = QgsProject::instance()->readBoolEntry( mNameConfig, "/Enabled", false );
  mSnapping = QgsProject::instance()->readBoolEntry( mNameConfig, "/Snapping", true );
  int myRedInt = QgsProject::instance()->readNumEntry( mNameConfig, "/ColorRedPart", 0 );
  int myGreenInt = QgsProject::instance()->readNumEntry( mNameConfig, "/ColorGreenPart", 0 );
  int myBlueInt = QgsProject::instance()->readNumEntry( mNameConfig, "/ColorBluePart", 0 );
  mColor = QColor( myRedInt, myGreenInt, myBlueInt );
}

void QgsDecorationScaleBar::saveToProject()
{
  QgsDecorationItem::saveToProject();
  QgsProject::instance()->writeEntry( mNameConfig, "/Placement", mPlacementIndex );
  QgsProject::instance()->writeEntry( mNameConfig, "/PreferredSize", mPreferredSize );
  QgsProject::instance()->writeEntry( mNameConfig, "/Snapping", mSnapping );
  // QgsProject::instance()->writeEntry( mNameConfig, "/Enabled", mEnabled );
  QgsProject::instance()->writeEntry( mNameConfig, "/Style", mStyleIndex );
  QgsProject::instance()->writeEntry( mNameConfig, "/ColorRedPart", mColor.red() );
  QgsProject::instance()->writeEntry( mNameConfig, "/ColorGreenPart", mColor.green() );
  QgsProject::instance()->writeEntry( mNameConfig, "/ColorBluePart", mColor.blue() );
}


void QgsDecorationScaleBar::run()
{
  QgsDecorationScaleBarDialog dlg( *this, QgisApp::instance()->mapCanvas()->mapUnits(), QgisApp::instance() );

  if ( dlg.exec() )
  {
    update();
  }
}


void QgsDecorationScaleBar::render( QPainter * theQPainter )
{
  QgsMapCanvas* canvas = QgisApp::instance()->mapCanvas();

  int myBufferSize = 1; //softcode this later

  //Get canvas dimensions
  int myCanvasHeight = theQPainter->device()->height();
  int myCanvasWidth = theQPainter->device()->width();

  //Get map units per pixel. This can be negative at times (to do with
  //projections) and that just confuses the rest of the code in this
  //function, so force to a positive number.
  double myMapUnitsPerPixelDouble = qAbs( canvas->mapUnitsPerPixel() );
  double myActualSize = mPreferredSize;

  // Exit if the canvas width is 0 or layercount is 0 or QGIS will freeze
  int myLayerCount = canvas->layerCount();
  if ( !myLayerCount || !myCanvasWidth || !myMapUnitsPerPixelDouble )
    return;

  //Large if statement which determines whether to render the scale bar
  if ( enabled() )
  {
    // Hard coded sizes
    int myMajorTickSize = 8;
    int myTextOffsetX = 3;
    int myMargin = 20;

    QSettings settings;
    QGis::UnitType myPreferredUnits = QGis::fromLiteral( settings.value( "/qgis/measure/displayunits", QGis::toLiteral( QGis::Meters ) ).toString() );
    QGis::UnitType myMapUnits = canvas->mapUnits();

    // Adjust units meter/feet/... or vice versa
    myMapUnitsPerPixelDouble *= QGis::fromUnitToUnitFactor( myMapUnits, myPreferredUnits );
    myMapUnits = myPreferredUnits;

    //Calculate size of scale bar for preferred number of map units
    double myScaleBarWidth = mPreferredSize / myMapUnitsPerPixelDouble;

    //If scale bar is very small reset to 1/4 of the canvas wide
    if ( myScaleBarWidth < 30 )
    {
      myScaleBarWidth = myCanvasWidth / 4; // pixels
      myActualSize = myScaleBarWidth * myMapUnitsPerPixelDouble; // map units
    };

    //if scale bar is more than half the canvas wide keep halving until not
    while ( myScaleBarWidth > myCanvasWidth / 3 )
    {
      myScaleBarWidth = myScaleBarWidth / 3;
    };
    myActualSize = myScaleBarWidth * myMapUnitsPerPixelDouble;

    // Work out the exponent for the number - e.g, 1234 will give 3,
    // and .001234 will give -3
    double myPowerOf10 = floor( log10( myActualSize ) );

    // snap to integer < 10 times power of 10
    if ( mSnapping )
    {
      double scaler = pow( 10.0, myPowerOf10 );
      myActualSize = qRound( myActualSize / scaler ) * scaler;
      myScaleBarWidth = myActualSize / myMapUnitsPerPixelDouble;
    }

    //Get type of map units and set scale bar unit label text
    QString myScaleBarUnitLabel;
    switch ( myMapUnits )
    {
      case QGis::Meters:
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
      case QGis::Feet:
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
      case QGis::Degrees:
        if ( myActualSize == 1.0 )
          myScaleBarUnitLabel = tr( " degree" );
        else
          myScaleBarUnitLabel = tr( " degrees" );
        break;
      case QGis::UnknownUnit:
        myScaleBarUnitLabel = tr( " unknown" );
      default:
        QgsDebugMsg( QString( "Error: not picked up map units - actual value = %1" ).arg( myMapUnits ) );
    };

    //Set font and calculate width of unit label
    int myFontSize = 10; //we use this later for buffering
    QFont myFont( "helvetica", myFontSize );
    theQPainter->setFont( myFont );
    QFontMetrics myFontMetrics( myFont );
    double myFontWidth = myFontMetrics.width( myScaleBarUnitLabel );
    double myFontHeight = myFontMetrics.height();

    //Set the maximum label
    QString myScaleBarMaxLabel = QLocale::system().toString( myActualSize );

    //Calculate total width of scale bar and label
    double myTotalScaleBarWidth = myScaleBarWidth + myFontWidth;

    //determine the origin of scale bar depending on placement selected
    int myOriginX = myMargin;
    int myOriginY = myMargin;
    switch ( mPlacementIndex )
    {
      case 0: // Bottom Left
        myOriginX = myMargin;
        myOriginY = myCanvasHeight - myMargin;
        break;
      case 1: // Top Left
        myOriginX = myMargin;
        myOriginY = myMargin;
        break;
      case 2: // Top Right
        myOriginX = myCanvasWidth - (( int ) myTotalScaleBarWidth ) - myMargin;
        myOriginY = myMargin;
        break;
      case 3: // Bottom Right
        myOriginX = myCanvasWidth - (( int ) myTotalScaleBarWidth ) - myMargin;
        myOriginY = myCanvasHeight - myMargin;
        break;
      default:
        QgsDebugMsg( "Unable to determine where to put scale bar so defaulting to top left" );
    }

    //Set pen to draw with
    QPen myForegroundPen( mColor, 2 );
    QPen myBackgroundPen( Qt::white, 4 );

    //Cast myScaleBarWidth to int for drawing
    int myScaleBarWidthInt = ( int ) myScaleBarWidth;

    //Create array of vertices for scale bar depending on style
    switch ( mStyleIndex )
    {
      case 0: // Tick Down
      {
        QPolygon myTickDownArray( 4 );
        //draw a buffer first so bar shows up on dark images
        theQPainter->setPen( myBackgroundPen );
        myTickDownArray.putPoints( 0, 4,
                                   myOriginX                    , ( myOriginY + myMajorTickSize ) ,
                                   myOriginX                    ,  myOriginY                    ,
                                   ( myScaleBarWidthInt + myOriginX ),  myOriginY                    ,
                                   ( myScaleBarWidthInt + myOriginX ), ( myOriginY + myMajorTickSize )
                                 );
        theQPainter->drawPolyline( myTickDownArray );
        //now draw the bar itself in user selected color
        theQPainter->setPen( myForegroundPen );
        myTickDownArray.putPoints( 0, 4,
                                   myOriginX                    , ( myOriginY + myMajorTickSize ) ,
                                   myOriginX                    ,  myOriginY                    ,
                                   ( myScaleBarWidthInt + myOriginX ),  myOriginY                    ,
                                   ( myScaleBarWidthInt + myOriginX ), ( myOriginY + myMajorTickSize )
                                 );
        theQPainter->drawPolyline( myTickDownArray );
        break;
      }
      case 1: // tick up
      {
        QPolygon myTickUpArray( 4 );
        //draw a buffer first so bar shows up on dark images
        theQPainter->setPen( myBackgroundPen );
        myTickUpArray.putPoints( 0, 4,
                                 myOriginX                    ,  myOriginY                    ,
                                 myOriginX                    ,  myOriginY + myMajorTickSize  ,
                                 ( myScaleBarWidthInt + myOriginX ),  myOriginY + myMajorTickSize  ,
                                 ( myScaleBarWidthInt + myOriginX ),  myOriginY
                               );
        theQPainter->drawPolyline( myTickUpArray );
        //now draw the bar itself in user selected color
        theQPainter->setPen( myForegroundPen );
        myTickUpArray.putPoints( 0, 4,
                                 myOriginX                    ,  myOriginY                    ,
                                 myOriginX                    ,  myOriginY + myMajorTickSize  ,
                                 ( myScaleBarWidthInt + myOriginX ),  myOriginY + myMajorTickSize  ,
                                 ( myScaleBarWidthInt + myOriginX ),  myOriginY
                               );
        theQPainter->drawPolyline( myTickUpArray );
        break;
      }
      case 2: // Bar
      {
        QPolygon myBarArray( 2 );
        //draw a buffer first so bar shows up on dark images
        theQPainter->setPen( myBackgroundPen );
        myBarArray.putPoints( 0, 2,
                              myOriginX                    , ( myOriginY + ( myMajorTickSize / 2 ) ),
                              ( myScaleBarWidthInt + myOriginX ), ( myOriginY + ( myMajorTickSize / 2 ) )
                            );
        theQPainter->drawPolyline( myBarArray );
        //now draw the bar itself in user selected color
        theQPainter->setPen( myForegroundPen );
        myBarArray.putPoints( 0, 2,
                              myOriginX                    , ( myOriginY + ( myMajorTickSize / 2 ) ),
                              ( myScaleBarWidthInt + myOriginX ), ( myOriginY + ( myMajorTickSize / 2 ) )
                            );
        theQPainter->drawPolyline( myBarArray );
        break;
      }
      case 3: // box
      {
        // Want square corners for a box
        myBackgroundPen.setJoinStyle( Qt::MiterJoin );
        myForegroundPen.setJoinStyle( Qt::MiterJoin );
        QPolygon myBoxArray( 5 );
        //draw a buffer first so bar shows up on dark images
        theQPainter->setPen( myBackgroundPen );
        myBoxArray.putPoints( 0, 5,
                              myOriginX                    ,  myOriginY,
                              ( myScaleBarWidthInt + myOriginX ),  myOriginY,
                              ( myScaleBarWidthInt + myOriginX ), ( myOriginY + myMajorTickSize ),
                              myOriginX                    , ( myOriginY + myMajorTickSize ),
                              myOriginX                    ,  myOriginY
                            );
        theQPainter->drawPolyline( myBoxArray );
        //now draw the bar itself in user selected color
        theQPainter->setPen( myForegroundPen );
        theQPainter->setBrush( QBrush( mColor, Qt::SolidPattern ) );
        int midPointX = myScaleBarWidthInt / 2 + myOriginX;
        myBoxArray.putPoints( 0, 5,
                              myOriginX                    ,  myOriginY,
                              midPointX,  myOriginY,
                              midPointX, ( myOriginY + myMajorTickSize ),
                              myOriginX                    , ( myOriginY + myMajorTickSize ),
                              myOriginX                    ,  myOriginY
                            );
        theQPainter->drawPolygon( myBoxArray );

        theQPainter->setBrush( Qt::NoBrush );
        myBoxArray.putPoints( 0, 5,
                              midPointX                    ,  myOriginY,
                              ( myScaleBarWidthInt + myOriginX ),  myOriginY,
                              ( myScaleBarWidthInt + myOriginX ), ( myOriginY + myMajorTickSize ),
                              midPointX                    , ( myOriginY + myMajorTickSize ),
                              midPointX                    ,  myOriginY
                            );
        theQPainter->drawPolygon( myBoxArray );
        break;
      }
      default:
        QgsDebugMsg( "Unknown style" );
    }

    //Do actual drawing of scale bar

    //
    //Do drawing of scale bar text
    //

    QColor myBackColor = Qt::white;
    QColor myForeColor = Qt::black;

    //Draw the minimum label buffer
    theQPainter->setPen( myBackColor );
    myFontWidth = myFontMetrics.width( "0" );
    myFontHeight = myFontMetrics.height();

    for ( int i = 0 - myBufferSize; i <= myBufferSize; i++ )
    {
      for ( int j = 0 - myBufferSize; j <= myBufferSize; j++ )
      {
        theQPainter->drawText( int( i + ( myOriginX - ( myFontWidth / 2 ) ) ),
                               int( j + ( myOriginY - ( myFontHeight / 4 ) ) ),
                               "0" );
      }
    }

    //Draw minimum label
    theQPainter->setPen( myForeColor );

    theQPainter->drawText(
      int( myOriginX - ( myFontWidth / 2 ) ),
      int( myOriginY - ( myFontHeight / 4 ) ),
      "0"
    );

    //
    //Draw maximum label
    //
    theQPainter->setPen( myBackColor );
    myFontWidth = myFontMetrics.width( myScaleBarMaxLabel );
    myFontHeight = myFontMetrics.height();
    //first the buffer
    for ( int i = 0 - myBufferSize; i <= myBufferSize; i++ )
    {
      for ( int j = 0 - myBufferSize; j <= myBufferSize; j++ )
      {
        theQPainter->drawText( int( i + ( myOriginX + myScaleBarWidthInt - ( myFontWidth / 2 ) ) ),
                               int( j + ( myOriginY - ( myFontHeight / 4 ) ) ),
                               myScaleBarMaxLabel );
      }
    }
    //then the text itself
    theQPainter->setPen( myForeColor );
    theQPainter->drawText(
      int( myOriginX + myScaleBarWidthInt - ( myFontWidth / 2 ) ),
      int( myOriginY - ( myFontHeight / 4 ) ),
      myScaleBarMaxLabel
    );

    //
    //Draw unit label
    //
    theQPainter->setPen( myBackColor );
    //first the buffer
    for ( int i = 0 - myBufferSize; i <= myBufferSize; i++ )
    {
      for ( int j = 0 - myBufferSize; j <= myBufferSize; j++ )
      {
        theQPainter->drawText( i + ( myOriginX + myScaleBarWidthInt + myTextOffsetX ),
                               j + ( myOriginY + myMajorTickSize ),
                               myScaleBarUnitLabel );
      }
    }
    //then the text itself
    theQPainter->setPen( myForeColor );
    theQPainter->drawText(
      ( myOriginX + myScaleBarWidthInt + myTextOffsetX ), ( myOriginY + myMajorTickSize ),
      myScaleBarUnitLabel
    );
  }
}
