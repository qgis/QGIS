/***************************************************************************
    qgsgeopackageprojectstorage.cpp
    ---------------------
    begin                : March 2019
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

#include "qgsgeopackageprojectstorage.h"
///@cond PRIVATE

#include <sqlite3.h>
#include <QUrlQuery>
#include <QUrl>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>

#include "qgsmessagelog.h"
#include "qgssqliteutils.h"
#include "qgsreadwritecontext.h"
#include "qgsapplication.h"
#include "qgsogrutils.h"
#include "qgsproject.h"


static bool _parseMetadataDocument( const QJsonDocument &doc, QgsProjectStorage::Metadata &metadata )
{
  if ( !doc.isObject() )
    return false;

  QJsonObject docObj = doc.object();
  metadata.lastModified = QDateTime();
  if ( docObj.contains( "last_modified_time" ) )
  {
    QString lastModifiedTimeStr = docObj["last_modified_time"].toString();
    if ( !lastModifiedTimeStr.isEmpty() )
    {
      QDateTime lastModifiedUtc = QDateTime::fromString( lastModifiedTimeStr, Qt::ISODate );
      lastModifiedUtc.setTimeSpec( Qt::UTC );
      metadata.lastModified = lastModifiedUtc.toLocalTime();
    }
  }
  return true;
}

static bool _projectsTableExists( const QString &database )
{
  QString errCause;
  bool ok { false };
  sqlite3_database_unique_ptr db;
  sqlite3_statement_unique_ptr statement;

  int status = db.open_v2( database, SQLITE_OPEN_READWRITE, nullptr );
  if ( status != SQLITE_OK )
  {
    errCause = QObject::tr( "There was an error opening the database <b>%1</b>: %2" )
               .arg( database,
                     QString::fromUtf8( sqlite3_errmsg( db.get() ) ) );
  }
  else
  {
    statement = db.prepare( QStringLiteral( "SELECT count(*) FROM sqlite_master WHERE type ='table' AND name = 'qgis_projects'" ), status );
    if ( status == SQLITE_OK )
    {
      if ( sqlite3_step( statement.get() ) == SQLITE_ROW )
      {
        ok = QString::fromUtf8( reinterpret_cast< const char * >( sqlite3_column_text( statement.get(), 0 ) ) ) == '1';
      }
      else
      {
        errCause = QObject::tr( "There was an error querying the database <b>%1</b>: %2" )
                   .arg( database,
                         QString::fromUtf8( sqlite3_errmsg( db.get() ) ) );

      }
    }
    else
    {
      errCause = QObject::tr( "There was an error querying the database <b>%1</b>: %2" )
                 .arg( database,
                       QString::fromUtf8( sqlite3_errmsg( db.get() ) ) );

    }
    if ( ! errCause.isEmpty() )
      QgsMessageLog::logMessage( errCause, QStringLiteral( "OGR" ), Qgis::MessageLevel::Info );
  }
  return ok;
}

QStringList QgsGeoPackageProjectStorage::listProjects( const QString &uri )
{
  QStringList lst;
  QString errCause;

  QgsGeoPackageProjectUri projectUri = decodeUri( uri );
  if ( !projectUri.valid || ! _projectsTableExists( projectUri.database ) )
    return lst;

  sqlite3_database_unique_ptr database;
  sqlite3_statement_unique_ptr statement;

  int status = database.open_v2( projectUri.database, SQLITE_OPEN_READWRITE, nullptr );
  if ( status != SQLITE_OK )
  {
    errCause = QObject::tr( "There was an error opening the database <b>%1</b>: %2" )
               .arg( projectUri.database,
                     QString::fromUtf8( sqlite3_errmsg( database.get() ) ) );
  }
  else
  {
    statement = database.prepare( QStringLiteral( "SELECT name FROM qgis_projects" ), status );
    if ( status == SQLITE_OK )
    {
      while ( sqlite3_step( statement.get() ) == SQLITE_ROW )
      {
        lst << QString::fromUtf8( reinterpret_cast< const char * >( sqlite3_column_text( statement.get(), 0 ) ) );
      }
    }
    else
    {
      errCause = QObject::tr( "There was an error querying the database <b>%1</b>: %2" )
                 .arg( projectUri.database,
                       QString::fromUtf8( sqlite3_errmsg( database.get() ) ) );
    }
  }
  if ( ! errCause.isEmpty() )
    QgsMessageLog::logMessage( errCause, QStringLiteral( "OGR" ), Qgis::MessageLevel::Info );
  return lst;
}

bool QgsGeoPackageProjectStorage::readProject( const QString &uri, QIODevice *device, QgsReadWriteContext &context )
{
  QgsGeoPackageProjectUri projectUri = decodeUri( uri );
  if ( !projectUri.valid )
  {
    context.pushMessage( QObject::tr( "Invalid URI for GeoPackage OGR provider: " ) + uri, Qgis::Critical );
    return false;
  }

  QString errCause;
  QString xml;
  bool ok = false;
  sqlite3_database_unique_ptr database;
  sqlite3_statement_unique_ptr statement;

  int status = database.open_v2( projectUri.database, SQLITE_OPEN_READWRITE, nullptr );
  if ( status != SQLITE_OK )
  {
    context.pushMessage( QObject::tr( "Could not connect to the database: " ) + projectUri.database, Qgis::Critical );
    return false;
  }
  else
  {
    statement = database.prepare( QStringLiteral( "SELECT content FROM qgis_projects WHERE name = %1" )
                                  .arg( QgsSqliteUtils::quotedValue( projectUri.projectName ) ), status );
    if ( status == SQLITE_OK )
    {
      if ( sqlite3_step( statement.get() ) == SQLITE_ROW )
      {
        xml = QString::fromUtf8( reinterpret_cast< const char * >( sqlite3_column_text( statement.get(), 0 ) ) );
        QString hexEncodedContent( xml );
        QByteArray binaryContent( QByteArray::fromHex( hexEncodedContent.toUtf8() ) );
        device->write( binaryContent );
        device->seek( 0 );
        ok = true;
      }
      else
      {
        errCause = QObject::tr( "There was an error querying the database <b>%1</b>: %2" )
                   .arg( projectUri.database,
                         QString::fromUtf8( sqlite3_errmsg( database.get() ) ) );

      }
    }
    else
    {
      errCause = QObject::tr( "There was an error querying the database <b>%1</b>: %2" )
                 .arg( projectUri.database,
                       QString::fromUtf8( sqlite3_errmsg( database.get() ) ) );
    }
  }
  // TODO: do not log if table does not exists
  if ( ! errCause.isEmpty() )
    QgsMessageLog::logMessage( errCause, QStringLiteral( "OGR" ), Qgis::MessageLevel::Info );

  return ok;

}

bool QgsGeoPackageProjectStorage::writeProject( const QString &uri, QIODevice *device, QgsReadWriteContext &context )
{
  QgsGeoPackageProjectUri projectUri = decodeUri( uri );

  QString errCause;

  if ( !QFile::exists( projectUri.database ) )
  {
    OGRSFDriverH hGpkgDriver = OGRGetDriverByName( "GPKG" );
    if ( !hGpkgDriver )
    {
      errCause = QObject::tr( "GeoPackage driver not found." );
    }
    else
    {
      gdal::ogr_datasource_unique_ptr hDS( OGR_Dr_CreateDataSource( hGpkgDriver, projectUri.database.toUtf8().constData(), nullptr ) );
      if ( !hDS )
        errCause = QObject::tr( "Creation of database failed (OGR error: %1)" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) );
    }
  }

  if ( errCause.isEmpty() && !_projectsTableExists( projectUri.database ) )
  {
    errCause = _executeSql( uri, QStringLiteral( "CREATE TABLE qgis_projects(name TEXT PRIMARY KEY, metadata BLOB, content BLOB)" ) );
  }

  if ( !errCause.isEmpty() )
  {
    errCause = QObject::tr( "Unable to save project. It's not possible to create the destination table on the database. <b>%1</b>: %2" )
               .arg( projectUri.database,
                     errCause );

    context.pushMessage( errCause, Qgis::Critical );
    return false;
  }

  // read from device and write to the table
  QByteArray content = device->readAll();
  QString metadataExpr = QStringLiteral( "{\"last_modified_time\": \"%1\", \"last_modified_user\": \"%2\" }" ).arg(
                           QDateTime::currentDateTime().toString( Qt::DateFormat::ISODate ),
                           QgsApplication::instance()->userLoginName()
                         );
  QString sql;
  if ( listProjects( uri ).contains( projectUri.projectName ) )
  {
    sql = QStringLiteral( "UPDATE qgis_projects SET metadata = %2, content = %3 WHERE name = %1" );
  }
  else
  {
    sql = QStringLiteral( "INSERT INTO qgis_projects VALUES (%1, %2, %3)" );
  }
  sql = sql.arg( QgsSqliteUtils::quotedIdentifier( projectUri.projectName ),
                 QgsSqliteUtils::quotedValue( metadataExpr ),
                 QgsSqliteUtils::quotedValue( QString::fromLatin1( content.toHex() ) )
               );

  errCause = _executeSql( projectUri.database, sql );
  if ( !errCause.isEmpty() )
  {
    const QString errCause = QObject::tr( "Unable to insert or update project (project=%1) in the destination table on the database: %2" )
                             .arg( uri,
                                   errCause );

    context.pushMessage( errCause, Qgis::Critical );
    return false;
  }
  return true;
}

QString QgsGeoPackageProjectStorage::encodeUri( const QgsGeoPackageProjectUri &gpkgUri )
{
  QUrl u;
  QUrlQuery urlQuery;
  u.setScheme( QStringLiteral( "geopackage" ) );

  // Check for windows network shares: github issue #31310
  QString database { gpkgUri.database };
  if ( database.startsWith( QStringLiteral( "//" ) ) )
  {
    u.setPath( database.replace( '/', '\\' ) );
  }
  else
  {
    u.setPath( database );
  }

  if ( !gpkgUri.projectName.isEmpty() )
    urlQuery.addQueryItem( QStringLiteral( "projectName" ), gpkgUri.projectName );
  u.setQuery( urlQuery );
  return QString::fromUtf8( u.toEncoded() );
}


QgsGeoPackageProjectUri QgsGeoPackageProjectStorage::decodeUri( const QString &uri )
{
  QUrl url = QUrl::fromEncoded( uri.toUtf8() );
  QUrlQuery urlQuery( url.query() );

  QgsGeoPackageProjectUri gpkgUri;

  // Check for windows network shares: github issue #31310
  const QString path { url.toString().startsWith( QStringLiteral( "//" ) ) ? url.toString() : url.path() };
  gpkgUri.valid = url.isValid() && QFile::exists( path );
  gpkgUri.database = path;

  gpkgUri.projectName = urlQuery.queryItemValue( "projectName" );
  return gpkgUri;
}

QString QgsGeoPackageProjectStorage::filePath( const QString &uri )
{
  const QgsGeoPackageProjectUri gpkgUri { decodeUri( uri ) };
  return gpkgUri.valid ? gpkgUri.database :  QString();
}


QString QgsGeoPackageProjectStorage::_executeSql( const QString &uri, const QString &sql )
{

  QgsGeoPackageProjectUri projectUri = decodeUri( uri );
  if ( !projectUri.valid )
  {
    return QObject::tr( "Invalid URI for GeoPackage OGR provider: %1" ).arg( uri );
  }

  sqlite3_database_unique_ptr db;
  sqlite3_statement_unique_ptr statement;

  int status = db.open_v2( projectUri.database, SQLITE_OPEN_READWRITE, nullptr );
  if ( status != SQLITE_OK )
  {
    return QObject::tr( "Could not connect to the database: %1" ).arg( projectUri.database );
  }

  QString errCause;
  char *errmsg = nullptr;
  ( void )sqlite3_exec(
    db.get(),                            /* An open database */
    sql.toLocal8Bit(),                   /* SQL to be evaluated */
    nullptr,                             /* Callback function */
    nullptr,                             /* 1st argument to callback */
    &errmsg                              /* Error msg written here */
  );
  if ( status != SQLITE_OK || errmsg )
  {
    errCause = QString::fromUtf8( errmsg );
  }
  return errCause;
}

