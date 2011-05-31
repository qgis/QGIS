/***************************************************************************
    sqlanyconnection.cpp - Class for pooling connections to a SQL Anywhere DBMS
    --------------------------
    begin                : Dec 2010
    copyright            : (C) 2010 by iAnywhere Solutions, Inc.
    author               : David DeHaan
    email                : ddehaan at sybase dot com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qgslogger.h>
#include <qgscredentials.h>
#include <qgsmessageoutput.h>

#include "sqlanystatement.h"
#include "sqlanyconnection.h"

// initialize static variables
bool SqlAnyConnection::sApiInit = false;
unsigned int SqlAnyConnection::connCount = 0;
QMap< QString, SqlAnyConnection * > SqlAnyConnection::connCache;
QMutex SqlAnyConnection::gMutex( QMutex::Recursive );
QgsVectorDataProvider::NativeType SqlAnyConnection::sInvalidType( "Invalid Type", "INVALID", QVariant::Invalid );
const char *SqlAnyConnection::sNotFoundMsg =
  "The SQL Anywhere C API library ("
#ifdef WIN32
  "dbcapi.dll"
#else
  "libdbcapi.so"
#endif
  ") was not found. "
  "Check that client files for SQL Anywhere 12 or higher "
  "have been installed, and that the installation "
  "directory is listed in your PATH.\n\n"
  "To download the free Developer Edition of SQL Anywhere, "
  "visit http://www.sybase.com/detail?id=1016644." ;

// static interface with accessor function
static SQLAnywhereInterface sApi;
SQLAnywhereInterface *
SqlAnyConnection::api() { return &sApi; }


/** Connection constructor
 *  Starts with refcount 1 for caller.
 *  connCache entry doesn't get a refcount, since it is managed internal
 *    to this object.
 */
SqlAnyConnection::SqlAnyConnection( QString mapKey, const QString uri, a_sqlany_connection *newconn, bool readOnly )
    : mMapKey( mapKey )
    , mUri( uri )
    , mReadOnly( readOnly )
    , mRefCount( 1 )
    , mHandle( newconn )
    , mMutex( QMutex::Recursive )
{
  gMutex.lock();
  connCount++;
  connCache.insert( mMapKey, this );
  gMutex.unlock();

  populateNativeMap();
}


/** Connection destructor
 *  Frees connection resources.
 */
SqlAnyConnection::~SqlAnyConnection()
{
  gMutex.lock();

  SaDebugMsg( QString( "Destructing %1 connection to SQL Anywhere database." )
              .arg( mReadOnly ? "read-only" : "read-write" ) );
  sApi.sqlany_disconnect( mHandle );
  sApi.sqlany_free_connection( mHandle );
  connCount--;
  if ( !mMapKey.isEmpty() )
    connCache.remove( mMapKey );

  gMutex.unlock();
}

/**
 * Unloads the C API library from the application
 */
void unload_dbcapi( SQLAnywhereInterface *theApi )
{
  // For some reason, the code below causes crashes on Windows.
  // Therefore, leave the library loaded until the application terminates.

  // sqlany_finalize_interface( theApi );
}

void
SqlAnyConnection::releaseApi()
{
  gMutex.lock();
  if ( sApiInit && connCount == 0 )
  {
    SaDebugMsg( "Freeing the SQL Anywhere interface resources." );
    sApi.sqlany_fini();
    unload_dbcapi( &sApi );
    sApiInit = false;
  }
  gMutex.unlock();
}


bool
SqlAnyConnection::initApi()
{
  sacapi_u32 api_version;

  gMutex.lock();
  if ( ! sApiInit )
  {
    SaDebugMsg( "Initializing the SQL Anywhere interface." );
    sApiInit = sqlany_initialize_interface( &sApi, NULL );
    if ( !sApiInit )
    {
      SaDebugMsg( "Failed to load the SQL Anywhere interface dbcapi.dll." );
    }
    else
    {
      sApiInit = sApi.sqlany_init( "QGIS", SQLANY_API_VERSION_2, &api_version );
      if ( !sApiInit )
      {
        SaDebugMsg( QString( "Failed to initialize the SQL Anywhere Interface. Supported version=%1" ).arg( api_version ) );

        unload_dbcapi( &sApi );
      }
    }
  }
  gMutex.unlock();
  return sApiInit;
}

