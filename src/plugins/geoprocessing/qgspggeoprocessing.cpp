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
#include <vector>
#include "qgisinterface.h"
#include "qgsmaplayer.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsfield.h"
#include "qgsdatasourceuri.h"
#include "qgslogger.h"

#include <QMessageBox>
#include <QAction>
#include <QApplication>
#include <QMenu>

#include "qgsdlgpgbuffer.h"
#include "qgspggeoprocessing.h"
#include "qgslogger.h"


static const char * const ident_ = "$Id$";

static const QString name_ = QObject::tr( "PostgreSQL Geoprocessing" );
static const QString description_ = QObject::tr( "Geoprocessing functions for working with PostgreSQL/PostGIS layers" );
static const QString version_ = QObject::tr( "Version 0.1" );
static const QgisPlugin::PLUGINTYPE type_ = QgisPlugin::UI;


/**
 * Constructor for the plugin. The plugin is passed a pointer to the main app
 * and an interface object that provides access to exposed functions in QGIS.
 * @param qgis Pointer to the QGIS main window
 * @parma _qI Pointer to the QGIS interface object
 */
QgsPgGeoprocessing::QgsPgGeoprocessing( QgisInterface * _qI )
    : QgisPlugin( name_, description_, version_, type_ ),
    qgisMainWindow( _qI->mainWindow() ),
    qI( _qI )
{
}

QgsPgGeoprocessing::~QgsPgGeoprocessing()
{

}


/*
 * Initialize the GUI interface for the plugin
 */
void QgsPgGeoprocessing::initGui()
{
  // Create the action for tool
  bufferAction = new QAction( QIcon( ":/geoprocessing.png" ), tr( "&Buffer features" ), this );
  bufferAction->setWhatsThis( tr( "Create a buffer for a PostgreSQL layer. A new layer is created in the database with the buffered features." ) );
  // Connect the action to the buffer slot
  connect( bufferAction, SIGNAL( triggered() ), this, SLOT( buffer() ) );

  // Add the icon to the toolbar
  qI->addToolBarIcon( bufferAction );
  qI->addPluginToMenu( tr( "&Geoprocessing" ), bufferAction );

}

