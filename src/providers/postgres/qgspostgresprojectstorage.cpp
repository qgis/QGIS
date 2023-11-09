/***************************************************************************
    qgspostgresprojectstorage.cpp
    ---------------------
    begin                : April 2018
    copyright            : (C) 2018 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgspostgresprojectstorage.h"

#include "qgspostgresconn.h"
#include "qgspostgresconnpool.h"

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


static bool _projectsTableExists( QgsPostgresConn &conn, const QString &schemaName )
{
  QString tableName( "qgis_projects" );
  QString sql( QStringLiteral( "SELECT COUNT(*) FROM information_schema.tables WHERE table_name=%1 and table_schema=%2" )
               .arg( QgsPostgresConn::quotedValue( tableName ), QgsPostgresConn::quotedValue( schemaName ) )
             );
  QgsPostgresResult res( conn.PQexec( sql ) );

  if ( ! res.result() )
  {
    return false;
  }

  return res.PQgetvalue( 0, 0 ).toInt() > 0;
}


QStringList QgsPostgresProjectStorage::listProjects( const QString &uri )
{
  QStringList lst;

  QgsPostgresProjectUri projectUri = decodeUri( uri );
  if ( !projectUri.valid )
    return lst;

  QgsPostgresConn *conn = QgsPostgresConnPool::instance()->acquireConnection( projectUri.connInfo.connectionInfo( false ) );
  if ( !conn )
    return lst;

  if ( _projectsTableExists( *conn, projectUri.schemaName ) )
  {
    QString sql( QStringLiteral( "SELECT name FROM %1.qgis_projects" ).arg( QgsPostgresConn::quotedIdentifier( projectUri.schemaName ) ) );
    QgsPostgresResult result( conn->PQexec( sql ) );
    if ( result.PQresultStatus() == PGRES_TUPLES_OK )
    {
      int count = result.PQntuples();
      for ( int i = 0; i < count; ++i )
      {
        QString name = result.PQgetvalue( i, 0 );
        lst << name;
      }
    }
  }

  QgsPostgresConnPool::instance()->releaseConnection( conn );

  return lst;
}


bool QgsPostgresProjectStorage::readProject( const QString &uri, QIODevice *device, QgsReadWriteContext &context )
{
  QgsPostgresProjectUri projectUri = decodeUri( uri );
  if ( !projectUri.valid )
  {
    context.pushMessage( QObject::tr( "Invalid URI for PostgreSQL provider: " ) + uri, Qgis::MessageLevel::Critical );
    return false;
  }

  QgsPostgresConn *conn = QgsPostgresConnPool::instance()->acquireConnection( projectUri.connInfo.connectionInfo( false ) );
  if ( !conn )
  {
    context.pushMessage( QObject::tr( "Could not connect to the database: " ) + projectUri.connInfo.connectionInfo( false ), Qgis::MessageLevel::Critical );
    return false;
  }

  if ( !_projectsTableExists( *conn, projectUri.schemaName ) )
  {
    context.pushMessage( QObject::tr( "Table qgis_projects does not exist or it is not accessible." ), Qgis::MessageLevel::Critical );
    QgsPostgresConnPool::instance()->releaseConnection( conn );
    return false;
  }

  bool ok = false;
  QString sql( QStringLiteral( "SELECT content FROM %1.qgis_projects WHERE name = %2" ).arg( QgsPostgresConn::quotedIdentifier( projectUri.schemaName ), QgsPostgresConn::quotedValue( projectUri.projectName ) ) );
  QgsPostgresResult result( conn->PQexec( sql ) );
  if ( result.PQresultStatus() == PGRES_TUPLES_OK )
  {
    if ( result.PQntuples() == 1 )
    {
      // TODO: would be useful to have QByteArray version of PQgetvalue to avoid bytearray -> string -> bytearray conversion
      QString hexEncodedContent( result.PQgetvalue( 0, 0 ) );
      QByteArray binaryContent( QByteArray::fromHex( hexEncodedContent.toUtf8() ) );
      device->write( binaryContent );
      device->seek( 0 );
      ok = true;
    }
    else
    {
      context.pushMessage( QObject::tr( "The project '%1' does not exist in schema '%2'." ).arg( projectUri.projectName, projectUri.schemaName ), Qgis::MessageLevel::Critical );
    }
  }

  QgsPostgresConnPool::instance()->releaseConnection( conn );

  return ok;
}


bool QgsPostgresProjectStorage::writeProject( const QString &uri, QIODevice *device, QgsReadWriteContext &context )
{
  QgsPostgresProjectUri projectUri = decodeUri( uri );
  if ( !projectUri.valid )
  {
    context.pushMessage( QObject::tr( "Invalid URI for PostgreSQL provider: " ) + uri, Qgis::MessageLevel::Critical );
    return false;
  }

  QgsPostgresConn *conn = QgsPostgresConnPool::instance()->acquireConnection( projectUri.connInfo.connectionInfo( false ) );
  if ( !conn )
  {
    context.pushMessage( QObject::tr( "Could not connect to the database: " ) + projectUri.connInfo.connectionInfo( false ), Qgis::MessageLevel::Critical );
    return false;
  }

  if ( !_projectsTableExists( *conn, projectUri.schemaName ) )
  {
    // try to create projects table
    QString sql = QStringLiteral( "CREATE TABLE %1.qgis_projects(name TEXT PRIMARY KEY, metadata JSONB, content BYTEA)" ).arg( QgsPostgresConn::quotedIdentifier( projectUri.schemaName ) );
    QgsPostgresResult res( conn->PQexec( sql ) );
    if ( res.PQresultStatus() != PGRES_COMMAND_OK )
    {
      QString errCause = QObject::tr( "Unable to save project. It's not possible to create the destination table on the database. Maybe this is due to database permissions (user=%1). Please contact your database admin." ).arg( projectUri.connInfo.username() );
      context.pushMessage( errCause, Qgis::MessageLevel::Critical );
      QgsPostgresConnPool::instance()->releaseConnection( conn );
      return false;
    }
  }

  // read from device and write to the table
  QByteArray content = device->readAll();

  QString metadataExpr = QStringLiteral( "(%1 || (now() at time zone 'utc')::text || %2 || current_user || %3)::jsonb" ).arg(
                           QgsPostgresConn::quotedValue( "{ \"last_modified_time\": \"" ),
                           QgsPostgresConn::quotedValue( "\", \"last_modified_user\": \"" ),
                           QgsPostgresConn::quotedValue( "\" }" )
                         );

  // TODO: would be useful to have QByteArray version of PQexec() to avoid bytearray -> string -> bytearray conversion
  QString sql( "INSERT INTO %1.qgis_projects VALUES (%2, %3, E'\\\\x" );
  sql = sql.arg( QgsPostgresConn::quotedIdentifier( projectUri.schemaName ),
                 QgsPostgresConn::quotedValue( projectUri.projectName ),
                 metadataExpr  // no need to quote: already quoted
               );
  sql += QString::fromLatin1( content.toHex() );
  sql += "') ON CONFLICT (name) DO UPDATE SET content = EXCLUDED.content, metadata = EXCLUDED.metadata;";

  QgsPostgresResult res( conn->PQexec( sql ) );
  if ( res.PQresultStatus() != PGRES_COMMAND_OK )
  {
    QString errCause = QObject::tr( "Unable to insert or update project (project=%1) in the destination table on the database. Maybe this is due to table permissions (user=%2). Please contact your database admin." ).arg( projectUri.projectName, projectUri.connInfo.username() );
    context.pushMessage( errCause, Qgis::MessageLevel::Critical );
    QgsPostgresConnPool::instance()->releaseConnection( conn );
    return false;
  }

  QgsPostgresConnPool::instance()->releaseConnection( conn );
  return true;
}


bool QgsPostgresProjectStorage::removeProject( const QString &uri )
{
  QgsPostgresProjectUri projectUri = decodeUri( uri );
  if ( !projectUri.valid )
    return false;

  QgsPostgresConn *conn = QgsPostgresConnPool::instance()->acquireConnection( projectUri.connInfo.connectionInfo( false ) );
  if ( !conn )
    return false;

  bool removed = false;
  if ( _projectsTableExists( *conn, projectUri.schemaName ) )
  {
    QString sql( QStringLiteral( "DELETE FROM %1.qgis_projects WHERE name = %2" ).arg( QgsPostgresConn::quotedIdentifier( projectUri.schemaName ), QgsPostgresConn::quotedValue( projectUri.projectName ) ) );
    QgsPostgresResult res( conn->PQexec( sql ) );
    removed = res.PQresultStatus() == PGRES_COMMAND_OK;
  }

  QgsPostgresConnPool::instance()->releaseConnection( conn );

  return removed;
}


bool QgsPostgresProjectStorage::readProjectStorageMetadata( const QString &uri, QgsProjectStorage::Metadata &metadata )
{
  QgsPostgresProjectUri projectUri = decodeUri( uri );
  if ( !projectUri.valid )
    return false;

  QgsPostgresConn *conn = QgsPostgresConnPool::instance()->acquireConnection( projectUri.connInfo.connectionInfo( false ) );
  if ( !conn )
    return false;

  bool ok = false;
  QString sql( QStringLiteral( "SELECT metadata FROM %1.qgis_projects WHERE name = %2" ).arg( QgsPostgresConn::quotedIdentifier( projectUri.schemaName ), QgsPostgresConn::quotedValue( projectUri.projectName ) ) );
  QgsPostgresResult result( conn->PQexec( sql ) );
  if ( result.PQresultStatus() == PGRES_TUPLES_OK )
  {
    if ( result.PQntuples() == 1 )
    {
      metadata.name = projectUri.projectName;
      QString metadataStr = result.PQgetvalue( 0, 0 );
      QJsonDocument doc( QJsonDocument::fromJson( metadataStr.toUtf8() ) );
      ok = _parseMetadataDocument( doc, metadata );
    }
  }

  QgsPostgresConnPool::instance()->releaseConnection( conn );

  return ok;
}


QString QgsPostgresProjectStorage::encodeUri( const QgsPostgresProjectUri &postUri )
{
  QUrl u;
  QUrlQuery urlQuery;

  u.setScheme( "postgresql" );
  u.setHost( postUri.connInfo.host() );
  if ( !postUri.connInfo.port().isEmpty() )
    u.setPort( postUri.connInfo.port().toInt() );
  u.setUserName( postUri.connInfo.username() );
  u.setPassword( postUri.connInfo.password() );

  if ( !postUri.connInfo.service().isEmpty() )
    urlQuery.addQueryItem( "service", postUri.connInfo.service() );
  if ( !postUri.connInfo.authConfigId().isEmpty() )
    urlQuery.addQueryItem( "authcfg", postUri.connInfo.authConfigId() );
  if ( postUri.connInfo.sslMode() != QgsDataSourceUri::SslPrefer )
    urlQuery.addQueryItem( "sslmode", QgsDataSourceUri::encodeSslMode( postUri.connInfo.sslMode() ) );

  urlQuery.addQueryItem( "dbname", postUri.connInfo.database() );

  urlQuery.addQueryItem( "schema", postUri.schemaName );
  if ( !postUri.projectName.isEmpty() )
    urlQuery.addQueryItem( "project", postUri.projectName );

  u.setQuery( urlQuery );

  return QString::fromUtf8( u.toEncoded() );
}


QgsPostgresProjectUri QgsPostgresProjectStorage::decodeUri( const QString &uri )
{
  QUrl u = QUrl::fromEncoded( uri.toUtf8() );
  QUrlQuery urlQuery( u.query() );

  QgsPostgresProjectUri postUri;
  postUri.valid = u.isValid();

  QString host = u.host();
  QString port = u.port() != -1 ? QString::number( u.port() ) : QString();
  QString username = u.userName();
  QString password = u.password();
  QgsDataSourceUri::SslMode sslMode = QgsDataSourceUri::decodeSslMode( urlQuery.queryItemValue( "sslmode" ) );
  QString authConfigId = urlQuery.queryItemValue( "authcfg" );
  QString dbName = urlQuery.queryItemValue( "dbname" );
  QString service = urlQuery.queryItemValue( "service" );
  if ( !service.isEmpty() )
    postUri.connInfo.setConnection( service, dbName, username, password, sslMode, authConfigId );
  else
    postUri.connInfo.setConnection( host, port, dbName, username, password, sslMode, authConfigId );

  postUri.schemaName = urlQuery.queryItemValue( "schema" );
  postUri.projectName = urlQuery.queryItemValue( "project" );
  return postUri;
}
