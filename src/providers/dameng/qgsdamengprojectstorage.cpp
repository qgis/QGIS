/***************************************************************************
    qgsdamengprojectstorage.cpp
    ---------------------
    begin                : 2025/01/14
    copyright            : ( C ) 2025 by Haiyang Zhao
    email                : zhaohaiyang@dameng.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   ( at your option ) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdamengprojectstorage.h"
#include "qgsdamengconn.h"
#include "qgsdamengconnpool.h"
#include "qgsreadwritecontext.h"

#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QUrlQuery>

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


static bool _projectsTableExists( QgsDamengConn &conn, const QString &schemaName )
{
  QString tableName( "QGIS_PROJECTS" );
  QString sql( QStringLiteral( "SELECT COUNT(*) FROM ALL_ALL_TABLES WHERE table_name=%1 and OWNER=%2" )
               .arg( QgsDamengConn::quotedValue( tableName ), QgsDamengConn::quotedValue( schemaName ) )
             );
  QgsDMResult *res = conn.DMexec( sql );
  
  res->fetchFirst();
  return res->value( 0 ).toInt() > 0;
}


QStringList QgsDamengProjectStorage::listProjects( const QString &uri )
{
  QStringList lst;

  QgsDamengProjectUri projectUri = decodeUri( uri );
  if ( !projectUri.valid )
    return lst;

  QgsDamengConn *conn = QgsDamengConnPool::instance()->acquireConnection( projectUri.connInfo.connectionInfo( false ) );
  if ( !conn )
    return lst;

  if ( _projectsTableExists( *conn, projectUri.schemaName ) )
  {
    QString sql( QStringLiteral( "SELECT name FROM %1.QGIS_PROJECTS" ).arg( QgsDamengConn::quotedIdentifier( projectUri.schemaName ) ) );
    QgsDMResult *res = conn->DMexec( sql );

    while ( res->fetchNext() )
    {
      lst << res->value( 0 ).toString();
    }
  }

  QgsDamengConnPool::instance()->releaseConnection( conn );

  return lst;
}


bool QgsDamengProjectStorage::readProject( const QString &uri, QIODevice *device, QgsReadWriteContext &context )
{
  QgsDamengProjectUri projectUri = decodeUri( uri );
  if ( !projectUri.valid )
  {
    context.pushMessage( QObject::tr( "Invalid URI for Dameng provider: " ) + uri, Qgis::MessageLevel::Critical );
    return false;
  }

  QgsDamengConn *conn = QgsDamengConnPool::instance()->acquireConnection( projectUri.connInfo.connectionInfo( false ) );
  if ( !conn )
  {
    context.pushMessage( QObject::tr( "Could not connect to the database: " ) + projectUri.connInfo.connectionInfo( false ), Qgis::MessageLevel::Critical );
    return false;
  }

  if ( !_projectsTableExists( *conn, projectUri.schemaName ) )
  {
    context.pushMessage( QObject::tr( "Table QGIS_PROJECTS does not exist or it is not accessible." ), Qgis::MessageLevel::Critical );
    QgsDamengConnPool::instance()->releaseConnection( conn );
    return false;
  }

  bool ok = false;
  QString sql( QStringLiteral( "SELECT content FROM %1.QGIS_PROJECTS WHERE name = %2" ).arg( QgsDamengConn::quotedIdentifier( projectUri.schemaName ), QgsDamengConn::quotedValue( projectUri.projectName ) ) );
  QgsDMResult *res = conn->DMexec( sql );

  if ( res->ntuples() > 0 )
  {
    res->fetchNext();
    QString hexEncodedContent( res->value( 0 ).toString() );
    hexEncodedContent[0] = '\\';
    QByteArray binaryContent( QByteArray::fromHex( hexEncodedContent.toUtf8() ) );
    device->write( binaryContent );
    device->seek( 0 );
    ok = true;
  }
  else
  {
    context.pushMessage( QObject::tr( "The project '%1' does not exist in schema '%2'." ).arg( projectUri.projectName, projectUri.schemaName ), Qgis::MessageLevel::Critical );
  }

  QgsDamengConnPool::instance()->releaseConnection( conn );

  return ok;
}


bool QgsDamengProjectStorage::writeProject( const QString &uri, QIODevice *device, QgsReadWriteContext &context )
{
  QgsDamengProjectUri projectUri = decodeUri( uri );
  if ( !projectUri.valid )
  {
    context.pushMessage( QObject::tr( "Invalid URI for Dameng provider: " ) + uri, Qgis::MessageLevel::Critical );
    return false;
  }

  QgsDamengConn *conn = QgsDamengConnPool::instance()->acquireConnection( projectUri.connInfo.connectionInfo( false ) );
  if ( !conn )
  {
    context.pushMessage( QObject::tr( "Could not connect to the database: " ) + projectUri.connInfo.connectionInfo( false ), Qgis::MessageLevel::Critical );
    return false;
  }

  if ( !_projectsTableExists( *conn, projectUri.schemaName ) )
  {
    // try to create projects table
    QString sql = QStringLiteral( "CREATE TABLE %1.QGIS_PROJECTS( name varchar( 256 ) PRIMARY KEY, metadata varchar check( metadata is json( lax ) ), content blob )" ).arg( QgsDamengConn::quotedIdentifier( projectUri.schemaName ) );
    QgsDamengResult res( conn->DMexec( sql ) );
    if ( res.DMresultStatus() != DmResCommandOk )
    {
      QString errCause = QObject::tr( "Unable to save project. It's not possible to create the destination table on the database. Maybe this is due to database permissions (user=%1). Please contact your database admin." ).arg( projectUri.connInfo.username() );
      context.pushMessage( errCause, Qgis::MessageLevel::Critical );
      QgsDamengConnPool::instance()->releaseConnection( conn );
      return false;
    }
  }

  // read from device and write to the table
  QByteArray content = device->readAll();

  QString metadataExpr = QStringLiteral( "(%1 || ( now() at time zone 'utc') || %2 || current_user || %3 )" ).arg(
                           QgsDamengConn::quotedValue( "{ \"last_modified_time\": \"" ),
                           QgsDamengConn::quotedValue( "\", \"last_modified_user\": \"" ),
                           QgsDamengConn::quotedValue( "\" }" )
                         );

  QString sql(  "begin "
                " delete from %1.QGIS_PROJECTS where name = %2 and "
                      " 1 = ( select count( name ) from %1.QGIS_PROJECTS where name = %2 );"
                " INSERT INTO %1.QGIS_PROJECTS VALUES (%2, %3, '0x%4');"
                "end;" );
  sql = sql.arg( QgsDamengConn::quotedIdentifier( projectUri.schemaName ),
                 QgsDamengConn::quotedValue( projectUri.projectName ),
                 metadataExpr,  // no need to quote: already quoted
                 QString::fromLatin1( content.toHex() )
               );

  QgsDamengResult res( conn->DMexec( sql ) );
  if ( res.DMresultStatus() != DmResCommandOk )
  {
    QString errCause = QObject::tr( "Unable to insert or update project (project=%1) in the destination table on the database. Maybe this is due to table permissions (user=%2). Please contact your database admin." ).arg( projectUri.projectName, projectUri.connInfo.username() );
    context.pushMessage( errCause, Qgis::MessageLevel::Critical );
    QgsDamengConnPool::instance()->releaseConnection( conn );
    return false;
  }

  QgsDamengConnPool::instance()->releaseConnection( conn );
  return true;
}


bool QgsDamengProjectStorage::removeProject( const QString &uri )
{
  QgsDamengProjectUri projectUri = decodeUri( uri );
  if ( !projectUri.valid )
    return false;

  QgsDamengConn *conn = QgsDamengConnPool::instance()->acquireConnection( projectUri.connInfo.connectionInfo( false ) );
  if ( !conn )
    return false;

  bool removed = false;
  if ( _projectsTableExists( *conn, projectUri.schemaName ) )
  {
    QString sql( QStringLiteral( "DELETE FROM %1.QGIS_PROJECTS WHERE name = %2" ).arg( QgsDamengConn::quotedIdentifier( projectUri.schemaName ), QgsDamengConn::quotedValue( projectUri.projectName ) ) );
    QgsDamengResult res( conn->DMexec( sql ) );
    removed = res.DMresultStatus() == DmResCommandOk;
  }

  QgsDamengConnPool::instance()->releaseConnection( conn );

  return removed;
}


bool QgsDamengProjectStorage::readProjectStorageMetadata( const QString &uri, QgsProjectStorage::Metadata &metadata )
{
  QgsDamengProjectUri projectUri = decodeUri( uri );
  if ( !projectUri.valid )
    return false;

  QgsDamengConn *conn = QgsDamengConnPool::instance()->acquireConnection( projectUri.connInfo.connectionInfo( false ) );
  if ( !conn )
    return false;

  bool ok = false;
  QString sql( QStringLiteral( "SELECT metadata FROM %1.QGIS_PROJECTS WHERE name = %2" ).arg( QgsDamengConn::quotedIdentifier( projectUri.schemaName ), QgsDamengConn::quotedValue( projectUri.projectName ) ) );
  QgsDMResult *res = conn->DMexec( sql );

  if ( res->ntuples() > 0 )
  {
    res->fetchNext();

    metadata.name = projectUri.projectName;
    QString metadataStr = res->value( 0 ).toString();
    QJsonDocument doc( QJsonDocument::fromJson( metadataStr.toUtf8() ) );
    ok = _parseMetadataDocument( doc, metadata );
  }

  QgsDamengConnPool::instance()->releaseConnection( conn );

  return ok;
}


QString QgsDamengProjectStorage::encodeUri( const QgsDamengProjectUri &postUri )
{
  QUrl u;
  QUrlQuery urlQuery;

  u.setScheme( "dameng" );
  u.setHost( postUri.connInfo.host() );
  if ( !postUri.connInfo.port().isEmpty() )
    u.setPort( postUri.connInfo.port().toInt() );
  u.setUserName( postUri.connInfo.username() );
  u.setPassword( postUri.connInfo.password() );

  if ( !postUri.connInfo.authConfigId().isEmpty() )
    urlQuery.addQueryItem( "authcfg", postUri.connInfo.authConfigId() );

  urlQuery.addQueryItem( "dbname", postUri.connInfo.database() );

  urlQuery.addQueryItem( "schema", postUri.schemaName );
  if ( !postUri.projectName.isEmpty() )
    urlQuery.addQueryItem( "project", postUri.projectName );

  u.setQuery( urlQuery );

  return QString::fromUtf8( u.toEncoded() );
}


QgsDamengProjectUri QgsDamengProjectStorage::decodeUri( const QString &uri )
{
  QUrl u = QUrl::fromEncoded( uri.toUtf8() );
  QUrlQuery urlQuery( u.query() );

  QgsDamengProjectUri postUri;
  postUri.valid = u.isValid();

  QString host = u.host();
  QString port = u.port() != -1 ? QString::number( u.port() ) : QString();
  QString username = u.userName();
  QString password = u.password();
  QString authConfigId = urlQuery.queryItemValue( "authcfg" );
  QString dbName = urlQuery.queryItemValue( "dbname" );
  postUri.connInfo.setConnection( host, port, dbName, username, password, QgsDataSourceUri::SslPrefer, authConfigId );

  postUri.schemaName = urlQuery.queryItemValue( "schema" );
  postUri.projectName = urlQuery.queryItemValue( "project" );
  return postUri;
}
