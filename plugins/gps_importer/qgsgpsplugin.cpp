/***************************************************************************
  qgsgpsplugin.cpp - GPS related tools
   -------------------
  Date                 : Jan 21, 2004
  Copyright            : (C) 2004 by Tim Sutton
  Email                : tim@linfiniti.com

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
#include "../../src/qgsdataprovider.h"
#include "qgsgpsplugin.h"


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
#include <qsettings.h>
#include <qstringlist.h>

//non qt includes
#include <cassert>
#include <iostream>

//the gui subclass
#include "qgsgpsplugingui.h"

// xpm for creating the toolbar icon
#include "icon.xpm"

#ifdef WIN32
#define QGISEXTERN extern "C" __declspec( dllexport )
#else
#define QGISEXTERN extern "C"
#endif


static const char * const ident_ = 
  "$Id$";
static const char * const name_ = "GPS Tools";
static const char * const description_ = 
  "Tools for loading and importing GPS data.";
static const char * const version_ = "Version 0.1";
static const QgisPlugin::PLUGINTYPE type_ = QgisPlugin::UI;


/**
 * Constructor for the plugin. The plugin is passed a pointer to the main app
 * and an interface object that provides access to exposed functions in QGIS.
 * @param qgis Pointer to the QGIS main window
 * @param _qI Pointer to the QGIS interface object
 */
QgsGPSPlugin::QgsGPSPlugin(QgisApp * theQGisApp, QgisIface * theQgisInterFace):
  mMainWindowPointer(theQGisApp), 
  mQGisInterface(theQgisInterFace),
  QgisPlugin(name_,description_,version_,type_)
{
  setupBabel();
}

QgsGPSPlugin::~QgsGPSPlugin()
{
  // delete all our babel formats
  BabelMap::iterator iter;
  for (iter = mImporters.begin(); iter != mImporters.end(); ++iter)
    delete iter->second;
  for (iter = mDevices.begin(); iter != mDevices.end(); ++iter)
    delete iter->second;
}


/*
 * Initialize the GUI interface for the plugin 
 */
void QgsGPSPlugin::initGui()
{
  // add a menu with 2 items
  QPopupMenu *pluginMenu = new QPopupMenu(mMainWindowPointer);
  pluginMenu->insertItem(QIconSet(icon),"&Gps Tools", this, SLOT(run()));
  mMenuBarPointer = ((QMainWindow *) mMainWindowPointer)->menuBar();
  mMenuId = mQGisInterface->addMenu("&Gps", pluginMenu);

  // add an action to the toolbar
  mQActionPointer = new QAction("Gps Tools", QIconSet(icon), "&Wmi",0, 
				this, "run");
  connect(mQActionPointer, SIGNAL(activated()), this, SLOT(run()));
  mQGisInterface->addToolBarIcon(mQActionPointer);
}

//method defined in interface
void QgsGPSPlugin::help()
{
  //implement me!
}

// Slot called when the buffer menu item is activated
void QgsGPSPlugin::run()
{
  // find all GPX layers
  std::vector<QgsVectorLayer*> gpxLayers;
  std::map<QString, QgsMapLayer*>::const_iterator iter;
  std::cerr<<"LAYERS: "<<mQGisInterface->getLayerRegistry()->
    mapLayers().size()<<std::endl;
  for (iter = mQGisInterface->getLayerRegistry()->mapLayers().begin();
       iter != mQGisInterface->getLayerRegistry()->mapLayers().end(); ++iter) {
    std::cerr<<iter->second->name()<<std::endl;
    if (iter->second->type() == QgsMapLayer::VECTOR) {
      QgsVectorLayer* vLayer = dynamic_cast<QgsVectorLayer*>(iter->second);
      if (vLayer->providerType() == "gpx")
	gpxLayers.push_back(vLayer);
    }
  }
  std::cerr<<std::endl;
  
  QgsGPSPluginGui *myPluginGui = 
    new QgsGPSPluginGui(mImporters, mDevices, gpxLayers, mMainWindowPointer, 
			"GPS Tools", true, 0);
  //listen for when the layer has been made so we can draw it
  connect(myPluginGui, SIGNAL(drawRasterLayer(QString)), 
	  this, SLOT(drawRasterLayer(QString)));
  connect(myPluginGui, SIGNAL(drawVectorLayer(QString,QString,QString)), 
	  this, SLOT(drawVectorLayer(QString,QString,QString)));
  connect(myPluginGui, SIGNAL(loadGPXFile(QString, bool, bool, bool)), 
	  this, SLOT(loadGPXFile(QString, bool, bool, bool)));
  connect(myPluginGui, SIGNAL(importGPSFile(QString, QgsBabelFormat*, bool, 
					    bool, bool, QString, QString)),
	  this, SLOT(importGPSFile(QString, QgsBabelFormat*, bool, bool, 
				   bool, QString, QString)));
  connect(myPluginGui, SIGNAL(downloadFromGPS(QString, QString, bool, bool,
					      bool, QString, QString)),
	  this, SLOT(downloadFromGPS(QString, QString, bool, bool, bool,
				     QString, QString)));
  connect(myPluginGui, SIGNAL(uploadToGPS(QgsVectorLayer*, QString, QString)),
	  this, SLOT(uploadToGPS(QgsVectorLayer*, QString, QString)));
  connect(this, SIGNAL(closeGui()), myPluginGui, SLOT(close()));

  myPluginGui->show();
}


