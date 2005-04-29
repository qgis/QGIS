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
#ifndef QGSFISHINGSPOTSPLUGIN
#define QGSFISHINGSPOTSPLUGIN
#include <qgisplugin.h>
#include <qwidget.h>

#include <qgisapp.h>

/**
* \class Plugin
* \brief OpenModeller plugin for QGIS
*
*/
class QgsFishingSpotsPlugin:public QObject, public QgisPlugin
{
  Q_OBJECT public:
      /** 
       * Constructor for a plugin. The QgisApp and QgisIface pointers are passed by 
       * QGIS when it attempts to instantiate the plugin.
       * @param qgis Pointer to the QgisApp object
       * @param qI Pointer to the QgisIface object. 
       */
      QgsFishingSpotsPlugin(QgisApp * , QgisIface * );
  //! Destructor
  virtual ~ QgsFishingSpotsPlugin();
  public slots:
  //! init the gui
  virtual void initGui();
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
    private:

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
