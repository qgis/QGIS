/***************************************************************************
  QgsGeoPackageProviderConnection.cpp - QgsGeoPackageProviderConnection

 ---------------------
 begin                : 6.8.2019
 copyright            : (C) 2019 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsgeopackageproviderconnection.h"
#include "qgsogrdbconnection.h"
#include "qgssettings.h"
#include "qgsogrprovider.h"
#include "qgsmessagelog.h"
#include "qgsproviderregistry.h"

#include <sqlite3.h>

QgsGeoPackageProviderConnection::QgsGeoPackageProviderConnection( const QString &name ):
  QgsAbstractDatabaseProviderConnection( name )
{
  setDefaultCapabilities();
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "ogr" ), QgsSettings::Section::Providers );
  settings.beginGroup( QStringLiteral( "GPKG" ) );
  settings.beginGroup( QStringLiteral( "connections" ) );
  settings.beginGroup( name );
  setUri( settings.value( QStringLiteral( "path" ) ).toString() );
}

QgsGeoPackageProviderConnection::QgsGeoPackageProviderConnection( const QString &name, const QString &uri ):
  QgsAbstractDatabaseProviderConnection( name )
{
  setDefaultCapabilities();
  setUri( uri );
}

void QgsGeoPackageProviderConnection::store( QVariantMap guiConfig )
{
  Q_UNUSED( guiConfig );
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "ogr" ), QgsSettings::Section::Providers );
  settings.beginGroup( QStringLiteral( "GPKG" ) );
  settings.beginGroup( QStringLiteral( "connections" ) );
  settings.beginGroup( name() );
  settings.setValue( QStringLiteral( "path" ), uri() );
}

void QgsGeoPackageProviderConnection::remove()
{
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "ogr" ), QgsSettings::Section::Providers );
  settings.beginGroup( QStringLiteral( "GPKG" ) );
  settings.beginGroup( QStringLiteral( "connections" ) );
  settings.remove( name() );
}


void QgsGeoPackageProviderConnection::createVectorTable( const QString &schema, const QString &name, const QgsFields &fields, QgsWkbTypes::Type wkbType, const QgsCoordinateReferenceSystem &srs, bool overwrite, const QMap<QString, QVariant> *options )
{
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Schema is not supported by GPKG, ignoring" ), QStringLiteral( "OGR" ), Qgis::Info );
  }
  if ( capabilities().testFlag( Capability::CreateVectorTable ) )
  {
    auto opts { *options };
    opts[ QStringLiteral( "layerName" ) ] = QVariant( name );
    QMap<int, int> map;
    QString errCause;
    QgsVectorLayerExporter::ExportError errCode = QgsOgrProvider::createEmptyLayer(
          uri(),
          fields,
          wkbType,
          srs,
          overwrite,
          &map,
          &errCause,
          &opts
        );
    if ( errCode != QgsVectorLayerExporter::ExportError::NoError )
    {
      throw new QgsProviderConnectionException( QObject::tr( "An error occourred while creating the vector layer: %1" ).arg( errCause ) );
    }
  }
}


void QgsGeoPackageProviderConnection::dropTable( const QString &schema, const QString &name )
{
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Schema is not supported by GPKG, ignoring" ), QStringLiteral( "OGR" ), Qgis::Info );
  }
  if ( ! capabilities().testFlag( Capability::DropTable ) )
  {
    throw new QgsProviderConnectionException( QObject::tr( "Method is not supported for this connection" ) );
  }
  QString errCause;
  // TODO: this won't work for rasters
  QgsOgrProviderUtils::deleteLayer( uri(), errCause );
  if ( ! errCause.isEmpty() )
  {
    throw new QgsProviderConnectionException( QObject::tr( "Error deleting table %1: %2" ).arg( name ).arg( errCause ) );
  }
}

void QgsGeoPackageProviderConnection::renameTable( const QString &schema, const QString &name, const QString &newName )
{
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Schema is not supported by GPKG, ignoring" ), QStringLiteral( "OGR" ), Qgis::Info );
  }
  // TODO: won't work for RASTER layers
  // TODO: maybe an index?
  QString sql( QStringLiteral( "ALTER TABLE %1 RENAME TO %2" )
               .arg( QgsSqliteUtils::quotedIdentifier( name ),
                     QgsSqliteUtils::quotedIdentifier( newName ) ) );
  executeGdalSqlPrivate( sql );
  sql = QStringLiteral( "UPDATE layer_styles SET f_table_name = %2 WHERE f_table_name = %1" )
        .arg( QgsSqliteUtils::quotedString( name ),
              QgsSqliteUtils::quotedString( newName ) );
  try
  {
    executeGdalSqlPrivate( sql );
  }
  catch ( QgsProviderConnectionException &ex )
  {
    QgsDebugMsgLevel( QStringLiteral( "Warning: error while updating the styles, perhaps there are no styles in this GPKG: %1" ).arg( ex.what() ), 4 );
  }
}

void QgsGeoPackageProviderConnection::executeSql( const QString &sql )
{
  // TODO: check if this is the right approach
  executeSqliteSqlPrivate( sql );
}

void QgsGeoPackageProviderConnection::vacuum( const QString &schema, const QString &name )
{
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Schema is not supported by GPKG, ignoring" ), QStringLiteral( "OGR" ), Qgis::Info );
  }
}

static int collect_strings( void *names, int, char **argv, char ** )
{
  *static_cast<QList<QString>*>( names ) << QString::fromUtf8( argv[ 0 ] );
  return 0;
}

QStringList QgsGeoPackageProviderConnection::tables( const QString &schema )
{
  if ( ! schema.isEmpty() )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Schema is not supported by GPKG, ignoring" ), QStringLiteral( "OGR" ), Qgis::Info );
  }
  QStringList names;
  QString errCause;
  QVariantMap pieces( QgsProviderRegistry::instance()->decodeUri( QLatin1String( "ogr" ), uri() ) );
  QString baseUri = pieces[QStringLiteral( "path" )].toString();
  if ( !baseUri.isEmpty() )
  {
    char *errmsg = nullptr;
    sqlite3_database_unique_ptr database;
    int status = database.open_v2( baseUri, SQLITE_OPEN_READONLY, nullptr );
    if ( status == SQLITE_OK )
    {
      char *sql = sqlite3_mprintf( "SELECT table_name FROM gpkg_contents;" );
      status = sqlite3_exec(
                 database.get(),              /* An open database */
                 sql,                         /* SQL to be evaluated */
                 collect_strings,             /* Callback function */
                 &names,                      /* 1st argument to callback */
                 &errmsg                      /* Error msg written here */
               );
      sqlite3_free( sql );
      if ( status != SQLITE_OK )
      {
        errCause = QStringLiteral( "There was an error reading tables from GPKG layer %1: %2" ).arg( uri(), QString::fromUtf8( errmsg ) );
      }
      sqlite3_free( errmsg );
    }
    else
    {
      errCause = QStringLiteral( "There was an error opening GPKG %1" ).arg( uri() );
    }
  }
  if ( ! errCause.isEmpty() )
  {
    throw new QgsProviderConnectionException( QObject::tr( "Error listing tables from %1: %2" ).arg( name() ).arg( errCause ) );
  }
  return  names;
}

