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
/*  $Id$ */

// includes

#include "qgisinterface.h"
#include "qgisgui.h"
#include "qgscoordinatetransform.h"
#include "qgsmaplayer.h"
#include "plugin.h"
#include "qgsproject.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaprenderer.h"
#include "qgsapplication.h"

// qt includes
#include <QPainter>
#include <QMenu>
#include <QDir>
#include <QFile>

//non qt includes
#include <iostream>
#include <cmath>
#include <cassert>

//the gui subclass
#include "plugingui.h"



#ifdef _MSC_VER
#define round(x)  ((x) >= 0 ? floor((x)+0.5) : floor((x)-0.5))
#endif

//
static const char * const ident_ = "$Id$";

static const QString name_ = QObject::tr( "NorthArrow" );
static const QString description_ = QObject::tr( "Displays a north arrow overlayed onto the map" );
static const QString version_ = QObject::tr( "Version 0.1" );
static const QgisPlugin::PLUGINTYPE type_ = QgisPlugin::UI;
static const QString icon_ = ":/north_arrow.png";

const double QgsNorthArrowPlugin::PI = 3.14159265358979323846;
//  const double QgsNorthArrowPlugin::DEG2RAD = 0.0174532925199433;
const double QgsNorthArrowPlugin::TOL = 1e-8;


/**
 * Constructor for the plugin. The plugin is passed a pointer to the main app
 * and an interface object that provides access to exposed functions in QGIS.
 * @param qgis Pointer to the QGIS main window
 * @param _qI Pointer to the QGIS interface object
 */
QgsNorthArrowPlugin::QgsNorthArrowPlugin( QgisInterface * theQgisInterFace ):
    QgisPlugin( name_, description_, version_, type_ ),
    qGisInterface( theQgisInterFace )
{
  mRotationInt = 0;
  mAutomatic = true;
  mPlacementLabels << tr( "Bottom Left" ) << tr( "Top Left" )
  << tr( "Top Right" ) << tr( "Bottom Right" );
}

QgsNorthArrowPlugin::~QgsNorthArrowPlugin()
{
}

/*
* Initialize the GUI interface for the plugin
*/
void QgsNorthArrowPlugin::initGui()
{
  // Create the action for tool
  myQActionPointer = new QAction( QIcon(), tr( "&North Arrow" ), this );
  setCurrentTheme( "" );
  myQActionPointer->setWhatsThis( tr( "Creates a north arrow that is displayed on the map canvas" ) );
  // Connect the action to the run
  connect( myQActionPointer, SIGNAL( triggered() ), this, SLOT( run() ) );
  //render the arrow each time the map is rendered
  connect( qGisInterface->mapCanvas(), SIGNAL( renderComplete( QPainter * ) ), this, SLOT( renderNorthArrow( QPainter * ) ) );
  //this resets this plugin up if a project is loaded
  connect( qGisInterface->mainWindow(), SIGNAL( projectRead() ), this, SLOT( projectRead() ) );
  // Add the icon to the toolbar & appropriate menu
  qGisInterface->addToolBarIcon( myQActionPointer );
  qGisInterface->addPluginToMenu( tr( "&Decorations" ), myQActionPointer );
  // this is called when the icon theme is changed
  connect( qGisInterface, SIGNAL( currentThemeChanged( QString ) ), this, SLOT( setCurrentTheme( QString ) ) );

  projectRead();
  refreshCanvas();

}

void QgsNorthArrowPlugin::projectRead()
{
  //default text to start with - try to fetch it from qgsproject

  mRotationInt = QgsProject::instance()->readNumEntry( "NorthArrow", "/Rotation", 0 );
  mPlacementIndex = QgsProject::instance()->readNumEntry( "NorthArrow", "/Placement", 0 );
  mEnable = QgsProject::instance()->readBoolEntry( "NorthArrow", "/Enabled", true );
  mAutomatic = QgsProject::instance()->readBoolEntry( "NorthArrow", "/Automatic", true );
}

//method defined in interface
void QgsNorthArrowPlugin::help()
{
  //implement me!
}

// Slot called when the buffer menu item is activated
void QgsNorthArrowPlugin::run()
{
  QgsNorthArrowPluginGui *myPluginGui = new QgsNorthArrowPluginGui( qGisInterface->mainWindow(), QgisGui::ModalDialogFlags );
  myPluginGui->setAttribute( Qt::WA_DeleteOnClose );
  //overides function by the same name created in .ui
  myPluginGui->setRotation( mRotationInt );
  myPluginGui->setPlacementLabels( mPlacementLabels );
  myPluginGui->setPlacement( mPlacementIndex );
  myPluginGui->setEnabled( mEnable );
  myPluginGui->setAutomatic( mAutomatic );

  //listen for when the layer has been made so we can draw it
  connect( myPluginGui, SIGNAL( rotationChanged( int ) ), this, SLOT( rotationChanged( int ) ) );
  connect( myPluginGui, SIGNAL( changePlacement( int ) ), this, SLOT( setPlacement( int ) ) );
  connect( myPluginGui, SIGNAL( enableAutomatic( bool ) ), this, SLOT( setAutomatic( bool ) ) );
  connect( myPluginGui, SIGNAL( enableNorthArrow( bool ) ), this, SLOT( setEnabled( bool ) ) );
  connect( myPluginGui, SIGNAL( needToRefresh() ), this, SLOT( refreshCanvas() ) );
  myPluginGui->show();
}