const char *
SqlAnyConnection::failedInitMsg()
{
  return sNotFoundMsg;
}

/**
 * Encodes connection parameters into a uri format that can be
 *   parsed by the constructor to QgsDataSourceUri and thereby
 *   passed to the constructor of a QgsDataProvider.
 * Due to the differences between SQL Anywhere connection parameters
 *  and the Postgres parameters assumed by QgsDataSourceUri
 *  most of the connection information is encoded within the DBNAME
 *  field.
 */
QString
SqlAnyConnection::makeUri( QString connName, QString host
                           , QString port, QString server, QString database
                           , QString parameters, QString username, QString password
                           , bool simpleEncrypt, bool estimateMeta )
{
  QString uri;

  // begin DBNAME
  uri = "dbname=\"";
  if ( !connName.isEmpty() )
    uri += "CON=" + connName + ";";
  if ( !host.isEmpty() || !port.isEmpty() )
  {
    uri += "HOST=";
    uri += ( host.isEmpty() ? "localhost" : host );
    if ( !port.isEmpty() )
      uri += ":" + port;
    uri += ";";
  }
  if ( !server.isEmpty() )
    uri += "SERVER=" + server + ";";
  if ( !database.isEmpty() )
    uri += "DBN=" + database + ";";
  if ( !parameters.isEmpty() )
    uri += parameters + ";";
  if ( simpleEncrypt )
    uri += "ENC=SIMPLE;";
  uri += "\" ";
  // end DBNAME

  uri += "user=\"" + username + "\" ";
  uri += "password=\"" + password + "\" ";
  if ( estimateMeta )
    uri += "estimatedmetadata=true ";

  return uri;
}

SqlAnyConnection *
SqlAnyConnection::connect( QString connName, QString host
                           , QString port, QString server, QString database
                           , QString parameters, QString username, QString password
                           , bool simpleEncrypt, bool estimateMeta, bool readOnly )
{
  return connect( makeUri( connName, host, port, server, database, parameters, username, password, simpleEncrypt, estimateMeta ), readOnly );
}

SqlAnyConnection *
SqlAnyConnection::connect( QString connName, QString host
                           , QString port, QString server, QString database
                           , QString parameters, QString username, QString password
                           , bool simpleEncrypt, bool estimateMeta, bool readOnly
                           , sacapi_i32 &code, char *errbuf, size_t size )
{
  return connect( makeUri( connName, host, port, server, database, parameters, username, password, simpleEncrypt, estimateMeta ), readOnly, code, errbuf, size );
}

/**
 * Create a new SqlAnyConnection or fetch one from connCache
 */
SqlAnyConnection *
SqlAnyConnection::connect( const QString uri, bool readOnly )
{
  char  errbuf[SACAPI_ERROR_SIZE];
  sacapi_i32  code;
  return connect( uri, readOnly, code, errbuf, sizeof( errbuf ) );
}

/**
 * Create a new SqlAnyConnection or fetch one from connCache
 */