// Slot called when the buffer menu item is triggered
void QgsPgGeoprocessing::buffer()
{
  // need to get a pointer to the current layer
  QgsMapLayer *layer = qI->activeLayer();
  if ( layer )
  {
    QgsVectorLayer *lyr = ( QgsVectorLayer* )layer;
    // check the layer to see if its a postgres layer
    if ( layer->type() != QgsMapLayer::RasterLayer &&
         lyr->providerType() == "postgres" )
    {

      QgsDataSourceURI uri( lyr->source() );

      QgsDebugMsg( "data source = " + uri.connectionInfo() );

      // connect to the database and check the capabilities
      PGconn *capTest = PQconnectdb( uri.connectionInfo().toUtf8() );
      if ( PQstatus( capTest ) == CONNECTION_OK )
      {
        postgisVersion( capTest );
      }
      PQfinish( capTest );
      if ( geosAvailable )
      {
        // show dialog to fetch buffer distrance, new layer name, and option to
        // add the new layer to the map
        QgsDlgPgBuffer *bb = new QgsDlgPgBuffer( qI );

        // set the label
        QString lbl = tr( "Buffer features in layer %1" ).arg( uri.table() );
        bb->setBufferLabel( lbl );
        // set a default output table name
        bb->setBufferLayerName( uri.table() + "_buffer" );

        QString tableName = uri.quotedTablename();

        // set the fields on the dialog box drop-down
        QgsVectorDataProvider *dp = qobject_cast<QgsVectorDataProvider *>( lyr->dataProvider() );
        QgsFieldMap flds = dp->fields();
        for ( QgsFieldMap::iterator it = flds.begin(); it != flds.end(); ++it )
        {
          // check the field type -- if its int we can use it
          if ( it->typeName().indexOf( "int" ) > -1 )
          {
            bb->addFieldItem( it->name() );
          }
        }
        // connect to the database
        PGconn *conn = PQconnectdb( uri.connectionInfo().toUtf8() );
        if ( PQstatus( conn ) == CONNECTION_OK )
        {
          // populate the schema drop-down
          QString schemaSql =
            QString( "select nspname from pg_namespace,pg_user where nspowner = usesysid and usename = '%1'" ).arg( uri.username() );
          PGresult *schemas = PQexec( conn, schemaSql.toUtf8() );
          if ( PQresultStatus( schemas ) == PGRES_TUPLES_OK )
          {
            // add the schemas to the drop-down, otherwise just public (the
            // default) will show up
            for ( int i = 0; i < PQntuples( schemas ); i++ )
            {
              bb->addSchema( PQgetvalue( schemas, i, 0 ) );
            }
          }
          PQclear( schemas );
          // query the geometry_columns table to get the srid and use it as default
          QString sridSql =
            QString( "select srid,f_geometry_column from geometry_columns where f_table_schema='%1' and f_table_name='%2'" )
            .arg( uri.schema() )
            .arg( uri.table() );

          QgsDebugMsg( "SRID SQL: " + sridSql );

          QString geometryCol;
          PGresult *sridq = PQexec( conn, sridSql.toUtf8() );
          if ( PQresultStatus( sridq ) == PGRES_TUPLES_OK )
          {
            bb->setSrid( PQgetvalue( sridq, 0, 0 ) );
            geometryCol = PQgetvalue( sridq, 0, 1 );
            bb->setGeometryColumn( geometryCol );
          }
          else
          {
            bb->setSrid( "-1" );
          }
          PQclear( sridq );
          // exec the dialog and process if user selects ok
          if ( bb->exec() )
          {
            QApplication::setOverrideCursor( Qt::WaitCursor );
            qApp->processEvents();
            // determine what column to use as the obj id
            QString objId = bb->objectIdColumn();
            QString objIdType = "int";
            QString objIdValue;
            if ( objId == "Create unique object id" )
            {
              objId = "objectid";
              objIdType = "serial";
              objIdValue = "DEFAULT";
            }
            else
            {
              objIdValue = objId;
            }
            // set the schema path (need public to find the postgis
            // functions)
            PGresult *result = PQexec( conn, "begin work" );
            PQclear( result );
            QString sql;
            // set the schema search path if schema is not public
            if ( bb->schema() != "public" )
            {
              sql = QString( "set search_path = '%1','public'" ).arg( bb->schema() );
              result = PQexec( conn, sql.toUtf8() );
              PQclear( result );

              QgsDebugMsg( "SQL: " + sql );
            }
            // first create the new table

            sql = QString( "create table %1.%2 (%3 %4 PRIMARY KEY)" )
                  .arg( bb->schema() )
                  .arg( bb->bufferLayerName() )
                  .arg( objId )
                  .arg( objIdType );
            QgsDebugMsg( "SQL: " + sql );

            result = PQexec( conn, sql.toUtf8() );
            QgsDebugMsg( QString( "Status from create table is %1" ).arg( PQresultStatus( result ) ) );
            QgsDebugMsg( QString( "Error message is %1" ).arg( PQresStatus( PQresultStatus( result ) ) ) );

            if ( PQresultStatus( result ) == PGRES_COMMAND_OK )
            {
              PQclear( result );
              // add the geometry column
              //<db_name>, <table_name>, <column_name>, <srid>, <type>, <dimension>
              sql = QString( "select addgeometrycolumn('%1','%2','%3',%4,'%5',%6)" )
                    .arg( bb->schema() )
                    .arg( bb->bufferLayerName() )
                    .arg( bb->geometryColumn() )
                    .arg( bb->srid() )
                    .arg( "POLYGON" )
                    .arg( "2" );
              QgsDebugMsg( "SQL: " + sql );

              PGresult *geoCol = PQexec( conn, sql.toUtf8() );

              if ( PQresultStatus( geoCol ) == PGRES_TUPLES_OK )
              {
                PQclear( geoCol );
                /* The constraint naming convention has changed in PostGIS
                 * from $2 to enforce_geotype_shape. This change means the
                 * buffer plugin will fail for older version of PostGIS.
                 */
                // drop the check constraint based on geometry type
                sql = QString( "alter table %1.%2 drop constraint \"enforce_geotype_shape\"" )
                      .arg( bb->schema() )
                      .arg( bb->bufferLayerName() );

                QgsDebugMsg( "SQL: " + sql );

                result = PQexec( conn, sql.toUtf8() );
                if ( PQresultStatus( result ) == PGRES_COMMAND_OK )
                {
                  PQclear( result );
                  // check pg version and formulate insert query accordingly
                  result = PQexec( conn, "select version()" );
                  QString versionString = PQgetvalue( result, 0, 0 );
                  QStringList versionParts = versionString.split( " ", QString::SkipEmptyParts );
                  // second element is the version number
                  QString version = versionParts[1];

                  if ( version < "7.4.0" )
                  {
                    // modify the tableName
                    tableName = uri.table();
                  }

                  QgsDebugMsg( "Table name for PG 7.3 is: " + uri.table() );

                  //   if(PQresultStatus(geoCol) == PGRES_COMMAND_OK) {
                  // do the buffer and insert the features
                  if ( objId == "objectid" )
                  {
                    sql = QString( "insert into %1 (%2) select buffer(%3,%4) from %5" )
                          .arg( bb->bufferLayerName() )
                          .arg( bb->geometryColumn() )
                          .arg( geometryCol )
                          .arg( bb->bufferDistance().toDouble() )
                          .arg( tableName );
                  }
                  else
                  {
                    sql = QString( "insert into %1 select %2, buffer(%3,%4) from %5" )
                          .arg( bb->bufferLayerName() )
                          .arg( objIdValue )
                          .arg( geometryCol )
                          .arg( bb->bufferDistance().toDouble() )
                          .arg( tableName );
                    QgsDebugMsg( "SQL: " + sql );
                  }
                  result = PQexec( conn, sql.toUtf8() );
                  PQclear( result );
                  // }

                  QgsDebugMsg( "SQL: " + sql );

                  result = PQexec( conn, "end work" );
                  PQclear( result );
                  result = PQexec( conn, "commit;vacuum" );
                  PQclear( result );
                  PQfinish( conn );
                  // QMessageBox::information(0, "Add to Map?", "Do you want to add the layer to the map?");
                  // add new layer to the map
                  if ( bb->addLayerToMap() )
                  {
                    // create the connection string
                    QString newLayerSource = uri.connectionInfo();
                    QgsDebugMsg( "newLayerSource: " + newLayerSource );

                    // add the schema.table and geometry column
                    /*  newLayerSource += "table=" + bb->schema() + "." + bb->bufferLayerName()
                        + " (" + bb->geometryColumn() + ")"; */

                    QgsDebugMsg( "Adding new layer using " + newLayerSource );

                    // host=localhost dbname=gis_data user=gsherman password= table=public.alaska (the_geom)
                    // Using addVectorLayer requires that be add a table=xxxx to the layer path since
                    // addVectorLayer is generic for all supported layers
                    QgsDebugMsg( "Building dataURI string" );
                    QString dataURI = newLayerSource + "table=" + bb->schema() + "." + bb->bufferLayerName()
                                      + " (" + bb->geometryColumn() + ")\n" +
                                      bb->schema() + "." + bb->bufferLayerName() + " (" + bb->geometryColumn() + ")\n" +
                                      "postgres";

                    QgsDebugMsg( QString( "Passing to addVectorLayer:\n%1" ).arg( dataURI ) );
                    qI->addVectorLayer( newLayerSource + "table=" + bb->schema() + "." + bb->bufferLayerName()
                                        + " (" + bb->geometryColumn() + ")",
                                        bb->schema() + "." + bb->bufferLayerName() + " (" + bb->geometryColumn() + ")",
                                        "postgres" );

                  }
                }
                else
                {
                  QgsDebugMsg( QString( "Status from drop constraint is %1" ).arg( PQresultStatus( result ) ) );
                  QgsDebugMsg( QString( "Error message is %1" ).arg( PQresStatus( PQresultStatus( result ) ) ) );
                }
              }
              else
              {
                QgsDebugMsg( QString( "Status from add geometry column is %1" ).arg( PQresultStatus( geoCol ) ) );
                QgsDebugMsg( QString( "Error message is %1" ).arg( PQresStatus( PQresultStatus( geoCol ) ) ) );

                QMessageBox::critical( 0, tr( "Unable to add geometry column" ),
                                       tr( "Unable to add geometry column to the output table %1-%2" )
                                       .arg( bb->bufferLayerName() ).arg( PQerrorMessage( conn ) ) );
              }
            }
            else
            {
              QMessageBox::critical( 0, tr( "Unable to create table" ),
                                     tr( "Failed to create the output table %1" )
                                     .arg( bb->bufferLayerName() ) );
            }
            QApplication::restoreOverrideCursor();
          }
          delete bb;
        }
        else
        {
          // connection error
          QString err = tr( "Error connecting to the database" );
          QMessageBox::critical( 0, err, PQerrorMessage( conn ) );
        }
      }
      else
      {
        QMessageBox::critical( 0, tr( "No GEOS support" ),
                               tr( "Buffer function requires GEOS support in PostGIS" ) );
      }
    }
    else
    {
      QMessageBox::critical( 0, tr( "Not a PostgreSQL/PostGIS Layer" ),
                             tr( "%1 is not a PostgreSQL/PostGIS layer.\nGeoprocessing functions are only available for PostgreSQL/PostGIS Layers" )
                             .arg( lyr->name() ) );
    }
  }
  else
  {
    QMessageBox::warning( 0, tr( "No Active Layer" ),
                          tr( "You must select a layer in the legend to buffer" ) );
  }
}
/* Functions for determining available features in postGIS */
QString QgsPgGeoprocessing::postgisVersion( PGconn *connection )
{
  PGresult *result = PQexec( connection, "select postgis_version()" );
  postgisVersionInfo = PQgetvalue( result, 0, 0 );

  QgsDebugMsg( "PostGIS version info: " + postgisVersionInfo );

  // assume no capabilities
  geosAvailable = false;
  gistAvailable = false;
  projAvailable = false;
  // parse out the capabilities and store them
  QStringList postgisParts = postgisVersionInfo.split( " ", QString::SkipEmptyParts );
  QStringList geos = postgisParts.filter( "GEOS" );
  if ( geos.size() == 1 )
  {
    geosAvailable = ( geos[0].indexOf( "=1" ) > -1 );
  }
  QStringList gist = postgisParts.filter( "STATS" );
  if ( gist.size() == 1 )
  {
    gistAvailable = ( geos[0].indexOf( "=1" ) > -1 );
  }
  QStringList proj = postgisParts.filter( "PROJ" );
  if ( proj.size() == 1 )
  {
    projAvailable = ( proj[0].indexOf( "=1" ) > -1 );
  }
  return postgisVersionInfo;
}
bool QgsPgGeoprocessing::hasGEOS( PGconn *connection )
{
  // make sure info is up to date for the current connection
  postgisVersion( connection );
  // get geos capability
  return geosAvailable;
}
bool QgsPgGeoprocessing::hasGIST( PGconn *connection )
{
  // make sure info is up to date for the current connection
  postgisVersion( connection );
  // get gist capability
  return gistAvailable;
}
bool QgsPgGeoprocessing::hasPROJ( PGconn *connection )
{
  // make sure info is up to date for the current connection
  postgisVersion( connection );
  // get proj4 capability
  return projAvailable;
}

// Unload the plugin by cleaning up the GUI
void QgsPgGeoprocessing::unload()
{
  // remove the GUI
  qI->removePluginMenu( tr( "&Geoprocessing" ), bufferAction );
  qI->removeToolBarIcon( bufferAction );
  delete bufferAction;
}
/**
 * Required extern functions needed  for every plugin
 * These functions can be called prior to creating an instance
 * of the plugin class
 */
// Class factory to return a new instance of the plugin class
QGISEXTERN QgisPlugin * classFactory( QgisInterface * qI )
{
  return new QgsPgGeoprocessing( qI );
}

// Return the name of the plugin
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
QGISEXTERN void unload( QgisPlugin * p )
{
  delete p;
}
