/***************************************************************************
    qgsgrassplugin.cpp  -  GRASS menu
                             -------------------
    begin                : March, 2004
    copyright            : (C) 2004 by Radim Blazek
    email                : blazek@itc.it
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
#include "qgsgrassplugin.h"


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
#include "qgsgrassselect.h"

// xpm for creating the toolbar icon
#include "add_vector.xpm"
#include "add_raster.xpm"
static const char *pluginVersion = "0.1";

/**
 * Constructor for the plugin. The plugin is passed a pointer to the main app
 * and an interface object that provides access to exposed functions in QGIS.
 * @param theQGisApp Pointer to the QGIS main window
 * @param theQgisInterFace Pointer to the QGIS interface object
 */
QgsGrassPlugin::QgsGrassPlugin(QgisApp * theQGisApp, QgisIface * theQgisInterFace):
          qgisMainWindowPointer(theQGisApp), qGisInterface(theQgisInterFace)
{
  /** Initialize the plugin and set the required attributes */
  pluginNameQString = "GrassVector";
  pluginVersionQString = "0.1";
  pluginDescriptionQString = "GRASS layer";
}

QgsGrassPlugin::~QgsGrassPlugin()
{

}

/* Following functions return name, description, version, and type for the plugin */
QString QgsGrassPlugin::name()
{
  return pluginNameQString;
}

QString QgsGrassPlugin::version()
{
  return pluginVersionQString;

}

QString QgsGrassPlugin::description()
{
  return pluginDescriptionQString;

}

int QgsGrassPlugin::type()
{
  return QgisPlugin::UI;
}

/*
 * Initialize the GUI interface for the plugin 
 */
void QgsGrassPlugin::initGui()
{
    QPopupMenu *pluginMenu = new QPopupMenu(qgisMainWindowPointer);

    pluginMenu->insertItem(QIconSet(icon_add_vector),"Add Grass &Vector", this, SLOT(addVector()));
    pluginMenu->insertItem(QIconSet(icon_add_raster),"Add Grass &Raster", this, SLOT(addRaster()));

    menuBarPointer = ((QMainWindow *) qgisMainWindowPointer)->menuBar();

    menuIdInt = qGisInterface->addMenu("&GRASS", pluginMenu);

    // Create the action for tool
    QAction *addVectorAction = new QAction("Add GRASS vector layer", QIconSet(icon_add_vector), 
	                                   "Add GRASS vector layer",0, this, "addVector");
    QAction *addRasterAction = new QAction("Add GRASS raster layer", QIconSet(icon_add_raster), 
	                                   "Add GRASS raster layer",0, this, "addRaster");

    // Connect the action 
    connect(addVectorAction, SIGNAL(activated()), this, SLOT(addVector()));
    connect(addRasterAction, SIGNAL(activated()), this, SLOT(addRaster()));

    // Add the toolbar
    toolBarPointer = new QToolBar((QMainWindow *) qgisMainWindowPointer, "GRASS");
    toolBarPointer->setLabel("Add GRASS layer");

    // Add to the toolbar
    addVectorAction->addTo(toolBarPointer);
    addRasterAction->addTo(toolBarPointer);
}

// Slot called when the "Add GRASS vector layer" menu item is activated
void QgsGrassPlugin::addVector()
{
    QString uri;

    QgsGrassSelect *sel = new QgsGrassSelect(QgsGrassSelect::VECTOR );
    if ( sel->exec() ) {
	uri = sel->gisdbase + "/" + sel->location + "/" + sel->mapset + "/" + sel->map + "/" + sel->layer;
    }
    #ifdef QGISDEBUG
    std::cerr << "plugin URI: " << uri << std::endl;
    #endif
    if ( uri.length() == 0 ) {
	std::cerr << "Nothing was selected" << std::endl;
	return;
    } else {
        #ifdef QGISDEBUG
	std::cout << "Add new vector layer" << std::endl;
        #endif
	// create vector name: vector layer
	int pos = uri.findRev('/');
	pos = uri.findRev('/', pos-1);
	QString name = uri.right( uri.length() - pos - 1 );
	name.replace('/', ' ');

        qGisInterface->addVectorLayer( uri, name, "grass");
    }
}

// Slot called when the "Add GRASS raster layer" menu item is activated
void QgsGrassPlugin::addRaster()
{
    QString uri;

    std::cerr << "QgsGrassPlugin::addRaster" << std::endl;

    QgsGrassSelect *sel = new QgsGrassSelect(QgsGrassSelect::RASTER );
    if ( sel->exec() ) {
	QString element;
	if ( sel->selectedType == QgsGrassSelect::RASTER ) {
	    element = "cellhd";
	} else { // GROUP
	    element = "group";
	}
	    
	uri = sel->gisdbase + "/" + sel->location + "/" + sel->mapset + "/" + element + "/" + sel->map;
    }
    #ifdef QGISDEBUG
    std::cerr << "plugin URI: " << uri << std::endl;
    #endif
    if ( uri.length() == 0 ) {
	std::cerr << "Nothing was selected" << std::endl;
	return;
    } else {
        #ifdef QGISDEBUG
	std::cout << "Add new raster layer" << std::endl;
        #endif
	// create raster name
	int pos = uri.findRev('/');
	pos = uri.findRev('/', pos-1);
	QString name = uri.right( uri.length() - pos - 1 );
	name.replace('/', ' ');

        qGisInterface->addRasterLayer( uri );
    }
}


// Unload the plugin by cleaning up the GUI
void QgsGrassPlugin::unload()
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
    return new QgsGrassPlugin(theQGisAppPointer, theQgisInterfacePointer);
}

// Return the name of the plugin - note that we do not user class members as
// the class may not yet be insantiated when this method is called.
extern "C" QString name()
{
    return QString("GRASS");
}

// Return the description
extern "C" QString description()
{
    return QString("GRASS layer");
}

// Return the type (either UI or MapLayer plugin)
extern "C" int type()
{
    return QgisPlugin::UI;
}

// Return the version number for the plugin
extern "C" QString version()
{
    return pluginVersion;
}

// Delete ourself
extern "C" void unload(QgisPlugin * thePluginPointer)
{
    delete thePluginPointer;
}