void QgsGPSPlugin::drawVectorLayer(QString thePathNameQString, 
				   QString theBaseNameQString, 
				   QString theProviderQString)
{
  mQGisInterface->addVectorLayer(thePathNameQString, theBaseNameQString, 
				 theProviderQString);
}

// Unload the plugin by cleaning up the GUI
void QgsGPSPlugin::unload()
{
  // remove the GUI
  mMenuBarPointer->removeItem(mMenuId);
  mQGisInterface->removeToolBarIcon(mQActionPointer);
  delete mQActionPointer;
}

void QgsGPSPlugin::loadGPXFile(QString filename, bool loadWaypoints, bool loadRoutes,
			       bool loadTracks) {

  //check if input file is readable
  QFileInfo fileInfo(filename);
  if (!fileInfo.isReadable()) {
    QMessageBox::warning(NULL, "GPX Loader",
			 "Unable to read the selected file.\n"
			 "Please reselect a valid file." );
    return;
  }
  
  // remember the directory
  QSettings settings;
  settings.writeEntry("/qgis/gps/gpxdirectory", fileInfo.dirPath());
  
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


void QgsGPSPlugin::importGPSFile(QString inputFilename, QgsBabelFormat* importer, 
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
  QStringList babelArgs = 
    importer->importCommand(mBabelPath, typeArg, 
			       inputFilename, outputFilename);
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


void QgsGPSPlugin::downloadFromGPS(QString device, QString port,
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
  QStringList babelArgs = 
    mDevices[device]->importCommand(mBabelPath, typeArg, 
				       port, outputFilename);
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
  
  // everything was OK, remember the device and port for next time
  QSettings settings;
  settings.writeEntry("/qgis/gps/lastdldevice", device);
  settings.writeEntry("/qgis/gps/lastdlport", port);
  
  emit closeGui();
}


void QgsGPSPlugin::uploadToGPS(QgsVectorLayer* gpxLayer, QString device,
			       QString port) {
  
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
  QStringList babelArgs = 
    mDevices[device]->exportCommand(mBabelPath, typeArg, 
				       source.left(source.findRev('?')), port);
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
  
  // everything was OK, remember this device for next time
  QSettings settings;
  settings.writeEntry("/qgis/gps/lastuldevice", device);
  settings.writeEntry("/qgis/gps/lastulport", port);
  
  emit closeGui();
}


void QgsGPSPlugin::setupBabel() {
  
  // where is gpsbabel?
  QSettings settings;
  mBabelPath = settings.readEntry("/qgis/gps/gpsbabelpath");
  if (mBabelPath.isEmpty())
    mBabelPath = "gpsbabel";
  // the importable formats
  mImporters["Geocaching.com .loc"] =
    new QgsSimpleBabelFormat("geo", true, false, false);
  mImporters["Magellan Mapsend"] = 
    new QgsSimpleBabelFormat("mapsend", true, true, true);
  mImporters["Garmin PCX5"] = 
    new QgsSimpleBabelFormat("pcx", true, false, true);
  mImporters["Garmin Mapsource"] = 
    new QgsSimpleBabelFormat("mapsource", true, true, true);
  mImporters["GPSUtil"] = 
    new QgsSimpleBabelFormat("gpsutil", true, false, false);
  mImporters["PocketStreets 2002/2003 Pushpin"] = 
    new QgsSimpleBabelFormat("psp", true, false, false);
  mImporters["CoPilot Flight Planner"] = 
    new QgsSimpleBabelFormat("copilot", true, false, false);
  mImporters["Magellan Navigator Companion"] = 
    new QgsSimpleBabelFormat("magnav", true, false, false);
  mImporters["Holux"] = 
    new QgsSimpleBabelFormat("holux", true, false, false);
  mImporters["Topo by National Geographic"] = 
    new QgsSimpleBabelFormat("tpg", true, false, false);
  mImporters["TopoMapPro"] = 
    new QgsSimpleBabelFormat("tmpro", true, false, false);
  mImporters["GeocachingDB"] = 
    new QgsSimpleBabelFormat("gcdb", true, false, false);
  mImporters["Tiger"] = 
    new QgsSimpleBabelFormat("tiger", true, false, false);
  mImporters["EasyGPS Binary Format"] = 
    new QgsSimpleBabelFormat("easygps", true, false, false);
  mImporters["Delorme Routes"] = 
    new QgsSimpleBabelFormat("saroute", false, false, true);
  mImporters["Navicache"] = 
    new QgsSimpleBabelFormat("navicache", true, false, false);
  mImporters["PSITrex"] = 
    new QgsSimpleBabelFormat("psitrex", true, true, true);
  mImporters["Delorme GPS Log"] = 
    new QgsSimpleBabelFormat("gpl", false, false, true);
  mImporters["OziExplorer"] = 
    new QgsSimpleBabelFormat("ozi", true, false, false);
  mImporters["NMEA Sentences"] = 
    new QgsSimpleBabelFormat("nmea", true, false, true);
  mImporters["Delorme Street Atlas 2004 Plus"] = 
    new QgsSimpleBabelFormat("saplus", true, false, false);
  mImporters["Microsoft Streets and Trips"] = 
    new QgsSimpleBabelFormat("s_and_t", true, false, false);
  mImporters["NIMA/GNIS Geographic Names"] = 
    new QgsSimpleBabelFormat("nima", true, false, false);
  mImporters["Maptech"] = 
    new QgsSimpleBabelFormat("mxf", true, false, false);
  mImporters["Mapopolis.com Mapconverter Application"] = 
    new QgsSimpleBabelFormat("mapconverter", true, false, false);
  mImporters["GPSman"] = 
    new QgsSimpleBabelFormat("gpsman", true, false, false);
  mImporters["GPSDrive"] = 
    new QgsSimpleBabelFormat("gpsdrive", true, false, false);
  mImporters["Fugawi"] = 
    new QgsSimpleBabelFormat("fugawi", true, false, false);
  mImporters["DNA"] = 
    new QgsSimpleBabelFormat("dna", true, false, false);

  // and the GPS devices
  mDevices["Garmin serial"] = 
    new QgsBabelCommand("%babel -i garmin -o gpx /dev/ttyS0 %out",
			"%babel -i gpx -o garmin %in /dev/ttyS0");
  QStringList deviceNames = settings.readListEntry("/qgis/gps/devicelist");
  QStringList::iterator iter;
  for (iter = deviceNames.begin(); iter != deviceNames.end(); ++iter) {
    QString download = settings.
      readEntry(QString("/qgis/gps/devices/%1/download").arg(*iter), "");
    QString upload = settings.
      readEntry(QString("/qgis/gps/devices/%1/upload").arg(*iter), "");
    mDevices[*iter] = new QgsBabelCommand(download, upload);
  }
}




/** 
 * Required extern functions needed  for every plugin 
 * These functions can be called prior to creating an instance
 * of the plugin class
 */
// Class factory to return a new instance of the plugin class
QGISEXTERN QgisPlugin * classFactory(QgisApp * theQGisAppPointer, 
				     QgisIface * theQgisInterfacePointer)
{
  return new QgsGPSPlugin(theQGisAppPointer, theQgisInterfacePointer);
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