SqlAnyConnection *
SqlAnyConnection::connect( const QString uri, bool readOnly, sacapi_i32 &code, char *errbuf, size_t size )
{
  SqlAnyConnection *retval = NULL;
  const QString mapKey = uri + readOnly;

  // ensure that the api has been loaded
  if ( !initApi() )
  {
    return NULL;
  }

  // grab global lock to protect connCache and connCount
  gMutex.lock();

  // look in connCache for an existing live connection
  if ( connCache.contains( mapKey ) )
  {
    retval = connCache[ mapKey ]->addRef();

    if ( !retval->isAlive() )
    {
      // remove dead connection from the cache
      connCache.remove( mapKey );
      retval->mMapKey.clear();
      retval->release();
      retval = NULL;
    }
  }

  if ( retval == NULL )
  {
    // create connection to the database
    a_sqlany_connection *newconn = sApi.sqlany_new_connection();
    QgsDataSourceURI    theUri( uri );
    QString      username = theUri.username();
    QString      password = theUri.password();
    QString      connStr = makeConnectString( theUri, true );
    QString      credKey = makeConnectString( theUri, false );
    bool ok = true;

    SaDebugMsg( QString( "Establishing %1 connection to SQL Anywhere with string: '%2' " )
                .arg( readOnly ? "read-only" : "read-write" )
                .arg( connStr ) );
    while ( ok && !sApi.sqlany_connect( newconn, connStr.toUtf8().constData() ) )
    {

      // Connection failed
      code = sApi.sqlany_error( newconn, errbuf, size );
      SaDebugMsg( QString( "Error %1: %2 " ).arg( code ).arg( errbuf ) );
      if ( code == -103 || code == -1074 )
      {
        // invalid login credentials
        ok = QgsCredentials::instance()->get( credKey, username, password, QString::fromUtf8( errbuf ) );
        if ( ok )
        {
          theUri.setUsername( username );
          theUri.setPassword( password );
          connStr = makeConnectString( theUri, true );
          SaDebugMsg( QString( "Connecting to SQL Anywhere with string: '%1' " ).arg( connStr ) );
        }
      }
      else
      {
        ok = false;
      }
    }

    if ( ok )
    {
      // save the credentials
      QgsCredentials::instance()->put( credKey, username, password );

      // set a default timeout of 5 seconds to avoid deadlocks
      ok = ok && ( bool ) sApi.sqlany_execute_immediate( newconn,
           "SET TEMPORARY OPTION blocking_timeout = 5000" );

      if ( readOnly )
      {
        // set as low priority so that if open cursors are
        // holding row/table/schema locks, they get killed
        // after 1 sec if someone else wants exclusive lock
        ok = ok && ( bool ) sApi.sqlany_execute_immediate( newconn,
             "SET TEMPORARY OPTION blocking_others_timeout = 1000" );
      }
    }

    if ( ok )
    {
      SaDebugMsg( "Connection successful." );

      // Wrap connection object (wrapper inserts itself into connCache)
      retval = new SqlAnyConnection( mapKey, theUri.uri(), newconn, readOnly );

    }
    else
    {
      SaDebugMsg( "Connection failed." );
    }
  }
  gMutex.unlock();

  return retval;
}

/**
 * Pings the server with a trivial SQL statement to test whether the
 * connection is still alive.
 */
bool
SqlAnyConnection::isAlive()
{
  return sApi.sqlany_execute_immediate( mHandle, "CREATE OR REPLACE VARIABLE myVar int" );
}

SqlAnyConnection *
SqlAnyConnection::addRef()
{
  // use the global mutex when adjusting the reference count
  gMutex.lock();
  mRefCount++;
  gMutex.unlock();
  return this;
}

void
SqlAnyConnection::release()
{
  // use the global mutex when adjusting the reference count
  gMutex.lock();
  mRefCount--;
  if ( mRefCount == 0 )
    delete this;
  gMutex.unlock();
}

QString
SqlAnyConnection::serverVersion()
{
  QString verStr;
  SqlAnyStatement *stmt;

  stmt = execute_direct( "SELECT PROPERTY( 'ProductVersion' )" );
  if ( !stmt->isValid() || !stmt->fetchNext() || !stmt->getString( 0, verStr ) )
  {
    verStr = QString();
  }
  delete stmt;
  return verStr;
}

/**
 * Makes a SQL Anywhere connection string from the
 * dbname, username, and password components of the uri
 */
QString
SqlAnyConnection::makeConnectString( QgsDataSourceURI uri, bool includeUidPwd )
{
  QString conStr = uri.database();  // connection parameters already encoded within dbname
  conStr += QString( ";CS=UTF-8" );  // force UTF-8 character set at the client

  if ( includeUidPwd )
  {
    QString username = uri.username();
    if ( !username.isEmpty() )
    {
      conStr += QString( ";UID=%1" ).arg( username );
    }
    QString password = uri.password();
    if ( !password.isEmpty() )
    {
      conStr += QString( ";PWD=%1" ).arg( password );
    }
  }

  return conStr;
}


