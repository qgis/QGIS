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
#include <qpixmap.h>
#include <qpainter.h>
#include <qfont.h>
#include <qpicture.h>

//non qt includes
#include <iostream>

//the gui subclass
#include "plugingui.h"

// xpm for creating the toolbar icon
#include "icon.xpm"
// 
static const char * const ident_ = "$Id$";

static const char * const name_ = "NorthArrow";
static const char * const description_ = "This plugin displays a north arrow overlayed onto the map.";
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

/*
 * Initialize the GUI interface for the plugin 
 */
void Plugin::initGui()
{
  // add a menu with 2 items
  QPopupMenu *pluginMenu = new QPopupMenu(qgisMainWindowPointer);

  pluginMenu->insertItem(QIconSet(icon),"&NorthArrow", this, SLOT(run()));

  menuBarPointer = ((QMainWindow *) qgisMainWindowPointer)->menuBar();

  menuIdInt = qGisInterface->addMenu("&Decorations", pluginMenu);
  // Create the action for tool
  QAction *myQActionPointer = new QAction("North Arrow", QIconSet(icon), "&Wmi",0, this, "run");
  // Connect the action to the run
  connect(myQActionPointer, SIGNAL(activated()), this, SLOT(run()));
  //render the arrow each time the map is rendered
  connect(qGisInterface->getMapCanvas(), SIGNAL(renderComplete()), this, SLOT(renderNorthArrow()));
  // Add the toolbar
  toolBarPointer = new QToolBar((QMainWindow *) qgisMainWindowPointer, "Decorations");
  toolBarPointer->setLabel("North Arrow");
  // Add the zoom previous tool to the toolbar
  myQActionPointer->addTo(toolBarPointer);
  refreshCanvas();

}
//method defined in interface
void Plugin::help()
{
  //implement me!
}

// Slot called when the buffer menu item is activated
void Plugin::run()
{
  PluginGui *myPluginGui=new PluginGui(qgisMainWindowPointer,"North Arrow",true,0);
  //listen for when the layer has been made so we can draw it
  connect(myPluginGui, SIGNAL(drawRasterLayer(QString)), this, SLOT(drawRasterLayer(QString)));
  connect(myPluginGui, SIGNAL(drawVectorLayer(QString,QString,QString)), this, SLOT(drawVectorLayer(QString,QString,QString)));
  myPluginGui->show();
  
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

//! Refresh the map display using the mapcanvas exported via the plugin interface
void Plugin::refreshCanvas()
{
 qGisInterface->getMapCanvas()->refresh();
}

void Plugin::renderNorthArrow()
{
  //how much should the north arrow be rotated by?
  int myRotationInt = 0;
  //dir where north arrows live
  //QString myDirString = QString(PKGDATAPATH)+"/svg/north_arrows/";
  //pixmap containing map
  QPixmap * myQPixmap = qGisInterface->getMapCanvas()->canvasPixmap();
  //paint device that we can draw our pixmap onto
  QPainter myQPainter(myQPixmap);
  //rotate the canvas
  //myQPainter.rotate(-myRotationInt);
  
  QPicture myQPicture;
  myQPicture.load(QString("/home/aps02ts/share/qgis/svg/north_arrows/Default.svg"));
  myQPainter.drawPicture(140,140,myQPicture);
  
  /* This is how you write some text to the qpainter */
  QFont myQFont("time", 24, QFont::Bold);
  myQPainter.setFont(myQFont);
  myQPainter.setPen(Qt::white);
  myQPainter.drawText(15, 50, QString("N"));
  /* */
  //myQPainter.rotate(myRotationInt);
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