void QgsGeoPackageProviderConnection::setDefaultCapabilities()
{
  mCapabilities =
  {
    Capability::Tables,
    Capability::CreateVectorTable,
    Capability::RenameTable,
    Capability::DropTable,
    Capability::Vacuum,
  };
}

void QgsGeoPackageProviderConnection::executeGdalSqlPrivate( const QString &sql )
{
  QString errCause;
  GDALDatasetH hDS = GDALOpenEx( uri().toUtf8().constData(), GDAL_OF_VECTOR | GDAL_OF_UPDATE, nullptr, nullptr, nullptr );
  if ( hDS )
  {
    OGRLayerH ogrLayer( GDALDatasetExecuteSQL( hDS, sql.toUtf8().constData(), nullptr, nullptr ) );
    if ( ogrLayer )
      GDALDatasetReleaseResultSet( hDS, ogrLayer );
    errCause = CPLGetLastErrorMsg( );
    GDALClose( hDS );
  }
  else
  {
    errCause = QObject::tr( "There was an error opening GPKG %1!" ).arg( uri() );
  }
  if ( ! errCause.isEmpty() )
  {
    throw new QgsProviderConnectionException( QObject::tr( "Error executing SQL %1: %2" ).arg( sql ).arg( errCause ) );
  }
}

void QgsGeoPackageProviderConnection::executeSqliteSqlPrivate( const QString &sql )
{

}
