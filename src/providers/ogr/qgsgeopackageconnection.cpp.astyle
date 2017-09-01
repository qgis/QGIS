/***************************************************************************
    qgsgeopackageconnection.cpp  -  selector for geopackage
                             -------------------
    begin                : August 2017
    copyright            : (C) 2017 by Alessandro Pasotti
    email                : apasotti at boundlessgeo dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgis.h"
#include "qgsdatasourceuri.h"
#include "qgssettings.h"
#include "qgsgeopackageconnection.h"
#include "qgslogger.h"
#include <QInputDialog>
#include <QMessageBox>

const QString QgsGeoPackageConnection::SETTINGS_PREFIX = QStringLiteral( "providers/geopackage" );


QgsGeoPackageConnection::QgsGeoPackageConnection( const QString &connName )
  : mConnName( connName )
{
  QgsSettings settings;

  QString key = QStringLiteral( "%1/%2/path" ).arg( connectionsPath( ), mConnName );
  mPath = settings.value( key ).toString();
}

QgsGeoPackageConnection::~QgsGeoPackageConnection()
{

}

QgsDataSourceUri QgsGeoPackageConnection::uri()
{
  QgsDataSourceUri uri;
  uri.setEncodedUri( mPath );
  return uri;
}

void QgsGeoPackageConnection::setPath( const QString &path )
{
  mPath = path;
}

void QgsGeoPackageConnection::save( )
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "%1/%2/path" ).arg( connectionsPath( ), mConnName ), mPath );
}

QString QgsGeoPackageConnection::connectionsPath()
{
  return QStringLiteral( "%1/connections" ).arg( SETTINGS_PREFIX );
}

QStringList QgsGeoPackageConnection::connectionList()
{
  QgsSettings settings;
  settings.beginGroup( connectionsPath( ) );
  return settings.childGroups();
}

QString QgsGeoPackageConnection::selectedConnection()
{
  QgsSettings settings;
  return settings.value( QStringLiteral( "%1/selected" ).arg( SETTINGS_PREFIX ) ).toString();
}

void QgsGeoPackageConnection::setSelectedConnection( const QString &name )
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "%1/selected" ).arg( SETTINGS_PREFIX ), name );
}

void QgsGeoPackageConnection::deleteConnection( const QString &name )
{
  QgsSettings settings;
  settings.remove( QStringLiteral( "%1/%2" ).arg( connectionsPath(), name ) );
}
