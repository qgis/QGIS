/***************************************************************************
  qgsosmdatabase.cpp
  --------------------------------------
  Date                 : January 2013
  Copyright            : (C) 2013 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsosmdatabase.h"

#include <spatialite.h>

#include "qgsgeometry.h"
#include "qgslogger.h"



QgsOSMDatabase::QgsOSMDatabase( const QString& dbFileName )
    : mDbFileName( dbFileName )
    , mDatabase( 0 )
    , mStmtNode( 0 )
    , mStmtNodeTags( 0 )
    , mStmtWay( 0 )
    , mStmtWayNode( 0 )
    , mStmtWayNodePoints( 0 )
    , mStmtWayTags( 0 )
{

}

QgsOSMDatabase::~QgsOSMDatabase()
{
  if ( isOpen() )
    close();
}

bool QgsOSMDatabase::isOpen() const
{
  return mDatabase != 0;
}


bool QgsOSMDatabase::open()
{
  // load spatialite extension
  spatialite_init( 0 );

  // open database
  int res = sqlite3_open_v2( mDbFileName.toUtf8().data(), &mDatabase, SQLITE_OPEN_READWRITE, 0 );
  if ( res != SQLITE_OK )
  {
    mError = QString( "Failed to open database [%1]: %2" ).arg( res ).arg( mDbFileName );
    close();
    return false;
  }

  if ( !prepareStatements() )
  {
    close();
    return false;
  }

  return true;
}


void QgsOSMDatabase::deleteStatement( sqlite3_stmt*& stmt )
{
  if ( stmt )
  {
    sqlite3_finalize( stmt );
    stmt = 0;
  }
}


bool QgsOSMDatabase::close()
{
  deleteStatement( mStmtNode );
  deleteStatement( mStmtNodeTags );
  deleteStatement( mStmtWay );
  deleteStatement( mStmtWayNode );
  deleteStatement( mStmtWayNodePoints );
  deleteStatement( mStmtWayTags );

  Q_ASSERT( mStmtNode == 0 );

  // close database
  if ( sqlite3_close( mDatabase ) != SQLITE_OK )
  {
    //mError = ( char * ) "Closing SQLite3 database failed.";
    //return false;
  }
  mDatabase = 0;
  return true;
}


int QgsOSMDatabase::runCountStatement( const char* sql ) const
{
  sqlite3_stmt* stmt;
  int res = sqlite3_prepare_v2( mDatabase, sql, -1, &stmt, 0 );
  if ( res != SQLITE_OK )
    return -1;

  res = sqlite3_step( stmt );
  if ( res != SQLITE_ROW )
    return -1;

  int count = sqlite3_column_int( stmt, 0 );
  sqlite3_finalize( stmt );
  return count;
}


int QgsOSMDatabase::countNodes() const
{
  return runCountStatement( "SELECT count(*) FROM nodes" );
}

int QgsOSMDatabase::countWays() const
{
  return runCountStatement( "SELECT count(*) FROM ways" );
}


QgsOSMNodeIterator QgsOSMDatabase::listNodes() const
{
  return QgsOSMNodeIterator( mDatabase );
}

QgsOSMWayIterator QgsOSMDatabase::listWays() const
{
  return QgsOSMWayIterator( mDatabase );
}


QgsOSMNode QgsOSMDatabase::node( QgsOSMId id ) const
{
  // bind the way identifier
  sqlite3_bind_int64( mStmtNode, 1, id );

  if ( sqlite3_step( mStmtNode ) != SQLITE_ROW )
  {
    //QgsDebugMsg( "Cannot get number of way members." );
    sqlite3_reset( mStmtNode );
    return QgsOSMNode();
  }

  double lon = sqlite3_column_double( mStmtNode, 0 );
  double lat = sqlite3_column_double( mStmtNode, 1 );

  QgsOSMNode node( id, QgsPoint( lon, lat ) );

  sqlite3_reset( mStmtNode );
  return node;
}

QgsOSMTags QgsOSMDatabase::tags( bool way, QgsOSMId id ) const
{
  QgsOSMTags t;

  sqlite3_stmt* stmtTags = way ? mStmtWayTags : mStmtNodeTags;

  sqlite3_bind_int64( stmtTags, 1, id );

  while ( sqlite3_step( stmtTags ) == SQLITE_ROW )
  {
    QString k = QString::fromUtf8(( const char* ) sqlite3_column_text( stmtTags, 0 ) );
    QString v = QString::fromUtf8(( const char* ) sqlite3_column_text( stmtTags, 1 ) );
    t.insert( k, v );
  }

  sqlite3_reset( stmtTags );
  return t;
}


QList<QgsOSMTagCountPair> QgsOSMDatabase::usedTags( bool ways ) const
{
  QList<QgsOSMTagCountPair> pairs;

  QString sql = QString( "SELECT k, count(k) FROM %1_tags GROUP BY k" ).arg( ways ? "ways" : "nodes" );

  sqlite3_stmt* stmt;
  if ( sqlite3_prepare_v2( mDatabase, sql.toUtf8().data(), -1, &stmt, 0 ) != SQLITE_OK )
    return pairs;

  while ( sqlite3_step( stmt ) == SQLITE_ROW )
  {
    QString k = QString::fromUtf8(( const char* ) sqlite3_column_text( stmt, 0 ) );
    int count = sqlite3_column_int( stmt, 1 );
    pairs.append( qMakePair( k, count ) );
  }

  sqlite3_finalize( stmt );
  return pairs;
}



QgsOSMWay QgsOSMDatabase::way( QgsOSMId id ) const
{
  // TODO: first check that way exists!
  // mStmtWay

  // bind the way identifier
  sqlite3_bind_int64( mStmtWayNode, 1, id );

  QList<QgsOSMId> nodes;

  while ( sqlite3_step( mStmtWayNode ) == SQLITE_ROW )
  {
    QgsOSMId nodeId = sqlite3_column_int64( mStmtWayNode, 0 );
    nodes.append( nodeId );
  }

  sqlite3_reset( mStmtWayNode );

  if ( nodes.isEmpty() )
    return QgsOSMWay();

  return QgsOSMWay( id, nodes );
}

/*
OSMRelation OSMDatabase::relation( OSMId id ) const
{
  // todo
  Q_UNUSED(id);
  return OSMRelation();
}*/

