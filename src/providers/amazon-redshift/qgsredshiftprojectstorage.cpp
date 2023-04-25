/***************************************************************************
   qgsredshiftprojectstorage.cpp
   --------------------------------------
   Date      : 16.02.2021
   Copyright : (C) 2021 Amazon Inc. or its affiliates
   Author    : Marcel Bezdrighin
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#include "qgsredshiftprojectstorage.h"

#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QUrlQuery>

#include "qgsreadwritecontext.h"
#include "qgsredshiftconnpool.h"

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

static bool _projectsTableExists( QgsRedshiftConn &conn, const QString &schemaName )
{
  QString tableName( "qgis_projects" );
  QString sql( QStringLiteral( "SELECT COUNT(*) FROM information_schema.tables "
                               "WHERE table_name=%1 and table_schema=%2" )
               .arg( QgsRedshiftConn::quotedValue( tableName ), QgsRedshiftConn::quotedValue( schemaName ) ) );
  QgsRedshiftResult res( conn.PQexec( sql ) );
  return res.PQgetvalue( 0, 0 ).toInt() > 0;
}

bool QgsRedshiftProjectStorage::getProjectUriAndConn( const QString &uri, QgsRedshiftProjectUri &projectUri, QgsRedshiftConn *&conn, QString &error_msg )
{
  projectUri = decodeUri( uri );
  if ( !projectUri.valid )
  {
    error_msg = QObject::tr( "Invalid URI for Amazon Redshift provider: " ) + uri;
    return false;
  }

  conn =
    QgsRedshiftConnPool::instance()->acquireConnection( projectUri.connInfo.connectionInfo( false ) );
  if ( !conn )
  {
    error_msg =  QObject::tr( "Could not connect to the database: " ) +
                 projectUri.connInfo.connectionInfo( false );
    return false;
  }

  return true;
}

QStringList QgsRedshiftProjectStorage::listProjects( const QString &uri )
{
  QStringList lst;

  QgsRedshiftProjectUri projectUri;
  QgsRedshiftConn *conn;
  QString error_msg;

  if ( !getProjectUriAndConn( uri, projectUri, conn, error_msg ) )
    return lst;

  if ( _projectsTableExists( *conn, projectUri.schemaName ) )
  {
    QString sql( QStringLiteral( "SELECT name FROM %1.qgis_projects" )
                 .arg( QgsRedshiftConn::quotedIdentifier( projectUri.schemaName ) ) );
    QgsRedshiftResult result( conn->PQexec( sql ) );
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

  QgsRedshiftConnPool::instance()->releaseConnection( conn );

  return lst;
}

bool QgsRedshiftProjectStorage::readProject( const QString &uri, QIODevice *device, QgsReadWriteContext &context )
{
  QgsRedshiftProjectUri projectUri;
  QgsRedshiftConn *conn;
  QString error_msg;
  if ( !getProjectUriAndConn( uri, projectUri, conn, error_msg ) )
  {
    context.pushMessage( error_msg, Qgis::Critical );
    return false;
  }

  if ( !_projectsTableExists( *conn, projectUri.schemaName ) )
  {
    context.pushMessage( QObject::tr( "Table qgis_projects does not exist or it is not accessible." ), Qgis::Critical );
    QgsRedshiftConnPool::instance()->releaseConnection( conn );
    return false;
  }

  bool ok = false;
  QString sql( QStringLiteral( "SELECT content FROM %1.qgis_projects WHERE name = %2" )
               .arg( QgsRedshiftConn::quotedIdentifier( projectUri.schemaName ),
                     QgsRedshiftConn::quotedValue( projectUri.projectName ) ) );
  QgsRedshiftResult result( conn->PQexec( sql ) );
  if ( result.PQresultStatus() == PGRES_TUPLES_OK )
  {
    if ( result.PQntuples() == 1 )
    {
      QString hexEncodedContent( result.PQgetvalue( 0, 0 ) );
      QByteArray binaryContent( QByteArray::fromHex( hexEncodedContent.toUtf8() ) );
      device->write( binaryContent );
      device->seek( 0 );
      ok = true;
    }
    else
    {
      context.pushMessage( QObject::tr( "The project '%1' does not exist in schema '%2'." )
                           .arg( projectUri.projectName, projectUri.schemaName ),
                           Qgis::Critical );
    }
  }

  QgsRedshiftConnPool::instance()->releaseConnection( conn );

  return ok;
}

bool QgsRedshiftProjectStorage::writeProject( const QString &uri, QIODevice *device, QgsReadWriteContext &context )
{
  QgsRedshiftProjectUri projectUri;
  QgsRedshiftConn *conn;
  QString error_msg;
  if ( !getProjectUriAndConn( uri, projectUri, conn, error_msg ) )
  {
    context.pushMessage( error_msg, Qgis::Critical );
    return false;
  }

  if ( !_projectsTableExists( *conn, projectUri.schemaName ) )
  {
    // try to create projects table
    QString sql = QStringLiteral( "CREATE TABLE %1.qgis_projects(name TEXT PRIMARY KEY, "
                                  "metadata text, content VARCHAR(max))" )
                  .arg( QgsRedshiftConn::quotedIdentifier( projectUri.schemaName ) );
    QgsRedshiftResult res( conn->PQexec( sql ) );
    if ( res.PQresultStatus() != PGRES_COMMAND_OK )
    {
      QString errCause = QObject::tr( "Unable to save project. It's not possible to create the "
                                      "destination table on the database. Maybe this is due to "
                                      "database permissions (user=%1). Please contact your "
                                      "database admin." )
                         .arg( projectUri.connInfo.username() );
      context.pushMessage( errCause, Qgis::Critical );
      QgsRedshiftConnPool::instance()->releaseConnection( conn );
      return false;
    }
  }

  try
  {
    conn->begin();
    // delete project if already exists
    QString sql = QString( "DELETE FROM %1.qgis_projects WHERE name=%2" )
                  .arg( QgsRedshiftConn::quotedIdentifier( projectUri.schemaName ),
                        QgsRedshiftConn::quotedValue( projectUri.projectName ) );

    QgsRedshiftResult res( conn->PQexec( sql ) );
    if ( res.PQresultStatus() != PGRES_COMMAND_OK )
      throw res.PQresultErrorMessage();

    // read from device and write to the table
    QByteArray content = device->readAll();

    QString metadataExpr = QStringLiteral( "(%1 || (sysdate at time zone 'utc')::text || %2 || "
                                           "current_user || %3)" )
                           .arg( QgsRedshiftConn::quotedValue( "{ \"last_modified_time\": \"" ),
                                 QgsRedshiftConn::quotedValue( "\", \"last_modified_user\": \"" ),
                                 QgsRedshiftConn::quotedValue( "\" }" ) );

    sql = "INSERT INTO %1.qgis_projects VALUES (%2, %3, %4)";
    sql = sql.arg( QgsRedshiftConn::quotedIdentifier( projectUri.schemaName ),
                   QgsRedshiftConn::quotedValue( projectUri.projectName ),
                   metadataExpr, // no need to quote: already quoted,
                   QgsRedshiftConn::quotedValue( QString::fromLatin1( content.toHex() ) ) );

    res = conn->PQexec( sql );
    if ( res.PQresultStatus() != PGRES_COMMAND_OK )
      throw res.PQresultErrorMessage();

    conn->commit();
  }
  catch ( QString &error )
  {
    QString errCause = QObject::tr( "Unable to insert or update project (project=%1) in the "
                                    "destination table on the database. Error from database: %2\n" )
                       .arg( projectUri.projectName, error );
    context.pushMessage( errCause, Qgis::Critical );
    conn->rollback();
    QgsRedshiftConnPool::instance()->releaseConnection( conn );

    return false;
  }

  QgsRedshiftConnPool::instance()->releaseConnection( conn );
  return true;
}

bool QgsRedshiftProjectStorage::removeProject( const QString &uri )
{
  QgsRedshiftProjectUri projectUri;
  QgsRedshiftConn *conn;
  QString error_msg;
  if ( !getProjectUriAndConn( uri, projectUri, conn, error_msg ) )
  {
    return false;
  }

  bool removed = false;
  if ( _projectsTableExists( *conn, projectUri.schemaName ) )
  {
    QString sql( QStringLiteral( "DELETE FROM %1.qgis_projects WHERE name = %2" )
                 .arg( QgsRedshiftConn::quotedIdentifier( projectUri.schemaName ),
                       QgsRedshiftConn::quotedValue( projectUri.projectName ) ) );
    QgsRedshiftResult res( conn->PQexec( sql ) );
    removed = res.PQresultStatus() == PGRES_COMMAND_OK;
  }

  QgsRedshiftConnPool::instance()->releaseConnection( conn );

  return removed;
}

bool QgsRedshiftProjectStorage::readProjectStorageMetadata( const QString &uri, QgsProjectStorage::Metadata &metadata )
{
  QgsRedshiftProjectUri projectUri;
  QgsRedshiftConn *conn;
  QString error_msg;
  if ( !getProjectUriAndConn( uri, projectUri, conn, error_msg ) )
  {
    return false;
  }

  bool ok = false;
  QString sql( QStringLiteral( "SELECT metadata FROM %1.qgis_projects WHERE name = %2" )
               .arg( QgsRedshiftConn::quotedIdentifier( projectUri.schemaName ),
                     QgsRedshiftConn::quotedValue( projectUri.projectName ) ) );
  QgsRedshiftResult result( conn->PQexec( sql ) );
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

  QgsRedshiftConnPool::instance()->releaseConnection( conn );

  return ok;
}

QString QgsRedshiftProjectStorage::encodeUri( const QgsRedshiftProjectUri &postUri )
{
  QUrl u;
  QUrlQuery urlQuery;

  u.setScheme( "redshift" );
  u.setHost( postUri.connInfo.host().simplified() );
  if ( !postUri.connInfo.port().isEmpty() )
    u.setPort( postUri.connInfo.port().toInt() );
  u.setUserName( postUri.connInfo.username() );
  u.setPassword( postUri.connInfo.password() );

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

QgsRedshiftProjectUri QgsRedshiftProjectStorage::decodeUri( const QString &uri )
{
  QUrl u = QUrl::fromEncoded( uri.toUtf8() );
  QUrlQuery urlQuery( u.query() );

  QgsRedshiftProjectUri postUri;
  postUri.valid = u.isValid();

  QString host = u.host();
  QString port = u.port() != -1 ? QString::number( u.port() ) : QString();
  QString username = u.userName();
  QString password = u.password();
  QgsDataSourceUri::SslMode sslMode = QgsDataSourceUri::decodeSslMode( urlQuery.queryItemValue( "sslmode" ) );
  QString authConfigId = urlQuery.queryItemValue( "authcfg" );
  QString dbName = urlQuery.queryItemValue( "dbname" );

  postUri.connInfo.setConnection( host, port, dbName, username, password, sslMode, authConfigId );

  postUri.schemaName = urlQuery.queryItemValue( "schema" );
  postUri.projectName = urlQuery.queryItemValue( "project" );
  return postUri;
}
