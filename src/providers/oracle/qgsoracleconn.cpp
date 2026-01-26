/***************************************************************************
  qgsoracleconn.cpp  -  connection class to Oracle
                             -------------------
    begin                : August 2012
    copyright            : (C) 2012 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsoracleconn.h"

#include "qgscredentials.h"
#include "qgsdatasourceuri.h"
#include "qgsdbquerylog.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsoracleconnpool.h"
#include "qgssettings.h"
#include "qgsvariantutils.h"

#include <QSqlDriver>
#include <QSqlError>
#include <QSqlField>

#include "moc_qgsoracleconn.cpp"

QMap<QPair<QString, QThread *>, QgsOracleConn *> QgsOracleConn::sConnections;
int QgsOracleConn::snConnections = 0;
const int QgsOracleConn::sGeomTypeSelectLimit = 100;
QMap<QString, QDateTime> QgsOracleConn::sBrokenConnections;

QgsOracleConn *QgsOracleConn::connectDb( const QgsDataSourceUri &uri, bool transaction )
{
  const QString conninfo = toPoolName( uri );
  QgsDebugMsgLevel( u"Trying to connect to %1 (in transaction: %2)"_s.arg( conninfo ).arg( transaction ? u"YES"_s : u"NO"_s ), 2 );
  const QPair<QString, QThread *> connInfoThread( conninfo, QThread::currentThread() );

  if ( !transaction )
  {
    if ( sConnections.contains( connInfoThread ) )
    {
      QgsDebugMsgLevel( u"Using cached connection for %1"_s.arg( conninfo ), 2 );
      sConnections[connInfoThread]->mRef++;
      return sConnections[connInfoThread];
    }
  }

  QgsDebugMsgLevel( u"Making new connection"_s, 2 );
  QgsOracleConn *conn = new QgsOracleConn( uri, transaction );

  if ( conn->mRef == 0 )
  {
    QgsDebugMsgLevel( u"Connection has no references, deleting..."_s, 2 );
    delete conn;
    return nullptr;
  }

  if ( !transaction )
  {
    QgsDebugMsgLevel( u"Storing connection for this thread"_s, 2 );
    sConnections.insert( connInfoThread, conn );
  }

  return conn;
}

QgsOracleConn::QgsOracleConn( const QgsDataSourceUri &uri, bool transaction )
  : mCurrentUser( QString() )
  , mTransaction( transaction )
{
  QgsDebugMsgLevel( u"New Oracle connection for "_s + uri.connectionInfo( false ), 2 );

  // will be used for logging and access connection from connection pool by name,
  // so we don't want login/password here
  mConnInfo = toPoolName( uri );

  QgsDataSourceUri expandedUri = QgsDataSourceUri( uri.connectionInfo( true ) );

  QString database = databaseName( expandedUri.database(), expandedUri.host(), expandedUri.port() );
  QgsDebugMsgLevel( u"New Oracle database "_s + database, 2 );

  mDatabase = QSqlDatabase::addDatabase( u"QOCISPATIAL"_s, u"oracle%1"_s.arg( snConnections++ ) );
  mDatabase.setDatabaseName( database );
  QString options = expandedUri.hasParam( u"dboptions"_s ) ? expandedUri.param( u"dboptions"_s ) : u"OCI_ATTR_PREFETCH_ROWS=1000"_s;
  if ( mTransaction )
    options += ( !options.isEmpty() ? u";"_s : QString() ) + u"COMMIT_ON_SUCCESS=false"_s;
  const QString workspace = expandedUri.hasParam( u"dbworkspace"_s ) ? expandedUri.param( u"dbworkspace"_s ) : QString();
  QgsDebugMsgLevel( u"Using workspace %1"_s.arg( workspace ), 2 );
  mDatabase.setConnectOptions( options );
  mDatabase.setUserName( expandedUri.username() );
  mDatabase.setPassword( expandedUri.password() );

  QString username = expandedUri.username();
  QString password = expandedUri.password();

  if ( sBrokenConnections.contains( mConnInfo ) )
  {
    QDateTime now( QDateTime::currentDateTime() );
    QDateTime since( sBrokenConnections[mConnInfo] );
    QgsDebugError( u"Broken since %1 [%2s ago]"_s.arg( since.toString( Qt::ISODate ) ).arg( since.secsTo( now ) ) );

    if ( since.secsTo( now ) < 30 )
    {
      QgsMessageLog::logMessage( tr( "Connection failed %1s ago - skipping retry" ).arg( since.secsTo( now ) ), tr( "Oracle" ) );
      mRef = 0;
      return;
    }
  }

  QgsDebugMsgLevel( u"Connecting with options: "_s + options, 2 );
  if ( !mDatabase.open() )
  {
    QgsCredentials::instance()->lock();

    while ( !mDatabase.open() )
    {
      bool ok = QgsCredentials::instance()->get( mConnInfo, username, password, mDatabase.lastError().text() );
      if ( !ok )
      {
        QDateTime now( QDateTime::currentDateTime() );
        QgsDebugError( u"get failed: %1 <= %2"_s.arg( mConnInfo, now.toString( Qt::ISODate ) ) );
        sBrokenConnections.insert( mConnInfo, now );
        break;
      }

      sBrokenConnections.remove( mConnInfo );

      if ( !username.isEmpty() )
      {
        expandedUri.setUsername( username );
      }

      if ( !password.isEmpty() )
        expandedUri.setPassword( password );

      QgsDebugMsgLevel( "Connecting to " + database, 2 );
      mDatabase.setUserName( username );
      mDatabase.setPassword( password );
    }

    if ( mDatabase.isOpen() )
      QgsCredentials::instance()->put( mConnInfo, username, password );

    QgsCredentials::instance()->unlock();
  }

  if ( !mDatabase.isOpen() )
  {
    mDatabase.close();
    QgsMessageLog::logMessage( tr( "Connection to database failed" ), tr( "Oracle" ) );
    mRef = 0;
    return;
  }

  QSqlQuery qry( mDatabase );
  if ( !LoggedExecPrivate( u"QgsOracleConn"_s, qry, u"alter session set nls_date_format = 'yyyy-mm-dd\"T\"HH24:MI:ss'"_s, QVariantList() ) )
  {
    mDatabase.close();
    const QString error { tr( "Error: Failed to switch the default format date to ISO" ) };
    QgsMessageLog::logMessage( error, tr( "Oracle" ) );
    mRef = 0;
    return;
  }

  if ( !workspace.isNull() )
  {
    if ( !qry.prepare( u"BEGIN\nDBMS_WM.GotoWorkspace(?);\nEND;"_s ) || !( qry.addBindValue( workspace ), qry.exec() ) )
    {
      mDatabase.close();
      QgsMessageLog::logMessage( tr( "Could not switch to workspace %1 [%2]" ).arg( workspace, qry.lastError().databaseText() ), tr( "Oracle" ) );
      mRef = 0;
      return;
    }
  }
}

QgsOracleConn::~QgsOracleConn()
{
  Q_ASSERT( mRef == 0 );
  if ( mDatabase.isOpen() )
    mDatabase.close();
}

QString QgsOracleConn::toPoolName( const QgsDataSourceUri &uri )
{
  QString conninfo = uri.connectionInfo( false );
  if ( uri.hasParam( u"dbworkspace"_s ) )
    conninfo += u" dbworkspace="_s + uri.param( u"dbworkspace"_s );
  return conninfo;
}

QString QgsOracleConn::connInfo()
{
  return mConnInfo;
}

void QgsOracleConn::disconnect()
{
  unref();
}

void QgsOracleConn::reconnect()
{
  if ( mDatabase.isOpen() )
  {
    mDatabase.close();
    ( void ) mDatabase.open();
  }
}

void QgsOracleConn::unref()
{
  QMutexLocker locker( &mLock );
  if ( --mRef > 0 )
    return;

  if ( !mTransaction )
  {
    QPair<QString, QThread *> key = sConnections.key( this, QPair<QString, QThread *>() );
    if ( !key.first.isNull() )
    {
      sConnections.remove( key );
    }
  }

  // to avoid destroying locked mutex
  locker.unlock();
  delete this;
}

QString QgsOracleConn::getLastExecutedQuery( const QSqlQuery &query )
{
  QString str = query.lastQuery();

  const QRegularExpression re( "(?:\\?|\\:[a-z|A-Z]*)" );
  QRegularExpressionMatch match;
  int start = 0;
  for ( QVariant value : query.boundValues() )
  {
    const QVariant &var { value.toString() };
    QSqlField field( QString(), var.metaType() );

    if ( var.isNull() )
    {
      field.clear();
    }
    else
    {
      field.setValue( var );
    }
    const QString formatV = query.driver()->formatValue( field );

    const int i = str.indexOf( re, start, &match );
    str.replace( i, match.captured().size(), formatV );
  }
  return str;
}

bool QgsOracleConn::exec( QSqlQuery &qry, const QString &sql, const QVariantList &params )
{
  QgsDebugMsgLevel( u"SQL: %1"_s.arg( sql ), 4 );

  bool res = qry.prepare( sql );
  if ( res )
  {
    for ( const auto &param : params )
    {
      QgsDebugMsgLevel( u" ARG: %1 [%2]"_s.arg( param.toString(), param.typeName() ), 4 );
      qry.addBindValue( param );
    }

    res = qry.exec();
  }

  if ( !res )
  {
    QgsDebugError( u"SQL: %1\nERROR: %2"_s
                     .arg( qry.lastQuery(), qry.lastError().text() ) );
  }

  return res;
}

bool QgsOracleConn::execLogged( QSqlQuery &qry, const QString &sql, const QVariantList &params, const QString &originatorClass, const QString &queryOrigin )
{
  QgsDebugMsgLevel( u"SQL: %1"_s.arg( sql ), 4 );

  QgsDatabaseQueryLogWrapper logWrapper { sql, mConnInfo, u"oracle"_s, originatorClass, queryOrigin };

  bool res = qry.prepare( sql );
  if ( res )
  {
    for ( const auto &param : params )
    {
      QgsDebugMsgLevel( u" ARG: %1 [%2]"_s.arg( param.toString(), param.typeName() ), 4 );
      qry.addBindValue( param );
    }

    res = qry.exec();
  }

  logWrapper.setQuery( getLastExecutedQuery( qry ) );

  if ( !res )
  {
    logWrapper.setError( qry.lastError().text() );
    QgsDebugError( u"SQL: %1\nERROR: %2"_s
                     .arg( qry.lastQuery(), qry.lastError().text() ) );
  }
  else
  {
    if ( qry.isSelect() )
    {
      logWrapper.setFetchedRows( qry.size() );
    }
    else
    {
      logWrapper.setFetchedRows( qry.numRowsAffected() );
    }
  }

  return res;
}

QStringList QgsOracleConn::pkCandidates( const QString &ownerName, const QString &viewName )
{
  QStringList cols;

  QSqlQuery qry( mDatabase );

  if ( !LoggedExecPrivate( u"QgsOracleConn"_s, qry, u"SELECT column_name FROM all_tab_columns WHERE owner=? AND table_name=? ORDER BY column_id"_s, QVariantList() << ownerName << viewName ) )
  {
    const QString error { tr( "SQL: %1 [owner: %2 table_name: %3]\nerror: %4\n" ).arg( qry.lastQuery(), qry.lastError().text(), ownerName, viewName ) };
    QgsMessageLog::logMessage( error, tr( "Oracle" ) );
    return cols;
  }

  while ( qry.next() )
  {
    cols << qry.value( 0 ).toString();
  }

  qry.finish();

  return cols;
}

bool QgsOracleConn::tableInfo( const QString &schema, bool geometryColumnsOnly, bool userTablesOnly, bool allowGeometrylessTables )
{
  QgsDebugMsgLevel( u"Entering."_s, 4 );

  mLayersSupported.clear();

  QString sql;

  QString prefix( userTablesOnly ? u"user"_s : u"all"_s ),
    owner( userTablesOnly ? u"user AS owner"_s : u"c.owner"_s );

  sql = QStringLiteral( "SELECT %1,c.table_name,c.column_name,%2,o.object_type AS type"
                        " FROM %3_%4 c"
                        " JOIN %3_objects o ON c.table_name=o.object_name AND o.object_type IN ('TABLE','VIEW','SYNONYM')%5%6" )
          .arg( owner, geometryColumnsOnly ? u"c.srid"_s : u"NULL AS srid"_s, prefix, geometryColumnsOnly ? u"sdo_geom_metadata"_s : u"tab_columns"_s, userTablesOnly ? QString() : u" AND c.owner=%1"_s.arg( schema.isEmpty() ? u"o.owner"_s : quotedValue( schema ) ), geometryColumnsOnly ? QString() : u" WHERE c.data_type='SDO_GEOMETRY'"_s );

  if ( allowGeometrylessTables )
  {
    // also here!
    sql += QStringLiteral( " UNION SELECT %1,object_name,NULL AS column_name,NULL AS srid,object_type AS type"
                           " FROM %2_objects c WHERE c.object_type IN ('TABLE','VIEW','SYNONYM') "
                           // get only geometry table without geometry column
                           " AND NOT EXISTS( SELECT 1 FROM %2_tab_columns cols WHERE cols.table_name=c.object_name AND cols.data_type='SDO_GEOMETRY') %3" )
             .arg( owner, prefix, userTablesOnly || schema.isEmpty() ? QString() : u" AND c.owner=%1"_s.arg( quotedValue( schema ) ) );
  }

  QSqlQuery qry( mDatabase );

  if ( !LoggedExecPrivate( u"QgsOracleConn"_s, qry, sql, QVariantList() ) )
  {
    const QString error { tr( "Querying available tables failed.\nSQL: %1\nerror: %2\n" ).arg( qry.lastQuery(), qry.lastError().text() ) };
    QgsMessageLog::logMessage( error, tr( "Oracle" ) );
    return false;
  }

  while ( qry.next() )
  {
    QgsOracleLayerProperty layerProperty;
    layerProperty.ownerName = qry.value( 0 ).toString();
    layerProperty.tableName = qry.value( 1 ).toString();
    layerProperty.geometryColName = qry.value( 2 ).toString();
    layerProperty.types = QList<Qgis::WkbType>() << ( qry.value( 2 ).isNull() ? Qgis::WkbType::NoGeometry : Qgis::WkbType::Unknown );
    layerProperty.srids = QList<int>() << qry.value( 3 ).toInt();
    layerProperty.isView = qry.value( 4 ) != "TABLE"_L1;
    layerProperty.pkCols.clear();

    mLayersSupported << layerProperty;
  }


  if ( mLayersSupported.size() == 0 )
  {
    QgsMessageLog::logMessage( tr( "Database connection was successful, but the accessible tables could not be determined." ), tr( "Oracle" ) );
  }

  return true;
}

bool QgsOracleConn::supportedLayers( QVector<QgsOracleLayerProperty> &layers, const QString &limitToSchema, bool geometryTablesOnly, bool userTablesOnly, bool allowGeometrylessTables )
{
  // Get the list of supported tables
  if ( !tableInfo( limitToSchema, geometryTablesOnly, userTablesOnly, allowGeometrylessTables ) )
  {
    QgsMessageLog::logMessage( tr( "Unable to get list of spatially enabled tables from the database" ), tr( "Oracle" ) );
    return false;
  }

  layers = mLayersSupported;

  QgsDebugMsgLevel( u"Exiting."_s, 4 );

  return true;
}

QString QgsOracleConn::quotedIdentifier( QString ident )
{
  ident.replace( '"', "\"\""_L1 );
  ident = ident.prepend( '\"' ).append( '\"' );
  return ident;
}

QString QgsOracleConn::quotedValue( const QVariant &value, QMetaType::Type type )
{
  if ( value.isNull() )
    return u"NULL"_s;

  if ( type == QMetaType::Type::UnknownType )
    type = static_cast<QMetaType::Type>( value.userType() );

  if ( value.canConvert( type ) )
  {
    switch ( type )
    {
      case QMetaType::Type::Int:
      case QMetaType::Type::LongLong:
      case QMetaType::Type::Double:
        return value.toString();

      case QMetaType::Type::QDateTime:
      {
        QDateTime datetime( value.toDateTime() );
        if ( datetime.isValid() )
          return u"TO_DATE('%1','YYYY-MM-DD HH24:MI:SS')"_s.arg( datetime.toString( u"yyyy-MM-dd hh:mm:ss"_s ) );
        break;
      }

      case QMetaType::Type::QDate:
      {
        QDate date( value.toDate() );
        if ( date.isValid() )
          return u"TO_DATE('%1','YYYY-MM-DD')"_s.arg( date.toString( u"yyyy-MM-dd"_s ) );
        break;
      }

      case QMetaType::Type::QTime:
      {
        QDateTime datetime( value.toDateTime() );
        if ( datetime.isValid() )
          return u"TO_DATE('%1','HH24:MI:SS')"_s.arg( datetime.toString( u"hh:mm:ss"_s ) );
        break;
      }

      default:
        break;
    }
  }

  QString v = value.toString();
  v.replace( '\'', "''"_L1 );
  v.replace( "\\\""_L1, "\\\\\""_L1 );
  return v.prepend( '\'' ).append( '\'' );
}

bool QgsOracleConn::exec( const QString &query, bool logError, QString *errorMessage )
{
  QMutexLocker locker( &mLock );
  QgsDebugMsgLevel( u"Executing SQL: %1"_s.arg( query ), 3 );

  QSqlQuery qry( mDatabase );

  if ( !exec( qry, query, QVariantList() ) )
  {
    QString error = qry.lastError().text();
    if ( logError )
    {
      const QString errorMsg { tr( "Connection error: %1 returned %2" )
                                 .arg( query, error ) };
      QgsMessageLog::logMessage( errorMsg, tr( "Oracle" ) );
    }
    else
    {
      const QString errorMsg { u"Connection error: %1 returned %2"_s
                                 .arg( query, error ) };
      QgsDebugError( errorMsg );
    }
    if ( errorMessage )
      *errorMessage = error;
    return false;
  }
  return true;
}

bool QgsOracleConn::execLogged( const QString &query, bool logError, QString *errorMessage, const QString &originatorClass, const QString &queryOrigin )
{
  QMutexLocker locker( &mLock );
  QgsDatabaseQueryLogWrapper logWrapper { query, mConnInfo, u"oracle"_s, originatorClass, queryOrigin };

  QgsDebugMsgLevel( u"Executing SQL: %1"_s.arg( query ), 3 );

  QSqlQuery qry( mDatabase );

  const bool res { exec( qry, query, QVariantList() ) };

  logWrapper.setQuery( qry.lastQuery() );

  if ( !res )
  {
    const QString error = qry.lastError().text();
    logWrapper.setError( error );
    if ( logError )
    {
      const QString errorMsg { tr( "Connection error: %1 returned %2" )
                                 .arg( query, error ) };
      QgsMessageLog::logMessage( errorMsg, tr( "Oracle" ) );
    }
    else
    {
      const QString errorMsg { u"Connection error: %1 returned %2"_s
                                 .arg( query, error ) };
      QgsDebugError( errorMsg );
    }
    if ( errorMessage )
      *errorMessage = error;
    return false;
  }
  else
  {
    if ( qry.isSelect() )
    {
      logWrapper.setFetchedRows( qry.size() );
    }
    else
    {
      logWrapper.setFetchedRows( qry.numRowsAffected() );
    }
  }

  return true;
}

bool QgsOracleConn::begin( QSqlDatabase &db )
{
  QMutexLocker locker( &mLock );
  if ( mTransaction )
  {
    return LoggedExec( u"QgsOracleConn"_s, u"SAVEPOINT sp%1"_s.arg( ++mSavePointId ) );
  }
  else
  {
    return db.transaction();
  }
}

bool QgsOracleConn::commit( QSqlDatabase &db )
{
  QMutexLocker locker( &mLock );
  if ( mTransaction )
  {
    return LoggedExec( u"QgsOracleConn"_s, u"SAVEPOINT sp%1"_s.arg( ++mSavePointId ) );
  }
  else
  {
    return db.commit();
  }
}

bool QgsOracleConn::rollback( QSqlDatabase &db )
{
  QMutexLocker locker( &mLock );
  if ( mTransaction )
  {
    return LoggedExec( u"QgsOracleConn"_s, u"ROLLBACK TO SAVEPOINT sp%1"_s.arg( mSavePointId ) );
  }
  else
  {
    return db.rollback();
  }
}

QString QgsOracleConn::fieldExpression( const QgsField &fld )
{
#if 0
  const QString &type = fld.typeName();
  if ( type == "money" )
  {
    return QString( "cash_out(%1)" ).arg( quotedIdentifier( fld.name() ) );
  }
  else if ( type.startsWith( "_" ) )
  {
    return QString( "array_out(%1)" ).arg( quotedIdentifier( fld.name() ) );
  }
  else if ( type == "bool" )
  {
    return QString( "boolout(%1)" ).arg( quotedIdentifier( fld.name() ) );
  }
  else if ( type == "geometry" )
  {
    return QString( "%1(%2)" )
           .arg( majorVersion() < 2 ? "asewkt" : "st_asewkt" )
           .arg( quotedIdentifier( fld.name() ) );
  }
  else if ( type == "geography" )
  {
    return QString( "st_astext(%1)" ).arg( quotedIdentifier( fld.name() ) );
  }
  else
  {
    return quotedIdentifier( fld.name() ) + "::text";
  }
#else
  return quotedIdentifier( fld.name() );
#endif
}

void QgsOracleConn::retrieveLayerTypes( QgsOracleLayerProperty &layerProperty, bool useEstimatedMetadata, bool onlyExistingTypes )
{
  QMutexLocker locker( &mLock );
  QgsDebugMsgLevel( u"entering: "_s + layerProperty.toString(), 3 );

  if ( layerProperty.isView )
  {
    layerProperty.pkCols = pkCandidates( layerProperty.ownerName, layerProperty.tableName );
    if ( layerProperty.pkCols.isEmpty() )
    {
      QgsMessageLog::logMessage( tr( "View %1.%2 doesn't have integer columns for use as keys." ).arg( layerProperty.ownerName, layerProperty.tableName ), tr( "Oracle" ) );
    }
  }

  if ( layerProperty.geometryColName.isEmpty() )
    return;

  QString table;
  QString where;

  if ( useEstimatedMetadata )
  {
    table = u"(SELECT %1 FROM %2.%3 WHERE %1 IS NOT NULL%4 AND rownum<=%5)"_s
              .arg( quotedIdentifier( layerProperty.geometryColName ), quotedIdentifier( layerProperty.ownerName ), quotedIdentifier( layerProperty.tableName ), layerProperty.sql.isEmpty() ? QString() : u" AND (%1)"_s.arg( layerProperty.sql ) )
              .arg( sGeomTypeSelectLimit );
  }
  else if ( !layerProperty.ownerName.isEmpty() )
  {
    table = u"%1.%2"_s
              .arg( quotedIdentifier( layerProperty.ownerName ), quotedIdentifier( layerProperty.tableName ) );
    where = layerProperty.sql;
  }
  else
  {
    table = quotedIdentifier( layerProperty.tableName );
    where = layerProperty.sql;
  }

  Qgis::WkbType detectedType = layerProperty.types.value( 0, Qgis::WkbType::Unknown );
  int detectedSrid = layerProperty.srids.value( 0, -1 );

  Q_ASSERT( detectedType == Qgis::WkbType::Unknown || detectedSrid <= 0 );

  QSqlQuery qry( mDatabase );
  int idx = 0;
  QString sql = u"SELECT DISTINCT "_s;
  if ( detectedType == Qgis::WkbType::Unknown )
  {
    sql += "t.%1.SDO_GTYPE"_L1;
    if ( detectedSrid <= 0 )
    {
      sql += ',';
      idx = 1;
    }
  }

  if ( detectedSrid <= 0 )
  {
    sql += "t.%1.SDO_SRID"_L1;
  }

  sql += " FROM %2 t WHERE NOT t.%1 IS NULL%3"_L1;

  if ( !LoggedExecPrivate( u"QgsOracleConn"_s, qry, sql.arg( quotedIdentifier( layerProperty.geometryColName ), table, where.isEmpty() ? QString() : u" AND (%1)"_s.arg( where ) ), QVariantList() ) )
  {
    const QString error { tr( "SQL: %1\nerror: %2\n" )
                            .arg( qry.lastQuery(), qry.lastError().text() ) };
    QgsMessageLog::logMessage( error, tr( "Oracle" ) );
    return;
  }

  layerProperty.types.clear();
  layerProperty.srids.clear();

  QSet<int> srids;
  while ( qry.next() )
  {
    if ( detectedType == Qgis::WkbType::Unknown )
    {
      Qgis::WkbType type = wkbTypeFromDatabase( qry.value( 0 ).toInt() );
      if ( type == Qgis::WkbType::Unknown )
      {
        QgsMessageLog::logMessage( tr( "Unsupported geometry type %1 in %2.%3.%4 ignored" ).arg( qry.value( 0 ).toInt() ).arg( layerProperty.ownerName, layerProperty.tableName, layerProperty.geometryColName ), tr( "Oracle" ) );
        continue;
      }
      QgsDebugMsgLevel( u"add type %1"_s.arg( qgsEnumValueToKey( type ) ), 2 );
      layerProperty.types << type;
    }
    else
    {
      layerProperty.types << detectedType;
    }

    int srid = detectedSrid > 0 ? detectedSrid : ( qry.value( idx ).isNull() ? -1 : qry.value( idx ).toInt() );
    layerProperty.srids << srid;
    srids << srid;
  }

  qry.finish();

  if ( !onlyExistingTypes )
  {
    layerProperty.types << Qgis::WkbType::Unknown;
    layerProperty.srids << ( detectedSrid > 0 ? detectedSrid : ( srids.size() == 1 ? *srids.constBegin() : 0 ) );
  }
}

QString QgsOracleConn::databaseTypeFilter( const QString &alias, QString geomCol, Qgis::WkbType geomType )
{
  geomCol = quotedIdentifier( alias ) + "." + quotedIdentifier( geomCol );

  switch ( geomType )
  {
    case Qgis::WkbType::Point:
    case Qgis::WkbType::Point25D:
    case Qgis::WkbType::PointZ:
    case Qgis::WkbType::MultiPoint:
    case Qgis::WkbType::MultiPoint25D:
    case Qgis::WkbType::MultiPointZ:
      return u"mod(%1.sdo_gtype,100) IN (1,5)"_s.arg( geomCol );
    case Qgis::WkbType::LineString:
    case Qgis::WkbType::LineString25D:
    case Qgis::WkbType::LineStringZ:
    case Qgis::WkbType::CircularString:
    case Qgis::WkbType::CircularStringZ:
    case Qgis::WkbType::CompoundCurve:
    case Qgis::WkbType::CompoundCurveZ:
    case Qgis::WkbType::MultiLineString:
    case Qgis::WkbType::MultiLineString25D:
    case Qgis::WkbType::MultiLineStringZ:
    case Qgis::WkbType::MultiCurve:
    case Qgis::WkbType::MultiCurveZ:
      return u"mod(%1.sdo_gtype,100) IN (2,6)"_s.arg( geomCol );
    case Qgis::WkbType::Polygon:
    case Qgis::WkbType::Polygon25D:
    case Qgis::WkbType::PolygonZ:
    case Qgis::WkbType::CurvePolygon:
    case Qgis::WkbType::CurvePolygonZ:
    case Qgis::WkbType::MultiPolygon:
    case Qgis::WkbType::MultiPolygonZ:
    case Qgis::WkbType::MultiPolygon25D:
    case Qgis::WkbType::MultiSurface:
    case Qgis::WkbType::MultiSurfaceZ:
      return u"mod(%1.sdo_gtype,100) IN (3,7)"_s.arg( geomCol );
    case Qgis::WkbType::NoGeometry:
      return u"%1 IS NULL"_s.arg( geomCol );
    case Qgis::WkbType::Unknown:
      Q_ASSERT( !"unknown geometry unexpected" );
      return QString();
    default:
      break;
  }

  Q_ASSERT( !"unexpected geomType" );
  return QString();
}

Qgis::WkbType QgsOracleConn::wkbTypeFromDatabase( int gtype )
{
  QgsDebugMsgLevel( u"entering %1"_s.arg( gtype ), 2 );
  int t = gtype % 100;

  if ( t == 0 )
    return Qgis::WkbType::Unknown;

  int d = gtype / 1000;
  if ( d == 2 )
  {
    switch ( t )
    {
      case 1:
        return Qgis::WkbType::Point;
      case 2:
        return Qgis::WkbType::CompoundCurve;
      case 3:
        return Qgis::WkbType::Polygon;
      case 4:
        QgsDebugError( u"geometry collection type %1 unsupported"_s.arg( gtype ) );
        return Qgis::WkbType::Unknown;
      case 5:
        return Qgis::WkbType::MultiPoint;
      case 6:
        return Qgis::WkbType::MultiCurve;
      case 7:
        return Qgis::WkbType::MultiPolygon;
      default:
        QgsDebugError( u"gtype %1 unsupported"_s.arg( gtype ) );
        return Qgis::WkbType::Unknown;
    }
  }
  else if ( d == 3 )
  {
    switch ( t )
    {
      case 1:
        return Qgis::WkbType::PointZ;
      case 2:
        return Qgis::WkbType::CompoundCurveZ;
      case 3:
        return Qgis::WkbType::PolygonZ;
      case 4:
        QgsDebugError( u"geometry collection type %1 unsupported"_s.arg( gtype ) );
        return Qgis::WkbType::Unknown;
      case 5:
        return Qgis::WkbType::MultiPointZ;
      case 6:
        return Qgis::WkbType::MultiCurveZ;
      case 7:
        return Qgis::WkbType::MultiPolygonZ;
      default:
        QgsDebugError( u"gtype %1 unsupported"_s.arg( gtype ) );
        return Qgis::WkbType::Unknown;
    }
  }
  else
  {
    QgsDebugError( u"dimension of gtype %1 unsupported"_s.arg( gtype ) );
    return Qgis::WkbType::Unknown;
  }
}

Qgis::WkbType QgsOracleConn::wkbTypeFromGeomType( Qgis::GeometryType geomType )
{
  switch ( geomType )
  {
    case Qgis::GeometryType::Point:
      return Qgis::WkbType::Point;
    case Qgis::GeometryType::Line:
      return Qgis::WkbType::LineString;
    case Qgis::GeometryType::Polygon:
      return Qgis::WkbType::Polygon;
    case Qgis::GeometryType::Null:
      return Qgis::WkbType::NoGeometry;
    case Qgis::GeometryType::Unknown:
      return Qgis::WkbType::Unknown;
  }

  Q_ASSERT( !"unexpected geomType" );
  return Qgis::WkbType::Unknown;
}

QStringList QgsOracleConn::connectionList()
{
  QgsSettings settings;
  settings.beginGroup( u"/Oracle/connections"_s );
  return settings.childGroups();
}

void QgsOracleConn::deleteConnection( const QString &connName )
{
  QgsSettings settings;

  QString key = u"/Oracle/connections/"_s + connName;
  settings.remove( key + u"/host"_s );
  settings.remove( key + u"/port"_s );
  settings.remove( key + u"/database"_s );
  settings.remove( key + u"/schema"_s );
  settings.remove( key + u"/username"_s );
  settings.remove( key + u"/password"_s );
  settings.remove( key + u"/authcfg"_s );
  settings.remove( key + u"/dboptions"_s );
  settings.remove( key + u"/dbworkspace"_s );
  settings.remove( key + u"/userTablesOnly"_s );
  settings.remove( key + u"/geometryColumnsOnly"_s );
  settings.remove( key + u"/allowGeometrylessTables"_s );
  settings.remove( key + u"/estimatedMetadata"_s );
  settings.remove( key + u"/onlyExistingTypes"_s );
  settings.remove( key + u"/includeGeoAttributes"_s );
  settings.remove( key + u"/projectsInDatabase"_s );
  settings.remove( key + u"/saveUsername"_s );
  settings.remove( key + u"/savePassword"_s );
  settings.remove( key + u"/save"_s );
  settings.remove( key );
}

void QgsOracleConn::duplicateConnection( const QString &src, const QString &dst )
{
  const QString key( u"/Oracle/connections/"_s + src );
  const QString newKey( u"/Oracle/connections/"_s + dst );

  QgsSettings settings;
  settings.setValue( newKey + u"/host"_s, settings.value( key + u"/host"_s ).toString() );
  settings.setValue( newKey + u"/port"_s, settings.value( key + u"/port"_s ).toString() );
  settings.setValue( newKey + u"/database"_s, settings.value( key + u"/database"_s ).toString() );
  settings.setValue( newKey + u"/schema"_s, settings.value( key + u"/schema"_s ).toString() );
  settings.setValue( newKey + u"/username"_s, settings.value( key + u"/username"_s ).toString() );
  settings.setValue( newKey + u"/password"_s, settings.value( key + u"/password"_s ).toString() );
  settings.setValue( newKey + u"/authcfg"_s, settings.value( key + u"/authcfg"_s ).toString() );
  settings.setValue( newKey + u"/dboptions"_s, settings.value( key + u"/dboptions"_s ).toString() );
  settings.setValue( newKey + u"/dbworkspace"_s, settings.value( key + u"/dbworkspace"_s ).toString() );
  settings.setValue( newKey + u"/userTablesOnly"_s, settings.value( key + u"/userTablesOnly"_s ).toBool() );
  settings.setValue( newKey + u"/geometryColumnsOnly"_s, settings.value( key + u"/geometryColumnsOnly"_s ).toBool() );
  settings.setValue( newKey + u"/allowGeometrylessTables"_s, settings.value( key + u"/allowGeometrylessTables"_s ).toBool() );
  settings.setValue( newKey + u"/estimatedMetadata"_s, settings.value( key + u"/estimatedMetadata"_s ).toBool() );
  settings.setValue( newKey + u"/onlyExistingTypes"_s, settings.value( key + u"/onlyExistingTypes"_s ).toBool() );
  settings.setValue( newKey + u"/includeGeoAttributes"_s, settings.value( key + u"/includeGeoAttributes"_s ).toBool() );
  settings.setValue( newKey + u"/projectsInDatabase"_s, settings.value( key + u"/projectsInDatabase"_s ).toBool() );
  settings.setValue( newKey + u"/saveUsername"_s, settings.value( key + u"/saveUsername"_s ).toString() );
  settings.setValue( newKey + u"/savePassword"_s, settings.value( key + u"/savePassword"_s ).toString() );

  settings.sync();
}

QString QgsOracleConn::selectedConnection()
{
  QgsSettings settings;
  return settings.value( u"/Oracle/connections/selected"_s ).toString();
}

void QgsOracleConn::setSelectedConnection( const QString &name )
{
  QgsSettings settings;
  return settings.setValue( u"/Oracle/connections/selected"_s, name );
}

QgsDataSourceUri QgsOracleConn::connUri( const QString &connName )
{
  QgsDebugMsgLevel( u"theConnName = "_s + connName, 3 );

  QgsSettings settings;

  QString key = u"/Oracle/connections/"_s + connName;

  QString database = settings.value( key + u"/database"_s ).toString();

  QString host = settings.value( key + u"/host"_s ).toString();
  QString port = settings.value( key + u"/port"_s ).toString();
  if ( port.length() == 0 )
  {
    port = u"1521"_s;
  }

  QString username;
  QString password;
  if ( settings.value( key + u"/saveUsername"_s ).toString() == "true"_L1 )
  {
    username = settings.value( key + u"/username"_s ).toString();
  }

  if ( settings.value( key + u"/savePassword"_s ).toString() == "true"_L1 )
  {
    password = settings.value( key + u"/password"_s ).toString();
  }

  QgsDataSourceUri uri;
  uri.setConnection( host, port, database, username, password );

  bool useEstimatedMetadata = settings.value( key + u"/estimatedMetadata"_s, false ).toBool();
  uri.setUseEstimatedMetadata( useEstimatedMetadata );

  if ( !settings.value( key + u"/dboptions"_s ).toString().isEmpty() )
  {
    uri.setParam( u"dboptions"_s, settings.value( key + u"/dboptions"_s ).toString() );
  }
  if ( !settings.value( key + u"/dbworkspace"_s ).toString().isEmpty() )
  {
    uri.setParam( u"dbworkspace"_s, settings.value( key + u"/dbworkspace"_s ).toString() );
  }

  QString authcfg = settings.value( key + u"/authcfg"_s ).toString();
  if ( !authcfg.isEmpty() )
  {
    uri.setAuthConfigId( authcfg );
  }

  return uri;
}

bool QgsOracleConn::userTablesOnly( const QString &connName )
{
  QgsSettings settings;
  return settings.value( u"/Oracle/connections/"_s + connName + u"/userTablesOnly"_s, false ).toBool();
}

QString QgsOracleConn::schemaToRestrict( const QString &connName )
{
  QgsSettings settings;
  return settings.value( u"/Oracle/connections/"_s + connName + u"/schema"_s ).toString();
}

bool QgsOracleConn::geometryColumnsOnly( const QString &connName )
{
  QgsSettings settings;
  return settings.value( u"/Oracle/connections/"_s + connName + u"/geometryColumnsOnly"_s, true ).toBool();
}

bool QgsOracleConn::allowGeometrylessTables( const QString &connName )
{
  QgsSettings settings;
  return settings.value( u"/Oracle/connections/"_s + connName + u"/allowGeometrylessTables"_s, false ).toBool();
}

bool QgsOracleConn::allowProjectsInDatabase( const QString &connName )
{
  QgsSettings settings;
  return settings.value( u"/Oracle/connections/"_s + connName + u"/projectsInDatabase"_s, false ).toBool();
}

bool QgsOracleConn::estimatedMetadata( const QString &connName )
{
  QgsSettings settings;
  return settings.value( u"/Oracle/connections/"_s + connName + u"/estimatedMetadata"_s, false ).toBool();
}

bool QgsOracleConn::onlyExistingTypes( const QString &connName )
{
  QgsSettings settings;
  return settings.value( u"/Oracle/connections/"_s + connName + u"/onlyExistingTypes"_s, false ).toBool();
}

QString QgsOracleConn::databaseName( const QString &database, const QString &host, const QString &port )
{
  QString db;

  if ( !host.isEmpty() )
  {
    db += host;

    if ( !port.isEmpty() && port != "1521"_L1 )
    {
      db += u":%1"_s.arg( port );
    }

    if ( !database.isEmpty() )
    {
      db += "/" + database;
    }
  }
  else if ( !database.isEmpty() )
  {
    db = database;
  }

  return db;
}

bool QgsOracleConn::hasSpatial()
{
  QMutexLocker locker( &mLock );
  if ( mHasSpatial == -1 )
  {
    QSqlQuery qry( mDatabase );
    const QString sql { u"SELECT 1 FROM v$option WHERE parameter='Spatial' AND value='TRUE'"_s };
    mHasSpatial = LoggedExecPrivate( u"QgsOracleConn"_s, qry, sql, QVariantList() ) && qry.next();
  }

  return mHasSpatial;
}

int QgsOracleConn::version()
{
  QSqlQuery qry( mDatabase );
  const QString sql = u"SELECT VERSION FROM PRODUCT_COMPONENT_VERSION"_s;
  if ( LoggedExecPrivate( u"QgsOracleConn"_s, qry, sql, QVariantList() ) && qry.next() )
  {
    return qry.value( 0 ).toString().split( '.' ).at( 0 ).toInt();
  }
  else
  {
    const QString error { tr( "Unable to execute the query.\nThe error message from the database was:\n%1.\nSQL: %2" )
                            .arg( qry.lastError().text() )
                            .arg( qry.lastQuery() ) };
    QgsMessageLog::logMessage( error, tr( "Oracle" ) );
    return -1;
  }
}


QString QgsOracleConn::currentUser()
{
  QMutexLocker locker( &mLock );
  if ( mCurrentUser.isNull() )
  {
    QSqlQuery qry( mDatabase );
    const QString sql { u"SELECT user FROM dual"_s };
    if ( LoggedExecPrivate( u"QgsOracleConn"_s, qry, sql, QVariantList() ) && qry.next() )
    {
      mCurrentUser = qry.value( 0 ).toString();
    }
  }

  return mCurrentUser;
}

QList<QgsVectorDataProvider::NativeType> QgsOracleConn::nativeTypes()
{
  return QList<QgsVectorDataProvider::NativeType>()
         // integer types
         << QgsVectorDataProvider::NativeType( tr( "Whole Number" ), "number(10,0)", QMetaType::Type::Int )
         << QgsVectorDataProvider::NativeType( tr( "Whole Big Number" ), "number(20,0)", QMetaType::Type::LongLong )
         << QgsVectorDataProvider::NativeType( tr( "Decimal Number (numeric)" ), "number", QMetaType::Type::Double, 1, 38, 0, 38 )
         << QgsVectorDataProvider::NativeType( tr( "Decimal Number (decimal)" ), "double precision", QMetaType::Type::Double )

         // floating point
         << QgsVectorDataProvider::NativeType( tr( "Decimal Number (real)" ), "binary_float", QMetaType::Type::Double )
         << QgsVectorDataProvider::NativeType( tr( "Decimal Number (double)" ), "binary_double", QMetaType::Type::Double )

         // string types
         << QgsVectorDataProvider::NativeType( tr( "Text, fixed length (char)" ), "CHAR", QMetaType::Type::QString, 1, 255 )
         << QgsVectorDataProvider::NativeType( tr( "Text, limited variable length (varchar2)" ), "VARCHAR2", QMetaType::Type::QString, 1, 255 )
         << QgsVectorDataProvider::NativeType( tr( "Text, unlimited length (long)" ), "LONG", QMetaType::Type::QString )

         // date type
         << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QDate ), "DATE", QMetaType::Type::QDate, 38, 38, 0, 0 )
         << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QDateTime ), "TIMESTAMP(6)", QMetaType::Type::QDateTime, 38, 38, 6, 6 );
}

QString QgsOracleConn::getSpatialIndexName( const QString &ownerName, const QString &tableName, const QString &geometryColumn, bool &isValid )
{
  QString name;

  QSqlQuery qry( mDatabase );

  if ( LoggedExecPrivate( u"QgsOracleConn"_s, qry, QStringLiteral( "SELECT i.index_name,i.domidx_opstatus"
                                                                   " FROM all_indexes i"
                                                                   " JOIN all_ind_columns c ON i.owner=c.index_owner AND i.index_name=c.index_name AND c.column_name=?"
                                                                   " WHERE i.table_owner=? AND i.table_name=? AND i.ityp_owner='MDSYS' AND i.ityp_name='SPATIAL_INDEX'" ),
                          QVariantList() << geometryColumn << ownerName << tableName ) )
  {
    if ( qry.next() )
    {
      name = qry.value( 0 ).toString();
      if ( qry.value( 1 ).toString() != "VALID" )
      {
        QgsMessageLog::logMessage( tr( "Invalid spatial index %1 on column %2.%3.%4 found - expect poor performance." ).arg( name ).arg( ownerName ).arg( tableName ).arg( geometryColumn ), tr( "Oracle" ) );
        isValid = false;
      }
      else
      {
        QgsDebugMsgLevel( u"Valid spatial index %1 found"_s.arg( name ), 2 );
        isValid = true;
      }
    }
    else
    {
      isValid = false;
    }
  }
  else
  {
    const QString error { tr( "Probing for spatial index on column %1.%2.%3 failed [%4]" )
                            .arg( ownerName )
                            .arg( tableName )
                            .arg( geometryColumn )
                            .arg( qry.lastError().text() ) };
    QgsMessageLog::logMessage( error, tr( "Oracle" ) );

    isValid = false;
  }

  return name;
}

QString QgsOracleConn::createSpatialIndex( const QString &ownerName, const QString &tableName, const QString &geometryColumn )
{
  QSqlQuery qry( mDatabase );

  int n = 0;
  const QString sql { u"SELECT coalesce(substr(max(index_name),10),'0') FROM all_indexes WHERE index_name LIKE 'QGIS_IDX_%' ESCAPE '#' ORDER BY index_name"_s };

  if ( LoggedExecPrivate( u"QgsOracleConn"_s, qry, sql, QVariantList() ) && qry.next() )
  {
    n = qry.value( 0 ).toInt() + 1;
  }

  const QString sql2 { u"CREATE INDEX QGIS_IDX_%1 ON %2.%3(%4) INDEXTYPE IS MDSYS.SPATIAL_INDEX PARALLEL"_s
                         .arg( n, 10, 10, QChar( '0' ) )
                         .arg( quotedIdentifier( ownerName ) )
                         .arg( quotedIdentifier( tableName ) )
                         .arg( quotedIdentifier( geometryColumn ) ) };
  if ( !LoggedExecPrivate( u"QgsOracleConn"_s, qry, sql2, QVariantList() ) )
  {
    const QString error { tr( "Creation spatial index failed.\nSQL: %1\nError: %2" )
                            .arg( qry.lastQuery() )
                            .arg( qry.lastError().text() ) };
    QgsMessageLog::logMessage( error, tr( "Oracle" ) );
    return QString();
  }

  return QString( "QGIS_IDX_%1" ).arg( n, 10, 10, QChar( '0' ) );
}

QStringList QgsOracleConn::getPrimaryKeys( const QString &ownerName, const QString &tableName )
{
  QSqlQuery qry( mDatabase );

  QStringList result;

  if ( !LoggedExecPrivate( u"QgsOracleConn"_s, qry, QStringLiteral( "SELECT column_name"
                                                                    " FROM all_cons_columns a"
                                                                    " JOIN all_constraints b ON a.constraint_name=b.constraint_name AND a.owner=b.owner"
                                                                    " WHERE b.constraint_type='P' AND b.owner=? AND b.table_name=?" ),
                           QVariantList() << ownerName << tableName ) )
  {
    const QString error { tr( "Unable to execute the query.\nThe error message from the database was:\n%1.\nSQL: %2" )
                            .arg( qry.lastError().text() )
                            .arg( qry.lastQuery() ) };
    QgsMessageLog::logMessage( error, tr( "Oracle" ) );
    return result;
  }

  while ( qry.next() )
  {
    QString name = qry.value( 0 ).toString();
    result << name;
  }

  return result;
}


QgsPoolOracleConn::QgsPoolOracleConn( const QString &connInfo )
  : mConn( QgsOracleConnPool::instance()->acquireConnection( connInfo ) )
{
}

QgsPoolOracleConn::~QgsPoolOracleConn()
{
  if ( mConn )
    QgsOracleConnPool::instance()->releaseConnection( mConn );
}

// vim: sw=2 :
