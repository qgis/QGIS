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
#include "qgscrscache.h"

// qt includes
#include <QPainter>
#include <QMenu>
#include <QDir>
#include <QFile>

//non qt includes
#include <cmath>
#include <cassert>


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
    : QgsDecorationItem( parent )
    , mRotationInt( 0 )
    , mAutomatic( true )
    , mMarginHorizontal( 0 )
    , mMarginVertical( 0 )
{
  mPlacement = BottomLeft;
  mMarginUnit = QgsSymbolV2::MM;

  setName( "North Arrow" );
  projectRead();
}

QgsDecorationNorthArrow::~QgsDecorationNorthArrow()
{
}

void QgsDecorationNorthArrow::projectRead()
{
  QgsDecorationItem::projectRead();
  mRotationInt = QgsProject::instance()->readNumEntry( mNameConfig, "/Rotation", 0 );
  mAutomatic = QgsProject::instance()->readBoolEntry( mNameConfig, "/Automatic", true );
  mMarginHorizontal = QgsProject::instance()->readNumEntry( mNameConfig, "/MarginH", 0 );
  mMarginVertical = QgsProject::instance()->readNumEntry( mNameConfig, "/MarginV", 0 );
}

void QgsDecorationNorthArrow::saveToProject()
{
  QgsDecorationItem::saveToProject();
  QgsProject::instance()->writeEntry( mNameConfig, "/Rotation", mRotationInt );
  QgsProject::instance()->writeEntry( mNameConfig, "/Automatic", mAutomatic );
  QgsProject::instance()->writeEntry( mNameConfig, "/MarginH", mMarginHorizontal );
  QgsProject::instance()->writeEntry( mNameConfig, "/MarginV", mMarginVertical );
}

// Slot called when the buffer menu item is activated
void QgsDecorationNorthArrow::run()
{
  QgsDecorationNorthArrowDialog dlg( *this, QgisApp::instance() );
  dlg.exec();
}

void QgsDecorationNorthArrow::render( QPainter * theQPainter )
{

  //Large IF statement controlled by enable check box
  if ( enabled() )
  {
    QPixmap myQPixmap; //to store the north arrow image in

    QString myFileNameQString = ":/images/north_arrows/default.png";

    if ( myQPixmap.load( myFileNameQString ) )
    {
      double centerXDouble = myQPixmap.width() / 2.0;
      double centerYDouble = myQPixmap.height() / 2.0;
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

      // Set  margin according to selected units
      int myXOffset = 0;
      int myYOffset = 0;
      switch ( mMarginUnit )
      {
        case QgsSymbolV2::MM:
        {
          int myPixelsInchX = theQPainter->device()->logicalDpiX();
          int myPixelsInchY = theQPainter->device()->logicalDpiY();
          myXOffset = myPixelsInchX * INCHES_TO_MM * mMarginHorizontal;
          myYOffset = myPixelsInchY * INCHES_TO_MM * mMarginVertical;
          break;
        }

        case QgsSymbolV2::Pixel:
          myXOffset = mMarginHorizontal - 5; // Minus 5 to shift tight into corner
          myYOffset = mMarginVertical - 5;
          break;

        case QgsSymbolV2::Percentage:
          myXOffset = (( myWidth - myQPixmap.width() ) / 100. ) * mMarginHorizontal;
          myYOffset = (( myHeight - myQPixmap.height() ) / 100. ) * mMarginVertical;
          break;

        default:  // Use default of top left
          break;
      }
      //Determine placement of label from form combo box
      switch ( mPlacement )
      {
        case BottomLeft:
          theQPainter->translate( myXOffset, myHeight - myYOffset - myQPixmap.height() );
          break;
        case TopLeft:
          theQPainter->translate( myXOffset, myYOffset );
          break;
        case TopRight:
          theQPainter->translate( myWidth - myXOffset - myQPixmap.width(), myYOffset );
          break;
        case BottomRight:
          theQPainter->translate( myWidth - myXOffset - myQPixmap.width(),
                                  myHeight - myYOffset - myQPixmap.height() );
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

  // Get the shown extent...
  QgsRectangle canvasExtent = mapCanvas->extent();
  // ... and all layers extent, ...
  QgsRectangle fullExtent = mapCanvas->fullExtent();
  // ... and combine
  QgsRectangle extent = canvasExtent.intersect( & fullExtent );

  // If no layers are added or shown, we can't get any direction
  if ( mapCanvas->layerCount() > 0 && ! extent.isEmpty() )
  {
    QgsCoordinateReferenceSystem outputCRS = mapCanvas->mapSettings().destinationCrs();

    if ( outputCRS.isValid() && !outputCRS.geographicFlag() )
    {
      // Use a geographic CRS to get lat/long to work out direction
      QgsCoordinateReferenceSystem ourCRS = QgsCRSCache::instance()->crsByOgcWmsCrs( GEO_EPSG_CRS_AUTHID );
      assert( ourCRS.isValid() );

      QgsCoordinateTransform transform( outputCRS, ourCRS );

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

      // Use TOL to decide if the quotient is big enough.
      // Both x and y can be very small, if heavily zoomed
      // For small y/x, we set directly angle 0. Not sure
      // if this is needed.
      if ( y > 0.0 )
      {
        if ( x > 0.0 && ( y / x ) > TOL )
          angle = atan( y / x );
        else if ( x < 0.0 && ( y / x ) < -TOL )
          angle = PI - atan( -y / x );
        else
          angle = 0.5 * PI;
      }
      else if ( y < 0.0 )
      {
        if ( x > 0.0 && ( y / x ) < -TOL )
          angle = -atan( -y / x );
        else if ( x < 0.0 && ( y / x ) > TOL )
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
      mRotationInt = qRound( fmod( 360.0 - angle * 180.0 / PI, 360.0 ) );
    }
    else
    {
      // For geographic CRS and for when there are no layers, set the
      // direction back to the default
      mRotationInt = 0;
    }
  }

  mRotationInt += mapCanvas->mapSettings().rotation();

  return goodDirn;
}
