/***************************************************************************
                          plugin.h 
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
#ifndef PLUGIN
#define PLUGIN
#include "../qgisplugin.h"
#include <qwidget.h>
#include <qgisapp.h>

class QgsVectorLayer;

/**
* \class Plugin
* \brief OpenModeller plugin for QGIS
*
*/
class Plugin:public QObject, public QgisPlugin
{
  Q_OBJECT public:
      /** 
       * Constructor for a plugin. The QgisApp and QgisIface pointers are passed by 
       * QGIS when it attempts to instantiate the plugin.
       * @param qgis Pointer to the QgisApp object
       * @param qI Pointer to the QgisIface object. 
       */
      Plugin(QgisApp * , QgisIface * );
  /**
   * Virtual function to return the name of the plugin. The name will be used when presenting a list 
   * of installable plugins to the user
   */
  virtual QString name();
  /**
   * Virtual function to return the version of the plugin. 
   */
  virtual QString version();
  /**
   * Virtual function to return a description of the plugins functions 
   */
  virtual QString description();
  /**
   * Return the plugin type
   */
  virtual int type();
  //! init the gui
  virtual void initGui();
  //! Destructor
  virtual ~ Plugin();
  public slots:
  //! Show the dialog box
  void run();
  //!draw a raster layer in the qui
  void drawRasterLayer(QString);
  //! Add a vector layer given vectorLayerPath, baseName, providerKey ("ogr" or "postgres");
  void drawVectorLayer(QString,QString,QString);
  //! unload the plugin
  void unload();
  //! show the help document
  void help();
  
  //! load a GPX or LOC file
  void loadGPXFile(QString filename, bool loadWaypoints, bool loadRoutes,
		   bool loadTracks);
  void importGPSFile(QString inputFilename, QString inputFormat, 
		     bool importWaypoints, bool importRoutes, 
		     bool importTracks, QString outputFilename, 
		     QString layerName);
  void downloadFromGPS(QString protocol, QString deviceFilename,
		       bool downloadWaypoints, bool downloadRoutes,
		       bool downloadTracks, QString outputFilename,
		       QString layerName);
  void uploadToGPS(QgsVectorLayer* gpxLayer, QString protocol,
		   QString deviceFilename);

 signals:
  
  void closeGui();

    private:


  //! Name of the plugin
  QString pluginNameQString;
  //! Version
  QString pluginVersionQString;
  //! Descrption of the plugin
  QString pluginDescriptionQString;
  //! Plugin type as defined in QgisPlugin::PLUGINTYPE
  int pluginType;
  //! Id of the plugin's menu. Used for unloading
  int menuIdInt;
  //! Pointer to our toolbar
  QToolBar *toolBarPointer;
  //! Pointer to our menu
  QMenuBar *menuBarPointer;
  //! Pionter to QGIS main application object
  QgisApp *qgisMainWindowPointer;
  //! Pointer to the QGIS interface object
  QgisIface *qGisInterface;
};

#endif