QgsPolyline QgsOSMDatabase::wayPoints( QgsOSMId id ) const
{
  QgsPolyline points;

  // bind the way identifier
  sqlite3_bind_int64( mStmtWayNodePoints, 1, id );

  while ( sqlite3_step( mStmtWayNodePoints ) == SQLITE_ROW )
  {
    if ( sqlite3_column_type( mStmtWayNodePoints, 0 ) == SQLITE_NULL )
      return QgsPolyline(); // missing some nodes
    double lon = sqlite3_column_double( mStmtWayNodePoints, 0 );
    double lat = sqlite3_column_double( mStmtWayNodePoints, 1 );
    points.append( QgsPoint( lon, lat ) );
  }

  sqlite3_reset( mStmtWayNodePoints );
  return points;
}



bool QgsOSMDatabase::prepareStatements()
{
  const char* sql[] =
  {
    "SELECT lon,lat FROM nodes WHERE id=?",
    "SELECT k,v FROM nodes_tags WHERE id=?",
    "SELECT id FROM ways WHERE id=?",
    "SELECT node_id FROM ways_nodes WHERE way_id=? ORDER BY way_pos",
    "SELECT n.lon, n.lat FROM ways_nodes wn LEFT JOIN nodes n ON wn.node_id = n.id WHERE wn.way_id=? ORDER BY wn.way_pos",
    "SELECT k,v FROM ways_tags WHERE id=?"
  };
  sqlite3_stmt** sqlite[] =
  {
    &mStmtNode,
    &mStmtNodeTags,
    &mStmtWay,
    &mStmtWayNode,
    &mStmtWayNodePoints,
    &mStmtWayTags
  };
  int count = sizeof( sql ) / sizeof( const char* );
  Q_ASSERT( count == sizeof( sqlite ) / sizeof( sqlite3_stmt** ) );

  for ( int i = 0; i < count; ++i )
  {
    if ( sqlite3_prepare_v2( mDatabase, sql[i], -1, sqlite[i], 0 ) != SQLITE_OK )
    {
      const char* errMsg = sqlite3_errmsg( mDatabase ); // does not require free
      mError = QString( "Error preparing SQL command:\n%1\nSQL:\n%2" )
               .arg( QString::fromUtf8( errMsg ) ).arg( QString::fromUtf8( sql[i] ) );
      return false;
    }
  }

  return true;
}



