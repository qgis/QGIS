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

#include <qgisapp.h>
#include <qgsmaplayer.h>
#include "plugin.h"
#include "qgsproject.h"


#include <qtoolbar.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qlineedit.h>
#include <qaction.h>
#include <qapplication.h>
#include <qcursor.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qfont.h>
#include <qpicture.h>
#include <qpointarray.h>
#include <qpaintdevicemetrics.h>
#include <qglobal.h>

//non qt includes
#include <iostream>
#include <cmath>
#include <cassert>

//the gui subclass
#include "plugingui.h"

// xpm for creating the toolbar icon
#include "icon.xpm"

#ifdef WIN32
#define QGISEXTERN extern "C" __declspec( dllexport )
#else
#define QGISEXTERN extern "C"
#endif

//
static const char * const ident_ = "$Id$";

static const char * const name_ = "NorthArrow";
static const char * const description_ = "This plugin displays a north arrow overlayed onto the map.";
static const char * const version_ = "Version 0.1";
static const QgisPlugin::PLUGINTYPE type_ = QgisPlugin::UI;

const double QgsNorthArrowPlugin::PI = 3.14159265358979323846;
//  const double QgsNorthArrowPlugin::DEG2RAD = 0.0174532925199433;
const double QgsNorthArrowPlugin::TOL = 1e-8;


/**
 * Constructor for the plugin. The plugin is passed a pointer to the main app
 * and an interface object that provides access to exposed functions in QGIS.
 * @param qgis Pointer to the QGIS main window
 * @param _qI Pointer to the QGIS interface object
 */
QgsNorthArrowPlugin::QgsNorthArrowPlugin(QgisApp * theQGisApp, QgisIface * theQgisInterFace):
    qgisMainWindowPointer(theQGisApp),
    qGisInterface(theQgisInterFace),
    QgisPlugin(name_,description_,version_,type_)
{
  mRotationInt=0;
  mAutomatic=true;
  mPlacement=tr("Bottom Left");
}

QgsNorthArrowPlugin::~QgsNorthArrowPlugin()
{
}

  /*
 * Initialize the GUI interface for the plugin
 */
void QgsNorthArrowPlugin::initGui()
{
  QPopupMenu *pluginMenu = qGisInterface->getPluginMenu("&Decorations");
  menuId = pluginMenu->insertItem(QIconSet(icon),"&NorthArrow", this, SLOT(run()));

  pluginMenu->setWhatsThis(menuId, "Creates a north arrow that is displayed on the map canvas");

  // Create the action for tool
#if QT_VERSION < 0x040000
  myQActionPointer = new QAction("North Arrow", QIconSet(icon), "&Wmi",0, this, "run");
#else
  myQActionPointer = new QAction(QIcon(icon), "North Arrow", this);
#endif
  myQActionPointer->setWhatsThis("Creates a north arrow that is displayed on the map canvas");
  // Connect the action to the run
  connect(myQActionPointer, SIGNAL(activated()), this, SLOT(run()));
  //render the arrow each time the map is rendered
  connect(qGisInterface->getMapCanvas(), SIGNAL(renderComplete(QPainter *)), this, SLOT(renderNorthArrow(QPainter *)));
  //this resets this plugin up if a project is loaded
  connect(qgisMainWindowPointer, SIGNAL(projectRead()), this, SLOT(projectRead()));
  // Add the icon to the toolbar
  qGisInterface->addToolBarIcon(myQActionPointer);
  projectRead();
  refreshCanvas();

}

void QgsNorthArrowPlugin::projectRead()
{
#ifdef QGISDEBUG
    std::cout << "+++++++++ north arrow plugin - project read slot called...." << std::endl;
#endif
    //default text to start with - try to fetch it from qgsproject

    mRotationInt = QgsProject::instance()->readNumEntry("NorthArrow","/Rotation",0);
    mPlacement = QgsProject::instance()->readEntry("NorthArrow","/Placement","Bottom Left");
    mEnable = QgsProject::instance()->readBoolEntry("NorthArrow","/Enabled",true);
    mAutomatic = QgsProject::instance()->readBoolEntry("NorthArrow","/Automatic",true);
}

//method defined in interface
void QgsNorthArrowPlugin::help()
{
  //implement me!
}

