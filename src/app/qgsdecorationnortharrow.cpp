/***************************************************************************
  plugin.cpp
  Import tool for various worldmap analysis output files
Functions:

-------------------
begin                : Jan 21, 2004
copyright            : (C) 2004 by Tim Sutton
email                : tim@linfiniti.com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// includes
#include "qgsdecorationnortharrow.h"

#include "qgsdecorationnortharrowdialog.h"

#include "qgisapp.h"
#include "qgscoordinatetransform.h"
#include "qgsmaplayer.h"
#include "qgsproject.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaprenderer.h"

// qt includes
#include <QPainter>
#include <QMenu>
#include <QDir>
#include <QFile>

//non qt includes
#include <iostream>
#include <cmath>
#include <cassert>


#ifdef _MSC_VER
#define round(x)  ((x) >= 0 ? floor((x)+0.5) : floor((x)-0.5))
#endif

const double QgsDecorationNorthArrow::PI = 3.14159265358979323846;
//  const double QgsNorthArrowPlugin::DEG2RAD = 0.0174532925199433;
const double QgsDecorationNorthArrow::TOL = 1e-8;


/**
 * Constructor for the plugin. The plugin is passed a pointer to the main app
 * and an interface object that provides access to exposed functions in QGIS.
 * @param qgis Pointer to the QGIS main window
 * @param _qI Pointer to the QGIS interface object
 */
QgsDecorationNorthArrow::QgsDecorationNorthArrow( QObject* parent )
    : QObject( parent )
{
  mRotationInt = 0;
  mAutomatic = true;
  mPlacementLabels << tr( "Bottom Left" ) << tr( "Top Left" )
  << tr( "Top Right" ) << tr( "Bottom Right" );

  projectRead();
}

QgsDecorationNorthArrow::~QgsDecorationNorthArrow()
{
}

void QgsDecorationNorthArrow::projectRead()
{
  //default text to start with - try to fetch it from qgsproject

  mRotationInt = QgsProject::instance()->readNumEntry( "NorthArrow", "/Rotation", 0 );
  mPlacementIndex = QgsProject::instance()->readNumEntry( "NorthArrow", "/Placement", 0 );
  mEnable = QgsProject::instance()->readBoolEntry( "NorthArrow", "/Enabled", false );
  mAutomatic = QgsProject::instance()->readBoolEntry( "NorthArrow", "/Automatic", true );
}

void QgsDecorationNorthArrow::saveToProject()
{
  QgsProject::instance()->writeEntry( "NorthArrow", "/Rotation", mRotationInt );
  QgsProject::instance()->writeEntry( "NorthArrow", "/Placement", mPlacementIndex );
  QgsProject::instance()->writeEntry( "NorthArrow", "/Enabled", mEnable );
  QgsProject::instance()->writeEntry( "NorthArrow", "/Automatic", mAutomatic );
}

// Slot called when the buffer menu item is activated
void QgsDecorationNorthArrow::run()
{
  QgsDecorationNorthArrowDialog dlg( *this, QgisApp::instance() );

  if ( dlg.exec() )
  {
    saveToProject();
    QgisApp::instance()->mapCanvas()->refresh();
  }
}

void QgsDecorationNorthArrow::renderNorthArrow( QPainter * theQPainter )
{

  //Large IF statement controlled by enable check box
  if ( mEnable )
  {
    if ( theQPainter->isActive() )
    {
      //QgsDebugMsg("Rendering north arrow on active painter");
    }
    else
    {
      //QgsDebugMsg("Rendering north arrow on INactive painter!!!");
    }

    QPixmap myQPixmap; //to store the north arrow image in

    QString myFileNameQString = ":/images/north_arrows/default.png";

    //QgsDebugMsg("Trying to load " + myFileNameQString);
    if ( myQPixmap.load( myFileNameQString ) )
    {

      double centerXDouble = myQPixmap.width() / 2;
      double centerYDouble = myQPixmap.height() / 2;
      //save the current canvas rotation
      theQPainter->save();
      //
      //work out how to shift the image so that it rotates
      //           properly about its center
      //(x cos a + y sin a - x, -x sin a + y cos a - y)
      //

      // could move this call to somewhere else so that it is only
      // called when the projection or map extent changes
      if ( mAutomatic )
        calculateNorthDirection();

      double myRadiansDouble = mRotationInt * PI / 180.0;
      int xShift = static_cast<int>((
                                      ( centerXDouble * cos( myRadiansDouble ) ) +
                                      ( centerYDouble * sin( myRadiansDouble ) )
                                    ) - centerXDouble );
      int yShift = static_cast<int>((
                                      ( -centerXDouble * sin( myRadiansDouble ) ) +
                                      ( centerYDouble * cos( myRadiansDouble ) )
                                    ) - centerYDouble );

      // need width/height of paint device
      int myHeight = theQPainter->device()->height();
      int myWidth = theQPainter->device()->width();

      //QgsDebugMsg("Rendering north arrow at " + mPlacementLabels.at(mPlacementIndex));

      //Determine placement of label from form combo box
      switch ( mPlacementIndex )
      {
        case 0: // Bottom Left
          theQPainter->translate( 0, myHeight - myQPixmap.height() );
          break;
        case 1: // Top Left
          //no need to translate for TL corner because we're already at the origin
          theQPainter->translate( 0, 0 );
          break;
        case 2: // Top Right
          theQPainter->translate( myWidth - myQPixmap.width(), 0 );
          break;
        case 3: // Bottom Right
          theQPainter->translate( myWidth - myQPixmap.width(),
                                  myHeight - myQPixmap.height() );
          break;
        default:
        {
          //QgsDebugMsg("Unable to determine where to put north arrow so defaulting to top left");
        }
      }
      //rotate the canvas by the north arrow rotation amount
      theQPainter->rotate( mRotationInt );
      //Now we can actually do the drawing, and draw a smooth north arrow even when rotated
      theQPainter->setRenderHint( QPainter::SmoothPixmapTransform );
      theQPainter->drawPixmap( xShift, yShift, myQPixmap );

      //unrotate the canvas again
      theQPainter->restore();
    }
    else
    {
      QFont myQFont( "time", 12, QFont::Bold );
      theQPainter->setFont( myQFont );
      theQPainter->setPen( Qt::black );
      theQPainter->drawText( 10, 20, tr( "North arrow pixmap not found" ) );
    }
  }

}