bool QgsOSMDatabase::exportSpatiaLite( ExportType type, const QString& tableName, const QStringList& tagKeys )
{
  mError.clear();

  // create SpatiaLite table

  QString geometryType;
  if ( type == Point ) geometryType = "POINT";
  else if ( type == Polyline ) geometryType = "LINESTRING";
  else if ( type == Polygon ) geometryType = "POLYGON";
  else Q_ASSERT( false && "Unknown export type" );

  if ( !createSpatialTable( tableName, geometryType, tagKeys ) )
    return false;

  // import data

  int retX = sqlite3_exec( mDatabase, "BEGIN", NULL, NULL, 0 );
  Q_ASSERT( retX == SQLITE_OK );
  Q_UNUSED( retX );

  if ( type == Polyline || type == Polygon )
    exportSpatiaLiteWays( type == Polygon, tableName, tagKeys );
  else if ( type == Point )
    exportSpatiaLiteNodes( tableName, tagKeys );
  else
    Q_ASSERT( false && "Unknown export type" );

  int retY = sqlite3_exec( mDatabase, "COMMIT", NULL, NULL, 0 );
  Q_ASSERT( retY == SQLITE_OK );
  Q_UNUSED( retY );

  if ( !createSpatialIndex( tableName ) )
    return false;

  return mError.isEmpty();
}


bool QgsOSMDatabase::createSpatialTable( const QString& tableName, const QString& geometryType, const QStringList& tagKeys )
{
  QString sqlCreateTable = QString( "CREATE TABLE %1 (id INTEGER PRIMARY KEY" ).arg( quotedIdentifier( tableName ) );
  for ( int i = 0; i < tagKeys.count(); ++i )
    sqlCreateTable += QString( ", %1 TEXT" ).arg( quotedIdentifier( tagKeys[i] ) );
  sqlCreateTable += ")";

  char *errMsg = NULL;
  int ret = sqlite3_exec( mDatabase, sqlCreateTable.toUtf8().constData(), NULL, NULL, &errMsg );
  if ( ret != SQLITE_OK )
  {
    mError = "Unable to create table:\n" + QString::fromUtf8( errMsg );
    sqlite3_free( errMsg );
    return false;
  }

  QString sqlAddGeomColumn = QString( "SELECT AddGeometryColumn(%1, 'geometry', 4326, %2, 'XY')" )
                             .arg( quotedValue( tableName ) )
                             .arg( quotedValue( geometryType ) );
  ret = sqlite3_exec( mDatabase, sqlAddGeomColumn.toUtf8().constData(), NULL, NULL, &errMsg );
  if ( ret != SQLITE_OK )
  {
    mError = "Unable to add geometry column:\n" + QString::fromUtf8( errMsg );
    sqlite3_free( errMsg );
    return false;
  }

  return true;
}


bool QgsOSMDatabase::createSpatialIndex( const QString& tableName )
{
  QString sqlSpatialIndex = QString( "SELECT CreateSpatialIndex(%1, 'geometry')" ).arg( quotedValue( tableName ) );
  char *errMsg = NULL;
  int ret = sqlite3_exec( mDatabase, sqlSpatialIndex.toUtf8().constData(), NULL, NULL, &errMsg );
  if ( ret != SQLITE_OK )
  {
    mError = "Unable to create spatial index:\n" + QString::fromUtf8( errMsg );
    sqlite3_free( errMsg );
    return false;
  }

  return true;
}


