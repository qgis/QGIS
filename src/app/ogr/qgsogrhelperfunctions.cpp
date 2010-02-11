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
    if ( port.isEmpty() )
      port = "5151";

    uri = "SDE:" + host + ",PORT:" + port + "," + database + "," + user + "," + password;
  }
  else if ( connectionType == "Informix DataBlade" )
  {
    //not tested
    uri = "IDB:dbname=" + database;

    if ( !host.isEmpty() )
      uri += QString( " server=%1" ).arg( host );

    if ( !user.isEmpty() )
    {
      uri += QString( " user=%1" ).arg( user );

      if ( !password.isEmpty() )
        uri += QString( " pass=%1" ).arg( password );
    }
  }
  else if ( connectionType == "INGRES" )
  {
    //not tested
    uri = "@driver=ingres,dbname=" + database;
    if ( !user.isEmpty() )
    {
      uri += QString( ",userid=%1" ).arg( user );

      if ( !password.isEmpty() )
        uri += QString( ",password=%1" ).arg( password );
    }
  }
  else if ( connectionType == "MySQL" )
  {
    uri = "MySQL:" + database;

    if ( !host.isEmpty() )
    {
      uri += QString( ",host=%1" ).arg( host );

      if ( !port.isEmpty() )
        uri += QString( ",port=%1" ).arg( port );
    }

    if ( !user.isEmpty() )
    {
      uri += QString( ",user=%1" ).arg( user );

      if ( !password.isEmpty() )
        uri += QString( ",password=%1" ).arg( password );
    }
  }
  else if ( connectionType == "Oracle Spatial" )
  {
    uri = "OCI:" + user;

    if (( !user.isEmpty() && !password.isEmpty() ) ||
        ( user.isEmpty() && password.isEmpty() ) )
    {
      uri += "/";
      if ( !password.isEmpty() )
        uri += password;
    }

    if ( !host.isEmpty() || !database.isEmpty() )
    {
      uri += "@";

      if ( !host.isEmpty() )
      {
        uri += host;
        if ( !port.isEmpty() )
          uri += ":" + port;
      }

      if ( !database.isEmpty() )
      {
        if ( !host.isEmpty() )
          uri += "/";
        uri += database;
      }
    }
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
    uri = "PG:dbname='" + database + "'";

    if ( !host.isEmpty() )
    {
      uri += QString( " host='%1'" ).arg( host );

      if ( !port.isEmpty() )
        uri += QString( " port='%1'" ).arg( port );
    }

    if ( !user.isEmpty() )
    {
      uri += QString( " user='%1'" ).arg( user );

      if ( !password.isEmpty() )
        uri += QString( " password='%1'" ).arg( password );
    }

    uri += " ";
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