//! Refresh the map display using the mapcanvas exported via the plugin interface
void QgsNorthArrowPlugin::refreshCanvas()
{
  qGisInterface->mapCanvas()->refresh();
}

void QgsNorthArrowPlugin::renderNorthArrow( QPainter * theQPainter )
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

    QString myFileNameQString = QDir::cleanPath( QgsApplication::pkgDataPath() +
                                "/images/north_arrows/default.png" );

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
// Unload the plugin by cleaning up the GUI
void QgsNorthArrowPlugin::unload()
{
  // remove the GUI
  qGisInterface->removePluginMenu( tr( "&Decorations" ), myQActionPointer );
  qGisInterface->removeToolBarIcon( myQActionPointer );
  // remove the northarrow from the canvas
  disconnect( qGisInterface->mapCanvas(), SIGNAL( renderComplete( QPainter * ) ),
              this, SLOT( renderNorthArrow( QPainter * ) ) );
  refreshCanvas();

  delete myQActionPointer;
}


void QgsNorthArrowPlugin::rotationChanged( int theInt )
{
  mRotationInt = theInt;
  QgsProject::instance()->writeEntry( "NorthArrow", "/Rotation", mRotationInt );
}

//! set placement of north arrow
void QgsNorthArrowPlugin::setPlacement( int placementIndex )
{
  mPlacementIndex = placementIndex;
  QgsProject::instance()->writeEntry( "NorthArrow", "/Placement", mPlacementIndex );
}

void QgsNorthArrowPlugin::setEnabled( bool theBool )
{
  mEnable = theBool;
  QgsProject::instance()->writeEntry( "NorthArrow", "/Enabled", mEnable );
}

void QgsNorthArrowPlugin::setAutomatic( bool theBool )
{
  mAutomatic = theBool;
  QgsProject::instance()->writeEntry( "NorthArrow", "/Automatic", mAutomatic );
  if ( mAutomatic )
    calculateNorthDirection();
}

bool QgsNorthArrowPlugin::calculateNorthDirection()
{
  QgsMapCanvas& mapCanvas = *( qGisInterface->mapCanvas() );

  bool goodDirn = false;

  if ( mapCanvas.layerCount() > 0 )
  {
    QgsCoordinateReferenceSystem outputCRS = mapCanvas.mapRenderer()->destinationSrs();

    if ( outputCRS.isValid() && !outputCRS.geographicFlag() )
    {
      // Use a geographic CRS to get lat/long to work out direction
      QgsCoordinateReferenceSystem ourCRS;
      ourCRS.createFromProj4( "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs" );
      assert( ourCRS.isValid() );

      QgsCoordinateTransform transform( outputCRS, ourCRS );

      QgsRectangle extent = mapCanvas.extent();
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

//! Set icons to the current theme
void QgsNorthArrowPlugin::setCurrentTheme( QString theThemeName )
{
  QString myCurThemePath = QgsApplication::activeThemePath() + "/plugins/north_arrow.png";
  QString myDefThemePath = QgsApplication::defaultThemePath() + "/plugins/north_arrow.png";
  QString myQrcPath = ":/north_arrow.png";
  if ( QFile::exists( myCurThemePath ) )
  {
    myQActionPointer->setIcon( QIcon( myCurThemePath ) );
  }
  else if ( QFile::exists( myDefThemePath ) )
  {
    myQActionPointer->setIcon( QIcon( myDefThemePath ) );
  }
  else if ( QFile::exists( myQrcPath ) )
  {
    myQActionPointer->setIcon( QIcon( myQrcPath ) );
  }
  else
  {
    myQActionPointer->setIcon( QIcon() );
  }
}

/**
 * Required extern functions needed  for every plugin
 * These functions can be called prior to creating an instance
 * of the plugin class
 */
// Class factory to return a new instance of the plugin class
QGISEXTERN QgisPlugin * classFactory( QgisInterface * theQgisInterfacePointer )
{
  return new QgsNorthArrowPlugin( theQgisInterfacePointer );
}

// Return the name of the plugin - note that we do not user class members as
// the class may not yet be insantiated when this method is called.
QGISEXTERN QString name()
{
  return name_;
}

// Return the description
QGISEXTERN QString description()
{
  return description_;
}

// Return the type (either UI or MapLayer plugin)
QGISEXTERN int type()
{
  return type_;
}

// Return the version number for the plugin
QGISEXTERN QString version()
{
  return version_;
}

QGISEXTERN QString icon()
{
  return icon_;
}

// Delete ourself
QGISEXTERN void unload( QgisPlugin * thePluginPointer )
{
  delete thePluginPointer;
}