SqlAnyStatement *
SqlAnyConnection::prepare( QString sql )
{
  SqlAnyStatement *stmt = new SqlAnyStatement( addRef() );

  SaDebugMsg( QString( "Preparing sql: %1" ).arg( sql ) );

  // preparing statements is not thread-safe, so needs the connection mutex
  mMutex.lock();
  stmt->mHndl = sApi.sqlany_prepare( mHandle, sql.toUtf8().constData() );
  if ( !stmt->isValid() )
  {
    getError( stmt->mErrCode, stmt->mErrMsg, sizeof( stmt->mErrMsg ) );
    SaDebugMsg( QString( "Error code %1: %2" )
                .arg( stmt->errCode() )
                .arg( stmt->errMsg() ) );
  }
  mMutex.unlock();

  return stmt;
}

SqlAnyStatement *
SqlAnyConnection::execute_direct( QString sql )
{
  SqlAnyStatement *stmt = prepare( sql );
  if ( stmt->isValid() && !stmt->execute( ) )
  {
    // mark stmt as invalid.
    // execute() already retrieved the error message
    sApi.sqlany_free_stmt( stmt->mHndl );
    stmt->mHndl = NULL;
  }
  return stmt;
}

bool
SqlAnyConnection::execute_immediate( QString sql )
{
  char reason[SACAPI_ERROR_SIZE];
  sacapi_i32 code;
  return execute_immediate( sql, code, reason, sizeof( reason ) );
}

bool
SqlAnyConnection::execute_immediate( QString sql, sacapi_i32 &code, char *errbuf, size_t size )
{
  sacapi_bool ok = 0;

  SaDebugMsg( QString( "Executing sql: %1" ).arg( sql ) );

  // immediate execution of statements is not thread-safe, so needs
  // the connection mutex
  mMutex.lock();
  ok = sApi.sqlany_execute_immediate( mHandle, sql.toUtf8().constData() );
  if ( !ok )
    getError( code, errbuf, size );
  mMutex.unlock();

  return ok != 0;
}

void
SqlAnyConnection::getError( sacapi_i32 &code, char *errbuf, size_t size )
{
  code = sApi.sqlany_error( mHandle, errbuf, size );
}

void
SqlAnyConnection::begin( )
// acquires the connection-level mutex and holds it for the
// duration of the transaction
{
  if ( !mReadOnly )
  {
    mMutex.lock();
  }
}

bool
SqlAnyConnection::commit( )
{
  char reason[SACAPI_ERROR_SIZE];
  sacapi_i32 code;
  return commit( code, reason, sizeof( reason ) );
}

bool
SqlAnyConnection::commit( sacapi_i32 &code, char *errbuf, size_t size )
// releases the connection-level mutex
{
  sacapi_bool ok = 1;
  if ( !mReadOnly )
  {
    SaDebugMsg( "Commit work." );
    ok = sApi.sqlany_commit( mHandle );
    if ( !ok )
    {
      getError( code, errbuf, size );
    }
    else
    {
      SaDebugMsg( "Work committed." );
    }
    mMutex.unlock();
  }
  return ok != 0;
}

bool
SqlAnyConnection::rollback( )
{
  char reason[SACAPI_ERROR_SIZE];
  sacapi_i32 code;
  return rollback( code, reason, sizeof( reason ) );
}

bool
SqlAnyConnection::rollback( sacapi_i32 &code, char *errbuf, size_t size )
// releases the connection-level mutex
{
  sacapi_bool ok = 1;
  if ( !mReadOnly )
  {
    SaDebugMsg( "Rollback work." );
    ok = sApi.sqlany_rollback( mHandle );
    if ( !ok )
    {
      getError( code, errbuf, size );
    }
    else
    {
      SaDebugMsg( "Work aborted." );
    }
    mMutex.unlock();
  }
  return ok != 0;
}

QgsVectorDataProvider::NativeType
SqlAnyConnection::mapType( a_sqlany_native_type t ) const
{
  return mNativeMap.value( t, sInvalidType );
}

