/***************************************************************************
                          qgspggeoprocessing.cpp 
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

// includes
#include <iostream>
#include <vector>
#include "../../src/qgisapp.h"
#include "../../src/qgsmaplayer.h"
#include "../../src/qgsvectorlayer.h"
#include "../../src/qgsdataprovider.h"
#include "../../src/qgsfield.h"

#include <qtoolbar.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qlineedit.h>
#include <qaction.h>
#include "qgsdlgpgbuffer.h"
#include "qgspggeoprocessing.h"
extern "C"
{
#include <libpq-fe.h>
}
// xpm for creating the toolbar icon
#include "matrix1.xpm"

/**
* Constructor for the plugin. The plugin is passed a pointer to the main app
* and an interface object that provides access to exposed functions in QGIS.
* @param qgis Pointer to the QGIS main window
* @parma _qI Pointer to the QGIS interface object
*/
QgsPgGeoprocessing::QgsPgGeoprocessing(QgisApp * qgis, QgisIface * _qI):qgisMainWindow(qgis), qI(_qI)
{
  /** Initialize the plugin and set the required attributes */
    pName = "PostgreSQL Geoprocessing";
    pVersion = "Version 0.1";
    pDescription = "Geoprocessing functions for working with PostgreSQL/PostGIS layers";

}

QgsPgGeoprocessing::~QgsPgGeoprocessing()
{

}

/* Following functions return name, description, version, and type for the plugin */
QString QgsPgGeoprocessing::name()
{
    return pName;
}

QString QgsPgGeoprocessing::version()
{
    return pVersion;

}

QString QgsPgGeoprocessing::description()
{
    return pDescription;

}

int QgsPgGeoprocessing::type()
{
    return QgisPlugin::UI;
}

/*
* Initialize the GUI interface for the plugin 
*/
void QgsPgGeoprocessing::initGui()
{
    // add a menu with 2 items
    QPopupMenu *pluginMenu = new QPopupMenu(qgisMainWindow);

    pluginMenu->insertItem("&Buffer Features", this, SLOT(buffer()));
    pluginMenu->insertItem("&Unload Geoprocessing Plugin", this, SLOT(unload()));

    menu = ((QMainWindow *) qgisMainWindow)->menuBar();

    menuId = menu->insertItem("&Geoprocessing", pluginMenu);


}

// Slot called when the buffer menu item is activated
void QgsPgGeoprocessing::buffer()
{
    // need to get a pointer to the current layer
    QgsVectorLayer *lyr = (QgsVectorLayer *) qI->activeLayer();
    if (lyr) {
        // check the layer to see if its a postgres layer
        if (lyr->providerType() == "postgres") {
            QString dataSource = lyr->source(); //qI->activeLayerSource();

            // create the connection string
            QString connInfo = dataSource.left(dataSource.find("table="));
            QMessageBox::information(0, "Data source", QString("Datasource:%1\n\nConnectionInfo:%2").arg(dataSource).arg(connInfo));
            // get the table name
            QStringList connStrings = QStringList::split(" ", dataSource);
            QStringList tables = connStrings.grep("table=");
            QString table = tables[0];
            QString tableName = table.mid(table.find("=") + 1);
            QStringList dbnames = connStrings.grep("dbname=");
            QString dbname = dbnames[0];
            dbname = dbname.mid(dbname.find("=") + 1);
            // show dialog to fetch buffer distrance, new layer name, and option to
            QgsDlgPgBuffer *bb = new QgsDlgPgBuffer();
            QString lbl = tr("Buffer features in layer %1").arg(tableName);

            bb->setBufferLabel(lbl);
            // set the fields 
            QgsDataProvider *dp = lyr->getDataProvider();
            std::vector<QgsField> flds = dp->fields();
            for(int i=0; i < flds.size(); i++){
              bb->addFieldItem(flds[i].getName());
            }
            if (bb->exec()) {
                // connect to the database
                PGconn *conn = PQconnectdb((const char *) connInfo);
                if (PQstatus(conn) == CONNECTION_OK) {
                    // get some info from the source layer so we can duplicate it in the new layer
                    QString sql = "select * from geometry_columns";
                    // first create the new table
                    sql = QString("create table %1 (objectid int)").arg(bb->bufferLayerName());
                    PGresult *result = PQexec(conn, (const char *) sql);
                    if (PQresultStatus(result) == PGRES_COMMAND_OK) {
                        // add the geometry column
                        sql = "select addgeometrycolumn(%1,%2,%3,%4,%5";
                        // add new layer to the map
                    } else {
                        QMessageBox::critical(0, "Unable to create table",
                                              QString("Failed to create the output table %1").arg(bb->bufferLayerName()));
                    }
                } else {
                    // connection error
                    QString err = tr("Error connecting to the database");
                    QMessageBox::critical(0, err, PQerrorMessage(conn));
                }
            }
        } else {
            QMessageBox::critical(0, "Not a PostgreSQL/PosGIS Layer",
                                  QString
                                  ("%1 is not a PostgreSQL/PosGIS layer. Geoprocessing functions are only available for PostgreSQL/PosGIS Layers").
                                  arg(lyr->name()));
        }
    } else {
        QMessageBox::warning(0, "No Active Layer", "You must select a layer in the legend to buffer");
    }
}

// Unload the plugin by cleaning up the GUI
void QgsPgGeoprocessing::unload()
{
    // remove the GUI
    menu->removeItem(menuId);
}

/** 
* Required extern functions needed  for every plugin 
* These functions can be called prior to creating an instance
* of the plugin class
*/
// Class factory to return a new instance of the plugin class
extern "C" QgisPlugin * classFactory(QgisApp * qgis, QgisIface * qI)
{
    return new QgsPgGeoprocessing(qgis, qI);
}

// Return the name of the plugin
extern "C" QString name()
{
    return QString("PostgreSQL Geoprocessing");
}

// Return the description
extern "C" QString description()
{
    return QString("Geoprocessing functions for working with PostgreSQL/PostGIS layers");
}

// Return the type (either UI or MapLayer plugin)
extern "C" int type()
{
    return QgisPlugin::UI;
}

// Delete ourself
extern "C" void unload(QgisPlugin * p)
{

    delete p;
}
