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
#ifndef QGSSPITPLUGIN_H
#define QGSSPITPLUGIN_H
#include "../qgisplugin.h"

extern "C"
{
#include <libpq-fe.h>
}

class QAction;
#include <QObject>

/**
* \class QgsSpitPlugin
* \brief SPIT PostgreSQL/PostGIS plugin for QGIS
*
*/
class QgsSpitPlugin: public QObject, public QgisPlugin
{
  Q_OBJECT public:
    /**
    * Constructor for a plugin. The QgisInterface pointer is passed by
    * QGIS when it attempts to instantiate the plugin.
    * @param qI Pointer to the QgisInterface object.
    */
    QgsSpitPlugin( QgisInterface * qI );

    //! Destructor
    virtual ~ QgsSpitPlugin();
  public slots:
    //! init the gui
    virtual void initGui();
    void spit();
    //! unload the plugin
    void unload();
    //! update the plugins theme when the app tells us its theme is changed
    void setCurrentTheme( QString theThemeName );
  private:
//! Name of the plugin
    QString pName;
    //! Version
    QString pVersion;
    //! Descrption of the plugin
    QString pDescription;
    //! Category of the plugin
    QString pCategory;
    //! Plugin type as defined in QgisPlugin::PLUGINTYPE
    int ptype;
    //! Pionter to QGIS main application object
    QWidget *qgisMainWindow;
    //! Pointer to the QGIS interface object
    QgisInterface *qI;
    //! Pointer to the QAction used in the menu and on the toolbar
    QAction *spitAction;
};

#endif