void QgsOSMDatabase::exportSpatiaLiteNodes( const QString& tableName, const QStringList& tagKeys )
{
  QString sqlInsertPoint = QString( "INSERT INTO %1 VALUES (?" ).arg( quotedIdentifier( tableName ) );
  for ( int i = 0; i < tagKeys.count(); ++i )
    sqlInsertPoint += QString( ",?" );
  sqlInsertPoint += ", GeomFromWKB(?, 4326))";
  sqlite3_stmt* stmtInsert;
  if ( sqlite3_prepare_v2( mDatabase, sqlInsertPoint.toUtf8().constData(), -1, &stmtInsert, 0 ) != SQLITE_OK )
  {
    mError = "Prepare SELECT FROM nodes failed.";
    return;
  }

  QgsOSMNodeIterator nodes = listNodes();
  QgsOSMNode n;
  while (( n = nodes.next() ).isValid() )
  {
    QgsOSMTags t = tags( false, n.id() );

    // skip untagged nodes: probably they form a part of ways
    if ( t.count() == 0 )
      continue;

    QgsGeometry* geom = QgsGeometry::fromPoint( n.point() );
    int col = 0;
    sqlite3_bind_int64( stmtInsert, ++col, n.id() );

    // tags
    for ( int i = 0; i < tagKeys.count(); ++i )
    {
      if ( t.contains( tagKeys[i] ) )
        sqlite3_bind_text( stmtInsert, ++col, t.value( tagKeys[i] ).toUtf8().constData(), -1, SQLITE_TRANSIENT );
      else
        sqlite3_bind_null( stmtInsert, ++col );
    }

    sqlite3_bind_blob( stmtInsert, ++col, geom->asWkb(), ( int ) geom->wkbSize(), SQLITE_STATIC );

    int insertRes = sqlite3_step( stmtInsert );
    if ( insertRes != SQLITE_DONE )
    {
      mError = QString( "Error inserting node %1 [%2]" ).arg( n.id() ).arg( insertRes );
      break;
    }

    sqlite3_reset( stmtInsert );
    sqlite3_clear_bindings( stmtInsert );
    delete geom;
  }

  sqlite3_finalize( stmtInsert );
}


void QgsOSMDatabase::exportSpatiaLiteWays( bool closed, const QString& tableName, const QStringList& tagKeys )
{
  Q_UNUSED( tagKeys );

  QString sqlInsertLine = QString( "INSERT INTO %1 VALUES (?" ).arg( quotedIdentifier( tableName ) );
  for ( int i = 0; i < tagKeys.count(); ++i )
    sqlInsertLine += QString( ",?" );
  sqlInsertLine += ", GeomFromWKB(?, 4326))";
  sqlite3_stmt* stmtInsert;
  if ( sqlite3_prepare_v2( mDatabase, sqlInsertLine.toUtf8().constData(), -1, &stmtInsert, 0 ) != SQLITE_OK )
  {
    mError = "Prepare SELECT FROM ways failed.";
    return;
  }

  QgsOSMWayIterator ways = listWays();
  QgsOSMWay w;
  while (( w = ways.next() ).isValid() )
  {
    QgsOSMTags t = tags( true, w.id() );

    QgsPolyline polyline = wayPoints( w.id() );

    if ( polyline.count() < 2 )
      continue; // invalid way

    bool isArea = ( polyline.first() == polyline.last() ); // closed way?
    // some closed ways are not really areas
    if ( isArea && ( t.contains( "highway" ) || t.contains( "barrier" ) ) )
    {
      if ( t.value( "area" ) != "yes" ) // even though "highway" is line by default, "area"="yes" may override that
        isArea = false;
    }

    if ( closed != isArea )
      continue; // skip if it's not what we're looking for

    QgsGeometry* geom = closed ? QgsGeometry::fromPolygon( QgsPolygon() << polyline ) : QgsGeometry::fromPolyline( polyline );
    int col = 0;
    sqlite3_bind_int64( stmtInsert, ++col, w.id() );

    // tags
    for ( int i = 0; i < tagKeys.count(); ++i )
    {
      if ( t.contains( tagKeys[i] ) )
        sqlite3_bind_text( stmtInsert, ++col, t.value( tagKeys[i] ).toUtf8().constData(), -1, SQLITE_TRANSIENT );
      else
        sqlite3_bind_null( stmtInsert, ++col );
    }

    sqlite3_bind_blob( stmtInsert, ++col, geom->asWkb(), ( int ) geom->wkbSize(), SQLITE_STATIC );

    int insertRes = sqlite3_step( stmtInsert );
    if ( insertRes != SQLITE_DONE )
    {
      mError = QString( "Error inserting way %1 [%2]" ).arg( w.id() ).arg( insertRes );
      break;
    }

    sqlite3_reset( stmtInsert );
    sqlite3_clear_bindings( stmtInsert );
    delete geom;
  }

  sqlite3_finalize( stmtInsert );
}



