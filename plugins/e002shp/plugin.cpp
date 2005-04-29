/***************************************************************************
 *  File Name:               plugin.cpp 
 * 
 *  Convert an e00 file to a shape file. Based on the e002shp command line tool by Adrian Gutarra Sebastian.
 *
 *--------------------------------------------------------------------------
 *    begin                : Jan 21, 2004
 *    copyright            : (C) 2004 by Tim Sutton
 *    email                : tim@linfiniti.com
 *
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

/***************************************************************************
 *   QGIS Programming conventions:
 *   
 *   mVariableName - a class level member variable
 *   sVariableName - a static class level member variable
 *   variableName() - accessor for a class member (no 'get' in front of name)
 *   setVariableName() - mutator for a class member (prefix with 'set')
 *
 *   Additional useful conventions:
 *
 *   theVariableName - a method parameter (prefix with 'the')
 *   myVariableName - a locally declared variable within a method ('my' prefix)
 * 
 *   DO: Use mixed case variable names - myVariableName
 *   DON'T: separate variable names using underscores: my_variable_name (NO!)
 *
 * **************************************************************************/

//
// Required qgis includes
// 

#include <qgisapp.h>
#include <qgsmaplayer.h>
#include <qgsrasterlayer.h>
#include "plugin.h"

//
// Required QT includes
//

#include <qtoolbar.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qlineedit.h>
#include <qaction.h>
#include <qapplication.h>
#include <qcursor.h>

//
//non qt includes
//
#include <iostream>

//
//the gui subclass
//
#include "plugingui.h"

//
// xpm for creating the toolbar icon
// 
#include "icon.xpm"

#ifdef WIN32
#define QGISEXTERN extern "C" __declspec( dllexport )
#else
#define QGISEXTERN extern "C"
#endif

static const char * const sIdent = "$Id$";
static const char * const sName = "e002shp";
static const char * const sDescription = "Convert an e00 file to a shape file. Based on the e002shp command line tool by Adrian Gutarra Sebastian.";
static const char * const sPluginVersion = "Version 0.1";
static const QgisPlugin::PLUGINTYPE sPluginType = QgisPlugin::UI;

//////////////////////////////////////////////////////////////////////
//
// THE FOLLOWING METHODS ARE MANDATORY FOR ALL PLUGINS
//
//////////////////////////////////////////////////////////////////////

/**
 * Constructor for the plugin. The plugin is passed a pointer to the main app
 * and an interface object that provides access to exposed functions in QGIS.
 * @param theQGisApp - Pointer to the QGIS main window
 * @param theQGisInterface - Pointer to the QGIS interface object
 */
QgsE002shpPlugin::QgsE002shpPlugin(QgisApp * theQGisApp, QgisIface * theQgisInterface):
                 mQGisApp(theQGisApp), 
                 mQGisIface(theQgisInterface),
                 QgisPlugin(sName,sDescription,sPluginVersion,sPluginType)
{
}

QgsE002shpPlugin::~QgsE002shpPlugin()
{

}

/*
 * Initialize the GUI interface for the plugin 
 */
void QgsE002shpPlugin::initGui()
{
  QPopupMenu *pluginMenu = mQGisIface->getPluginMenu("&Tools");
  mMenuId = pluginMenu->insertItem(QIconSet(icon),"&e002shp", this, SLOT(run()));

  // Create the action for tool
  mQActionPointer = new QAction("e002shp", QIconSet(icon), "&icon",0, this, "run");
  // Connect the action to the run
  connect(mQActionPointer, SIGNAL(activated()), this, SLOT(run()));
  // Add the toolbar
  mToolBarPointer = new QToolBar((QMainWindow *) mQGisApp, "e002shp");
  mToolBarPointer->setLabel("e002shp");
  // Add the to the toolbar
  mQGisIface->addToolBarIcon(mQActionPointer);

}
//method defined in interface
void QgsE002shpPlugin::help()
{
  //implement me!
}

// Slot called when the buffer menu item is activated
void QgsE002shpPlugin::run()
{
  QgsE002shpPluginGui *myPluginGui=new QgsE002shpPluginGui(mQGisApp,"e002shp",true,0);
  //listen for when the layer has been made so we can draw it
  connect(myPluginGui, SIGNAL(drawRasterLayer(QString)), this, SLOT(drawRasterLayer(QString)));
  connect(myPluginGui, SIGNAL(drawVectorLayer(QString,QString,QString)), this, SLOT(drawVectorLayer(QString,QString,QString)));
  myPluginGui->show();
}

// Unload the plugin by cleaning up the GUI
void QgsE002shpPlugin::unload()
{
  // remove the GUI
  mQGisIface->removePluginMenuItem("&Tools",mMenuId);
  mQGisIface->removeToolBarIcon(mQActionPointer);
  delete mQActionPointer;
}

//////////////////////////////////////////////////////////////////////
//
//                  END OF MANDATORY PLUGIN METHODS
//
//////////////////////////////////////////////////////////////////////
//
// The following methods are provided to demonstrate how you can 
// load a vector or raster layer into the main gui. Please delete
// if you are not intending to use these. Note also that there are
// ways in which layers can be loaded.
//

//!draw a raster layer in the qui - intended to respond to signal sent by diolog when it as finished creating
//layer
void QgsE002shpPlugin::drawRasterLayer(QString theQString)
{
  mQGisIface->addRasterLayer(theQString);
}

//!draw a vector layer in the qui - intended to respond to signal sent by 
// dialog when it as finished creating a layer. It needs to be given 
// vectorLayerPath, baseName, providerKey ("ogr" or "postgres");
void QgsE002shpPlugin::drawVectorLayer(QString thePathNameQString, QString theBaseNameQString, QString theProviderQString)
{
  mQGisIface->addVectorLayer( thePathNameQString, theBaseNameQString, theProviderQString);
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
  return new QgsE002shpPlugin(theQGisAppPointer, theQgisInterfacePointer);
}
// Return the name of the plugin - note that we do not user class members as
// the class may not yet be insantiated when this method is called.
QGISEXTERN QString name()
{
  return sName;
}

// Return the description
QGISEXTERN QString description()
{
  return sDescription;
}

// Return the type (either UI or MapLayer plugin)
QGISEXTERN int type()
{
  return sPluginType;
}

// Return the version number for the plugin
QGISEXTERN QString version()
{
  return sPluginVersion;
}

// Delete ourself
QGISEXTERN void unload(QgisPlugin * thePluginPointer)
{
  delete thePluginPointer;
}