bool QgsGeoPackageProjectStorage::removeProject( const QString &uri )
{
  QgsGeoPackageProjectUri projectUri = decodeUri( uri );
  QString errCause = _executeSql( uri, QStringLiteral( "DELETE FROM qgis_projects WHERE name = %1" ).arg( QgsSqliteUtils::quotedValue( projectUri.projectName ) ) );
  if ( ! errCause.isEmpty() )
  {
    errCause = QObject::tr( "Could not remove project %1: %2" ).arg( uri, errCause );
    QgsMessageLog::logMessage( errCause, QStringLiteral( "OGR" ), Qgis::MessageLevel::Warning );
  }
  else if ( QgsProject::instance()->fileName() == uri )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Current project was removed from storage, marking it dirty." ), QStringLiteral( "OGR" ), Qgis::MessageLevel::Warning );
    QgsProject::instance()->setDirty( true );
  }
  return errCause.isEmpty();
}

bool QgsGeoPackageProjectStorage::renameProject( const QString &uri, const QString &uriNew )
{
  QgsGeoPackageProjectUri projectNewUri = decodeUri( uriNew );
  QgsGeoPackageProjectUri projectUri = decodeUri( uri );
  QString errCause = _executeSql( uri, QStringLiteral( "UPDATE qgis_projects SET name = %1 WHERE name = %1" )
                                  .arg( QgsSqliteUtils::quotedValue( projectUri.projectName ) )
                                  .arg( QgsSqliteUtils::quotedValue( projectNewUri.projectName ) ) );
  if ( ! errCause.isEmpty() )
  {
    errCause = QObject::tr( "Could not rename project %1: %2" ).arg( uri, errCause );
    QgsMessageLog::logMessage( errCause, QStringLiteral( "OGR" ), Qgis::MessageLevel::Warning );
  }
  return  errCause.isEmpty();
}

