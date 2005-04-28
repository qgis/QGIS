/***************************************************************************
                          qgsworldmapimporter.h 
 WorldMap analysis result importer for qgis.
 Functions:
   Buffer
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
#ifndef QGISWorldMapImporter
#define QGISWorldMapImporter

#include <qgis/qgisplugin.h>
#include <qwidget.h>

/**
* \class QgsWorldMapImporter
* \brief WorldMap analysis importer for QGIS
*
*/
class QgsWorldMapImporter:public QObject, public QgisPlugin
{
  Q_OBJECT public:
      /** 
       * Constructor for a plugin. The QgisApp and QgisIface pointers are passed by 
       * QGIS when it attempts to instantiate the plugin.
       * @param qgis Pointer to the QgisApp object
       * @param qI Pointer to the QgisIface object. 
       */
      QgsWorldMapImporter(QgisApp * , QgisIface * );
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
  //! Destructor
  virtual ~ QgsWorldMapImporter();
  public slots:
  //! init the gui
  virtual void initGui();
  //! Show the dialog box
  void run();
  //!draw a raster layer in the qui
  void drawLayer(QString);
  //! unload the plugin
  void unload();
  //! show the help document
  void help();
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
  int menuId;
  //! Pointer to our toolbar
  QToolBar *toolBarPointer;
  //! Pionter to QGIS main application object
  QgisApp *qgisMainWindowPointer;
  //! Pointer to the QGIS interface object
  QgisIface *qGisInterface;
};

#endif
