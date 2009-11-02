/***************************************************************************
    osmhandler.cpp - handler for parsing OSM data
    ------------------
    begin                : October 2008
    copyright            : (C) 2008 by Lukas Berka
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "osmhandler.h"

#include <iostream>
#include <cstring>

#include "qgslogger.h"
#include "qgsapplication.h"
#include "qgsgeometry.h"

#include <QtXml/QXmlSimpleReader>
#include <QtXml/QXmlInputSource>
#include <QtGui/QApplication>
#include <QtGui/QLabel>
#include <QtXml/QXmlAttributes>
#include <QtCore/QFile>

#define MAX_FEATURE_ID 99999999
#define COMMIT_AFTER_TAGS 300000


// object construction
OsmHandler::OsmHandler( QFile *f, sqlite3 *database )
{
  mDatabase = database;
  mCnt = 0;
  mPointCnt = mLineCnt = mPolygonCnt = 0;
  mPosId = 1;
  xMin = yMin = MAX_FEATURE_ID;
  xMax = yMax = -MAX_FEATURE_ID;
  firstWayMemberId = "";
  mFirstMemberAppeared = 0;

  char sqlInsertNode[] = "INSERT INTO node ( id, lat, lon, timestamp, user, usage ) VALUES (?,?,?,?,?,'0');";
  if ( sqlite3_prepare_v2( mDatabase, sqlInsertNode, sizeof( sqlInsertNode ), &mStmtInsertNode, 0 ) != SQLITE_OK )
  {
    QgsDebugMsg( "failed to prepare sqlInsertNode!!!" );
  }

  char sqlInsertWay[] = "INSERT INTO way ( id, timestamp, user, closed ) VALUES (?,?,?,?);";
  if ( sqlite3_prepare_v2( mDatabase, sqlInsertWay, sizeof( sqlInsertWay ), &mStmtInsertWay, 0 ) != SQLITE_OK )
  {
    QgsDebugMsg( "failed to prepare sqlInsertWay!!!" );
  }

  char sqlInsertTag[] = "INSERT INTO tag ( key, val, object_id, object_type ) VALUES (?,?,?,?);";
  if ( sqlite3_prepare_v2( mDatabase, sqlInsertTag, sizeof( sqlInsertTag ), &mStmtInsertTag, 0 ) != SQLITE_OK )
  {
    QgsDebugMsg( "failed to prepare sqlInsertTag!!!" );
  }

  char sqlInsertWayMember[] = "INSERT INTO way_member ( way_id, pos_id, node_id ) VALUES (?,?,?);";
  if ( sqlite3_prepare_v2( mDatabase, sqlInsertWayMember, sizeof( sqlInsertWayMember ), &mStmtInsertWayMember, 0 ) != SQLITE_OK )
  {
    QgsDebugMsg( "failed to prepare sqlInsertWayMember!!!" );
  }

  char sqlInsertRelation[] = "INSERT INTO relation ( id, timestamp, user, type ) VALUES (?,?,?,?);";
  if ( sqlite3_prepare_v2( mDatabase, sqlInsertRelation, sizeof( sqlInsertRelation ), &mStmtInsertRelation, 0 ) != SQLITE_OK )
  {
    QgsDebugMsg( "failed to prepare sqlInsertRelation!!!" );
  }

  char sqlInsertRelationMember[] = "INSERT INTO relation_member ( relation_id, pos_id, member_id, member_type, role ) VALUES (?,?,?,?,?);";
  if ( sqlite3_prepare_v2( mDatabase, sqlInsertRelationMember, sizeof( sqlInsertRelationMember ), &mStmtInsertRelationMember, 0 ) != SQLITE_OK )
  {
    QgsDebugMsg( "failed to prepare sqlInsertRelationMember!!!" );
  }

  char sqlInsertVersion[] = "INSERT INTO version (object_id,object_type,version_id) VALUES (?,?,?);";
  if ( sqlite3_prepare_v2( mDatabase, sqlInsertVersion, sizeof( sqlInsertVersion ), &mStmtInsertVersion, 0 ) != SQLITE_OK )
  {
    QgsDebugMsg( "failed to prepare sqlInsertVersion!!!" );
  }
}

OsmHandler::~OsmHandler()
{
  sqlite3_finalize( mStmtInsertTag );
  sqlite3_finalize( mStmtInsertRelation );
  sqlite3_finalize( mStmtInsertRelationMember );
  sqlite3_finalize( mStmtInsertVersion );
}


bool OsmHandler::startDocument()
{
  sqlite3_exec( mDatabase, "BEGIN;", 0, 0, 0 );
  return true;
}


QString OsmHandler::errorString()
{
  return mError;
}


bool OsmHandler::startElement( const QString & pUri, const QString & pLocalName, const QString & pName, const QXmlAttributes & pAttrs )
{
  QString name = pLocalName;

  if ( name == "osm" )
  {
    if ( pAttrs.value( "version" ) != "0.6" )
    {
      mError = "Invalid OSM version. Only files of v0.6 are supported.";
      return false;
    }
  }
  else if ( name == "node" )
  {
    //todo: test if pAttrs.value("visible").toUtf8() is "true" -> if not, node has to be ignored!

    mObjectId = pAttrs.value( "id" );
    mObjectType = "node";

    double id = pAttrs.value( "id" ).toInt();
    double lat = pAttrs.value( "lat" ).toDouble();
    double lon = pAttrs.value( "lon" ).toDouble();
    QString timestamp = pAttrs.value( "timestamp" );
    QString user = pAttrs.value( "user" );

    if ( lat < yMin ) yMin = lat;
    if ( lat > yMax ) yMax = lat;
    if ( lon < xMin ) xMin = lon;
    if ( lon > xMax ) xMax = lon;

    sqlite3_bind_int( mStmtInsertNode, 1, id );
    sqlite3_bind_double( mStmtInsertNode, 2, lat );
    sqlite3_bind_double( mStmtInsertNode, 3, lon );
    sqlite3_bind_text( mStmtInsertNode, 4, timestamp.toUtf8(), -1, SQLITE_TRANSIENT ); // TODO: maybe static?
    sqlite3_bind_text( mStmtInsertNode, 5, user.toUtf8(), -1, SQLITE_TRANSIENT ); // TODO: maybe static?

    if ( sqlite3_step( mStmtInsertNode ) != SQLITE_DONE )
    {
      QgsDebugMsg( "Storing node information into database failed." );
      return false;
    }

    sqlite3_reset( mStmtInsertNode ); // make ready for next insert

    // store version number of this object
    sqlite3_bind_text( mStmtInsertVersion, 1, pAttrs.value( "id" ).toUtf8(), -1, SQLITE_TRANSIENT );
    sqlite3_bind_text( mStmtInsertVersion, 2, mObjectType.toUtf8(), -1, SQLITE_TRANSIENT );
    sqlite3_bind_text( mStmtInsertVersion, 3, pAttrs.value( "version" ).toUtf8(), -1, SQLITE_TRANSIENT );

    if ( sqlite3_step( mStmtInsertVersion ) != SQLITE_DONE )
    {
      QgsDebugMsg( "Storing version information into database failed." );
      return false;
    }
    sqlite3_reset( mStmtInsertVersion ); // make ready for next insert

    // increase node counter
    mPointCnt++;
  }
  else if ( name == "way" )
  {
    if ( mObjectType != "way" )
    {
      sqlite3_finalize( mStmtInsertNode );
    }

    mObjectId = pAttrs.value( "id" );
    mObjectType = "way";
    mPosId = 1;
    mFirstMemberAppeared = 0;

    //todo: test if pAttrs.value("visible").toUtf8() is "true" -> if not, way has to be ignored!

    sqlite3_bind_text( mStmtInsertWay, 1, mObjectId.toUtf8(), -1, SQLITE_TRANSIENT );
    sqlite3_bind_text( mStmtInsertWay, 2, pAttrs.value( "timestamp" ).toUtf8(), -1, SQLITE_TRANSIENT );
    sqlite3_bind_text( mStmtInsertWay, 3, pAttrs.value( "user" ).toUtf8(), -1, SQLITE_TRANSIENT );

    // store version number of this object
    sqlite3_bind_text( mStmtInsertVersion, 1, pAttrs.value( "id" ).toUtf8(), -1, SQLITE_TRANSIENT );
    sqlite3_bind_text( mStmtInsertVersion, 2, mObjectType.toUtf8(), -1, SQLITE_TRANSIENT );
    sqlite3_bind_text( mStmtInsertVersion, 3, pAttrs.value( "version" ).toUtf8(), -1, SQLITE_TRANSIENT );

    if ( sqlite3_step( mStmtInsertVersion ) != SQLITE_DONE )
    {
      QgsDebugMsg( "Storing version information into database failed." );
      return false;
    }
    sqlite3_reset( mStmtInsertVersion ); // make ready for next insert
  }
  else if ( name == "nd" )
  {
    // store id of the first and last way member to be able to decide if the way is closed (polygon) or not
    if ( firstWayMemberId == "" )
    {
      firstWayMemberId = pAttrs.value( "ref" );
    }
    lastWayMemberId = pAttrs.value( "ref" );

    if ( firstWayMemberId == lastWayMemberId )
      mFirstMemberAppeared++;

    if (( firstWayMemberId != lastWayMemberId ) || ( mFirstMemberAppeared < 2 ) )
    {
      sqlite3_bind_text( mStmtInsertWayMember, 1, mObjectId.toUtf8(), -1, SQLITE_TRANSIENT ); // TODO: maybe static?
      sqlite3_bind_int( mStmtInsertWayMember, 2, mPosId );
      sqlite3_bind_text( mStmtInsertWayMember, 3, pAttrs.value( "ref" ).toUtf8(), -1, SQLITE_TRANSIENT );

      if ( sqlite3_step( mStmtInsertWayMember ) != SQLITE_DONE )
      {
        QgsDebugMsg( "Storing way-node relationship into database failed." );
        return false;
      };
      sqlite3_reset( mStmtInsertWayMember );
    }
    mPosId++;
  }
  else if ( name == "relation" )
  {
    if ( mObjectType != "relation" )
    {
      sqlite3_finalize( mStmtInsertWay );
      sqlite3_finalize( mStmtInsertWayMember );
    }

    mObjectId = pAttrs.value( "id" );
    mRelationType = "";
    mObjectType = "relation";
    mPosId = 1;

    //todo: test if pAttrs.value("visible").toUtf8() is "true" -> if not, relation has to be ignored!

    sqlite3_bind_text( mStmtInsertRelation, 1, mObjectId.toUtf8(), -1, SQLITE_TRANSIENT );
    sqlite3_bind_text( mStmtInsertRelation, 2, pAttrs.value( "timestamp" ).toUtf8(), -1, SQLITE_TRANSIENT );
    sqlite3_bind_text( mStmtInsertRelation, 3, pAttrs.value( "user" ).toUtf8(), -1, SQLITE_TRANSIENT );

    // store version number of this object
    sqlite3_bind_text( mStmtInsertVersion, 1, pAttrs.value( "id" ).toUtf8(), -1, SQLITE_TRANSIENT );
    sqlite3_bind_text( mStmtInsertVersion, 2, mObjectType.toUtf8(), -1, SQLITE_TRANSIENT );
    sqlite3_bind_text( mStmtInsertVersion, 3, pAttrs.value( "version" ).toUtf8(), -1, SQLITE_TRANSIENT );

    if ( sqlite3_step( mStmtInsertVersion ) != SQLITE_DONE )
    {
      QgsDebugMsg( "Storing version information into database failed." );
      return false;
    }
    sqlite3_reset( mStmtInsertVersion ); // make ready for next insert
  }
  else if ( name == "member" )
  {
    sqlite3_bind_text( mStmtInsertRelationMember, 1, mObjectId.toUtf8(), -1, SQLITE_TRANSIENT );
    sqlite3_bind_int( mStmtInsertRelationMember, 2, mPosId );
    sqlite3_bind_text( mStmtInsertRelationMember, 3, pAttrs.value( "ref" ).toUtf8(), -1, SQLITE_TRANSIENT );
    sqlite3_bind_text( mStmtInsertRelationMember, 4, pAttrs.value( "type" ).toUtf8(), -1, SQLITE_TRANSIENT );
    sqlite3_bind_text( mStmtInsertRelationMember, 5, pAttrs.value( "role" ).toUtf8(), -1, SQLITE_TRANSIENT );

    if ( sqlite3_step( mStmtInsertRelationMember ) != SQLITE_DONE )
    {
      QgsDebugMsg( "Storing relation-feature relationship into database failed." );
      return false;
    };

    sqlite3_reset( mStmtInsertRelationMember );
    mPosId++;
  }
  else if ( name == "tag" )
  {
    if ( mCnt == COMMIT_AFTER_TAGS )
    {
      sqlite3_exec( mDatabase, "COMMIT;", 0, 0, 0 );
      sqlite3_exec( mDatabase, "BEGIN;", 0, 0, 0 );
      mCnt = 0;
    }
    mCnt++;

    sqlite3_bind_text( mStmtInsertTag, 1, pAttrs.value( "k" ).toUtf8(), -1, SQLITE_TRANSIENT );
    sqlite3_bind_text( mStmtInsertTag, 2, pAttrs.value( "v" ).toUtf8(), -1, SQLITE_TRANSIENT );
    sqlite3_bind_text( mStmtInsertTag, 3, mObjectId.toUtf8(), -1, SQLITE_TRANSIENT );
    sqlite3_bind_text( mStmtInsertTag, 4, mObjectType.toUtf8(), -1, SQLITE_TRANSIENT );

    // we've got node parameters -> let's create new database record
    if ( sqlite3_step( mStmtInsertTag ) != SQLITE_DONE )
    {
      QgsDebugMsg( QString( "Storing tag into database failed. K:%1, V:%2." ).arg( pAttrs.value( "k" ) ).arg( pAttrs.value( "v" ) ) );
      return false;
    }
    sqlite3_reset( mStmtInsertTag );

    // if we are under xml tag <relation> and we reach xml tag <tag k="type" v="...">, lets insert prepared relation into DB
    if (( mObjectType == "relation" ) && ( pAttrs.value( "k" ) == "type" ) )
    {
      mRelationType = pAttrs.value( "v" );
    }
  }
  else if ( name == "bounds" )
  {
    // e.g. <bounds minlat="41.388625" minlon="2.15426" maxlat="41.391732" maxlon="2.158192"/>
    // notice: getting boundaries from OSM file <bounds> tag was not correct for some maps - cannot be used

    // xMin = pAttrs.value("minlon").toDouble();
    // xMax = pAttrs.value("maxlon").toDouble();
    // yMin = pAttrs.value("minlat").toDouble();
    // yMax = pAttrs.value("maxlat").toDouble();
  }
  return true;
}


bool OsmHandler::endElement( const QString & pURI, const QString & pLocalName, const QString & pName )
{
  QString name = pLocalName;
  if ( name == "way" )
  {
    int isPolygon = false;
    int cntMembers = mPosId - 1;

    if ( firstWayMemberId == lastWayMemberId )
      isPolygon = true;

    // test if polygon is correct; it should have >2 member points
    if (( isPolygon ) && ( cntMembers < 4 ) )
    {
      sqlite3_reset( mStmtInsertWay );
      return true;
    }

    // test if way is correct; it should have more then 1 member point
    if ( cntMembers < 2 )
    {
      sqlite3_reset( mStmtInsertWay );
      return true;
    }

    // we should bind the last information needed for way insertion -> if the way is closed (polygon) or not
    sqlite3_bind_int( mStmtInsertWay, 4, ( isPolygon ? 1 : 0 ) );

    // well, insert new way
    if ( sqlite3_step( mStmtInsertWay ) != SQLITE_DONE )
    {
      QgsDebugMsg( "Storing way information into database failed." );
      return false;
    };

    // make statement ready for next insert
    sqlite3_reset( mStmtInsertWay );

    if ( isPolygon )
      mPolygonCnt++;
    else
      mLineCnt++;

    // make variables ready for next way parsing
    firstWayMemberId = "";
  }
  else if ( name == "relation" )
  {
    sqlite3_bind_text( mStmtInsertRelation, 4, mRelationType.toUtf8(), -1, SQLITE_TRANSIENT );

    if ( sqlite3_step( mStmtInsertRelation ) != SQLITE_DONE )
    {
      QgsDebugMsg( QString( "Storing relation into database failed." ) );
      return false;
    }
    sqlite3_reset( mStmtInsertRelation );
  }
  return true;
}


bool OsmHandler::endDocument()
{
  // first commit all database actions connected to xml parsing
  sqlite3_exec( mDatabase, "COMMIT;", 0, 0, 0 );
  return true;
}

