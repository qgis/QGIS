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
#include "../../src/qgsmaplayerregistry.h"
#include "../../src/qgsmaplayer.h"
#include "../../src/qgsvectorlayer.h"
#include "../../src/qgsrasterlayer.h"
#include "../../src/qgsdataprovider.h"
#include "plugin.h"


#include <qeventloop.h>
#include <qfiledialog.h>
#include <qtoolbar.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qlineedit.h>
#include <qaction.h>
#include <qapplication.h>
#include <qcursor.h>
#include <qprocess.h>
#include <qprogressdialog.h>

//non qt includes
#include <cassert>
#include <iostream>

//the gui subclass
#include "plugingui.h"

// xpm for creating the toolbar icon
#include "icon.xpm"

#ifdef WIN32
#define QGISEXTERN extern "C" __declspec( dllexport )
#else
#define QGISEXTERN extern "C"
#endif

// 
static const char * const ident_ = "$Id$";

static const char * const name_ = "GPS Tools";
static const char * const description_ = "Tools for loading and importing GPS data.";
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

  pluginMenu->insertItem(QIconSet(icon),"&Gps Tools", this, SLOT(run()));
  
  menuBarPointer = ((QMainWindow *) qgisMainWindowPointer)->menuBar();

  menuIdInt = qGisInterface->addMenu("&Gps", pluginMenu);
  // Create the action for tool
  QAction *myQActionPointer = new QAction("Import Gps Data", QIconSet(icon), "&Wmi",0, this, "run");
  // Connect the action to the run
  connect(myQActionPointer, SIGNAL(activated()), this, SLOT(run()));

  // Add the icon to the toolbar
  qGisInterface->addToolBarIcon(myQActionPointer);

}

//method defined in interface
void Plugin::help()
{
  //implement me!
}

