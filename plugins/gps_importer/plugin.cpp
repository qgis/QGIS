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

#include "../../src/qgisapp.h"
#include "../../src/qgsmaplayer.h"
#include "../../src/qgsrasterlayer.h"
#include "plugin.h"


#include <qfiledialog.h>
#include <qtoolbar.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qlineedit.h>
#include <qaction.h>
#include <qapplication.h>
#include <qcursor.h>

//non qt includes
#include <iostream>

//the gui subclass
#include "plugingui.h"

// xpm for creating the toolbar icon
#include "icon.xpm"
// 
static const char * const ident_ = "$Id$";

static const char * const name_ = "GPS Dump Importer";
static const char * const description_ = "An importer for GPS dump files.";
static const char * const version_ = "Version 0.1";
static const QgisPlugin::PLUGINTYPE type_ = QgisPlugin::UI;
/**
 * Constructor for the plugin. The plugin is passed a pointer to the main app
 * and an interface object that provides access to exposed functions in QGIS.
 * @param qgis Pointer to the QGIS main window
 * @param _qI Pointer to the QGIS interface object
 */
Plugin::Plugin(QgisApp * theQGisApp, QgisIface * theQgisInterFace):
          qgisMainWindowPointer(theQGisApp), 
          qGisInterface(theQgisInterFace),
          QgisPlugin(name_,description_,version_,type_)
{
}

Plugin::~Plugin()
{

}

/* Following functions return name, description, version, and type for the plugin */
QString Plugin::name()
{
  return pluginNameQString;
}

QString Plugin::version()
{
  return pluginVersionQString;

}

QString Plugin::description()
{
  return pluginDescriptionQString;

}

int Plugin::type()
{
  return QgisPlugin::UI;
}

/*
 * Initialize the GUI interface for the plugin 
 */
void Plugin::initGui()
{
  // add a menu with 2 items
  QPopupMenu *pluginMenu = new QPopupMenu(qgisMainWindowPointer);

  pluginMenu->insertItem(QIconSet(icon),"&GpsImporter", this, SLOT(run()));
  pluginMenu->insertItem("&Add GPX or LOC layer", this, SLOT(addGPXLayer()));
  
  menuBarPointer = ((QMainWindow *) qgisMainWindowPointer)->menuBar();

  menuIdInt = qGisInterface->addMenu("&Gps", pluginMenu);
  // Create the action for tool
  QAction *myQActionPointer = new QAction("Import GPS Data", QIconSet(icon), "&Wmi",0, this, "run");
  // Connect the action to the run
  connect(myQActionPointer, SIGNAL(activated()), this, SLOT(run()));
  // Add the toolbar
  toolBarPointer = new QToolBar((QMainWindow *) qgisMainWindowPointer, "Gps");
  toolBarPointer->setLabel("Import GPS Data");
  // Add the zoom previous tool to the toolbar
  myQActionPointer->addTo(toolBarPointer);


}

//method defined in interface
void Plugin::help()
{
  //implement me!
}

// Slot called when the buffer menu item is activated
void Plugin::run()
{
  PluginGui *myPluginGui=new PluginGui(qgisMainWindowPointer,"Import GPS Data",true,0);
  //listen for when the layer has been made so we can draw it
  connect(myPluginGui, SIGNAL(drawRasterLayer(QString)), this, SLOT(drawRasterLayer(QString)));
  connect(myPluginGui, SIGNAL(drawVectorLayer(QString,QString,QString)), this, SLOT(drawVectorLayer(QString,QString,QString)));
  myPluginGui->show();
}

// Slot calles when the user wants to add a GPX or LOC layer
void Plugin::addGPXLayer() {
  QString gpxFileName = 
    QFileDialog::getOpenFileName("", "GPS eXchange format (*.gpx);;"
				 "Geocaching locations (*.loc)", 
				 qgisMainWindowPointer,
				 "Select a GPX or LOC file",
				 "Select a GPX or LOC file");
  if (gpxFileName != 0) {
    qGisInterface->addVectorLayer(gpxFileName + "?type=track", 
				  "Tracks", "gpx");
    qGisInterface->addVectorLayer(gpxFileName + "?type=route",
				  "Routes", "gpx");
    qGisInterface->addVectorLayer(gpxFileName + "?type=waypoint", 
				  "Waypoints", "gpx");
  }
}

//!draw a raster layer in the qui - intended to respond to signal sent by diolog when it as finished creating
//layer
void Plugin::drawRasterLayer(QString theQString)
{
  qGisInterface->addRasterLayer(theQString);
}
//!draw a vector layer in the qui - intended to respond to signal sent by diolog when it as finished creating a layer
////needs to be given vectorLayerPath, baseName, providerKey ("ogr" or "postgres");
void Plugin::drawVectorLayer(QString thePathNameQString, QString theBaseNameQString, QString theProviderQString)
{
 qGisInterface->addVectorLayer( thePathNameQString, theBaseNameQString, theProviderQString);
}

// Unload the plugin by cleaning up the GUI
void Plugin::unload()
{
  // remove the GUI
  menuBarPointer->removeItem(menuIdInt);
  delete toolBarPointer;
}
/** 
 * Required extern functions needed  for every plugin 
 * These functions can be called prior to creating an instance
 * of the plugin class
 */
// Class factory to return a new instance of the plugin class
extern "C" QgisPlugin * classFactory(QgisApp * theQGisAppPointer, QgisIface * theQgisInterfacePointer)
{
  return new Plugin(theQGisAppPointer, theQgisInterfacePointer);
}

// Return the name of the plugin - note that we do not user class members as
// the class may not yet be insantiated when this method is called.
extern "C" QString name()
{
    return name_;
}

// Return the description
extern "C" QString description()
{
    return description_;
}

// Return the type (either UI or MapLayer plugin)
extern "C" int type()
{
    return type_;
}

// Return the version number for the plugin
extern "C" QString version()
{
  return version_;
}

// Delete ourself
extern "C" void unload(QgisPlugin * thePluginPointer)
{
  delete thePluginPointer;
}
