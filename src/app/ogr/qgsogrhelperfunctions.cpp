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

#include "qgsogrhelperfunctions.h"
#include "qgslogger.h"
#include <QRegExp>

QString createDatabaseURI( const QString &connectionType, const QString &host, const QString &database, QString port, const QString &user, const QString &password )
{
  QString uri = QLatin1String( "" );

  //todo:add default ports for all kind of databases
  if ( connectionType == QLatin1String( "ESRI Personal GeoDatabase" ) )
  {
    uri = "PGeo:" + database;
  }
  else if ( connectionType == QLatin1String( "ESRI ArcSDE" ) )
  {
    if ( port.isEmpty() )
      port = QStringLiteral( "5151" );

    uri = "SDE:" + host + ",PORT:" + port + ',' + database + ',' + user + ',' + password;
  }
  else if ( connectionType == QLatin1String( "Informix DataBlade" ) )
  {
    //not tested
    uri = "IDB:dbname=" + database;

    if ( !host.isEmpty() )
      uri += QStringLiteral( " server=%1" ).arg( host );

    if ( !user.isEmpty() )
    {
      uri += QStringLiteral( " user=%1" ).arg( user );

      if ( !password.isEmpty() )
        uri += QStringLiteral( " pass=%1" ).arg( password );
    }
  }
  else if ( connectionType == QLatin1String( "Ingres" ) )
  {
    //not tested
    uri = "@driver=ingres,dbname=" + database;
    if ( !user.isEmpty() )
    {
      uri += QStringLiteral( ",userid=%1" ).arg( user );

      if ( !password.isEmpty() )
        uri += QStringLiteral( ",password=%1" ).arg( password );
    }
  }
  else if ( connectionType == QLatin1String( "MySQL" ) )
  {
    uri = "MySQL:" + database;

    if ( !host.isEmpty() )
    {
      uri += QStringLiteral( ",host=%1" ).arg( host );

      if ( !port.isEmpty() )
        uri += QStringLiteral( ",port=%1" ).arg( port );
    }

    if ( !user.isEmpty() )
    {
      uri += QStringLiteral( ",user=%1" ).arg( user );

      if ( !password.isEmpty() )
        uri += QStringLiteral( ",password=%1" ).arg( password );
    }
  }
  else if ( connectionType == QLatin1String( "MSSQL" ) )
  {
    uri = QStringLiteral( "MSSQL:" );

    if ( !host.isEmpty() )
    {
      uri += QStringLiteral( ";server=%1" ).arg( host );

      if ( !port.isEmpty() )
        uri += QStringLiteral( ",%1" ).arg( port );
    }

    if ( !user.isEmpty() )
    {
      uri += QStringLiteral( ";uid=%1" ).arg( user );

      if ( !password.isEmpty() )
        uri += QStringLiteral( ";pwd=%1" ).arg( password );
    }
    else
      uri += QLatin1String( ";trusted_connection=yes" );

    if ( !database.isEmpty() )
      uri += QStringLiteral( ";database=%1" ).arg( database );
  }
  else if ( connectionType == QLatin1String( "Oracle Spatial" ) )
  {
    uri = "OCI:" + user;

    if ( ( !user.isEmpty() && !password.isEmpty() ) ||
         ( user.isEmpty() && password.isEmpty() ) )
    {
      uri += '/';
      if ( !password.isEmpty() )
        uri += password;
    }

    if ( !host.isEmpty() || !database.isEmpty() )
    {
      uri += '@';

      if ( !host.isEmpty() )
      {
        uri += host;
        if ( !port.isEmpty() )
          uri += ':' + port;
      }

      if ( !database.isEmpty() )
      {
        if ( !host.isEmpty() )
          uri += '/';
        uri += database;
      }
    }
  }
  else if ( connectionType == QLatin1String( "ODBC" ) )
  {
    if ( !user.isEmpty() )
    {
      if ( password.isEmpty() )
      {
        uri = "ODBC:" + user + '@' + database;
      }
      else
      {
        uri = "ODBC:" + user + '/' + password + '@' + database;
      }

    }
    else
    {
      uri = "ODBC:" + database;
    }
  }
  else if ( connectionType == QLatin1String( "OGDI Vectors" ) )
  {
  }
  else if ( connectionType == QLatin1String( "PostgreSQL" ) )
  {
    uri = "PG:dbname='" + database + '\'';

    if ( !host.isEmpty() )
    {
      uri += QStringLiteral( " host='%1'" ).arg( host );

      if ( !port.isEmpty() )
        uri += QStringLiteral( " port='%1'" ).arg( port );
    }

    if ( !user.isEmpty() )
    {
      uri += QStringLiteral( " user='%1'" ).arg( user );

      if ( !password.isEmpty() )
        uri += QStringLiteral( " password='%1'" ).arg( password );
    }

    uri += ' ';
  }

  QgsDebugMsg( "Connection type is=" + connectionType + " and uri=" + uri );
  return uri;
}


QString createProtocolURI( const QString &type, const QString &url )
{
  QString uri = QLatin1String( "" );
  if ( type == QLatin1String( "GeoJSON" ) )
  {
    uri = url;
  }
  else if ( type == QLatin1String( "CouchDB" ) )
  {
    uri = QStringLiteral( "couchdb:%1" ).arg( url );
  }
  else if ( type == QLatin1String( "DODS/OPeNDAP" ) )
  {
    uri = QStringLiteral( "DODS:%1" ).arg( url );
  }
  QgsDebugMsg( "Connection type is=" + type + " and uri=" + uri );
  return uri;
}
