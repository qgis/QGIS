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
#include "moc_qgsogrdbconnection.cpp"
const QgsSettingsEntryString *QgsOgrDbConnection::settingsOgrConnectionPath = new QgsSettingsEntryString( QStringLiteral( "providers/ogr/%1/connections/%2/path" ), QString(), QString() );

const QgsSettingsEntryString *QgsOgrDbConnection::settingsOgrConnectionSelected = new QgsSettingsEntryString( QStringLiteral( "providers/ogr/%1/connections/selected" ), QString() );

///@cond PRIVATE

#include "qgis.h"
#include "qgsdatasourceuri.h"
#include "qgsprovidermetadata.h"
#include "qgsproviderregistry.h"

#include "qgssettings.h"

QgsOgrDbConnection::QgsOgrDbConnection( const QString &connName, const QString &settingsKey )
  : mConnName( connName )
{
  mSettingsKey = settingsKey;
  mPath = settingsOgrConnectionPath->value( {settingsKey, mConnName} );
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
  settingsOgrConnectionPath->setValue( mPath, {mSettingsKey, mConnName} );
}

bool QgsOgrDbConnection::allowProjectsInDatabase()
{
  return mSettingsKey == QLatin1String( "GPKG" );
}

const QStringList QgsOgrDbConnection::connectionList( const QString &driverName )
{
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "providers/ogr/%1/connections" ).arg( driverName ) );
  return settings.childGroups();
}

QString QgsOgrDbConnection::selectedConnection( const QString &driverName )
{
  return settingsOgrConnectionSelected->value( driverName );
}

void QgsOgrDbConnection::setSelectedConnection( const QString &connName, const QString &driverName )
{
  settingsOgrConnectionSelected->setValue( connName, {driverName} );
}

void QgsOgrDbConnection::deleteConnection( const QString &connName )
{
  QgsProviderMetadata *providerMetadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ogr" ) );
  providerMetadata->deleteConnection( connName );
}

///@endcond
