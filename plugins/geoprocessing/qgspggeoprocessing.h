/***************************************************************************
                          qgspggeoprocessing.h 
 Geoprocessing plugin for PostgreSQL/PostGIS layers
 Functions:
   Buffer
                             -------------------
    begin                : Jan 21, 2004
    copyright            : (C) 2004 by Gary E.Sherman
    email                : sherman at mrcc.com
  
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
#ifndef QGISQgsPgGeoprocessing_H
#define QGISQgsPgGeoprocessing_H
#include "../qgisplugin.h"
#include <qwidget.h>
#include <qmainwindow.h>
extern "C"
{
#include <libpq-fe.h>
}
class QMessageBox;
class QToolBar;
class QMenuBar;
class QPopupMenu;

//#include "qgsworkerclass.h"
#include "../../src/qgisapp.h"

/**
* \class QgsPgGeoprocessing
* \brief PostgreSQL/PostGIS plugin for QGIS
*
*/
class QgsPgGeoprocessing:public QObject, public QgisPlugin
{
  Q_OBJECT public:
/** 
* Constructor for a plugin. The QgisApp and QgisIface pointers are passed by 
* QGIS when it attempts to instantiate the plugin.
* @param qgis Pointer to the QgisApp object
* @param qI Pointer to the QgisIface object. 
*/
    QgsPgGeoprocessing(QgisApp * qgis, QgisIface * qI);

    //! init the gui
    virtual void initGui();
    //! Destructor
      virtual ~ QgsPgGeoprocessing();
    public slots:
      //! buffer features in a layer
    void buffer();
    //! unload the plugin
    void unload();
    //! show the help document
    void help();
  private:
  //! get postgis version string
   QString postgisVersion(PGconn *);
  //! get status of GEOS capability
  bool hasGEOS(PGconn *);
  //! get status of GIST capability
  bool hasGIST(PGconn *);
  //! get status of PROJ4 capability
  bool hasPROJ(PGconn *);
  
  QString postgisVersionInfo;
  bool geosAvailable;
  bool gistAvailable;
  bool projAvailable;
  
    //! Id of the plugin's menu. Used for unloading
    int menuId;
    //! Pointer to our toolbar
    QToolBar *toolBar;
    //! Pointer to our menu
    QMenuBar *menu;
    //! Pionter to QGIS main application object
    QgisApp *qgisMainWindow;
    //! Pointer to the QGIS interface object
    QgisIface *qI;
};

#endif
