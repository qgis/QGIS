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
#include "icon_buffer.xpm"

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
     // Create the action for tool
    QAction *bufferAction = new QAction("Buffer features", QIconSet(icon_buffer), "&Buffer",
                                              0, this, "buffer");
    // Connect the action to the zoomPrevous slot
    connect(bufferAction, SIGNAL(activated()), this, SLOT(buffer()));
    // Add the toolbar
    toolBar = new QToolBar((QMainWindow *) qgisMainWindow, "geoprocessing");
    toolBar->setLabel("PostgreSQL/PostGIS Geoprocessing");
    // Add the zoom previous tool to the toolbar
    bufferAction->addTo(toolBar);
    

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
            std::cerr << "Data source = " << QString("Datasource:%1\n\nConnectionInfo:%2").arg(dataSource).arg(connInfo) << std::endl;
            // get the table name
            QStringList connStrings = QStringList::split(" ", dataSource);
            QStringList tables = connStrings.grep("table=");
            QString table = tables[0];
            QString tableName = table.mid(table.find("=") + 1);
            // get the schema
            QString schema = tableName.left(tableName.find("."));
            // get the database name
            QStringList dbnames = connStrings.grep("dbname=");
            QString dbname = dbnames[0];
            dbname = dbname.mid(dbname.find("=") + 1);
            // get the user name
            QStringList userNames = connStrings.grep("user=");
            QString user = userNames[0];
            user = user.mid(user.find("=") + 1);

            // show dialog to fetch buffer distrance, new layer name, and option to
            QgsDlgPgBuffer *bb = new QgsDlgPgBuffer();
            // set the label
            QString lbl = tr("Buffer features in layer %1").arg(tableName);
            bb->setBufferLabel(lbl);
            // set a default output table name
            bb->setBufferLayerName(tableName.mid(tableName.find(".") + 1) + "_buffer");
            // set the fields on the dialog box drop-down
            QgsDataProvider *dp = lyr->getDataProvider();
            std::vector < QgsField > flds = dp->fields();
            for (int i = 0; i < flds.size(); i++) {
                // check the field type -- if its int we can use it
                if (flds[i].getType().find("int") > -1) {
                    bb->addFieldItem(flds[i].getName());
                }
            }
            // connect to the database
            PGconn *conn = PQconnectdb((const char *) connInfo);
            if (PQstatus(conn) == CONNECTION_OK) {
                // populate the schema drop-down
                QString schemaSql =
                  QString("select nspname from pg_namespace,pg_user where nspowner = usesysid and usename = '%1'").arg(user);
                PGresult *schemas = PQexec(conn, (const char *) schemaSql);
                if (PQresultStatus(schemas) == PGRES_TUPLES_OK) {
                    // add the schemas to the drop-down, otherwise just public (the
                    // default) will show up
                    for (int i = 0; i < PQntuples(schemas); i++) {
                        bb->addSchema(PQgetvalue(schemas, i, 0));
                    }
                }
                PQclear(schemas);
                // query the geometry_columns table to get the srid and use it as default
                QString sridSql =
                  QString("select srid,f_geometry_column from geometry_columns where f_table_schema='%1' and f_table_name='%2'")
                  .arg(schema)
                  .arg(tableName.mid(tableName.find(".") + 1));
                std::cerr << "SRID SQL" << sridSql << std::endl;
                QString geometryCol;
                PGresult *sridq = PQexec(conn, (const char *) sridSql);
                if (PQresultStatus(sridq) == PGRES_TUPLES_OK) {
                    bb->setSrid(PQgetvalue(sridq, 0, 0));
                    geometryCol = PQgetvalue(sridq, 0, 1);
                    bb->setGeometryColumn(geometryCol);
                } else {
                    bb->setSrid("-1");
                }
                PQclear(sridq);
                // exec the dialog and process if user selects ok
                if (bb->exec()) {
                    // determine what column to use as the obj id
                    QString objId = bb->objectIdColumn();
                    QString objIdType = "int";
                    QString objIdValue;
                    if (objId == "Create unique object id") {
                        objId = "objectid";
                        objIdType = "serial";
                        objIdValue = "DEFAULT";
                    } else {
                        objIdValue = objId;
                    }
                    // set the schema path (need public to find the postgis
                    // functions)
                    PGresult *result = PQexec(conn, "begin work");
                    PQclear(result);
                    QString sql;
                    // set the schema search path if schema is not public
                    if(bb->schema() != "public")
                    {
                      sql = QString("set search_path = '%1','public'").arg(bb->schema());
                      result = PQexec(conn, (const char *) sql);
                      PQclear(result);
                      std::cerr << sql << std::endl;
                    }
                    // first create the new table

                    sql = QString("create table %1.%2 (%3 %4)")
                      .arg(bb->schema())
                      .arg(bb->bufferLayerName())
                      .arg(objId)
                      .arg(objIdType);
                    std::cerr << sql << std::endl;
                    result = PQexec(conn, (const char *) sql);
                    std::cerr << "Status from create table is " << PQresultStatus(result) << std::endl;
                    if (PQresultStatus(result) == PGRES_COMMAND_OK) {
                        PQclear(result);
                        // add the geometry column
                        //<db_name>, <table_name>, <column_name>, <srid>, <type>, <dimension>
                        sql = QString("select addgeometrycolumn('%1','%2','%3',%4,'%5',%6)")
                          .arg(dbname)
                          .arg(bb->bufferLayerName())
                          .arg(bb->geometryColumn())
                          .arg(bb->srid())
                          .arg("POLYGON")
                          .arg("2");
                        std::cerr << sql << std::endl;
                        PGresult *geoCol = PQexec(conn, (const char *) sql);
                        PQclear(geoCol);
                        // drop the check constraint based on geometry type
                        sql = QString("alter table %1.%2 drop constraint \"$2\"")
                          .arg(bb->schema())
                          .arg(bb->bufferLayerName());
                        std::cerr << sql << std::endl;
                        result = PQexec(conn, (const char *) sql);
                        PQclear(result);
                        // check pg version and formulate insert query accordingly
                        result = PQexec(conn,"select version()");
                        QString versionString = PQgetvalue(result,0,0);
                        QStringList versionParts = QStringList::split(" ", versionString);
                        // second element is the version number
                        QString version = versionParts[1];
                        if(version < "7.4.0"){
                          // modify the tableName 
                          tableName = tableName.mid(tableName.find(".")+1);
                        }
                          std::cerr << "Table name for PG 7.3 is: " << tableName.mid(tableName.find(".")+1) << std::endl;
                        //   if(PQresultStatus(geoCol) == PGRES_COMMAND_OK) {
                        // do the buffer and insert the features
                        if (objId == "objectid") {
                            sql = QString("insert into %1 (%2) select buffer(%3,%4) from %5")
                              .arg(bb->bufferLayerName())
                              .arg(bb->geometryColumn())
                              .arg(geometryCol)
                              .arg(bb->bufferDistance().toDouble())
                              .arg(tableName);
                        } else {
                            sql = QString("insert into %1 select %2, buffer(%3,%4) from %5")
                              .arg(bb->bufferLayerName())
                              .arg(objIdValue)
                              .arg(geometryCol)
                              .arg(bb->bufferDistance().toDouble())
                              .arg(tableName);
                            std::cerr << sql << std::endl;

                        }
                        result = PQexec(conn, (const char *) sql);
                        PQclear(result);
                        // }
                        std::cerr << sql << std::endl;
                        result = PQexec(conn, "end work");
                        PQclear(result);
                        result = PQexec(conn, "commit;vacuum");
                        PQclear(result);
                        PQfinish(conn);
                       // QMessageBox::information(0, "Add to Map?", "Do you want to add the layer to the map?");
                        // add new layer to the map
                        if (bb->addLayerToMap()) {
                            // create the connection string
                            QString newLayerSource = dataSource.left(dataSource.find("table="));
                            std::cerr << "newLayerSource: " << newLayerSource << std::endl;
                            // add the schema.table and geometry column
                            /*  newLayerSource += "table=" + bb->schema() + "." + bb->bufferLayerName()  
                               + " (" + bb->geometryColumn() + ")"; */
                            std::cerr << "newLayerSource: " << newLayerSource << std::endl;
                            std::cerr << "Adding new layer using\n\t" << newLayerSource << std::endl;
                            // host=localhost dbname=gis_data user=gsherman password= table=public.alaska (the_geom)
                            qI->addVectorLayer(newLayerSource, bb->schema() + "." + bb->bufferLayerName()
                                               + " (" + bb->geometryColumn() + ")", "postgres");

                        }
                    } else {
                        QMessageBox::critical(0, "Unable to create table",
                                              QString("Failed to create the output table %1").arg(bb->bufferLayerName()));
                    }

                }
                delete bb;
            } else {
                // connection error
                QString err = tr("Error connecting to the database");
                QMessageBox::critical(0, err, PQerrorMessage(conn));
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
    delete toolBar;
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
