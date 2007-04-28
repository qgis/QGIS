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
/*  $Id: qgslauncherplugin.cpp 3249 2005-04-28 01:20:24Z g_j_m $ */

// includes

#include <qgisapp.h>
#include <qgsmaplayer.h>
//#include <qgsrasterlayer.h>
#include "qgslauncherplugin.h"


#include <QToolBar>
#include <QMenuBar>
#include <QMessageBox>
#include <QMenu>
#include <QLineEdit>
#include <QAction>
#include <QApplication>
#include <QCursor>
#include <QProcess>

//non qt includes
#include <iostream>

//the gui subclass
#include "qgslauncherplugingui.h"

// xpm for creating the toolbar icon
#include "icon.xpm"
// 
static const char * const ident_ = "$Id: qgslauncherplugin.cpp 3249 2005-04-28 01:20:24Z g_j_m $";

static const char * const name_ = "Launcher";
static const char * const description_ = "Launches a program or script from within QGIS";
static const char * const version_ = "Version 0.2";
static const QgisPlugin::PLUGINTYPE type_ = QgisPlugin::UI;


/**
 * Constructor for the plugin. The plugin is passed a pointer to the main app
 * and an interface object that provides access to exposed functions in QGIS.
 * @param qgis Pointer to the QGIS main window
 * @param _qI Pointer to the QGIS interface object
 */
QgsLauncherPlugin::QgsLauncherPlugin(QgisApp * theQGisApp, QgisIface * theQgisInterFace):
          qgisMainWindowPointer(theQGisApp), 
          qGisInterface(theQgisInterFace),
          QgisPlugin(name_,description_,version_,type_)
{
}

QgsLauncherPlugin::~QgsLauncherPlugin()
{

}

/*
 * Initialize the GUI interface for the plugin 
 */
void QgsLauncherPlugin::initGui()
{
  //QMenu *pluginMenu = qGisInterface->getPluginMenu("&Launcher");
  //menuId = pluginMenu->insertItem(QIconSet(icon),"&Run...", this, SLOT(run()));

  // Create the action for tool
  myQActionPointer = new QAction(QIcon(icon), tr("Run..."), this);
  // Connect the action to the run
  connect(myQActionPointer, SIGNAL(activated()), this, SLOT(run()));
  // Add the toolbar
  qGisInterface->addToolBarIcon(myQActionPointer);

  qGisInterface->addPluginMenu(tr("&Run..."),myQActionPointer);

}
//method defined in interface
void QgsLauncherPlugin::help()
{
  //implement me!
}

// Slot called when the run button is pushed
void QgsLauncherPlugin::run()
{
  QgsLauncherPluginGui *myPluginGui=new QgsLauncherPluginGui(qgisMainWindowPointer,"Run...",true,0);
  myPluginGui->show();
}
//!draw a raster layer in the qui - intended to respond to signal sent by diolog when it as finished creating
//layer
void QgsLauncherPlugin::drawRasterLayer(QString theQString)
{
  qGisInterface->addRasterLayer(theQString);
}
// Unload the plugin by cleaning up the GUI
void QgsLauncherPlugin::unload()
{
  // remove the GUI
  qGisInterface->removePluginMenu(tr("&Launcher"),myQActionPointer);
  qGisInterface->removeToolBarIcon(myQActionPointer); 
  delete myQActionPointer;
}
/** 
 * Required extern functions needed  for every plugin 
 * These functions can be called prior to creating an instance
 * of the plugin class
 */
// Class factory to return a new instance of the plugin class
extern "C" QgisPlugin * classFactory(QgisApp * theQGisAppPointer, QgisIface * theQgisInterfacePointer)
{
  return new QgsLauncherPlugin(theQGisAppPointer, theQgisInterfacePointer);
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
