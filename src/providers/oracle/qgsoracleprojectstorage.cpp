/***************************************************************************
    qgsoracleprojectstorage.cpp
    ---------------------
    begin                : March 2022
    copyright            : (C) 2022 by Julien Cabieces
    email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsoracleprojectstorage.h"

#include "qgsoracleconn.h"
#include "qgsoracleconnpool.h"

#include "qgsreadwritecontext.h"

#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QUrlQuery>
#include <QSqlError>

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


static bool _projectsTableExists( QgsOracleConn *conn, const QString &owner )
{
  QSqlQuery qry( *conn );
  qry.prepare( QStringLiteral( "SELECT 1 FROM all_tables WHERE owner=? AND table_name=?" ) );
  qry.addBindValue( owner );
  qry.addBindValue( "qgis_projects" );

  if ( !qry.exec() )
  {
    QgsDebugMsg( QStringLiteral( "SQL: %1\nERROR: %2" )
                 .arg( qry.lastQuery(), qry.lastError().text() ) );
    return false;
  }

  return qry.next();
}


QStringList QgsOracleProjectStorage::listProjects( const QString &uri )
{
  QStringList lst;

  QgsOracleProjectUri projectUri = decodeUri( uri );
  if ( !projectUri.valid )
    return lst;

  QgsPoolOracleConn pconn( projectUri.connInfo.connectionInfo( false ) );
  if ( !pconn.get() )
    return lst;

  if ( _projectsTableExists( pconn.get(), projectUri.owner ) )
  {
    QString sql( QStringLiteral( "SELECT name FROM %1.\"qgis_projects\"" ).arg( QgsOracleConn::quotedIdentifier( projectUri.owner ) ) );
    QSqlQuery qry( *pconn.get() );
    if ( !qry.exec( sql ) )
    {
      QgsDebugMsg( QStringLiteral( "SQL: %1\nERROR: %2" )
                   .arg( qry.lastQuery(), qry.lastError().text() ) );
      return lst;
    }

    while ( qry.next() )
      lst << qry.value( 0 ).toString();
  }

  return lst;
}


bool QgsOracleProjectStorage::readProject( const QString &uri, QIODevice *device, QgsReadWriteContext &context )
{
  QgsOracleProjectUri projectUri = decodeUri( uri );
  if ( !projectUri.valid )
  {
    context.pushMessage( QObject::tr( "Invalid URI for Oracle provider: " ) + uri, Qgis::MessageLevel::Critical );
    return false;
  }

  QgsPoolOracleConn pconn( projectUri.connInfo.connectionInfo( false ) );
  if ( !pconn.get() )
  {
    context.pushMessage( QObject::tr( "Could not connect to the database: " ) + projectUri.connInfo.connectionInfo( false ), Qgis::MessageLevel::Critical );
    return false;
  }

  if ( !_projectsTableExists( pconn.get(), projectUri.owner ) )
  {
    context.pushMessage( QObject::tr( "Table qgis_projects does not exist or it is not accessible." ), Qgis::MessageLevel::Critical );
    return false;
  }

  QString sql( QStringLiteral( "SELECT content FROM %1.\"qgis_projects\" WHERE name = ?" ).arg( QgsOracleConn::quotedIdentifier( projectUri.owner ) ) );
  QSqlQuery qry( *pconn.get() );
  qry.prepare( sql );
  qry.addBindValue( projectUri.projectName );

  if ( !qry.exec() )
  {
    QgsDebugMsg( QStringLiteral( "SQL: %1\nERROR: %2" )
                 .arg( qry.lastQuery(), qry.lastError().text() ) );
    return false;
  }

  if ( qry.next() )
  {
    const QByteArray binaryContent( qry.value( 0 ).toByteArray() );
    device->write( binaryContent );
    device->seek( 0 );
    return true;
  }
  else
  {
    context.pushMessage( QObject::tr( "The project '%1' does not exist for owner '%2'." ).arg( projectUri.projectName, projectUri.owner ), Qgis::MessageLevel::Critical );
    return false;
  }
}


bool QgsOracleProjectStorage::writeProject( const QString &uri, QIODevice *device, QgsReadWriteContext &context )
{
  const QgsOracleProjectUri projectUri = decodeUri( uri );
  if ( !projectUri.valid )
  {
    context.pushMessage( QObject::tr( "Invalid URI for Oracle provider: " ) + uri, Qgis::MessageLevel::Critical );
    return false;
  }

  QgsPoolOracleConn pconn( projectUri.connInfo.connectionInfo( false ) );
  if ( !pconn.get() )
  {
    context.pushMessage( QObject::tr( "Could not connect to the database: " ) + projectUri.connInfo.connectionInfo( false ), Qgis::MessageLevel::Critical );
    return false;
  }

  if ( !_projectsTableExists( pconn.get(), projectUri.owner ) )
  {
    // try to create projects table

    // TODO for Oracle version starting at 21c, we could use JSON native support type : https://blogs.oracle.com/database/post/json-datatype-support-in-oracle-21c
    // For now, the official supported version is 18.4 (the one we use for tests)
    const QString sql = QStringLiteral( "CREATE TABLE %1.\"qgis_projects\"(name VARCHAR2(2047) PRIMARY KEY, metadata CLOB, content BLOB)" ).arg( QgsOracleConn::quotedIdentifier( projectUri.owner ) );
    QSqlQuery qry( *pconn.get() );
    if ( !qry.exec( sql ) )
    {
      QgsDebugMsg( QStringLiteral( "SQL: %1\nERROR: %2" )
                   .arg( qry.lastQuery(), qry.lastError().text() ) );
      return false;
    }
  }

  // read from device and write to the table
  QByteArray content = device->readAll();

  QString metadataExpr = QStringLiteral( "%1 || to_char(current_timestamp AT TIME ZONE 'UTC', 'YYYY-MM-DD HH24:MI:SS.FF5') || %2 || sys_context('USERENV', 'CURRENT_USER') || %3" ).arg(
                           QgsOracleConn::quotedValue( "{ \"last_modified_time\": \"" ),
                           QgsOracleConn::quotedValue( "\", \"last_modified_user\": \"" ),
                           QgsOracleConn::quotedValue( "\" }" )
                         );

  // Upsert into projects table
  QSqlQuery qry( *pconn.get() );
  qry.prepare( QStringLiteral( "MERGE INTO %1.\"qgis_projects\" "
                               "USING dual "
                               "ON (name = :projectname) "
                               "WHEN MATCHED THEN UPDATE SET metadata = %2, content = :content "
                               "WHEN NOT MATCHED THEN INSERT VALUES (:projectname, %2, :content)" )
               .arg( QgsOracleConn::quotedIdentifier( projectUri.owner ), metadataExpr ) );
  qry.bindValue( QStringLiteral( ":projectname" ), projectUri.projectName );
  qry.bindValue( QStringLiteral( ":content" ), content );

  if ( !qry.exec() )
  {
    QString errCause = QObject::tr( "Unable to insert or update project (project=%1) in the destination table on the database. Maybe this is due to table permissions (user=%2). Please contact your database admin." ).arg( projectUri.projectName, projectUri.connInfo.username() );
    context.pushMessage( errCause, Qgis::MessageLevel::Critical );
    return false;
  }

  return true;
}


bool QgsOracleProjectStorage::removeProject( const QString &uri )
{
  QgsOracleProjectUri projectUri = decodeUri( uri );
  if ( !projectUri.valid )
    return false;

  QgsPoolOracleConn pconn( projectUri.connInfo.connectionInfo( false ) );
  if ( !pconn.get() )
    return false;

  if ( _projectsTableExists( pconn.get(), projectUri.owner ) )
  {
    QSqlQuery qry( *pconn.get() );
    qry.prepare( QStringLiteral( "DELETE FROM %1.\"qgis_projects\" WHERE name = ?" ).arg( QgsOracleConn::quotedIdentifier( projectUri.owner ) ) );
    qry.addBindValue( projectUri.projectName );
    return qry.exec();
  }

  return false;
}


bool QgsOracleProjectStorage::readProjectStorageMetadata( const QString &uri, QgsProjectStorage::Metadata &metadata )
{
  QgsOracleProjectUri projectUri = decodeUri( uri );
  if ( !projectUri.valid )
    return false;

  QgsPoolOracleConn pconn( projectUri.connInfo.connectionInfo( false ) );
  if ( !pconn.get() )
    return false;

  if ( !_projectsTableExists( pconn.get(), projectUri.owner ) )
    return false;

  QString sql( QStringLiteral( "SELECT metadata FROM %1.\"qgis_projects\" WHERE name = ?" ).arg( QgsOracleConn::quotedIdentifier( projectUri.owner ) ) );
  QSqlQuery qry( *pconn.get() );
  qry.prepare( sql );
  qry.addBindValue( projectUri.projectName );

  if ( !qry.exec() )
    return false;

  if ( qry.next() )
  {
    metadata.name = projectUri.projectName;
    const QString metadataStr = qry.value( 0 ).toString();
    QJsonDocument doc( QJsonDocument::fromJson( metadataStr.toUtf8() ) );
    return _parseMetadataDocument( doc, metadata );
  }
  else
    return false;
}


QString QgsOracleProjectStorage::encodeUri( const QgsOracleProjectUri &postUri )
{
  QUrl u;
  QUrlQuery urlQuery;

  u.setScheme( "oracle" );
  u.setHost( postUri.connInfo.host() );
  if ( !postUri.connInfo.port().isEmpty() )
    u.setPort( postUri.connInfo.port().toInt() );
  u.setUserName( postUri.connInfo.username() );
  u.setPassword( postUri.connInfo.password() );

  if ( !postUri.connInfo.service().isEmpty() )
    urlQuery.addQueryItem( "service", postUri.connInfo.service() );
  if ( !postUri.connInfo.authConfigId().isEmpty() )
    urlQuery.addQueryItem( "authcfg", postUri.connInfo.authConfigId() );

  urlQuery.addQueryItem( "dbname", postUri.connInfo.database() );

  urlQuery.addQueryItem( "schema", postUri.owner );
  if ( !postUri.projectName.isEmpty() )
    urlQuery.addQueryItem( "project", postUri.projectName );

  u.setQuery( urlQuery );

  return QString::fromUtf8( u.toEncoded() );
}


QgsOracleProjectUri QgsOracleProjectStorage::decodeUri( const QString &uri )
{
  QUrl u = QUrl::fromEncoded( uri.toUtf8() );
  QUrlQuery urlQuery( u.query() );

  QgsOracleProjectUri projectUri;
  projectUri.valid = u.isValid();

  QString host = u.host();
  QString port = u.port() != -1 ? QString::number( u.port() ) : QString();
  QString username = u.userName();
  QString password = u.password();
  QString authConfigId = urlQuery.queryItemValue( "authcfg" );
  QString dbName = urlQuery.queryItemValue( "dbname" );
  QString service = urlQuery.queryItemValue( "service" );
  if ( !service.isEmpty() )
    projectUri.connInfo.setConnection( service, dbName, username, password,
                                       QgsDataSourceUri::SslPrefer /* meaningless for oracle */,
                                       authConfigId );
  else
    projectUri.connInfo.setConnection( host, port, dbName, username, password,
                                       QgsDataSourceUri::SslPrefer /* meaningless for oracle */,
                                       authConfigId );

  projectUri.owner = urlQuery.queryItemValue( "schema" );
  projectUri.projectName = urlQuery.queryItemValue( "project" );
  return projectUri;
}
