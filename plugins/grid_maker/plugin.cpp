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
#ifdef WIN32
#define QGISEXTERN extern "C" __declspec( dllexport )
#else
#define QGISEXTERN extern "C"
#endif


static const char * const ident_ = "$Id$";

static const char * const name_ = "Graticule Creator";
static const char * const description_ = "A graticule building plugin.";
static const char * const version_ = "Version 0.1";
static const QgisPlugin::PLUGINTYPE type_ = QgisPlugin::UI;
/**
 * Constructor for the plugin. The plugin is passed a pointer to the main app
 * and an interface object that provides access to exposed functions in QGIS.
 * @param qgis Pointer to the QGIS main window
 * @param _qI Pointer to the QGIS interface object
 */
QgsGridMakerPlugin::QgsGridMakerPlugin(QgisApp * theQGisApp, 
				       QgisIface * theQgisInterFace):
          qgisMainWindowPointer(theQGisApp), 
          qGisInterface(theQgisInterFace),
          QgisPlugin(name_,description_,version_,type_)
{
}

QgsGridMakerPlugin::~QgsGridMakerPlugin()
{

}

/* Following functions return name, description, version, and type for the plugin */
QString QgsGridMakerPlugin::name()
{
  return pluginNameQString;
}

QString QgsGridMakerPlugin::version()
{
  return pluginVersionQString;

}

QString QgsGridMakerPlugin::description()
{
  return pluginDescriptionQString;

}

int QgsGridMakerPlugin::type()
{
  return QgisPlugin::UI;
}

/*
 * Initialize the GUI interface for the plugin 
 */
void QgsGridMakerPlugin::initGui()
{
  // add a menu with 2 items
  QPopupMenu *pluginMenu = new QPopupMenu(qgisMainWindowPointer);

  int menuId = pluginMenu->insertItem(QIconSet(icon),"&GraticuleMaker", this, SLOT(run()));
  pluginMenu->setWhatsThis(menuId, "Creates a graticule (grid) and stores the result as a shapefile");
  menuBarPointer = ((QMainWindow *) qgisMainWindowPointer)->menuBar();

  menuIdInt = qGisInterface->addMenu("&Graticules", pluginMenu);
  // Create the action for tool
  myQActionPointer = new QAction("Graticule Creator", QIconSet(icon), "&Wmi",0, this, "run");
  myQActionPointer->setWhatsThis("Creates a graticule (grid) and stores the result as a shapefile");
  // Connect the action to the run
  connect(myQActionPointer, SIGNAL(activated()), this, SLOT(run()));

  // Add the icon to the toolbar
  qGisInterface->addToolBarIcon(myQActionPointer);

}
//method defined in interface
void QgsGridMakerPlugin::help()
{
  //implement me!
}

// Slot called when the buffer menu item is activated
void QgsGridMakerPlugin::run()
{
  PluginGui *myPluginGui=new PluginGui(qgisMainWindowPointer,"Graticule Builder",true,0);
  //listen for when the layer has been made so we can draw it
  connect(myPluginGui, SIGNAL(drawRasterLayer(QString)), this, SLOT(drawRasterLayer(QString)));
  connect(myPluginGui, SIGNAL(drawVectorLayer(QString,QString,QString)), this, SLOT(drawVectorLayer(QString,QString,QString)));
  myPluginGui->show();
}
//!draw a raster layer in the qui - intended to respond to signal sent by diolog when it as finished creating
//layer
void QgsGridMakerPlugin::drawRasterLayer(QString theQString)
{
  qGisInterface->addRasterLayer(theQString);
}
//!draw a vector layer in the qui - intended to respond to signal sent by diolog when it as finished creating a layer
////needs to be given vectorLayerPath, baseName, providerKey ("ogr" or "postgres");
void QgsGridMakerPlugin::drawVectorLayer(QString thePathNameQString, QString theBaseNameQString, QString theProviderQString)
{
 qGisInterface->addVectorLayer( thePathNameQString, theBaseNameQString, theProviderQString);
}

// Unload the plugin by cleaning up the GUI
void QgsGridMakerPlugin::unload()
{
  // remove the GUI
  menuBarPointer->removeItem(menuIdInt);
  qGisInterface->removeToolBarIcon(myQActionPointer);
  delete myQActionPointer;
}
/** 
 * Required extern functions needed  for every plugin 
 * These functions can be called prior to creating an instance
 * of the plugin class
 */
// Class factory to return a new instance of the plugin class
QGISEXTERN QgisPlugin * classFactory(QgisApp * theQGisAppPointer, QgisIface * theQgisInterfacePointer)
{
  return new QgsGridMakerPlugin(theQGisAppPointer, theQgisInterfacePointer);
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
