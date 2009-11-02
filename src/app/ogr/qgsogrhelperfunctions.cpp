/***************************************************************************
                          qgsogrhelperfunctions.cpp
    helper functions to create ogr uris for database and protocol drivers
                             -------------------
    begin                : Mon Jan 2 2009
    copyright            : (C) 2009 by Godofredo Contreras Nava
    email                : frdcn at hotmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id:$ */

#include "qgsogrhelperfunctions.h"
#include "qgslogger.h"
#include <QRegExp>

QString createDatabaseURI( QString connectionType, QString host, QString database, QString port, QString user, QString password )
{
  QString uri = "";
  //todo:add default ports for all kind of databases
  if ( connectionType == "ESRI Personal GeoDatabase" )
  {
    uri = "PGeo:" + database;
  }
  else if ( connectionType == "ESRI ArcSDE" )
  {
    if ( port.isNull() || port.isEmpty() )
      port = "5151";
    uri = "SDE:" + host + ",PORT:" + port + "," + database + "," + user + "," + password;
  }
  else if ( connectionType == "Informix DataBlade" )
  {
    //not tested
    uri = "IDB:dbname=" + database + " server=" + host
          + " user=" + user
          + " pass=" + password + " ";

  }
  else if ( connectionType == "INGRES" )
  {
    //not tested
    uri = "@driver=ingres,dbname=" + database + ",userid=" + user + ", password=" + password + " ";
  }
  else if ( connectionType == "MySQL" )
  {
    uri = "MySQL:" + database + ",host=" + host
          + ",port=" + port + ",user=" + user
          + ", password=" + password + " ";
  }
  else if ( connectionType == "Oracle Spatial" )
  {
    uri = "OCI:" + user + "/" + password
          + "@" + host;
          //MH 091102: connection to orcale does not seem to work with database name in uri
          //+ "/" + database;
  }
  else if ( connectionType == "ODBC" )
  {
    if ( !user.isEmpty() )
    {
      if ( password.isEmpty() )
      {
        uri = "ODBC:" + user + "@" + database;
      }
      else
      {
        uri = "ODBC:" + user + "/" + password + "@" + database;
      }

    }
    else
    {
      uri = "ODBC:" + database;
    }
  }
  else if ( connectionType == "OGDI Vectors" )
  {
  }
  else if ( connectionType == "PostgreSQL" )
  {
    uri = "PG:dbname='" + database + "' host='" + host
          + "' port='" + port + "' user='" + user
          + "' password='" + password + "' ";

  }
  QgsDebugMsg( "Connection type is=" + connectionType + " and uri=" + uri );
  return uri;
}


QString createProtocolURI( QString type, QString url )
{
  QString uri = "";
  if ( type == "GeoJSON" )
  {
    uri = url;
  }
  QgsDebugMsg( "Connection type is=" + type + " and uri=" + uri );
  return uri;
}