QString QgsOSMDatabase::quotedIdentifier( QString id )
{
  id.replace( "\"", "\"\"" );
  return QString( "\"%1\"" ).arg( id );
}

QString QgsOSMDatabase::quotedValue( QString value )
{
  if ( value.isNull() )
    return "NULL";

  value.replace( "'", "''" );
  return QString( "'%1'" ).arg( value );
}

///////////////////////////////////


QgsOSMNodeIterator::QgsOSMNodeIterator( sqlite3* handle )
    : mStmt( 0 )
{
  const char* sql = "SELECT id,lon,lat FROM nodes";
  if ( sqlite3_prepare_v2( handle, sql, -1, &mStmt, 0 ) != SQLITE_OK )
  {
    qDebug( "OSMNodeIterator: error prepare" );
  }
}

QgsOSMNodeIterator::~QgsOSMNodeIterator()
{
  close();
}


QgsOSMNode QgsOSMNodeIterator::next()
{
  if ( !mStmt )
    return QgsOSMNode();

  if ( sqlite3_step( mStmt ) != SQLITE_ROW )
  {
    close();
    return QgsOSMNode();
  }

  QgsOSMId id   = sqlite3_column_int64( mStmt, 0 );
  double lon = sqlite3_column_double( mStmt, 1 );
  double lat = sqlite3_column_double( mStmt, 2 );

  return QgsOSMNode( id, QgsPoint( lon, lat ) );
}

void QgsOSMNodeIterator::close()
{
  if ( mStmt )
  {
    sqlite3_finalize( mStmt );
    mStmt = 0;
  }
}

///////////////////////////////////


QgsOSMWayIterator::QgsOSMWayIterator( sqlite3* handle )
    : mStmt( 0 )
{
  const char* sql = "SELECT id FROM ways";
  if ( sqlite3_prepare_v2( handle, sql, -1, &mStmt, 0 ) != SQLITE_OK )
  {
    qDebug( "OSMWayIterator: error prepare" );
  }
}

QgsOSMWayIterator::~QgsOSMWayIterator()
{
  close();
}


QgsOSMWay QgsOSMWayIterator::next()
{
  if ( !mStmt )
    return QgsOSMWay();

  if ( sqlite3_step( mStmt ) != SQLITE_ROW )
  {
    close();
    return QgsOSMWay();
  }

  QgsOSMId id = sqlite3_column_int64( mStmt, 0 );

  return QgsOSMWay( id, QList<QgsOSMId>() ); // TODO[MD]: ?
}

void QgsOSMWayIterator::close()
{
  if ( mStmt )
  {
    sqlite3_finalize( mStmt );
    mStmt = 0;
  }
}
