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
#include <qgsrasterlayer.h>
#include "plugin.h"


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

static const char * const name_ = "FishingSpots";
static const char * const description_ = "A simple plugin to map fishing spots and harbours mentioned in fishing reports.";
static const char * const version_ = "Version 0.1";
static const QgisPlugin::PLUGINTYPE type_ = QgisPlugin::UI;


/**
 * Constructor for the plugin. The plugin is passed a pointer to the main app
 * and an interface object that provides access to exposed functions in QGIS.
 * @param qgis Pointer to the QGIS main window
 * @param _qI Pointer to the QGIS interface object
 */
QgsFishingSpotsPlugin::QgsFishingSpotsPlugin(QgisApp * theQGisApp, QgisIface * theQgisInterFace):
          qgisMainWindowPointer(theQGisApp), 
          qGisInterface(theQgisInterFace),
          QgisPlugin(name_,description_,version_,type_)
{
}

QgsFishingSpotsPlugin::~QgsFishingSpotsPlugin()
{

}

/*
 * Initialize the GUI interface for the plugin 
 */
void QgsFishingSpotsPlugin::initGui()
{
  QPopupMenu *pluginMenu = qGisInterface->getPluginMenu("&Tools");
  menuId = pluginMenu->insertItem(QIconSet(icon),"&Fishing Spots", this, SLOT(run()));

  // Create the action for tool
  QAction *myQActionPointer = new QAction("Fishing Spots", QIconSet(icon), "&Wmi",0, this, "run");
  // Connect the action to the run
  connect(myQActionPointer, SIGNAL(activated()), this, SLOT(run()));
  // Add the toolbar
  toolBarPointer = new QToolBar((QMainWindow *) qgisMainWindowPointer, "Tools");
  toolBarPointer->setLabel("Fishing Spots");
  // Add the zoom previous tool to the toolbar
  myQActionPointer->addTo(toolBarPointer);


}
//method defined in interface
void QgsFishingSpotsPlugin::help()
{
  //implement me!
}

// Slot called when the buffer menu item is activated
void QgsFishingSpotsPlugin::run()
{
  QgsFishingSpotsPluginGui *myPluginGui=new QgsFishingSpotsPluginGui(qgisMainWindowPointer,"Fishing Spots",true,0);
  //listen for when the layer has been made so we can draw it
  connect(myPluginGui, SIGNAL(drawRasterLayer(QString)), this, SLOT(drawRasterLayer(QString)));
  connect(myPluginGui, SIGNAL(drawVectorLayer(QString,QString,QString)), this, SLOT(drawVectorLayer(QString,QString,QString)));
  myPluginGui->show();
}
//!draw a raster layer in the qui - intended to respond to signal sent by diolog when it as finished creating
//layer
void QgsFishingSpotsPlugin::drawRasterLayer(QString theQString)
{
  qGisInterface->addRasterLayer(theQString);
}
//!draw a vector layer in the qui - intended to respond to signal sent by diolog when it as finished creating a layer
////needs to be given vectorLayerPath, baseName, providerKey ("ogr" or "postgres");
void QgsFishingSpotsPlugin::drawVectorLayer(QString thePathNameQString, QString theBaseNameQString, QString theProviderQString)
{
 qGisInterface->addVectorLayer( thePathNameQString, theBaseNameQString, theProviderQString);
}

// Unload the plugin by cleaning up the GUI
void QgsFishingSpotsPlugin::unload()
{
  // remove the GUI
  qGisInterface->removePluginMenuItem("&Tools",menuId);
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
  return new QgsFishingSpotsPlugin(theQGisAppPointer, theQgisInterfacePointer);
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