// Slot called when the buffer menu item is activated
void QgsNorthArrowPlugin::run()
{
  QgsNorthArrowPluginGui *myPluginGui = new QgsNorthArrowPluginGui(qgisMainWindowPointer,"North Arrow",true,0);
  //overides function byt the same name created in .ui
  myPluginGui->setRotation(mRotationInt);
  myPluginGui->setPlacement(mPlacement);
  myPluginGui->setEnabled(mEnable);
  myPluginGui->setAutomatic(mAutomatic);

  //listen for when the layer has been made so we can draw it
  connect(myPluginGui, SIGNAL(rotationChanged(int)), this, SLOT(rotationChanged(int)));
  connect(myPluginGui, SIGNAL(changePlacement(QString)), this, SLOT(setPlacement(QString)));
  connect(myPluginGui, SIGNAL(enableAutomatic(bool)), this, SLOT(setAutomatic(bool)));
  connect(myPluginGui, SIGNAL(enableNorthArrow(bool)), this, SLOT(setEnabled(bool)));
  connect(myPluginGui, SIGNAL(needToRefresh()), this, SLOT(refreshCanvas()));
  myPluginGui->show();
}

//! Refresh the map display using the mapcanvas exported via the plugin interface
void QgsNorthArrowPlugin::refreshCanvas()
{
  qGisInterface->getMapCanvas()->refresh();
}

void QgsNorthArrowPlugin::renderNorthArrow(QPainter * theQPainter)
{
#ifdef QGISDEBUG
      std::cout << "Rendering n-arrow"  << std::endl;
#endif
  //Large IF statement controlled by enable check box
  if (mEnable)
  {
    QPixmap myQPixmap; //to store the north arrow image in
#if defined(WIN32) || defined(Q_OS_MACX)
    QString PKGDATAPATH = qApp->applicationDirPath() + "/share/qgis";
#endif
    QString myFileNameQString = QString(PKGDATAPATH) +
                                QString("/images/north_arrows/default.png");
    //std::cout << "Trying to load " << myFileNameQString << std::cout;
    if (myQPixmap.load(myFileNameQString))
    {

      double centerXDouble = myQPixmap.width()/2;
      double centerYDouble = myQPixmap.height()/2;
      //save the current canvas rotation
      theQPainter->save();
      //
      //work out how to shift the image so that it rotates
      //           properly about its center
      //(x cos a + y sin a - x, -x sin a + y cos a - y)
      //

      // could move this call to somewhere else so that it is only
      // called when the projection or map extent changes
      if (mAutomatic)
        calculateNorthDirection();

      double myRadiansDouble = mRotationInt * PI / 180.0;
      int xShift = static_cast<int>((
                                      (centerXDouble * cos(myRadiansDouble)) +
                                      (centerYDouble * sin(myRadiansDouble))
                                    ) - centerXDouble);
      int yShift = static_cast<int>((
                                      (-centerXDouble * sin(myRadiansDouble)) +
                                      (centerYDouble * cos(myRadiansDouble))
                                    ) - centerYDouble);

      // need width/height of paint device
      QPaintDeviceMetrics myMetrics( theQPainter->device() );
      int myHeight = myMetrics.height();
      int myWidth = myMetrics.width();

#ifdef QGISDEBUG
      std::cout << "Rendering n-arrow at " << mPlacement.local8Bit() << std::endl;
#endif
      //Determine placement of label from form combo box
      if (mPlacement==tr("Bottom Left"))
      {
        theQPainter->translate(0,myHeight-myQPixmap.height());
      }
      else if (mPlacement==tr("Top Right"))
      {
        theQPainter->translate(myWidth-myQPixmap.width(),0);
      }
      else if (mPlacement==tr("Bottom Right"))
      {
        theQPainter->translate(myWidth-myQPixmap.width(),
                             myHeight-myQPixmap.height());
      }
      else // defaulting to top left
      {
        //no need to translate for TL corner because we're already at the origin
        theQPainter->translate(0, 0);
      }
      //rotate the canvas by the north arrow rotation amount
      theQPainter->rotate( mRotationInt );
      //Now we can actually do the drawing
      theQPainter->drawPixmap(xShift,yShift,myQPixmap);

      //unrotate the canvase again
      theQPainter->restore();

      //bitBlt ( qGisInterface->getMapCanvas()->canvasPixmap(), 0, 0, &myPainterPixmap, 0, 0, -1 , -1, Qt::CopyROP, false);

    }
    else
    {
      QFont myQFont("time", 32, QFont::Bold);
      theQPainter->setFont(myQFont);
      theQPainter->setPen(Qt::black);
      theQPainter->drawText(10, 20, QString("Pixmap Not Found"));
    }
  }

}
// Unload the plugin by cleaning up the GUI
void QgsNorthArrowPlugin::unload()
{
  // remove the GUI
  qGisInterface->removePluginMenuItem("&Decorations",menuId);
  qGisInterface->removeToolBarIcon(myQActionPointer);
  // remove the northarrow from the canvas
  disconnect(qGisInterface->getMapCanvas(), SIGNAL(renderComplete(QPainter *)),
	     this, SLOT(renderNorthArrow(QPainter *)));
  refreshCanvas();

  delete myQActionPointer;
}


