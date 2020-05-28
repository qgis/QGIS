/***************************************************************************
    qgsogrdbconnection.cpp  QgsOgrDbConnection handles connection storage
                            for OGR/GDAL file-based DBs (i.e. GeoPackage)
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
#include "qgsogrdbconnection.h"
///@cond PRIVATE

#include "qgis.h"
#include "qgsdatasourceuri.h"
#include "qgssettings.h"

#include "qgslogger.h"
#include <QInputDialog>
#include <QMessageBox>



QgsOgrDbConnection::QgsOgrDbConnection( const QString &connName, const QString &settingsKey )
  : mConnName( connName )
{
  mSettingsKey = settingsKey;
  QgsSettings settings;
  QString key = QStringLiteral( "%1/%2/path" ).arg( connectionsPath( settingsKey ), mConnName );
  mPath = settings.value( key ).toString();
}

QgsDataSourceUri QgsOgrDbConnection::uri()
{
  QgsDataSourceUri uri;
  uri.setEncodedUri( mPath );
  return uri;
}

void QgsOgrDbConnection::setPath( const QString &path )
{
  mPath = path;
}

void QgsOgrDbConnection::save( )
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "%1/%2/path" ).arg( connectionsPath( mSettingsKey ), mConnName ), mPath );
}

bool QgsOgrDbConnection::allowProjectsInDatabase()
{
  return mSettingsKey == QStringLiteral( "GPKG" );
}

QString QgsOgrDbConnection::fullKey( const QString &settingsKey )
{
  return QStringLiteral( "providers/ogr/%1" ).arg( settingsKey );
}

QString QgsOgrDbConnection::connectionsPath( const QString &settingsKey )
{
  return QStringLiteral( "%1/connections" ).arg( fullKey( settingsKey ) );
}

const QStringList QgsOgrDbConnection::connectionList( const QString &driverName )
{
  QgsSettings settings;
  settings.beginGroup( connectionsPath( driverName ) );
  return settings.childGroups();
}

QString QgsOgrDbConnection::selectedConnection( const QString &settingsKey )
{
  QgsSettings settings;
  return settings.value( QStringLiteral( "%1/selected" ).arg( connectionsPath( settingsKey ) ) ).toString();
}

void QgsOgrDbConnection::setSelectedConnection( const QString &connName, const QString &settingsKey )
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "%1/selected" ).arg( connectionsPath( settingsKey ) ), connName );
}

void QgsOgrDbConnection::deleteConnection( const QString &connName, const QString &settingsKey )
{
  QgsSettings settings;
  settings.remove( QStringLiteral( "%1/%2" ).arg( connectionsPath( settingsKey ), connName ) );
}

///@endcond
