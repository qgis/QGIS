/***************************************************************************
                          qgsspitplugin.h 
 Shapefile to PostgreSQL Import Tool plugin 
                             -------------------
    begin                : Jan 30, 2004
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
#ifndef QGSSPITPLUGIN_H
#define QGSSPITPLUGIN_H
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
* \class QgsSpitPlugin
* \brief SPIT PostgreSQL/PostGIS plugin for QGIS
*
*/
class QgsSpitPlugin:public QObject, public QgisPlugin
{
  Q_OBJECT public:
/** 
* Constructor for a plugin. The QgisApp and QgisIface pointers are passed by 
* QGIS when it attempts to instantiate the plugin.
* @param qgis Pointer to the QgisApp object
* @param qI Pointer to the QgisIface object. 
*/
    QgsSpitPlugin(QgisApp * qgis, QgisIface * qI);

    //! Destructor
      virtual ~ QgsSpitPlugin();
    public slots:
    //! init the gui
    virtual void initGui();
    void spit();
    //! unload the plugin
    void unload();
  private:
//! Name of the plugin
      QString pName;
    //! Version
    QString pVersion;
    //! Descrption of the plugin
    QString pDescription;
    //! Plugin type as defined in QgisPlugin::PLUGINTYPE
    int ptype;
    //! Id of the plugin's menu. Used for unloading
    int menuId;
    //! Pointer to our toolbar
    QToolBar *toolBar;
    //! Pionter to QGIS main application object
    QgisApp *qgisMainWindow;
    //! Pointer to the QGIS interface object
    QgisIface *qI;
    //! Pointer to the QAction used in the menu and on the toolbar
    QAction *spitAction;
};

#endif
