/***************************************************************************
                             qgsoraclestablecache.cpp
                              -------------------
begin                : April 2014
copyright            : (C) 2014 by Martin Dobias
email                : wonder.sk at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsoracletablecache.h"

#include <sqlite3.h>

#include "qgsapplication.h"

#include <QDir>



static bool _executeSqliteStatement( sqlite3* db, const QString& sql )
{
  sqlite3_stmt* stmt;
  if ( sqlite3_prepare_v2( db, sql.toUtf8().data(), -1, &stmt, nullptr ) != SQLITE_OK )
    return false;

  return sqlite3_step( stmt ) == SQLITE_DONE;
}

static bool _removeFromCache( sqlite3* db, const QString& connName )
{
  QString tblName = "oracle_" + connName;

  QString sqlDeleteFromMeta = QString( "DELETE FROM meta_oracle WHERE conn = %1" ).arg( QgsOracleConn::quotedValue( connName ) );
  bool res1 = _executeSqliteStatement( db, sqlDeleteFromMeta );

  QString sqlDropTable = QString( "DROP TABLE IF EXISTS %1" ).arg( QgsOracleConn::quotedIdentifier( tblName ) );
  bool res2 = _executeSqliteStatement( db, sqlDropTable );

  return res1 && res2;
}


static sqlite3* _openCacheDatabase()
{
  sqlite3* database;
  if ( sqlite3_open_v2( QgsOracleTableCache::cacheDatabaseFilename().toUtf8().data(), &database, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0 ) != SQLITE_OK )
    return 0;

  if ( !_executeSqliteStatement( database, "CREATE TABLE IF NOT EXISTS meta_oracle(conn text primary_key, flags int)" ) )
  {
    sqlite3_close( database );
    return 0;
  }

  return database;
}


static bool _hasCache( sqlite3* db, const QString& connName, int flags = -1 ) // flags == -1 implies any flags
{
  QString sqlCacheForConn = QString( "SELECT * FROM meta_oracle WHERE conn = %1" ).arg( QgsOracleConn::quotedValue( connName ) );
  if ( flags >= 0 )
    sqlCacheForConn.append( QString( " AND flags = %1" ).arg( flags ) );

  char **results;
  int rows, columns;
  char *errMsg = nullptr;
  bool res = sqlite3_get_table( db, sqlCacheForConn.toUtf8(), &results, &rows, &columns, &errMsg ) == SQLITE_OK;
  bool hasCache = ( res && rows == 1 );
  sqlite3_free_table( results );

  return hasCache;
}


static bool _renameConnectionInCache( sqlite3* db, const QString& oldName, const QString& newName )
{
  if ( !_hasCache( db, oldName ) )
    return true;

  QString sql1 = QString( "ALTER TABLE %1 RENAME TO %2" ).arg( QgsOracleConn::quotedIdentifier( "oracle_" + oldName ) ).arg( QgsOracleConn::quotedIdentifier( "oracle_" + newName ) );
  bool res1 = _executeSqliteStatement( db, sql1 );

  QString sql2 = QString( "UPDATE meta_oracle SET conn = %1 WHERE conn = %2" ).arg( QgsOracleConn::quotedIdentifier( newName ) ).arg( QgsOracleConn::quotedIdentifier( oldName ) );
  bool res2 = _executeSqliteStatement( db, sql2 );

  return res1 && res2;
}



QString QgsOracleTableCache::cacheDatabaseFilename()
{
  return QgsApplication::qgisSettingsDirPath() + QDir::separator() + "data_sources_cache.db";
}

bool QgsOracleTableCache::hasCache( const QString& connName, CacheFlags flags )
{
  sqlite3* db = _openCacheDatabase();
  if ( !db )
    return false;

  bool hasCache = _hasCache( db, connName, ( int ) flags );

  sqlite3_close( db );
  return hasCache;
}


bool QgsOracleTableCache::saveToCache( const QString& connName, CacheFlags flags, const QVector<QgsOracleLayerProperty>& layers )
{
  sqlite3* db = _openCacheDatabase();
  if ( !db )
    return false;

  QString tblNameRaw = "oracle_" + connName;
  QString tblName = QgsOracleConn::quotedIdentifier( tblNameRaw );

  // recreate the cache table

  if ( !_removeFromCache( db, connName ) )
  {
    sqlite3_close( db );
    return false;
  }

  QString sqlCreateTable = QString( "CREATE TABLE %1 (ownername text, tablename text, geometrycolname text, isview int, sql text, pkcols text, geomtypes text, geomsrids text)" ).arg( tblName );
  QString sqlInsertToMeta = QString( "INSERT INTO meta_oracle VALUES (%1, %2)" ).arg( QgsOracleConn::quotedValue( connName ) ).arg(( int ) flags );

  bool res1 = _executeSqliteStatement( db, sqlCreateTable );
  bool res2 = _executeSqliteStatement( db, sqlInsertToMeta );
  if ( !res1 || !res2 )
  {
    sqlite3_close( db );
    return false;
  }

  // insert data

  _executeSqliteStatement( db, "BEGIN" );

  QString sqlInsert = QString( "INSERT INTO %1 VALUES(?,?,?,?,?,?,?,?)" ).arg( tblName );
  sqlite3_stmt* stmtInsert;
  if ( sqlite3_prepare_v2( db, sqlInsert.toUtf8().data(), -1, &stmtInsert, 0 ) != SQLITE_OK )
  {
    sqlite3_close( db );
    return false;
  }

  bool insertOk = true;
  Q_FOREACH ( const QgsOracleLayerProperty& item, layers )
  {
    sqlite3_bind_text( stmtInsert, 1, item.ownerName.toUtf8().data(), -1, SQLITE_TRANSIENT );
    sqlite3_bind_text( stmtInsert, 2, item.tableName.toUtf8().data(), -1, SQLITE_TRANSIENT );
    sqlite3_bind_text( stmtInsert, 3, item.geometryColName.toUtf8().data(), -1, SQLITE_TRANSIENT );
    sqlite3_bind_int( stmtInsert, 4, item.isView );
    sqlite3_bind_text( stmtInsert, 5, item.sql.toUtf8().data(), -1, SQLITE_TRANSIENT );

    sqlite3_bind_text( stmtInsert, 6, item.pkCols.join( "," ).toUtf8().data(), -1, SQLITE_TRANSIENT );

    QStringList geomTypes;
    Q_FOREACH ( QGis::WkbType geomType, item.types )
      geomTypes.append( QString::number( static_cast<ulong>( geomType ) ) );
    sqlite3_bind_text( stmtInsert, 7, geomTypes.join( "," ).toUtf8().data(), -1, SQLITE_TRANSIENT );

    QStringList geomSrids;
    Q_FOREACH ( int geomSrid, item.srids )
      geomSrids.append( QString::number( geomSrid ) );
    sqlite3_bind_text( stmtInsert, 8, geomSrids.join( "," ).toUtf8().data(), -1, SQLITE_TRANSIENT );

    if ( sqlite3_step( stmtInsert ) != SQLITE_DONE )
      insertOk = false;

    sqlite3_reset( stmtInsert );
  }

  sqlite3_finalize( stmtInsert );

  _executeSqliteStatement( db, "COMMIT" );

  sqlite3_close( db );
  return insertOk;
}


bool QgsOracleTableCache::loadFromCache( const QString& connName, CacheFlags flags, QVector<QgsOracleLayerProperty>& layers )
{
  sqlite3* db = _openCacheDatabase();
  if ( !db )
    return false;

  if ( !_hasCache( db, connName, ( int ) flags ) )
    return false;

  sqlite3_stmt* stmt;
  QString sql = QString( "SELECT * FROM %1" ).arg( QgsOracleConn::quotedIdentifier( "oracle_" + connName ) );
  if ( sqlite3_prepare_v2( db, sql.toUtf8().data(), -1, &stmt, nullptr ) != SQLITE_OK )
  {
    sqlite3_close( db );
    return false;
  }

  while ( sqlite3_step( stmt ) == SQLITE_ROW )
  {
    QgsOracleLayerProperty layer;
    layer.ownerName = QString::fromUtf8(( const char * ) sqlite3_column_text( stmt, 0 ) );
    layer.tableName = QString::fromUtf8(( const char * ) sqlite3_column_text( stmt, 1 ) );
    layer.geometryColName = QString::fromUtf8(( const char * ) sqlite3_column_text( stmt, 2 ) );
    layer.isView = sqlite3_column_int( stmt, 3 );
    layer.sql = QString::fromUtf8(( const char* ) sqlite3_column_text( stmt, 4 ) );

    QString pkCols = QString::fromUtf8(( const char* ) sqlite3_column_text( stmt, 5 ) );
    layer.pkCols = pkCols.split( ",", QString::SkipEmptyParts );

    QString geomTypes = QString::fromUtf8(( const char* ) sqlite3_column_text( stmt, 6 ) );
    Q_FOREACH ( QString geomType, geomTypes.split( ",", QString::SkipEmptyParts ) )
      layer.types.append( static_cast<QGis::WkbType>( geomType.toInt() ) );

    QString geomSrids = QString::fromUtf8(( const char* ) sqlite3_column_text( stmt, 7 ) );
    Q_FOREACH ( QString geomSrid, geomSrids.split( ",", QString::SkipEmptyParts ) )
      layer.srids.append( geomSrid.toInt() );

    layers.append( layer );
  }

  sqlite3_finalize( stmt );

  sqlite3_close( db );
  return true;
}


bool QgsOracleTableCache::removeFromCache( const QString& connName )
{
  sqlite3* db = _openCacheDatabase();
  if ( !db )
    return false;

  bool res = _removeFromCache( db, connName );

  sqlite3_close( db );
  return res;
}


bool QgsOracleTableCache::renameConnectionInCache( const QString& oldName, const QString& newName )
{
  sqlite3* db = _openCacheDatabase();
  if ( !db )
    return false;

  bool res = _renameConnectionInCache( db, oldName, newName );

  sqlite3_close( db );
  return res;
}


#if 0
// testing routine - ideally it should be a unit test
void _testTableCache()
{
  QString connName = "local";
  QVector<QgsOracleLayerProperty> layers;

  // fetch

  QgsOracleConn* c = QgsOracleConnectionPool::instance()->acquireConnection( QgsOracleConn::toPoolName( QgsOracleConn::connUri( connName ) ) );
  if ( !c )
    return;

  c->supportedLayers( layers, true );

  bool useEstimated = true;
  bool onlyExisting = QgsOracleConn::onlyExistingTypes( connName );

  for ( QVector<QgsOracleLayerProperty>::iterator it = layers.begin(), end = layers.end(); it != end; ++it )
  {
    QgsOracleLayerProperty &layerProperty = *it;
    c->retrieveLayerTypes( layerProperty, useEstimated, onlyExisting );
  }

  QgsOracleConnPool::instance()->releaseConnection( c );

  // save

  QgsOracleTableCache::CacheFlags flags = QgsOracleTableCache::UseEstimatedTableMetadata | QgsOracleTableCache::OnlyExistingGeometryTypes;
  QgsOracleTableCache::saveToCache( connName, flags, layers );

  // load

  QVector<QgsOracleLayerProperty> layersLoaded;
  QgsOracleTableCache::loadFromCache( connName, flags, layersLoaded );

  // compare

  Q_FOREACH ( const QgsOracleLayerProperty& item, layers )
    qDebug( "== %s %s", item.tableName.toAscii().data(), item.geometryColName.toAscii().data() );

  Q_FOREACH ( const QgsOracleLayerProperty& item, layersLoaded )
    qDebug( "++ %s %s", item.tableName.toAscii().data(), item.geometryColName.toAscii().data() );

  Q_ASSERT( layers == layersLoaded );
}
#endif
