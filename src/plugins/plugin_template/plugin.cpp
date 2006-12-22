/***************************************************************************
  [pluginlcasename].cpp 
  [plugindescription]

  -------------------
         begin                : [PluginDate]
         copyright            : [(C) Your Name and Date]
         email                : [Your Email]

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*  $Id$ */

//
// QGIS Specific includes
//

#include <qgisapp.h>
#include <qgisgui.h>
#include <qgsmaplayer.h>
#include <qgsrasterlayer.h>
#include "[pluginlcasename].h"
//the gui subclass
#include "[pluginlcasename]gui.h"

//
// Qt4 Related Includes
//

#include <QToolBar>
#include <QMenuBar>
#include <QMessageBox>
#include <QMenu>
#include <QLineEdit>
#include <QAction>
#include <QApplication>
#include <QCursor>

//non qt includes
#include <iostream>


#ifdef WIN32
#define QGISEXTERN extern "C" __declspec( dllexport )
#else
#define QGISEXTERN extern "C"
#endif

static const char * const sIdent = "$Id$";
static const QString sName = QObject::tr("[menuitemname]");
static const QString sDescription = QObject::tr("[plugindescription]");
static const QString sPluginVersion = QObject::tr("Version 0.1");
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
[pluginname]::[pluginname](QgisApp * theQGisApp, 
				       QgisIface * theQgisInterface):
                 QgisPlugin(sName,sDescription,sPluginVersion,sPluginType),
                 mQGisApp(theQGisApp), 
                 mQGisIface(theQgisInterface)
{
}

[pluginname]::~[pluginname]()
{

}

/*
 * Initialize the GUI interface for the plugin - this is only called once when the plugin is 
 * added to the plugin registry in the QGIS application.
 */
void [pluginname]::initGui()
{

  // Create the action for tool
  mQActionPointer = new QAction(QIcon(":/[pluginlcasename]/[pluginlcasename].png"),"[menuitemname]", this);
  // Set the what's this text
  mQActionPointer->setWhatsThis(tr("Replace this with a short description of the what the plugin does"));
  // Connect the action to the run
  connect(mQActionPointer, SIGNAL(activated()), this, SLOT(run()));
  // Add the toolbar
  mToolBarPointer = new QToolBar((QMainWindow *) mQGisApp, "[menuname]");
  mToolBarPointer->setLabel("[menuitemname]");
  // Add the to the toolbar
  mQGisIface->addToolBarIcon(mQActionPointer);
  mQGisIface->addPluginMenu("&[menuname]", mQActionPointer);

}
//method defined in interface
void [pluginname]::help()
{
  //implement me!
}

// Slot called when the menu item is activated
// If you created more menu items / toolbar buttons in initiGui, you should 
// create a separate handler for each action - this single run() method will
// not be enough
void [pluginname]::run()
{
  [pluginname]Gui *myPluginGui=new [pluginname]Gui(mQGisApp, QgisGui::ModalDialogFlags);
  //listen for when the layer has been made so we can draw it
  connect(myPluginGui, SIGNAL(drawRasterLayer(QString)), this, SLOT(drawRasterLayer(QString)));
  connect(myPluginGui, SIGNAL(drawVectorLayer(QString,QString,QString)), this, SLOT(drawVectorLayer(QString,QString,QString)));
  myPluginGui->show();
}

// Unload the plugin by cleaning up the GUI
void [pluginname]::unload()
{
  // remove the GUI
  mQGisIface->removePluginMenu("&[menuname]",mQActionPointer);
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
void [pluginname]::drawRasterLayer(QString theQString)
{
  mQGisIface->addRasterLayer(theQString);
}

//!draw a vector layer in the qui - intended to respond to signal sent by 
// dialog when it as finished creating a layer. It needs to be given 
// vectorLayerPath, baseName, providerKey ("ogr" or "postgres");
void [pluginname]::drawVectorLayer(QString thePathNameQString, QString theBaseNameQString, QString theProviderQString)
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
  return new [pluginname](theQGisAppPointer, theQgisInterfacePointer);
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