bool QgsDecorationNorthArrow::calculateNorthDirection()
{
  QgsMapCanvas* mapCanvas = QgisApp::instance()->mapCanvas();

  bool goodDirn = false;

  if ( mapCanvas->layerCount() > 0 )
  {
    QgsCoordinateReferenceSystem outputCRS = mapCanvas->mapRenderer()->destinationCrs();

    if ( outputCRS.isValid() && !outputCRS.geographicFlag() )
    {
      // Use a geographic CRS to get lat/long to work out direction
      QgsCoordinateReferenceSystem ourCRS;
      ourCRS.createFromOgcWmsCrs( GEO_EPSG_CRS_AUTHID );
      assert( ourCRS.isValid() );

      QgsCoordinateTransform transform( outputCRS, ourCRS );

      QgsRectangle extent = mapCanvas->extent();
      QgsPoint p1( extent.center() );
      // A point a bit above p1. XXX assumes that y increases up!!
      // May need to involve the maptopixel transform if this proves
      // to be a problem.
      QgsPoint p2( p1.x(), p1.y() + extent.height() * 0.25 );

      // project p1 and p2 to geographic coords
      try
      {
        p1 = transform.transform( p1 );
        p2 = transform.transform( p2 );
      }
      catch ( QgsCsException &e )
      {
        Q_UNUSED( e );
        // just give up
        QgsDebugMsg( "North Arrow: Transformation error, quitting" );
        return false;
      }

      // Work out the value of the initial heading one takes to go
      // from point p1 to point p2. The north direction is then that
      // many degrees anti-clockwise or vertical.

      // Take some care to not divide by zero, etc, and ensure that we
      // get sensible results for all possible values for p1 and p2.

      goodDirn = true;
      double angle = 0.0;

      // convert to radians for the equations below
      p1.multiply( PI / 180.0 );
      p2.multiply( PI / 180.0 );

      double y = sin( p2.x() - p1.x() ) * cos( p2.y() );
      double x = cos( p1.y() ) * sin( p2.y() ) -
                 sin( p1.y() ) * cos( p2.y() ) * cos( p2.x() - p1.x() );

      if ( y > 0.0 )
      {
        if ( x > TOL )
          angle = atan( y / x );
        else if ( x < -TOL )
          angle = PI - atan( -y / x );
        else
          angle = 0.5 * PI;
      }
      else if ( y < 0.0 )
      {
        if ( x > TOL )
          angle = -atan( -y / x );
        else if ( x < -TOL )
          angle = atan( y / x ) - PI;
        else
          angle = 1.5 * PI;
      }
      else
      {
        if ( x > TOL )
          angle = 0.0;
        else if ( x < -TOL )
          angle = PI;
        else
        {
          angle = 0.0; // p1 = p2
          goodDirn = false;
        }
      }
      // And set the angle of the north arrow. Perhaps do something
      // different if goodDirn = false.
      mRotationInt = static_cast<int>( round( fmod( 360.0 - angle * 180.0 / PI, 360.0 ) ) );
    }
    else
    {
      // For geographic CRS and for when there are no layers, set the
      // direction back to the default
      mRotationInt = 0;
    }
  }
  return goodDirn;
}