// Slot called when the buffer menu item is activated
void Plugin::run()
{
  // find all GPX layers
  std::vector<QgsVectorLayer*> gpxLayers;
  std::map<QString, QgsMapLayer*>::const_iterator iter;
  for (iter = qGisInterface->getLayerRegistry()->mapLayers().begin();
       iter != qGisInterface->getLayerRegistry()->mapLayers().end(); ++iter) {
    if (iter->second->type() == QgsMapLayer::VECTOR) {
      QgsVectorLayer* vectorLayer =dynamic_cast<QgsVectorLayer*>(iter->second);
      if (vectorLayer->providerType() == "gpx")
	gpxLayers.push_back(vectorLayer);
    }
  }
  
  PluginGui *myPluginGui=new PluginGui(gpxLayers, qgisMainWindowPointer, "GPS Tools", true, 0);
  //listen for when the layer has been made so we can draw it
  connect(myPluginGui, SIGNAL(drawRasterLayer(QString)), this, SLOT(drawRasterLayer(QString)));
  connect(myPluginGui, SIGNAL(drawVectorLayer(QString,QString,QString)), this, SLOT(drawVectorLayer(QString,QString,QString)));
  connect(myPluginGui, SIGNAL(loadGPXFile(QString, bool, bool, bool)), 
	  this, SLOT(loadGPXFile(QString, bool, bool, bool)));
  connect(myPluginGui, SIGNAL(importGPSFile(QString, QString, bool, bool, bool,
					    QString, QString)),
	  this, SLOT(importGPSFile(QString, QString, bool, bool, bool, QString,
				   QString)));
  connect(myPluginGui, SIGNAL(downloadFromGPS(QString, QString, bool, bool,
					      bool, QString, QString)),
	  this, SLOT(downloadFromGPS(QString, QString, bool, bool, bool,
				     QString, QString)));
  connect(myPluginGui, SIGNAL(uploadToGPS(QgsVectorLayer*, QString, QString)),
	  this, SLOT(uploadToGPS(QgsVectorLayer*, QString, QString)));
  connect(this, SIGNAL(closeGui()), myPluginGui, SLOT(close()));

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

// Unload the plugin by cleaning up the GUI
void Plugin::unload()
{
  // remove the GUI
  menuBarPointer->removeItem(menuIdInt);
  delete toolBarPointer;
}

void Plugin::loadGPXFile(QString filename, bool loadWaypoints, bool loadRoutes,
			 bool loadTracks) {

  //check if input file is readable
  QFileInfo fileInfo(filename);
  if (!fileInfo.isReadable()) {
    QMessageBox::warning(NULL, "GPX/LOC Loader",
			 "Unable to read the selected file.\n"
			 "Please reselect a valid file." );
    return;
  }
  
    // add the requested layers
  if (loadTracks)
    emit drawVectorLayer(filename + "?type=track", 
			 fileInfo.baseName() + ", tracks", "gpx");
  if (loadRoutes)
    emit drawVectorLayer(filename + "?type=route", 
			 fileInfo.baseName() + ", routes", "gpx");
  if (loadWaypoints)
    emit drawVectorLayer(filename + "?type=waypoint", 
			 fileInfo.baseName() + ", waypoints", "gpx");
  
  emit closeGui();
}


void Plugin::importGPSFile(QString inputFilename, QString inputFormat, 
			   bool importWaypoints, bool importRoutes, 
			   bool importTracks, QString outputFilename, 
			   QString layerName) {

  // what features does the user want to import?
  QString typeArg;
  if (importWaypoints)
    typeArg = "-w";
  else if (importRoutes)
    typeArg = "-r";
  else if (importTracks)
    typeArg = "-t";
  
  // try to start the gpsbabel process
  QStringList babelArgs;
  babelArgs<<"gpsbabel"<<typeArg<<"-i"<<inputFormat<<"-o"<<"gpx"
	   <<inputFilename<<outputFilename;
  QProcess babelProcess(babelArgs);
  if (!babelProcess.start()) {
    QMessageBox::warning(NULL, "Could not start process",
			 "Could not start GPSBabel!");
    return;
  }
  
  // wait for gpsbabel to finish (or the user to cancel)
  QProgressDialog progressDialog("Importing data...", "Cancel", 0,
				 NULL, 0, true);
  progressDialog.show();
  for (int i = 0; babelProcess.isRunning(); ++i) {
    QApplication::eventLoop()->processEvents(0);
    progressDialog.setProgress(i/64);
    if (progressDialog.wasCancelled())
      return;
  }
  
  // did we get any data?
  if (babelProcess.exitStatus() != 0) {
    QString babelError(babelProcess.readStderr());
    QString errorMsg(QString("Could not import data from %1!\n\n")
		     .arg(inputFilename));
    errorMsg += babelError;
    QMessageBox::warning(NULL, "Error importing data", errorMsg);
    return;
  }
  
  // add the layer
  if (importTracks)
    emit drawVectorLayer(outputFilename + "?type=track", 
			 layerName, "gpx");
  if (importRoutes)
    emit drawVectorLayer(outputFilename + "?type=route", 
			 layerName, "gpx");
  if (importWaypoints)
    emit drawVectorLayer(outputFilename + "?type=waypoint", 
			 layerName, "gpx");
  
  emit closeGui();
}


void Plugin::downloadFromGPS(QString protocol, QString deviceFilename,
			     bool downloadWaypoints, bool downloadRoutes,
			     bool downloadTracks, QString outputFilename,
			     QString layerName) {

  // what does the user want to download?
  QString typeArg;
  if (downloadWaypoints)
    typeArg = "-w";
  else if (downloadRoutes)
    typeArg = "-r";
  else if (downloadTracks)
    typeArg = "-t";
  
  // try to start the gpsbabel process
  QStringList babelArgs;
  babelArgs<<"gpsbabel"<<typeArg<<"-i"<<protocol<<"-o"<<"gpx"
	   <<deviceFilename<<outputFilename;
  QProcess babelProcess(babelArgs);
  if (!babelProcess.start()) {
    QMessageBox::warning(NULL, "Could not start process",
			 "Could not start GPSBabel!");
    return;
  }
  
  // wait for gpsbabel to finish (or the user to cancel)
  QProgressDialog progressDialog("Downloading data...", "Cancel", 0,
				 NULL, 0, true);
  progressDialog.show();
  for (int i = 0; babelProcess.isRunning(); ++i) {
    QApplication::eventLoop()->processEvents(0);
    progressDialog.setProgress(i/64);
    if (progressDialog.wasCancelled())
      return;
  }
  
  // did we get any data?
  if (babelProcess.exitStatus() != 0) {
    QString babelError(babelProcess.readStderr());
    QString errorMsg("Could not download data from GPS!\n\n");
    errorMsg += babelError;
    QMessageBox::warning(NULL, "Error downloading data", errorMsg);
    return;
  }
  
  // add the layer
  if (downloadWaypoints)
    emit drawVectorLayer(outputFilename + "?type=waypoint", 
			 layerName, "gpx");
  if (downloadRoutes)
    emit drawVectorLayer(outputFilename + "?type=route", 
			 layerName, "gpx");
  if (downloadTracks)
    emit drawVectorLayer(outputFilename + "?type=track", 
			 layerName, "gpx");
  
  emit closeGui();
}


void Plugin::uploadToGPS(QgsVectorLayer* gpxLayer, QString protocol,
			 QString deviceFilename) {
  
  const QString& source(gpxLayer->getDataProvider()->getDataSourceUri());
  
  // what kind of data does the user want to upload?
  QString typeArg;
  if (source.right(8) == "waypoint")
    typeArg = "-w";
  else if (source.right(5) == "route")
    typeArg = "-r";
  else if (source.right(5) == "track")
    typeArg = "-t";
  else {
    std::cerr<<source.right(8)<<std::endl;
    assert(false);
  }
  
  // try to start the gpsbabel process
  QStringList babelArgs;
  babelArgs<<"gpsbabel"<<typeArg<<"-i"<<"gpx"
	   <<"-o"<<protocol
	   <<source.left(source.findRev('?'))<<deviceFilename;
  QProcess babelProcess(babelArgs);
  if (!babelProcess.start()) {
    QMessageBox::warning(NULL, "Could not start process",
			 "Could not start GPSBabel!");
    return;
  }
  
  // wait for gpsbabel to finish (or the user to cancel)
  QProgressDialog progressDialog("Uploading data...", "Cancel", 0,
				 NULL, 0, true);
  progressDialog.show();
  for (int i = 0; babelProcess.isRunning(); ++i) {
    QApplication::eventLoop()->processEvents(0);
    progressDialog.setProgress(i/64);
    if (progressDialog.wasCancelled())
      return;
  }
  
  // did we get an error?
  if (babelProcess.exitStatus() != 0) {
    QString babelError(babelProcess.readStderr());
    QString errorMsg("Error while uploading data to GPS!\n\n");
    errorMsg += babelError;
    QMessageBox::warning(NULL, "Error uploading data", errorMsg);
    return;
  }
  
  emit closeGui();
}



/** 
 * Required extern functions needed  for every plugin 
 * These functions can be called prior to creating an instance
 * of the plugin class
 */
// Class factory to return a new instance of the plugin class
QGISEXTERN QgisPlugin * classFactory(QgisApp * theQGisAppPointer, QgisIface * theQgisInterfacePointer)
{
  return new Plugin(theQGisAppPointer, theQgisInterfacePointer);
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
