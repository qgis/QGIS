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
#include <plugin.h>


#include <qtoolbar.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qaction.h>
#include <qapplication.h>
#include <qfileinfo.h>
//non qt includes
#include <iostream>
#include <openmodellergui.h>
#include <openmodeller/om.hh>
// xpm for creating the toolbar icon
#include "icon_om.xpm"
// 
#ifdef WIN32
#define QGISEXTERN extern "C" __declspec( dllexport )
#else
#define QGISEXTERN extern "C"
#endif

static const char * const sIdent = "$Id$";
static const char * const sName = "openModeller Wizard";
static const char * const sDescription = "Plugin to run openModeller in QGIS.";
static const char * const sVersion = "Version 0.3.1";
static const QgisPlugin::PLUGINTYPE sType = QgisPlugin::UI;

/**
 * Constructor for the plugin. The plugin is passed a pointer to the main app
 * and an interface object that provides access to exposed functions in QGIS.
 * @param theQGisApp - Pointer to the QGIS main window
 * @param theQGisInterface - Pointer to the QGIS interface object
 */
QgsOpenModellerPlugin::QgsOpenModellerPlugin(QgisApp * theQGisApp, QgisIface * theQgisInterface):
                 mQGisApp(theQGisApp), 
                 mQGisIface(theQgisInterface),
                 QgisPlugin(sName,sDescription,sVersion,sType)
{
  mReport = new OmGuiReportBase(theQGisApp);
}

QgsOpenModellerPlugin::~QgsOpenModellerPlugin()
{
  delete mReport; 
}


/*
 * Initialize the GUI interface for the plugin 
 */
void QgsOpenModellerPlugin::initGui()
{
#ifdef WIN32
  // add a menu with 2 items (following QGIS 0.6 object model)
  QPopupMenu *pluginMenu = new QPopupMenu(mQGisApp);
  pluginMenu->insertItem(QIconSet(icon_om),"&openModeller Wizard Plugin", this, SLOT(run()));
  mMenuBarPointer = ((QMainWindow *) mQGisApp)->menuBar();
  mMenuId = mQGisIface->addMenu("&Biodiversity", pluginMenu);
#else

  // New way of adding menu (following QGIS 0.7 object model)
  QPopupMenu *pluginMenu = mQGisIface->getPluginMenu("&Biodiversity");
  mMenuId = pluginMenu->insertItem(QIconSet(icon_om),"&OpenModeller Wizard Plugin", this, SLOT(run()));
#endif

  // Create the action for tool  
  QAction *myQActionPointer = new QAction("openModeller Plugin", QIconSet(icon_om), "&Wmi",0, this, "run");
  // Connect the action to the run
  connect(myQActionPointer, SIGNAL(activated()), this, SLOT(run()));
  // Add the toolbar
  mToolBarPointer = new QToolBar((QMainWindow *) mQGisApp, "Biodiversity");
  mToolBarPointer->setLabel("openModeller plugin");
  // Add the zoom previous tool to the toolbar
  myQActionPointer->addTo(mToolBarPointer);

  //
  // Note this function searches for om plugins - and should only 
  // ever be run once in the life of the qgis session (the om 
  // plugin registry is a singleton), so we do it here
  // rather than in the ctor of the plugin.
  //
  AlgorithmFactory::searchDefaultDirs();

}

// Defined by interface
void QgsOpenModellerPlugin::help()
{
  //implement me
}

// Slot called when the buffer menu item is activated
void QgsOpenModellerPlugin::run()
{
  OpenModellerGui *myOpenModellerGui=new OpenModellerGui(mQGisApp,"openModeller Wizard",true,0);
  //listen for when the layer has been made so we can draw it
  connect(myOpenModellerGui, SIGNAL(drawRasterLayer(QString)), this, SLOT(drawRasterLayer(QString)));
  connect(myOpenModellerGui, SIGNAL(modelDone(QString)), this, SLOT(modelDone(QString)));
  mReport->txtbLog->setText("");
  if (myOpenModellerGui->exec())
  {
    mReport->show();
  }
}
//!draw a raster layer in the qui - intended to respond to signal sent by diolog when it as finished creating
//layer
void QgsOpenModellerPlugin::drawRasterLayer(QString theQString)
{
  QFileInfo myFileInfo(theQString);
  QString myDirNameQString = myFileInfo.dirPath();
  QString myBaseNameQString = myFileInfo.baseName();
  QgsRasterLayer *layer = new QgsRasterLayer(theQString, myBaseNameQString);
  layer->setColorRampingType(QgsRasterLayer::BLUE_GREEN_RED);
  layer->setDrawingStyle(QgsRasterLayer::SINGLE_BAND_PSEUDO_COLOR);
  mQGisIface->addRasterLayer(layer, true);
}
void QgsOpenModellerPlugin::modelDone(QString theText)
{
  // remove the GUI
  mReport->txtbLog->append(theText);
  //std::cout << theText << std::endl;
}
// Unload the plugin by cleaning up the GUI
void QgsOpenModellerPlugin::unload()
{
#ifdef WIN32
  // remove the GUI: the old (QGIS 0.6.0) way
  mMenuBarPointer->removeItem(mMenuId);
  delete mToolBarPointer;
#else
  // remove the GUI
  mQGisIface->removePluginMenuItem("&Biodiversity",mMenuId);
  delete mToolBarPointer;
#endif
}
//////////////////////////////////////////////////////////////////////////
//
//
//  THE FOLLOWING CODE IS AUTOGENERATED BY THE PLUGIN BUILDER SCRIPT
//    YOU WOULD NORMALLY NOT NEED TO MODIFY THIS, AND YOUR PLUGIN
//      MAY NOT WORK PROPERLY IF YOU MODIFY THIS INCORRECTLY
//
//
//////////////////////////////////////////////////////////////////////////


/** 
 * Required extern functions needed  for every plugin 
 * These functions can be called prior to creating an instance
 * of the plugin class
 */
// Class factory to return a new instance of the plugin class
QGISEXTERN QgisPlugin * classFactory(QgisApp * theQGisAppPointer, QgisIface * theQgisInterfacePointer)
{
  std::cout << " omgui classfactory called" << std::endl;
  return new QgsOpenModellerPlugin(theQGisAppPointer, theQgisInterfacePointer);
}
// Return the name of the plugin - note that we do not user class members as
// the class may not yet be insantiated when this method is called.
QGISEXTERN QString name()
{
  std::cout << " omgui classfactory called" << std::endl;
  return sName;
}

// Return the description
QGISEXTERN QString description()
{
  std::cout << " omgui description called" << std::endl;
  return sDescription;
}

// Return the type (either UI or MapLayer plugin)
QGISEXTERN int type()
{
  std::cout << " omgui type called" << std::endl;
  return sType;
}

// Return the version number for the plugin
QGISEXTERN QString version()
{
  std::cout << " omgui version called" << std::endl;
  return sVersion;
}

// Delete ourself
QGISEXTERN void unload(QgisPlugin * thePluginPointer)
{
  std::cout << " omgui unload called" << std::endl;
  delete thePluginPointer;
}
