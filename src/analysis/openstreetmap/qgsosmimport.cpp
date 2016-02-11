/***************************************************************************
  qgsosmimport.cpp
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

#include "qgsosmimport.h"
#include "qgsslconnect.h"

#include <QStringList>
#include <QXmlStreamReader>


QgsOSMXmlImport::QgsOSMXmlImport( const QString& xmlFilename, const QString& dbFilename )
    : mXmlFileName( xmlFilename )
    , mDbFileName( dbFilename )
    , mDatabase( nullptr )
    , mStmtInsertNode( nullptr )
    , mStmtInsertNodeTag( nullptr )
    , mStmtInsertWay( nullptr )
    , mStmtInsertWayNode( nullptr )
    , mStmtInsertWayTag( nullptr )
{

}

bool QgsOSMXmlImport::import()
{
  mError.clear();

  // open input
  mInputFile.setFileName( mXmlFileName );
  if ( !mInputFile.open( QIODevice::ReadOnly ) )
  {
    mError = QString( "Cannot open input file: %1" ).arg( mXmlFileName );
    return false;
  }

  // open output

  if ( QFile::exists( mDbFileName ) )
  {
    if ( !QFile( mDbFileName ).remove() )
    {
      mError = QString( "Database file cannot be overwritten: %1" ).arg( mDbFileName );
      return false;
    }
  }

  if ( !createDatabase() )
  {
    // mError is set in createDatabase()
    return false;
  }

  qDebug( "starting import" );

  int retX = sqlite3_exec( mDatabase, "BEGIN", nullptr, nullptr, nullptr );
  Q_ASSERT( retX == SQLITE_OK );
  Q_UNUSED( retX );

  // start parsing

  QXmlStreamReader xml( &mInputFile );

  while ( !xml.atEnd() )
  {
    xml.readNext();

    if ( xml.isEndDocument() )
      break;

    if ( xml.isStartElement() )
    {
      if ( xml.name() == "osm" )
        readRoot( xml );
      else
        xml.raiseError( "Invalid root tag" );
    }
  }

  int retY = sqlite3_exec( mDatabase, "COMMIT", nullptr, nullptr, nullptr );
  Q_ASSERT( retY == SQLITE_OK );
  Q_UNUSED( retY );

  createIndexes();

  if ( xml.hasError() )
  {
    mError = QString( "XML error: %1" ).arg( xml.errorString() );
    return false;
  }

  closeDatabase();

  return true;
}

bool QgsOSMXmlImport::createIndexes()
{
  // index on tags for faster access
  const char* sqlIndexes[] =
  {
    "CREATE INDEX nodes_tags_idx ON nodes_tags(id)",
    "CREATE INDEX ways_tags_idx ON ways_tags(id)",
    "CREATE INDEX ways_nodes_way ON ways_nodes(way_id)"
  };
  int count = sizeof( sqlIndexes ) / sizeof( const char* );
  for ( int i = 0; i < count; ++i )
  {
    int ret = sqlite3_exec( mDatabase, sqlIndexes[i], nullptr, nullptr, nullptr );
    if ( ret != SQLITE_OK )
    {
      mError = "Error creating indexes!";
      return false;
    }
  }

  return true;
}


bool QgsOSMXmlImport::createDatabase()
{
  char **results;
  int rows, columns;
  if ( QgsSLConnect::sqlite3_open_v2( mDbFileName.toUtf8().data(), &mDatabase, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr ) != SQLITE_OK )
    return false;

  bool above41 = false;
  int ret = sqlite3_get_table( mDatabase, "select spatialite_version()", &results, &rows, &columns, nullptr );
  if ( ret == SQLITE_OK && rows == 1 && columns == 1 )
  {
    QString version = QString::fromUtf8( results[1] );
    QStringList parts = version.split( ' ', QString::SkipEmptyParts );
    if ( parts.size() >= 1 )
    {
      QStringList verparts = parts[0].split( '.', QString::SkipEmptyParts );
      above41 = verparts.size() >= 2 && ( verparts[0].toInt() > 4 || ( verparts[0].toInt() == 4 && verparts[1].toInt() >= 1 ) );
    }
  }
  sqlite3_free_table( results );

  const char* sqlInitStatements[] =
  {
    "PRAGMA cache_size = 100000", // TODO!!!
    "PRAGMA synchronous = OFF", // TODO!!!
    above41 ? "SELECT InitSpatialMetadata(1)" : "SELECT InitSpatialMetadata()",
    "CREATE TABLE nodes ( id INTEGER PRIMARY KEY, lat REAL, lon REAL )",
    "CREATE TABLE nodes_tags ( id INTEGER, k TEXT, v TEXT )",
    "CREATE TABLE ways ( id INTEGER PRIMARY KEY )",
    "CREATE TABLE ways_nodes ( way_id INTEGER, node_id INTEGER, way_pos INTEGER )",
    "CREATE TABLE ways_tags ( id INTEGER, k TEXT, v TEXT )",
  };

  int initCount = sizeof( sqlInitStatements ) / sizeof( const char* );
  for ( int i = 0; i < initCount; ++i )
  {
    char* errMsg;
    if ( sqlite3_exec( mDatabase, sqlInitStatements[i], nullptr, nullptr, &errMsg ) != SQLITE_OK )
    {
      mError = QString( "Error executing SQL command:\n%1\nSQL:\n%2" )
               .arg( QString::fromUtf8( errMsg ), QString::fromUtf8( sqlInitStatements[i] ) );
      sqlite3_free( errMsg );
      closeDatabase();
      return false;
    }
  }

  const char* sqlInsertStatements[] =
  {
    "INSERT INTO nodes ( id, lat, lon ) VALUES (?,?,?)",
    "INSERT INTO nodes_tags ( id, k, v ) VALUES (?,?,?)",
    "INSERT INTO ways ( id ) VALUES (?)",
    "INSERT INTO ways_nodes ( way_id, node_id, way_pos ) VALUES (?,?,?)",
    "INSERT INTO ways_tags ( id, k, v ) VALUES (?,?,?)"
  };
  sqlite3_stmt** sqliteInsertStatements[] =
  {
    &mStmtInsertNode,
    &mStmtInsertNodeTag,
    &mStmtInsertWay,
    &mStmtInsertWayNode,
    &mStmtInsertWayTag
  };
  Q_ASSERT( sizeof( sqlInsertStatements ) / sizeof( const char* ) == sizeof( sqliteInsertStatements ) / sizeof( sqlite3_stmt** ) );
  int insertCount = sizeof( sqlInsertStatements ) / sizeof( const char* );

  for ( int i = 0; i < insertCount; ++i )
  {
    if ( sqlite3_prepare_v2( mDatabase, sqlInsertStatements[i], -1, sqliteInsertStatements[i], nullptr ) != SQLITE_OK )
    {
      const char* errMsg = sqlite3_errmsg( mDatabase ); // does not require free
      mError = QString( "Error preparing SQL command:\n%1\nSQL:\n%2" )
               .arg( QString::fromUtf8( errMsg ), QString::fromUtf8( sqlInsertStatements[i] ) );
      closeDatabase();
      return false;
    }
  }

  return true;
}


void QgsOSMXmlImport::deleteStatement( sqlite3_stmt*& stmt )
{
  if ( stmt )
  {
    sqlite3_finalize( stmt );
    stmt = nullptr;
  }
}


bool QgsOSMXmlImport::closeDatabase()
{
  if ( !mDatabase )
    return false;

  deleteStatement( mStmtInsertNode );
  deleteStatement( mStmtInsertNodeTag );
  deleteStatement( mStmtInsertWay );
  deleteStatement( mStmtInsertWayNode );
  deleteStatement( mStmtInsertWayTag );

  Q_ASSERT( !mStmtInsertNode );

  QgsSLConnect::sqlite3_close( mDatabase );
  mDatabase = nullptr;
  return true;
}


void QgsOSMXmlImport::readRoot( QXmlStreamReader& xml )
{
  int i = 0;
  int percent = -1;

  while ( !xml.atEnd() )
  {
    xml.readNext();

    if ( xml.isEndElement() )  // </osm>
      break;

    if ( xml.isStartElement() )
    {
      if ( ++i == 500 )
      {
        int new_percent = 100 * mInputFile.pos() / mInputFile.size();
        if ( new_percent > percent )
        {
          emit progress( new_percent );
          percent = new_percent;
        }
        i = 0;
      }

      if ( xml.name() == "node" )
        readNode( xml );
      else if ( xml.name() == "way" )
        readWay( xml );
      else
        xml.skipCurrentElement();
    }
  }
}


void QgsOSMXmlImport::readNode( QXmlStreamReader& xml )
{
  // <node id="2197214" lat="50.0682113" lon="14.4348483" user="viduka" uid="595326" visible="true" version="10" changeset="10714591" timestamp="2012-02-17T19:58:49Z">
  QXmlStreamAttributes attrs = xml.attributes();
  QgsOSMId id = attrs.value( "id" ).toString().toLongLong();
  double lat = attrs.value( "lat" ).toString().toDouble();
  double lon = attrs.value( "lon" ).toString().toDouble();

  // insert to DB
  sqlite3_bind_int64( mStmtInsertNode, 1, id );
  sqlite3_bind_double( mStmtInsertNode, 2, lat );
  sqlite3_bind_double( mStmtInsertNode, 3, lon );

  if ( sqlite3_step( mStmtInsertNode ) != SQLITE_DONE )
  {
    xml.raiseError( QString( "Storing node %1 failed." ).arg( id ) );
  }

  sqlite3_reset( mStmtInsertNode );

  while ( !xml.atEnd() )
  {
    xml.readNext();

    if ( xml.isEndElement() ) // </node>
      break;

    if ( xml.isStartElement() )
    {
      if ( xml.name() == "tag" )
        readTag( false, id, xml );
      else
        xml.raiseError( "Invalid tag in <node>" );
    }
  }
}

void QgsOSMXmlImport::readTag( bool way, QgsOSMId id, QXmlStreamReader& xml )
{
  QXmlStreamAttributes attrs = xml.attributes();
  QByteArray k = attrs.value( "k" ).toString().toUtf8();
  QByteArray v = attrs.value( "v" ).toString().toUtf8();
  xml.skipCurrentElement();

  sqlite3_stmt* stmtInsertTag = way ? mStmtInsertWayTag : mStmtInsertNodeTag;

  sqlite3_bind_int64( stmtInsertTag, 1, id );
  sqlite3_bind_text( stmtInsertTag, 2, k.constData(), -1, SQLITE_STATIC );
  sqlite3_bind_text( stmtInsertTag, 3, v.constData(), -1, SQLITE_STATIC );

  int res = sqlite3_step( stmtInsertTag );
  if ( res != SQLITE_DONE )
  {
    xml.raiseError( QString( "Storing tag failed [%1]" ).arg( res ) );
  }

  sqlite3_reset( stmtInsertTag );
}

void QgsOSMXmlImport::readWay( QXmlStreamReader& xml )
{
  /*
   <way id="141756602" user="Vratislav Filler" uid="527259" visible="true" version="1" changeset="10145142" timestamp="2011-12-18T10:43:14Z">
    <nd ref="318529958"/>
    <nd ref="1551725779"/>
    <nd ref="1551725792"/>
    <nd ref="809695938"/>
    <nd ref="1551725689"/>
    <nd ref="809695935"/>
    <tag k="highway" v="service"/>
    <tag k="oneway" v="yes"/>
   </way>
  */
  QXmlStreamAttributes attrs = xml.attributes();
  QgsOSMId id = attrs.value( "id" ).toString().toLongLong();

  // insert to DB
  sqlite3_bind_int64( mStmtInsertWay, 1, id );

  if ( sqlite3_step( mStmtInsertWay ) != SQLITE_DONE )
  {
    xml.raiseError( QString( "Storing way %1 failed." ).arg( id ) );
  }

  sqlite3_reset( mStmtInsertWay );

  int way_pos = 0;

  while ( !xml.atEnd() )
  {
    xml.readNext();

    if ( xml.isEndElement() ) // </way>
      break;

    if ( xml.isStartElement() )
    {
      if ( xml.name() == "nd" )
      {
        QgsOSMId node_id = xml.attributes().value( "ref" ).toString().toLongLong();

        sqlite3_bind_int64( mStmtInsertWayNode, 1, id );
        sqlite3_bind_int64( mStmtInsertWayNode, 2, node_id );
        sqlite3_bind_int( mStmtInsertWayNode, 3, way_pos );

        if ( sqlite3_step( mStmtInsertWayNode ) != SQLITE_DONE )
        {
          xml.raiseError( QString( "Storing ways_nodes %1 - %2 failed." ).arg( id ).arg( node_id ) );
        }

        sqlite3_reset( mStmtInsertWayNode );

        way_pos++;

        xml.skipCurrentElement();
      }
      else if ( xml.name() == "tag" )
        readTag( true, id, xml );
      else
        xml.skipCurrentElement();
    }
  }
}
