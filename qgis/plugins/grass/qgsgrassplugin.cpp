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
#include "../../src/qgisiface.h"
#include "../../src/qgsmapcanvas.h"
#include "../../src/qgsmaplayer.h"
#include "../../src/qgsvectorlayer.h"
#include "../../src/qgsdataprovider.h"
#include "../../src/qgsfeatureattribute.h"

#include <qtoolbar.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qlineedit.h>
#include <qaction.h>
#include <qapplication.h>
#include <qcursor.h>
#include <qfileinfo.h>

//non qt includes
#include <iostream>

extern "C" {
#include <gis.h>
#include <Vect.h>
}

#include "qgsgrassplugin.h"
#include "../../providers/grass/qgsgrass.h"
#include "../../providers/grass/qgsgrassprovider.h"

//the gui subclass
#include "qgsgrassattributes.h"
#include "qgsgrassselect.h"
#include "qgsgrassedit.h"

// xpm for creating the toolbar icon
#include "add_vector.xpm"
#include "add_raster.xpm"
#include "grass_edit.xpm"
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

void QgsGrassPlugin::help()
{
    //TODO
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
    menuBarPointer = 0;
    toolBarPointer = 0;
    // Check GISBASE
    char *gb = getenv("GISBASE");
    if ( !gb ) {
	QMessageBox::warning( 0, "Warning", "Enviroment variable 'GISBASE' is not set,\nGRASS data "
		"cannot be used.\nSet 'GISBASE' and restart QGIS.\nGISBASE is full path to the\n"
	        "directory where GRASS is installed." );
	return;
    }

    QString gbs ( gb );
    QFileInfo gbi ( gbs );
    if ( !gbi.exists() ) {
	QMessageBox::warning( 0, "Warning", "GISBASE:\n'" + gbs + "'\ndoes not exist,\n"
		"GRASS data cannot be used." );
	return;
    }
    
    QPopupMenu *pluginMenu = new QPopupMenu(qgisMainWindowPointer);

    pluginMenu->insertItem(QIconSet(icon_add_vector),"Add Grass &Vector", this, SLOT(addVector()));
    pluginMenu->insertItem(QIconSet(icon_add_raster),"Add Grass &Raster", this, SLOT(addRaster()));
    pluginMenu->insertItem(QIconSet(icon_grass_edit),"&Edit Grass Vector", this, SLOT(edit()));

    menuBarPointer = ((QMainWindow *) qgisMainWindowPointer)->menuBar();

    menuIdInt = qGisInterface->addMenu("&GRASS", pluginMenu);

    // Create the action for tool
    QAction *addVectorAction = new QAction("Add GRASS vector layer", QIconSet(icon_add_vector), 
	                                   "Add GRASS vector layer",0, this, "addVector");
    QAction *addRasterAction = new QAction("Add GRASS raster layer", QIconSet(icon_add_raster), 
	                                   "Add GRASS raster layer",0, this, "addRaster");
    QAction *editAction = new QAction("Edit Grass Vector layer", QIconSet(icon_grass_edit), 
	                        "Edit Grass Vector layer",0, this, "edit");

    // Connect the action 
    connect(addVectorAction, SIGNAL(activated()), this, SLOT(addVector()));
    connect(addRasterAction, SIGNAL(activated()), this, SLOT(addRaster()));
    connect(editAction, SIGNAL(activated()), this, SLOT(edit()));

    // Add the toolbar
    toolBarPointer = new QToolBar((QMainWindow *) qgisMainWindowPointer, "GRASS");
    toolBarPointer->setLabel("Add GRASS layer");

    // Add to the toolbar
    addVectorAction->addTo(toolBarPointer);
    addRasterAction->addTo(toolBarPointer);
    editAction->addTo(toolBarPointer);
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

// Start vector editing
void QgsGrassPlugin::edit()
{
    if ( QgsGrassEdit::isRunning() ) {
	QMessageBox::warning( 0, "Warning", "GRASS Edit is already running." );
	return;
    }

    QgsGrassEdit *ed = new QgsGrassEdit( qgisMainWindowPointer, qGisInterface, qgisMainWindowPointer, 0, 
	                                 Qt::WType_Dialog | Qt::WStyle_Customize | Qt::WStyle_Tool  );

    if ( ed->isValid() ) {
        ed->show();
	QgsMapCanvas *canvas = qGisInterface->getMapCanvas();
	canvas->refresh();
    } else {
	delete ed;
    }
}

// Unload the plugin by cleaning up the GUI
void QgsGrassPlugin::unload()
{
    // remove the GUI
    if ( menuBarPointer )
        menuBarPointer->removeItem(menuIdInt);

    if ( toolBarPointer )
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