bool QgsGeoPackageProjectStorage::readProjectStorageMetadata( const QString &uri, QgsProjectStorage::Metadata &metadata )
{
  QgsGeoPackageProjectUri projectUri = decodeUri( uri );
  if ( !projectUri.valid )
    return false;

  bool ok = false;

  sqlite3_database_unique_ptr database;
  sqlite3_statement_unique_ptr statement;

  int status = database.open_v2( projectUri.database, SQLITE_OPEN_READWRITE, nullptr );
  if ( status != SQLITE_OK )
  {
    return false;
  }
  else
  {
    statement = database.prepare( QStringLiteral( "SELECT metadata FROM qgis_projects WHERE name = %1" )
                                  .arg( QgsSqliteUtils::quotedValue( projectUri.projectName ) ), status );

    if ( status == SQLITE_OK )
    {
      if ( sqlite3_step( statement.get() ) == SQLITE_ROW )
      {
        QString metadataStr = QString::fromUtf8( reinterpret_cast< const char * >( sqlite3_column_text( statement.get(), 0 ) ) );
        metadata.name = projectUri.projectName;
        QJsonDocument doc( QJsonDocument::fromJson( metadataStr.toUtf8() ) );
        ok = _parseMetadataDocument( doc, metadata );
      }
    }
  }

  return ok;
}

///@endcond
