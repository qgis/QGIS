/***************************************************************************
                          qgsgpsplugin.h 
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
#ifndef QGSGPSPLUGIN_H
#define QGSGPSPLUGIN_H
#include "qgsbabelformat.h"
#include "../qgisplugin.h"
#include <qwidget.h>
#include <qgisapp.h>

class QgsVectorLayer;

/** A plugin with various GPS tools.
*/
class QgsGPSPlugin:public QObject, public QgisPlugin
{
  Q_OBJECT
public:
  /** Constructor for a plugin. The QgisApp and QgisIface pointers 
      are passed by QGIS when it attempts to instantiate the plugin.
      @param qgis Pointer to the QgisApp object
      @param qI Pointer to the QgisIface object. 
  */
  QgsGPSPlugin(QgisApp * , QgisIface * );

  //! init the gui
  virtual void initGui();
  //! Destructor
  virtual ~QgsGPSPlugin();

public slots:
  //! Show the dialog box
  void run();
  //! Create a new GPX layer
  void createGPX();
  //! Add a vector layer given vectorLayerPath, baseName, providerKey
  void drawVectorLayer(QString,QString,QString);
  //! unload the plugin
  void unload();
  //! show the help document
  void help();
  
  //! load a GPX file
  void loadGPXFile(QString filename, bool loadWaypoints, bool loadRoutes,
		   bool loadTracks);
  void importGPSFile(QString inputFilename, QgsBabelFormat* importer, 
		     bool importWaypoints, bool importRoutes, 
		     bool importTracks, QString outputFilename, 
		     QString layerName);
  void downloadFromGPS(QString device, QString port,
		       bool downloadWaypoints, bool downloadRoutes,
		       bool downloadTracks, QString outputFilename,
		       QString layerName);
  void uploadToGPS(QgsVectorLayer* gpxLayer, QString device,
		   QString port);

signals:
  
  void closeGui();

private:
  
  //! Initializes all variables needed to run GPSBabel.
  void setupBabel();

  //! Id of the plugin's menu. Used for unloading
  int mMenuId;
  //! Pointer to our menu
  QMenuBar *mMenuBarPointer;
  //! Pionter to QGIS main application object
  QgisApp *mMainWindowPointer;
  //! Pointer to the QGIS interface object
  QgisIface *mQGisInterface;
  //! Pointer to the QAction object used in the menu and toolbar
  QAction *mQActionPointer;
  
  //! The path to the GPSBabel program
  QString mBabelPath;
  //! Importers for external GPS data file formats
  std::map<QString, QgsBabelFormat*> mImporters;
  //! Upload/downloaders for GPS devices
  std::map<QString, QgsBabelFormat*> mDevices;
};

#endif