void
SqlAnyConnection::populateNativeMap()
{
  // Character types
  mNativeMap.insert( DT_FIXCHAR,
                     QgsVectorDataProvider::NativeType( "Text, fixed length", "CHAR", QVariant::String, 1, 32767 ) );
  mNativeMap.insert( DT_VARCHAR,
                     QgsVectorDataProvider::NativeType( "Text, variable length", "VARCHAR", QVariant::String, 1, 32767 ) );
  mNativeMap.insert( DT_LONGVARCHAR,
                     QgsVectorDataProvider::NativeType( "Text, long variable length", "LONG VARCHAR", QVariant::String ) );
  mNativeMap.insert( DT_LONGNVARCHAR,
                     QgsVectorDataProvider::NativeType( "Text, long variable length Unicode", "LONG NVARCHAR", QVariant::String ) );
  mNativeMap.insert( DT_STRING,
                     QgsVectorDataProvider::NativeType( "Text, unlimited length", "TEXT", QVariant::String ) );

  // Binary types
  mNativeMap.insert( DT_BINARY,
                     QgsVectorDataProvider::NativeType( "Binary, variable length", "BINARY", QVariant::ByteArray, 1, 32767 ) );
  mNativeMap.insert( DT_LONGBINARY,
                     QgsVectorDataProvider::NativeType( "Binary, long variable length", "LONG BINARY", QVariant::ByteArray ) );

  // Floating point numbers
  mNativeMap.insert( DT_DOUBLE,
                     QgsVectorDataProvider::NativeType( "Decimal number, double precision", "DOUBLE", QVariant::Double ) );
  mNativeMap.insert( DT_FLOAT,
                     QgsVectorDataProvider::NativeType( "Decimal number, single precision", "REAL", QVariant::Double ) );
  mNativeMap.insert( DT_DECIMAL, // passed as String by interface
                     QgsVectorDataProvider::NativeType( "Decimal number, variable precision", "DECIMAL", QVariant::String, 1, 127, 0, 127 ) );

  // Integral numbers
  mNativeMap.insert( DT_BIT,
                     QgsVectorDataProvider::NativeType( "Integer number, unsigned 1bit", "BIT", QVariant::UInt ) );
  mNativeMap.insert( DT_TINYINT,
                     QgsVectorDataProvider::NativeType( "Integer number, unsigned 8bit", "TINYINT", QVariant::UInt ) );
  mNativeMap.insert( DT_SMALLINT,
                     QgsVectorDataProvider::NativeType( "Integer number, 16bit", "SMALLINT", QVariant::UInt ) );
  mNativeMap.insert( DT_UNSSMALLINT,
                     QgsVectorDataProvider::NativeType( "Integer number, unsigned 16bit", "UNSIGNED SMALLINT", QVariant::UInt ) );
  mNativeMap.insert( DT_INT,
                     QgsVectorDataProvider::NativeType( "Integer number, 32bit", "INTEGER", QVariant::Int ) );
  mNativeMap.insert( DT_UNSINT,
                     QgsVectorDataProvider::NativeType( "Integer number, unsigned 32bit", "UNSIGNED INTEGER", QVariant::UInt ) );
  mNativeMap.insert( DT_BIGINT,
                     QgsVectorDataProvider::NativeType( "Integer number, 64bit", "BIGINT", QVariant::LongLong ) );
  mNativeMap.insert( DT_UNSBIGINT,
                     QgsVectorDataProvider::NativeType( "Integer number, unsigned 64bit", "UNSIGNED BIGINT", QVariant::ULongLong ) );

  // Date and time types (passed as string by interface)
  mNativeMap.insert( DT_DATE,
                     QgsVectorDataProvider::NativeType( "Date", "DATE", QVariant::String ) );
  mNativeMap.insert( DT_TIME,
                     QgsVectorDataProvider::NativeType( "Time of day", "TIME", QVariant::String ) );
  mNativeMap.insert( DT_TIMESTAMP,
                     QgsVectorDataProvider::NativeType( "Timestamp (date and time)", "TIMESTAMP", QVariant::String ) );

  // Unknown type
  mNativeMap.insert( DT_NOTYPE, sInvalidType );

}

