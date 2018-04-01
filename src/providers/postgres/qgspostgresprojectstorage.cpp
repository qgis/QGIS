#include "qgspostgresprojectstorage.h"

#include "qgspostgresconn.h"
#include "qgspostgresconnpool.h"

#include "qgsreadwritecontext.h"

#include <QIODevice>
#include <QUrl>

static bool _projectsTableExists( QgsPostgresConn &conn, const QString &schemaName )
{
  QString tableName( "qgis_projects" );
  QString sql( QStringLiteral( "SELECT COUNT(*) FROM information_schema.tables WHERE table_name=%1 and table_schema=%2" )
               .arg( QgsPostgresConn::quotedValue( tableName ), QgsPostgresConn::quotedValue( schemaName ) )
             );
  QgsPostgresResult res( conn.PQexec( sql ) );
  return res.PQgetvalue( 0, 0 ).toInt() > 0;
}


QgsPostgresProjectStorage::QgsPostgresProjectStorage()
{
}


QStringList QgsPostgresProjectStorage::listProjects( const QString &uri )
{
  QStringList lst;

  QgsPostgresProjectUri projectUri = parseUri( uri );
  if ( !projectUri.valid )
    return lst;

  QgsPostgresConn *conn = QgsPostgresConnPool::instance()->acquireConnection( projectUri.connInfo.connectionInfo( false ) );

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
  QgsPostgresProjectUri projectUri = parseUri( uri );
  if ( !projectUri.valid )
  {
    context.pushMessage( "Invalid URI for PostgreSQL provider: " + uri, Qgis::Critical );
    return false;
  }

  QgsPostgresConn *conn = QgsPostgresConnPool::instance()->acquireConnection( projectUri.connInfo.connectionInfo( false ) );

  if ( !_projectsTableExists( *conn, projectUri.schemaName ) )
  {
    // TODO: write to context
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
  }

  QgsPostgresConnPool::instance()->releaseConnection( conn );

  return ok;
}


bool QgsPostgresProjectStorage::writeProject( const QString &uri, QIODevice *device, QgsReadWriteContext &context )
{
  QgsPostgresProjectUri projectUri = parseUri( uri );
  if ( !projectUri.valid )
  {
    context.pushMessage( "Invalid URI for PostgreSQL provider: " + uri, Qgis::Critical );
    return false;
  }

  QgsPostgresConn *conn = QgsPostgresConnPool::instance()->acquireConnection( projectUri.connInfo.connectionInfo( false ) );

  if ( !_projectsTableExists( *conn, projectUri.schemaName ) )
  {
    // try to create projects table
    QString sql = QStringLiteral( "CREATE TABLE %1.qgis_projects(name TEXT PRIMARY KEY, metadata JSONB, content BYTEA)" ).arg( projectUri.schemaName );
    QgsPostgresResult res( conn->PQexec( sql ) );
    if ( res.PQresultStatus() != PGRES_COMMAND_OK )
    {
      QString errCause = QObject::tr( "Unable to save project. It's not possible to create the destination table on the database. Maybe this is due to table permissions (user=%1). Please contact your database admin" ).arg( projectUri.connInfo.username() );
      context.pushMessage( errCause, Qgis::Critical );
      QgsPostgresConnPool::instance()->releaseConnection( conn );
      return false;
    }
  }

  // read from device and write to the table
  QByteArray content = device->readAll();

  QString metadata = "{ \"last_modified\": 123 }";  // TODO: real metadata

  // TODO: would be useful to have QByteArray version of PQexec() to avoid bytearray -> string -> bytearray conversion
  QString sql( "INSERT INTO %1.qgis_projects VALUES (%2, %3, E'\\\\x" );
  sql = sql.arg( QgsPostgresConn::quotedIdentifier( projectUri.schemaName ),
                 QgsPostgresConn::quotedValue( projectUri.projectName ),
                 QgsPostgresConn::quotedValue( metadata ) );
  sql += QString::fromAscii( content.toHex() );
  sql += "') ON CONFLICT (name) DO UPDATE SET content = EXCLUDED.content, metadata = EXCLUDED.metadata;";

  QgsPostgresResult res( conn->PQexec( sql ) );
  bool ok = res.PQresultStatus() == PGRES_COMMAND_OK;

  QgsPostgresConnPool::instance()->releaseConnection( conn );

  return ok;
}


bool QgsPostgresProjectStorage::removeProject( const QString &uri )
{
  QgsPostgresProjectUri projectUri = parseUri( uri );
  if ( !projectUri.valid )
    return false;

  QgsPostgresConn *conn = QgsPostgresConnPool::instance()->acquireConnection( projectUri.connInfo.connectionInfo( false ) );

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


QgsPostgresProjectUri QgsPostgresProjectStorage::parseUri( const QString &uri )
{
  QUrl u = QUrl::fromEncoded( uri.toUtf8() );
  QUrlQuery urlQuery( u.query() );

  QgsPostgresProjectUri postUri;
  postUri.valid = u.isValid();

  QString host = u.host();
  QString port = u.port() != -1 ? QString::number( u.port() ) : QString();
  QString username = u.userName();
  QString password = u.password();
  QgsDataSourceUri::SslMode sslMode = QgsDataSourceUri::SslPrefer;
  QString authConfigId;  // TODO: ?
  QString dbName = urlQuery.queryItemValue( "dbname" );
  postUri.connInfo.setConnection( host, port, dbName, username, password, sslMode, authConfigId );
  // TODO: "service"

  postUri.schemaName = urlQuery.queryItemValue( "schema" );
  postUri.projectName = urlQuery.queryItemValue( "project" );
  return postUri;
}