void QgsNorthArrowPlugin::rotationChanged(int theInt)
{
  mRotationInt = theInt;
  QgsProject::instance()->writeEntry("NorthArrow","/Rotation", mRotationInt  );
}

//! set placement of north arrow
void QgsNorthArrowPlugin::setPlacement(QString theQString)
{
  mPlacement = theQString;
  QgsProject::instance()->writeEntry("NorthArrow","/Placement", mPlacement);
}

void QgsNorthArrowPlugin::setEnabled(bool theBool)
{
  mEnable = theBool;
  QgsProject::instance()->writeEntry("NorthArrow","/Enabled", mEnable );
}

void QgsNorthArrowPlugin::setAutomatic(bool theBool)
{
  mAutomatic = theBool;
  QgsProject::instance()->writeEntry("NorthArrow","/Automatic", mAutomatic );
  if (mAutomatic)
    calculateNorthDirection();
}

bool QgsNorthArrowPlugin::calculateNorthDirection()
{
  QgsMapCanvas& mapCanvas = *(qGisInterface->getMapCanvas());

  bool goodDirn = false;

  if (mapCanvas.layerCount() > 0)
  {
    // Grab an SRS from any layer
    QgsMapLayer& mapLayer = *(mapCanvas.getZpos(0));
    QgsSpatialRefSys& outputSRS = mapLayer.coordinateTransform()->destSRS();

    bool yy = outputSRS.geographicFlag();

    if (outputSRS.isValid() && !outputSRS.geographicFlag())
    {
      // Use a geographic SRS to get lat/long to work out direction
      QgsSpatialRefSys ourSRS;
      ourSRS.createFromProj4("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");
      assert(ourSRS.isValid());

      QgsCoordinateTransform transform(outputSRS, ourSRS);

      QgsRect extent = mapCanvas.extent();
      QgsPoint p1(extent.center());
      // A point a bit above p1. XXX assumes that y increases up!!
      // May need to involve the maptopixel transform if this proves
      // to be a problem.
      QgsPoint p2(p1.x(), p1.y() + extent.height() * 0.25); 

      // project p1 and p2 to geographic coords
      try
      {        
	p1 = transform.transform(p1);
	p2 = transform.transform(p2);
      }
      catch (QgsException &e)
      {
	// just give up
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
      p1.multiply(PI/180.0);
      p2.multiply(PI/180.0);

      double y = sin(p2.x() - p1.x()) * cos(p2.y());
      double x = cos(p1.y()) * sin(p2.y()) - 
	         sin(p1.y()) * cos(p2.y()) * cos(p2.x()-p1.x());

      if (y > 0.0)
      {
	if (x > TOL) 
	  angle = atan(y/x);
	else if (x < -TOL) 
	  angle = PI - atan(-y/x);
	else
	  angle = 0.5 * PI;
      }
      else if (y < 0.0)
      {
	if (x > TOL)
	  angle = -atan(-y/x);
	else if (x < -TOL)
	  angle = atan(y/x) - PI;
	else
	  angle = 1.5 * PI;
      }
      else
      {
	if (x > TOL)
	  angle = 0.0;
	else if (x < -TOL)
	  angle = PI;
	else
        {
	  angle = 0.0; // p1 = p2
	  goodDirn = false;
	}
      }
      // And set the angle of the north arrow. Perhaps do something
      // different if goodDirn = false.
      mRotationInt = static_cast<int>(round(fmod(360.0-angle*180.0/PI, 360.0)));
    }
    else
    {
      // For geographic SRS and for when there are no layers, set the
      // direction back to the default
      mRotationInt = 0;
    }
  }
  return goodDirn;
}





/**
 * Required extern functions needed  for every plugin
 * These functions can be called prior to creating an instance
 * of the plugin class
 */
// Class factory to return a new instance of the plugin class
QGISEXTERN QgisPlugin * classFactory(QgisApp * theQGisAppPointer, QgisIface * theQgisInterfacePointer)
{
  return new QgsNorthArrowPlugin(theQGisAppPointer, theQgisInterfacePointer);
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

// Delete ourself
QGISEXTERN void unload(QgisPlugin * thePluginPointer)
{
  delete thePluginPointer;
}
