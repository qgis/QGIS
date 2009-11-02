/***************************************************************************
    osmprovider.cpp - provider for OSM; stores OSM data in sqlite3 DB
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

#include "osmprovider.h"
#include "osmhandler.h"
#include "osmrenderer.h"

#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsvectordataprovider.h"
#include "qgsapplication.h"

#include <cstring>
#include <iostream>
#include <QQueue>
#include <QFileInfo>
#include <QDateTime>
#include <QByteArray>

using namespace std;


static const QString TEXT_PROVIDER_KEY = "osm";
static const QString TEXT_PROVIDER_DESCRIPTION = "Open Street Map data provider";
static const QString DATE_TIME_FMT = "dd.MM.yyyy HH:mm:ss";
static const QString PLUGIN_VERSION = "0.4";

// supported attributes
const char* QgsOSMDataProvider::attr[] = { "timestamp", "user", "tags" };



QgsOSMDataProvider::QgsOSMDataProvider( QString uri )
    : QgsVectorDataProvider( uri )
{
  mDatabaseStmt = NULL;
  mValid = false;
  QgsDebugMsg( "Initializing provider: " + uri );

  // set the selection rectangle to null
  mSelectionRectangle = 0;
  mSelectionRectangleGeom = NULL;
  mDatabase = NULL;
  mInitObserver = NULL;
  mFeatureType = PointType; // default

  // set default boundaries
  xMin = -DEFAULT_EXTENT;
  xMax = DEFAULT_EXTENT;
  yMin = -DEFAULT_EXTENT;
  yMax = DEFAULT_EXTENT;

  // get the filename and the type parameter from the URI
  int fileNameEnd = uri.indexOf( '?' );
  if ( fileNameEnd == -1 )
  {
    QgsDebugMsg( "Bad URI - you need to specify the feature type, OSM provider cannot be constructed." );
    return;
  }

  QString uriEnd = uri.mid( fileNameEnd + 1 );
  QStringList props = uriEnd.split( "&" );
  for ( QStringList::iterator it = props.begin(); it != props.end(); ++it )
  {
    QStringList prop = it->split( "=" );
    if ( prop.count() != 2 )
    {
      QgsDebugMsg( "Incorrectly formed input property!" );
      return;
    }

    QString propName = prop[0];
    QString propValue = prop[1];

    if ( propName == "type" )
    {
      if ( propValue == "line" )
        mFeatureType = LineType;
      else if ( propValue == "point" )
        mFeatureType = PointType;
      else if ( propValue == "polygon" )
        mFeatureType = PolygonType;
      else
        QgsDebugMsg( "Unknown feature type: " + propValue );
    }
    if ( propName == "observer" )
    {
      ulong observerAddr = propValue.toULong();
      mInitObserver = ( QObject* ) observerAddr;
      mInitObserver->setProperty( "osm_state", QVariant( 1 ) );
    }
    if ( propName == "tag" )
    {
      mCustomTagsList = propValue.split( "+" );
    }
    if ( propName == "style" )
    {
      mStyleFileName = propValue;
      int p1 = mStyleFileName.lastIndexOf( "/" );
      int p2 = mStyleFileName.lastIndexOf( "_" );

      mStyle = mStyleFileName.mid( p1 + 1, p2 - p1 - 1 );    // "medium", "big", "small"
    }
  }

  // set up the attributes and the geometry type depending on the feature type - same attributes for both point and way type so far
  mAttributeFields[TimestampAttr] = QgsField( attr[TimestampAttr], QVariant::String, "string" );
  mAttributeFields[UserAttr] = QgsField( attr[UserAttr], QVariant::String, "string" );
  mAttributeFields[TagAttr] = QgsField( attr[TagAttr], QVariant::String, "string" );
  // add custom tags
  for ( int tagId = 0; tagId < mCustomTagsList.count(); ++tagId )
  {
    mAttributeFields[CustomTagAttr+tagId] = QgsField( mCustomTagsList[tagId], QVariant::String, "string" );
  }

  // get source file name and database file name
  mFileName = uri.left( fileNameEnd );
  mDatabaseFileName = mFileName + ".db";
  QFile dbFile( mDatabaseFileName );
  QFile osmFile( mFileName );
  bool databaseExists = dbFile.exists();

  // open (and create schema if neccessary) database
  if ( !openDatabase() )
  {
    QgsDebugMsg( "Opening sqlite3 database failed, OSM provider cannot be constructed." );
    closeDatabase();
    return;
  };

  // flag determining if OSM file parsing is neccessary
  bool shouldParse = true;

  // test if db file belonging to source OSM file exists and if it has the right version
  if ( databaseExists && isDatabaseCompatibleWithInput( mFileName ) && isDatabaseCompatibleWithPlugin() )
    shouldParse = false;

  if ( shouldParse )
  {
    if ( databaseExists )
      dropDatabaseSchema();

    if ( !createDatabaseSchema() )
    {
      QgsDebugMsg( "Creating OSM database schema failed, OSM provider cannot be constructed." );
      dropDatabaseSchema();
      return;
    }

    // load OSM file to database (uses XML parsing)
    if ( !( loadOsmFile( mFileName ) ) )
    {
      QgsDebugMsg( "Parsing OSM data file failed, OSM provider cannot be constructed." );
      // process of creating osm database wasn't successfull -> remove all new database files
      if ( dbFile.exists() )
        dbFile.remove();
      // and remove also new database journal file if any
      QFile dbJournalFile( QString( mFileName + ".db-journal" ) );
      if ( dbJournalFile.exists() )
        dbJournalFile.remove();
      // now stop the osmprovider construction!
      return;
    }
  }
  else
  {
    // there was no parsing, we must find out default area boundaries from database meta information
    // prepare select command
    QString cmd = QString( "SELECT val FROM meta WHERE key='default-area-boundaries';" );

    // just conversion "cmd" to "const char*"
    QByteArray cmd_bytes = cmd.toAscii();
    const char *ptr = cmd_bytes.data();

    sqlite3_stmt *databaseStmt;
    if ( sqlite3_prepare_v2( mDatabase, ptr, cmd_bytes.size(), &databaseStmt, 0 ) != SQLITE_OK )
    {
      QgsDebugMsg( "Getting default area boundaries failed." );
      // don't worry, we just let default values in xMax, yMax, xMin and yMin variables
    }
    else
    {
      // select command has just run successful
      if ( sqlite3_step( databaseStmt ) != SQLITE_ROW )
      {
        QgsDebugMsg( "Getting default area boundaries failed." );
        // don't worry again, we just let default values in boundary variables
      }
      else
      {
        const unsigned char *boundaries_char = sqlite3_column_text( databaseStmt, 0 );
        QString boundaries(( const char * ) boundaries_char );
        // boundaries should be string of the following format: "xMin-yMin-xMax-yMax"
        int separ1_pos = boundaries.indexOf( "-" );
        int separ2_pos = boundaries.indexOf( "-", separ1_pos + 1 );
        int separ3_pos = boundaries.indexOf( "-", separ2_pos + 1 );
        xMin = boundaries.left( separ1_pos ).toDouble();
        yMin = boundaries.mid( separ1_pos + 1, separ2_pos - separ1_pos - 1 ).toDouble();
        xMax = boundaries.mid( separ2_pos + 1, separ3_pos - separ2_pos - 1 ).toDouble();
        yMax = boundaries.right( boundaries.size() - separ3_pos - 1 ).toDouble();
      }
    }

    // destroy database statement
    sqlite3_finalize( databaseStmt );
  }

  // prepare statement for tag retrieval
  const char *zSql = "SELECT key, val FROM tag WHERE object_id=? AND object_type=?";
  int rc = sqlite3_prepare_v2( mDatabase, zSql, -1, &mTagsStmt, 0 );
  if ( rc != SQLITE_OK )
    QgsDebugMsg( "tags for object - prepare failed." );

  const char *zSqlC = "SELECT val FROM tag WHERE object_id=? AND object_type=? AND key=?";
  rc = sqlite3_prepare_v2( mDatabase, zSqlC, -1, &mCustomTagsStmt, 0 );
  if ( rc != SQLITE_OK )
    QgsDebugMsg( "custom tags for object - prepare failed." );

  // prepare statements for feature retrieval
  const char *zSqlW = "SELECT id, wkb, timestamp, user FROM way WHERE id=? AND status<>'R' AND u=1";
  rc = sqlite3_prepare_v2( mDatabase, zSqlW, -1, &mWayStmt, 0 );
  if ( rc != SQLITE_OK )
    QgsDebugMsg( "sqlite3 statement for way retrieval - prepare failed." );

  const char *zSqlN = "SELECT id, lat, lon, timestamp, user FROM node WHERE id=? AND usage=0 AND status<>'R' AND u=1";
  rc = sqlite3_prepare_v2( mDatabase, zSqlN, -1, &mNodeStmt, 0 );
  if ( rc != SQLITE_OK )
    QgsDebugMsg( "sqlite3 statement for node retrieval - prepare failed." );

  mValid = true;
}


QgsOSMDataProvider::~QgsOSMDataProvider()
{
  // destruct selected geometry
  delete mSelectionRectangleGeom;
  sqlite3_finalize( mTagsStmt );
  sqlite3_finalize( mCustomTagsStmt );
  sqlite3_finalize( mWayStmt );
  sqlite3_finalize( mNodeStmt );
  sqlite3_finalize( mDatabaseStmt );
  closeDatabase();
}


bool QgsOSMDataProvider::isDatabaseCompatibleWithInput( QString mFileName )
{
  QFile osmFile( mFileName );
  QFileInfo osmFileInfo( osmFile );
  QDateTime mOsmFileLastModif = osmFileInfo.lastModified();

  QString cmd = QString( "SELECT val FROM meta WHERE key='osm-file-last-modified';" );
  QByteArray cmd_bytes = cmd.toAscii();
  const char *ptr = cmd_bytes.data();

  sqlite3_stmt *databaseStmt;
  if ( sqlite3_prepare_v2( mDatabase, ptr, cmd_bytes.size(), &databaseStmt, 0 ) == SQLITE_OK )
  {
    if ( sqlite3_step( databaseStmt ) == SQLITE_ROW )
    {
      QString oldOsmLastModifString = ( const char * ) sqlite3_column_text( databaseStmt, 0 );
      QDateTime oldOsmFileLastModif = QDateTime::fromString( oldOsmLastModifString, DATE_TIME_FMT );

      if ( mOsmFileLastModif == oldOsmFileLastModif )
      {
        sqlite3_finalize( databaseStmt );
        return true;
      }
    }
  }
  // destroy database statement
  sqlite3_finalize( databaseStmt );
  return false;
}


bool QgsOSMDataProvider::isDatabaseCompatibleWithPlugin()
{
  QString cmd = QString( "SELECT val FROM meta WHERE key='osm-plugin-version';" );
  QByteArray cmd_bytes = cmd.toAscii();
  const char *ptr = cmd_bytes.data();

  sqlite3_stmt *databaseStmt;
  if ( sqlite3_prepare_v2( mDatabase, ptr, cmd_bytes.size(), &databaseStmt, 0 ) == SQLITE_OK )
  {
    if ( sqlite3_step( databaseStmt ) == SQLITE_ROW )
    {
      QString osmPluginVersion = ( const char * ) sqlite3_column_text( databaseStmt, 0 );

      if ( osmPluginVersion == PLUGIN_VERSION )
      {
        sqlite3_finalize( databaseStmt );
        return true;
      }
    }
  }
  // destroy database statement
  sqlite3_finalize( databaseStmt );
  return false;
}


QString QgsOSMDataProvider::storageType() const
{
  // just return string reprezenting data storage type
  return tr( "Open Street Map format" );
}


void QgsOSMDataProvider::select( QgsAttributeList fetchAttributes,
                                 QgsRectangle rect,
                                 bool fetchGeometry,
                                 bool useIntersect )
{
  // clear
  delete mSelectionRectangleGeom;
  if ( mDatabaseStmt )
    sqlite3_finalize( mDatabaseStmt );

  // store list of attributes to fetch, rectangle of area, geometry, etc.
  mSelectionRectangle = rect;
  mSelectionRectangleGeom = QgsGeometry::fromRect( rect );
  mAttributesToFetch = fetchAttributes;

  // set flags
  mFetchGeom = fetchGeometry;
  mSelectUseIntersect = useIntersect;

  QString cmd;
  // select data from sqlite3 database
  if ( mFeatureType == PointType )
  {
    // prepare select: get all the NODE identifiers from specified area -> future calls of nextFeature () will pick them up one-by-one
    cmd = QString( "SELECT id, lat, lon, timestamp, user FROM node WHERE usage=0 AND status<>'R' AND u=1" );

    if ( !mSelectionRectangle.isEmpty() )
    {
      cmd += QString( " AND lat>=%1 AND lat<=%2 AND lon>=%3 AND lon<=%4;" )
             .arg( mSelectionRectangle.yMinimum(), 0, 'f', 20 )
             .arg( mSelectionRectangle.yMaximum(), 0, 'f', 20 )
             .arg( mSelectionRectangle.xMinimum(), 0, 'f', 20 )
             .arg( mSelectionRectangle.xMaximum(), 0, 'f', 20 );
    }
  }
  else if ( mFeatureType == LineType )
  {
    cmd = "SELECT w.id, w.wkb, w.timestamp, w.user FROM way w WHERE w.closed=0 AND w.status<>'R' AND w.u=1";

    if ( !mSelectionRectangle.isEmpty() )
    {
      cmd += QString( " AND (((w.max_lat between %1 AND %2) OR (w.min_lat between %1 AND %2) OR (w.min_lat<%1 AND w.max_lat>%2)) OR ((w.max_lon between %3 AND %4) OR (w.min_lon between %3 AND %4) OR (w.min_lon<%3 AND w.max_lon>%4)));" )
             .arg( mSelectionRectangle.yMinimum(), 0, 'f', 20 )
             .arg( mSelectionRectangle.yMaximum(), 0, 'f', 20 )
             .arg( mSelectionRectangle.xMinimum(), 0, 'f', 20 )
             .arg( mSelectionRectangle.xMaximum(), 0, 'f', 20 );
    }
  }
  else if ( mFeatureType == PolygonType )
  {
    cmd = "SELECT w.id, w.wkb, w.timestamp, w.user FROM way w WHERE w.closed=1 AND w.status<>'R' AND w.u=1";

    if ( !mSelectionRectangle.isEmpty() )
    {
      cmd += QString( " AND (((w.max_lat between %1 AND %2) OR (w.min_lat between %1 AND %2) OR (w.min_lat<%1 AND w.max_lat>%2)) OR ((w.max_lon between %3 AND %4) OR (w.min_lon between %3 AND %4) OR (w.min_lon<%3 AND w.max_lon>%4)));" )
             .arg( mSelectionRectangle.yMinimum(), 0, 'f', 20 )
             .arg( mSelectionRectangle.yMaximum(), 0, 'f', 20 )
             .arg( mSelectionRectangle.xMinimum(), 0, 'f', 20 )
             .arg( mSelectionRectangle.xMaximum(), 0, 'f', 20 );
    }
  }

  // just conversion "cmd" to "const char*"
  QByteArray cmd_bytes  = cmd.toAscii();
  const char *ptr = cmd_bytes.data();

  if ( sqlite3_prepare_v2( mDatabase, ptr, cmd_bytes.size(), &mDatabaseStmt, 0 ) != SQLITE_OK )
  {
    QgsDebugMsg( "Selecting object information failed." );
    return;
  }
  QgsDebugMsg( QString( "SELECTING FEATURES OF TYPE %1." ).arg( mFeatureType ) );
  QgsDebugMsg( cmd );
}


int QgsOSMDataProvider::wayMemberCount( int wayId )
{
  // prepare select: get all the WAY members
  QString cmd = QString( "SELECT count(n.id) FROM way_member wm, node n WHERE wm.way_id=%1 AND wm.node_id=n.id AND wm.u=1 AND n.u=1;" )
                .arg( wayId );

  // just conversion "cmd" to "const char*"
  QByteArray cmd_bytes = cmd.toAscii();
  const char *ptr = cmd_bytes.data();

  sqlite3_stmt *databaseStmt;
  if ( sqlite3_prepare_v2( mDatabase, ptr, cmd_bytes.size(), &databaseStmt, 0 ) != SQLITE_OK )
  {
    QgsDebugMsg( "Selecting way members failed." );
    return -1;
  }

  if ( sqlite3_step( databaseStmt ) != SQLITE_ROW )
  {
    QgsDebugMsg( "Cannot find out way members count." );
    return -1;
  }
  int wayMemberCnt = sqlite3_column_int( databaseStmt, 0 );
  // destroy database statement
  sqlite3_finalize( databaseStmt );

  return wayMemberCnt;
}


bool QgsOSMDataProvider::nextFeature( QgsFeature& feature )
{
  // load next requested feature from sqlite3 database
  switch ( sqlite3_step( mDatabaseStmt ) )
  {
    case SQLITE_DONE:  // no more features to return
      feature.setValid( false );
      return false;

    case SQLITE_ROW:  // another feature to return
      if ( mFeatureType == PointType )
        return fetchNode( feature, mDatabaseStmt, mFetchGeom, mAttributesToFetch );
      else if ( mFeatureType == LineType )
        return fetchWay( feature, mDatabaseStmt, mFetchGeom, mAttributesToFetch );
      else if ( mFeatureType == PolygonType )
        return fetchWay( feature, mDatabaseStmt, mFetchGeom, mAttributesToFetch );

    default:
      if ( mFeatureType == PointType )
        QgsDebugMsg( "Getting next feature of type <point> failed." );
      else if ( mFeatureType == LineType )
        QgsDebugMsg( "Getting next feature of type <line> failed." );
      else if ( mFeatureType == PolygonType )
        QgsDebugMsg( "Getting next feature of type <polygon> failed." );
      feature.setValid( false );
      return false;
  }
}


bool QgsOSMDataProvider::featureAtId( int featureId,
                                      QgsFeature& feature,
                                      bool fetchGeometry,
                                      QgsAttributeList fetchAttributes )
{
//QgsDebugMsg(QString("!!! Asking for feature:%1.").arg(featureId));

  // load exact feature from sqlite3 database
  if ( mFeatureType == PointType )
  {
    sqlite3_bind_int( mNodeStmt, 1, featureId );

    if ( sqlite3_step( mNodeStmt ) != SQLITE_ROW )
    {
      QgsDebugMsg( QString( "Getting information about point with id=%1 failed." ).arg( featureId ) );
      sqlite3_reset( mNodeStmt );
      return false;
    }

    fetchNode( feature, mNodeStmt, fetchGeometry, fetchAttributes );

    // prepare statement for next call
    sqlite3_reset( mNodeStmt );
  }
  else if (( mFeatureType == LineType ) || ( mFeatureType == PolygonType ) )
  {
    sqlite3_bind_int( mWayStmt, 1, featureId );

    if ( sqlite3_step( mWayStmt ) != SQLITE_ROW )
    {
      QgsDebugMsg( QString( "Getting information about way with id=%1 failed." ).arg( featureId ) );
      sqlite3_reset( mWayStmt );
      return false;
    }

    fetchWay( feature, mWayStmt, fetchGeometry, fetchAttributes );

    // prepare statement for next call
    sqlite3_reset( mWayStmt );
  }
  return true;
}


bool QgsOSMDataProvider::fetchNode( QgsFeature& feature, sqlite3_stmt* stmt, bool fetchGeometry, QgsAttributeList& fetchAttrs )
{
  int selId = sqlite3_column_int( stmt, 0 );
  double selLat = sqlite3_column_double( stmt, 1 );
  double selLon = sqlite3_column_double( stmt, 2 );
  const char* selTimestamp = ( const char* ) sqlite3_column_text( stmt, 3 );
  const char* selUser = ( const char* ) sqlite3_column_text( stmt, 4 );

  // fetch feature's geometry
  if ( fetchGeometry )
  {
    char* geo = new char[21];
    std::memset( geo, 0, 21 );
    geo[0] = QgsApplication::endian();
    geo[geo[0] == QgsApplication::NDR ? 1 : 4] = QGis::WKBPoint;
    std::memcpy( geo + 5, &selLon, sizeof( double ) );
    std::memcpy( geo + 13, &selLat, sizeof( double ) );
    feature.setGeometryAndOwnership(( unsigned char * )geo, 24 );    // 24 is size of wkb point structure!
  }

  // fetch attributes
  QgsAttributeList::const_iterator iter;
  for ( iter = fetchAttrs.begin(); iter != fetchAttrs.end(); ++iter )
  {
    switch ( *iter )
    {
      case TimestampAttr:
        feature.addAttribute( TimestampAttr, QString::fromUtf8( selTimestamp ) ); break;
      case UserAttr:
        feature.addAttribute( UserAttr, QString::fromUtf8( selUser ) ); break;
      case TagAttr:
        feature.addAttribute(TagAttr, tagsForObject("node",selId));
        break;

      default: // suppose it's a custom tag
        if ( *iter >= CustomTagAttr && *iter < CustomTagAttr + mCustomTagsList.count() )
        {
          feature.addAttribute( *iter, tagForObject( "node", selId, mCustomTagsList[*iter-CustomTagAttr] ) );
        }
    }
  }

  feature.setFeatureId( selId );
  feature.setValid( true );
  return true;
}


bool QgsOSMDataProvider::fetchWay( QgsFeature& feature, sqlite3_stmt* stmt, bool fetchGeometry, QgsAttributeList& fetchAttrs )
{
  int selId;
  const char* selTimestamp;
  const char* selUser;
  QgsGeometry *theGeometry = NULL;
  bool fetchMoreRows = true;
  int rc = -1;

  do
  {
    selId = sqlite3_column_int( stmt, 0 );
    selTimestamp = ( const char* ) sqlite3_column_text( stmt, 2 );
    selUser = ( const char* ) sqlite3_column_text( stmt, 3 );
    unsigned char *pzBlob = 0;
    int pnBlob = 0;

    if ( fetchGeometry || mSelectUseIntersect || !mSelectionRectangle.isEmpty() )
    {
      pnBlob = sqlite3_column_bytes( stmt, 1 );
      pzBlob = new unsigned char[pnBlob];
      memcpy( pzBlob, sqlite3_column_blob( stmt, 1 ), pnBlob );

      // create geometry
      theGeometry = new QgsGeometry();
      theGeometry->fromWkb(( unsigned char * ) pzBlob, pnBlob );
    }

    if ( theGeometry && ( theGeometry->type() == 3 ) && ( selId != 0 ) )
    {
      // line/polygon geometry is not cached!
      char *geo;
      int geolen;
      updateWayWKB( selId, ( mFeatureType == LineType ) ? 0 : 1, &geo, &geolen );
      theGeometry->fromWkb(( unsigned char * ) geo, ( size_t ) geolen );
    }

    if ( mSelectUseIntersect )
    {
      // when using intersect, some features might be ignored if they don't intersect the selection rect
      // intersect is a costly operation, use rectangle converted to geos for less conversions
      // (this is usually used during identification of an object)
      if ( theGeometry->intersects( mSelectionRectangleGeom ) )
        fetchMoreRows = false;
    }
    else if ( !mSelectionRectangle.isEmpty() )
    {
      // when using selection rectangle but without exact intersection, check only overlap of bounding box
      // (usually used when drawing)
      if ( mSelectionRectangle.intersects( theGeometry->boundingBox() ) )
        fetchMoreRows = false;
    }
    else
    {
      // no filter => always accept the new feature
      // (used in attribute table)
      fetchMoreRows = false;
    }

    // delete the geometry (if any) in case we're not going to use it anyway
    if ( fetchMoreRows )
      delete theGeometry;
  }
  while ( fetchMoreRows && (( rc = sqlite3_step( stmt ) ) == SQLITE_ROW ) );

  // no more features to return
  if ( rc == SQLITE_DONE )
  {
    sqlite3_exec( mDatabase, "COMMIT;", 0, 0, 0 );
    feature.setValid( false );
    return false;
  }

  // fetch feature's geometry
  if ( fetchGeometry )
  {
    feature.setGeometry( theGeometry );
  }
  else
  {
    delete theGeometry; // make sure it's deleted
  }

  // fetch attributes
  QgsAttributeList::const_iterator iter;
  for ( iter = fetchAttrs.begin(); iter != fetchAttrs.end(); ++iter )
  {
    switch ( *iter )
    {
      case TimestampAttr:
        feature.addAttribute( TimestampAttr, QString::fromUtf8( selTimestamp ) );
        break;
      case UserAttr:
        feature.addAttribute( UserAttr, QString::fromUtf8( selUser ) );
        break;
      case TagAttr:
        feature.addAttribute(TagAttr, tagsForObject("way",selId));
        break;
      default: // suppose it's a custom tag
        if ( *iter >= CustomTagAttr && *iter < CustomTagAttr + mCustomTagsList.count() )
        {
          feature.addAttribute( *iter, tagForObject( "way", selId, mCustomTagsList[*iter-CustomTagAttr] ) );
        }
    }
  }
  feature.setFeatureId( selId );
  feature.setValid( true );
  return true;
}



QString QgsOSMDataProvider::tagForObject( const char* type, int id, QString tagKey )
{
  sqlite3_bind_int( mCustomTagsStmt, 1, id );
  sqlite3_bind_text( mCustomTagsStmt, 2, type, -1, 0 );
  QByteArray tag = tagKey.toUtf8(); // must keep the byte array until the query is run
  sqlite3_bind_text( mCustomTagsStmt, 3, tag.data(), -1, 0 );

  QString value;
  int rc;

  if (( rc = sqlite3_step( mCustomTagsStmt ) ) == SQLITE_ROW )
  {
    const char* tagVal = ( const char* ) sqlite3_column_text( mCustomTagsStmt, 0 );
    value = QString::fromUtf8( tagVal );
  }
  else
  {
    // tag wasn't found
    //QgsDebugMsg(QString("tag for object failed (%1): type %2 id %3 tag %4").arg(rc).arg(type).arg(id).arg(tagKey));
  }

  sqlite3_reset( mCustomTagsStmt ); // make ready for next retrieval
  return value;
}


QString QgsOSMDataProvider::tagsForObject( const char* type, int id )
{
  sqlite3_bind_int( mTagsStmt, 1, id );
  sqlite3_bind_text( mTagsStmt, 2, type, -1, 0 );

  QString tags;
  int rc;

  while (( rc = sqlite3_step( mTagsStmt ) ) == SQLITE_ROW )
  {
    const char* tagKey = ( const char* ) sqlite3_column_text( mTagsStmt, 0 );
    const char* tagVal = ( const char* ) sqlite3_column_text( mTagsStmt, 1 );
    QString key = QString::fromUtf8( tagKey );
    QString val = QString::fromUtf8( tagVal );

    // we concatenate tags this way: "key1"="val1","key2"="val2","key3"="val3"
    // -all ; in keyX and valX are replaced by ;;
    // -all , in keyX and valX are replaced by ;
    // -all - in keyX and valX are replaced by --
    // -all = in keyX and valX are replaced by -
    key = key.replace( ';', ";;" );
    val = val.replace( ';', ";;" );
    key = key.replace( ',', ";" );
    val = val.replace( ',', ";" );

    key = key.replace( '-', "--" );
    val = val.replace( '-', "--" );
    key = key.replace( '=', "-" );
    val = val.replace( '=', "-" );

    if ( tags.count() > 0 )
      tags += ",";

    tags += QString( "\"%1\"=\"%2\"" ).arg( key ).arg( val );
  }

  if ( rc != SQLITE_DONE )
  {
    // no tags for object
    //QgsDebugMsg(QString("tags for object failed: type %1 id %2").arg(type).arg(id));
  }

  sqlite3_reset( mTagsStmt ); // make ready for next retrieval
  return tags;
}


QgsRectangle QgsOSMDataProvider::extent()
{
  return QgsRectangle( xMin, yMin, xMax, yMax );
}


QGis::WkbType QgsOSMDataProvider::geometryType() const
{
  if ( mFeatureType == PointType )
    return QGis::WKBPoint;
  if ( mFeatureType == LineType )
    return QGis::WKBLineString;
  if ( mFeatureType == PolygonType )
    return QGis::WKBPolygon;

  return QGis::WKBUnknown;
}


long QgsOSMDataProvider::featureCount() const
{
  sqlite3_stmt* countStmt;
  long cnt = 0;

  if ( mFeatureType == PointType )
    sqlite3_prepare_v2( mDatabase, "SELECT COUNT(*) FROM node where usage=0", -1, &countStmt, 0 );
  else if ( mFeatureType == LineType )
//        sqlite3_prepare_v2(mDatabase, "SELECT COUNT(*) FROM way WHERE closed=0", -1, &countStmt, 0);
    sqlite3_prepare_v2( mDatabase, "SELECT count(*) FROM way w WHERE w.closed=0 AND w.status<>'R' AND w.u=1", -1, &countStmt, 0 );
  else if ( mFeatureType == PolygonType )
//        sqlite3_prepare_v2(mDatabase, "SELECT COUNT(*) FROM way WHERE closed=1", -1, &countStmt, 0);
    sqlite3_prepare_v2( mDatabase, "SELECT count(*) FROM way w WHERE w.closed=1 AND w.status<>'R' AND w.u=1", -1, &countStmt, 0 );
  else return -1;

  if ( sqlite3_step( countStmt ) == SQLITE_ROW )
    cnt = sqlite3_column_int( countStmt, 0 );

  sqlite3_finalize( countStmt );
  QgsDebugMsg( QString( "Type:%1,FeatureCnt:%2" ).arg( mFeatureType ).arg( cnt ) );
  return cnt;
}


uint QgsOSMDataProvider::fieldCount() const
{
  return mAttributeFields.size();;
}


bool QgsOSMDataProvider::isValid()
{
  return mValid;
}


QString QgsOSMDataProvider::name() const
{
  // return key representing this provider
  return TEXT_PROVIDER_KEY;
}


QString QgsOSMDataProvider::description() const
{
  // return description of this provider
  return TEXT_PROVIDER_DESCRIPTION;
}


QGISEXTERN QgsOSMDataProvider *classFactory( const QString *uri )
{
  return new QgsOSMDataProvider( *uri );
}


/**
 * Required key function (used to map the plugin to a data store type)
 */
QGISEXTERN QString providerKey()
{
  // return key representing this provider
  return TEXT_PROVIDER_KEY;
}


/**
 * Required description function
 */
QGISEXTERN QString description()
{
  // just return simple one-line provider description
  return TEXT_PROVIDER_DESCRIPTION;
}


/**
 * Required isProvider function. Used to determine if this shared library
 * is a data provider plugin
 */
QGISEXTERN bool isProvider()
{
  // just return positive answer
  return true;
}


const QgsFieldMap & QgsOSMDataProvider::fields() const
{
  return mAttributeFields;
}


QgsCoordinateReferenceSystem QgsOSMDataProvider::crs()
{
  return QgsCoordinateReferenceSystem();    // use default CRS - it's WGS84
}


void QgsOSMDataProvider::rewind()
{
  // we have to reset precompiled database statement; thanx to this action the first feature
  // (returned by the query) will be selected again with the next calling of sqlite3_step(mDatabaseStmt)
  if ( mDatabaseStmt )
    sqlite3_reset( mDatabaseStmt );
}


int QgsOSMDataProvider::freeFeatureId()
{
  // todo: optimalization - wouldn't be better to keep minimum id in meta table?
  const char *zSql = "SELECT min(id) FROM (SELECT min(id) id FROM node \
                                       UNION SELECT min(id) id FROM way \
                                       UNION SELECT min(id) id FROM relation)";

  sqlite3_stmt *pStmt;
  int rc = sqlite3_prepare_v2( mDatabase, zSql, -1, &pStmt, 0 );

  if ( rc != SQLITE_OK )
  {
    QgsDebugMsg( QString( "Getting pseudo id for new feature failed (1)." ) );
    return 0;
  }

  rc = sqlite3_step( pStmt );
  if ( rc != SQLITE_ROW )
  {
    QgsDebugMsg( QString( "Getting pseudo id for new feature failed (2)." ) );
    return 0;
  }

  int newFeatureId = sqlite3_column_int( pStmt, 0 ) - 1;

  // destroy database statement
  sqlite3_finalize( pStmt );
  return ( newFeatureId >= 0 ) ? -1 : newFeatureId;
}


bool QgsOSMDataProvider::changeAttributeValues( const QgsChangedAttributesMap & attr_map )
{
  QgsDebugMsg( QString( "In changeAttributeValues(...)." ) );

  // VERY VERY ugly hack to assign custom renderer for OSM layer
  // but probably there's no simple way how to set our custom renderer from python plugin
  if ( attr_map.contains( 0x12345678 ) )
  {
    const QgsAttributeMap& x = attr_map.value( 0x12345678 );
    QgsVectorLayer* layer = ( QgsVectorLayer* ) x.value( 0 ).toUInt();
    QgsDebugMsg( "SETTING CUSTOM RENDERER!" );
    layer->setRenderer( new OsmRenderer( layer->geometryType(), mStyleFileName ) );
  }
  return true;
}


int QgsOSMDataProvider::capabilities() const
{
  return QgsVectorDataProvider::SelectAtId | QgsVectorDataProvider::SelectGeometryAtId;
}


bool QgsOSMDataProvider::updateWayWKB( int wayId, int isClosed, char **geo, int *geolen )
{
  sqlite3_stmt *stmtSelectMembers;
  char sqlSelectMembers[] = "SELECT n.lat, n.lon, n.id FROM way_member wm, node n WHERE wm.way_id=? AND wm.node_id=n.id AND n.u=1 AND wm.u=1 ORDER BY wm.pos_id ASC;";
  if ( sqlite3_prepare_v2( mDatabase, sqlSelectMembers, sizeof( sqlSelectMembers ), &stmtSelectMembers, 0 ) != SQLITE_OK )
  {
    QgsDebugMsg( "Failed to prepare sqlSelectMembers!!!" );
    return false;
  }

  sqlite3_stmt *stmtUpdateWay;
  char sqlUpdateWay[] = "UPDATE way SET wkb=?, membercnt=?, min_lat=?, min_lon=?, max_lat=?, max_lon=? WHERE id=? AND u=1";
  if ( sqlite3_prepare_v2( mDatabase, sqlUpdateWay, sizeof( sqlUpdateWay ), &stmtUpdateWay, 0 ) != SQLITE_OK )
  {
    QgsDebugMsg( "Failed to prepare sqlUpdateWay!!!" );
    return false;
  }

  // create wkb for selected way (if way is closed then it's polygon and it's geometry is different)
  int memberCnt = wayMemberCount( wayId );
  if ( memberCnt == -1 )
    return false;

  double minLat = 1000.0, minLon = 1000.0;
  double maxLat = -1000.0, maxLon = -1000.0;

  if ( !isClosed )
  {
    ( *geo ) = new char[9 + 16 * memberCnt];
    ( *geolen ) = 9 + 16 * memberCnt;

    std::memset(( *geo ), 0, 9 + 16 * memberCnt );

    ( *geo )[0] = QgsApplication::endian();
    ( *geo )[( *geo )[0] == QgsApplication::NDR ? 1 : 4] = QGis::WKBLineString;
    std::memcpy(( *geo ) + 5, &memberCnt, 4 );

    sqlite3_bind_int( stmtSelectMembers, 1, wayId );

    int step_result;
    int i = 0;
    while (( step_result = sqlite3_step( stmtSelectMembers ) ) != SQLITE_DONE )
    {
      if ( step_result != SQLITE_ROW )
      {
        QgsDebugMsg( QString( "Selecting members of way %1 failed." ).arg( wayId ) );
        break;
      }

      double selLat = sqlite3_column_double( stmtSelectMembers, 0 );
      double selLon = sqlite3_column_double( stmtSelectMembers, 1 );

      if ( selLat < minLat ) minLat = selLat;
      if ( selLon < minLon ) minLon = selLon;
      if ( selLat > maxLat ) maxLat = selLat;
      if ( selLon > maxLon ) maxLon = selLon;

      std::memcpy(( *geo ) + 9 + 16 * i, &selLon, sizeof( double ) );
      std::memcpy(( *geo ) + 9 + 16 * i + 8, &selLat, sizeof( double ) );
      i++;
    }

    sqlite3_bind_blob( stmtUpdateWay, 1, ( *geo ), 9 + 16 * memberCnt, SQLITE_TRANSIENT );
  }
  else
  {
    // it's a polygon
    int ringsCnt = 1;
    memberCnt++;
    ( *geo ) = new char[13 + 16 * memberCnt];
    ( *geolen ) = 13 + 16 * memberCnt;
    std::memset(( *geo ), 0, 13 + 16 * memberCnt );
    ( *geo )[0] = QgsApplication::endian();
    ( *geo )[( *geo )[0] == QgsApplication::NDR ? 1 : 4] = QGis::WKBPolygon;
    std::memcpy(( *geo ) + 5, &ringsCnt, 4 );
    std::memcpy(( *geo ) + 9, &memberCnt, 4 );

    sqlite3_bind_int( stmtSelectMembers, 1, wayId );

    int step_result;
    int i = 0;
    double firstLat = -1000.0;
    double firstLon = -1000.0;
    while (( step_result = sqlite3_step( stmtSelectMembers ) ) != SQLITE_DONE )
    {
      if ( step_result != SQLITE_ROW )
      {
        QgsDebugMsg( QString( "Selecting members of polygon %1 failed." ).arg( wayId ) );
        break;
      }

      double selLat = sqlite3_column_double( stmtSelectMembers, 0 );
      double selLon = sqlite3_column_double( stmtSelectMembers, 1 );

      if ( selLat < minLat ) minLat = selLat;
      if ( selLon < minLon ) minLon = selLon;
      if ( selLat > maxLat ) maxLat = selLat;
      if ( selLon > maxLon ) maxLon = selLon;

      std::memcpy(( *geo ) + 13 + 16 * i, &selLon, sizeof( double ) );
      std::memcpy(( *geo ) + 13 + 16 * i + 8, &selLat, sizeof( double ) );

      if ( firstLat == -1000.0 )
        firstLat = selLat;
      if ( firstLon == -1000.0 )
        firstLon = selLon;
      i++;
    }
    // add last polygon line
    std::memcpy(( *geo ) + 13 + 16 * i, &firstLon, sizeof( double ) );
    std::memcpy(( *geo ) + 13 + 16 * i + 8, &firstLat, sizeof( double ) );

    sqlite3_bind_blob( stmtUpdateWay, 1, ( *geo ), 13 + 16 * memberCnt, SQLITE_TRANSIENT );
  }

  sqlite3_reset( stmtSelectMembers );

  // now update way record
  sqlite3_bind_int( stmtUpdateWay, 2, memberCnt );
  sqlite3_bind_double( stmtUpdateWay, 3, minLat );
  sqlite3_bind_double( stmtUpdateWay, 4, minLon );
  sqlite3_bind_double( stmtUpdateWay, 5, maxLat );
  sqlite3_bind_double( stmtUpdateWay, 6, maxLon );
  sqlite3_bind_int( stmtUpdateWay, 7, wayId );

  if ( sqlite3_step( stmtUpdateWay ) != SQLITE_DONE )
  {
    QgsDebugMsg( QString( "Updating way with id=%1 failed." ).arg( wayId ) );
    return false;
  }
  sqlite3_reset( stmtUpdateWay );

  // destroy database statements
  sqlite3_finalize( stmtSelectMembers );
  sqlite3_finalize( stmtUpdateWay );
  return true;
}


bool QgsOSMDataProvider::updateNodes()
{
  char sqlUpdateNodes[] = "update node set usage=(select count(distinct way_id) from way_member wm where wm.node_id=id);";
  if ( sqlite3_exec( mDatabase, sqlUpdateNodes, 0, 0, 0 ) != SQLITE_OK )
  {
    QgsDebugMsg( "Failed to update node table!!!" );
    return false;
  }
  return true;
}


bool QgsOSMDataProvider::removeIncorrectWays()
{
  sqlite3_exec( mDatabase, "BEGIN;", 0, 0, 0 );
  int wayId;

  char sqlRemoveWay[] = "delete from way where id=?";
  sqlite3_stmt *stmtRemoveWay;
  if ( sqlite3_prepare_v2( mDatabase, sqlRemoveWay, sizeof( sqlRemoveWay ), &stmtRemoveWay, 0 ) != SQLITE_OK )
  {
    QgsDebugMsg( "failed to prepare stmtRemoveWay!!!" );
  }

  char sqlRemoveWayMembers[] = "delete from way_member where way_id=?";
  sqlite3_stmt *stmtRemoveWayMembers;
  if ( sqlite3_prepare_v2( mDatabase, sqlRemoveWayMembers, sizeof( sqlRemoveWayMembers ), &stmtRemoveWayMembers, 0 ) != SQLITE_OK )
  {
    QgsDebugMsg( "failed to prepare stmtRemoveWayMembers!!!" );
  }

  char sqlRemoveWayTags[] = "delete from tag where object_id=? and object_type='way'";
  sqlite3_stmt *stmtRemoveWayTags;
  if ( sqlite3_prepare_v2( mDatabase, sqlRemoveWayTags, sizeof( sqlRemoveWayTags ), &stmtRemoveWayTags, 0 ) != SQLITE_OK )
  {
    QgsDebugMsg( "failed to prepare stmtRemoveWayTags!!!" );
  }

  char sqlSelectWays[] = "select distinct way_id wid from way_member wm where not exists(select 1 from node n where wm.node_id=n.id);";
  sqlite3_stmt *stmtSelectWays;
  if ( sqlite3_prepare_v2( mDatabase, sqlSelectWays, sizeof( sqlSelectWays ), &stmtSelectWays, 0 ) != SQLITE_OK )
  {
    QgsDebugMsg( "failed to prepare stmtSelectWays!!!" );
  }

  int i = 0;
  while ( sqlite3_step( stmtSelectWays ) == SQLITE_ROW )
  {
    // remove both way, tag records, way_member records
    wayId = sqlite3_column_int( stmtSelectWays, 0 );

    sqlite3_bind_int( stmtRemoveWay, 1, wayId );
    sqlite3_bind_int( stmtRemoveWayMembers, 1, wayId );
    sqlite3_bind_int( stmtRemoveWayTags, 1, wayId );

    // run steps
    if ( sqlite3_step( stmtRemoveWay ) != SQLITE_DONE )
    {
      QgsDebugMsg( "Removing way failed." );
      sqlite3_exec( mDatabase, "ROLLBACK;", 0, 0, 0 );
      return false;
    }
    if ( sqlite3_step( stmtRemoveWayMembers ) != SQLITE_DONE )
    {
      QgsDebugMsg( "Removing way members failed." );
      sqlite3_exec( mDatabase, "ROLLBACK;", 0, 0, 0 );
      return false;
    }
    if ( sqlite3_step( stmtRemoveWayTags ) != SQLITE_DONE )
    {
      QgsDebugMsg( "Removing way tags failed." );
      sqlite3_exec( mDatabase, "ROLLBACK;", 0, 0, 0 );
      return false;
    }

    // make statements ready for the next run
    sqlite3_reset( stmtRemoveWay );
    sqlite3_reset( stmtRemoveWayMembers );
    sqlite3_reset( stmtRemoveWayTags );
    i++;
  }
  // destroy database statements
  sqlite3_finalize( stmtRemoveWay );
  sqlite3_finalize( stmtRemoveWayMembers );
  sqlite3_finalize( stmtRemoveWayTags );
  sqlite3_finalize( stmtSelectWays );

  // commit actions
  sqlite3_exec( mDatabase, "COMMIT;", 0, 0, 0 );

  return true;
}


bool QgsOSMDataProvider::postparsing()
{
  if ( mInitObserver ) mInitObserver->setProperty( "osm_status", QVariant( "Post-parsing: Nodes." ) );
  if ( mInitObserver ) mInitObserver->setProperty( "osm_max", QVariant( 3 ) );
  if ( mInitObserver ) mInitObserver->setProperty( "osm_value", QVariant( 0 ) );

  // update node table
  updateNodes();

  if ( mInitObserver ) mInitObserver->setProperty( "osm_status", QVariant( "Post-parsing: Removing incorrect ways." ) );
  if ( mInitObserver ) mInitObserver->setProperty( "osm_max", QVariant( 3 ) );
  if ( mInitObserver ) mInitObserver->setProperty( "osm_value", QVariant( 1 ) );

  removeIncorrectWays();

  if ( mInitObserver ) mInitObserver->setProperty( "osm_status", QVariant( "Post-parsing: Caching ways geometries." ) );
  if ( mInitObserver ) mInitObserver->setProperty( "osm_max", QVariant( 3 ) );
  if ( mInitObserver ) mInitObserver->setProperty( "osm_value", QVariant( 2 ) );

  // select ways, for each of them compute its wkb and store it into database
  sqlite3_exec( mDatabase, "BEGIN;", 0, 0, 0 );

  int wayId, isClosed;
  // prepare select: get information about one specified point
  QString cmd = QString( "SELECT id, closed FROM way;" );
  QByteArray cmd_bytes = cmd.toAscii();
  const char *ptr = cmd_bytes.data();

  sqlite3_stmt *databaseStmt;
  if ( sqlite3_prepare_v2( mDatabase, ptr, cmd_bytes.size(), &databaseStmt, 0 ) != SQLITE_OK )
  {
    QgsDebugMsg( QString( "Creating BLOBs in postprocessing failed." ) );
    sqlite3_exec( mDatabase, "ROLLBACK;", 0, 0, 0 );
    return false;
  }

  while ( sqlite3_step( databaseStmt ) == SQLITE_ROW )
  {
    if (( mInitObserver ) && ( mInitObserver->property( "osm_stop_parsing" ).toInt() == 1 ) )
    {
      QgsDebugMsg( QString( "Loading the OSM data was stopped." ) );
      sqlite3_exec( mDatabase, "ROLLBACK;", 0, 0, 0 );
      return false;
    }
    wayId = sqlite3_column_int( databaseStmt, 0 );
    isClosed = sqlite3_column_int( databaseStmt, 1 );
    char *geo;
    int geolen;
    updateWayWKB( wayId, isClosed, &geo, &geolen );   // todo: return value!
  }

  // destroy database statements
  sqlite3_finalize( databaseStmt );

  // commit our actions
  sqlite3_exec( mDatabase, "COMMIT;", 0, 0, 0 );

  if ( mInitObserver ) mInitObserver->setProperty( "osm_max", QVariant( 3 ) );
  if ( mInitObserver ) mInitObserver->setProperty( "osm_value", QVariant( 3 ) );

  return true;
}


bool QgsOSMDataProvider::loadOsmFile( QString osm_filename )
{
  QFile f( osm_filename );
  if ( !f.exists() )
    return false;

  if ( mInitObserver ) mInitObserver->setProperty( "osm_status", QVariant( "Parsing the OSM file." ) );

  OsmHandler *handler = new OsmHandler( &f, mDatabase );
  QXmlSimpleReader reader;
  reader.setContentHandler( handler );

  const int sectorSize = 8192;
  int cntSectors = f.size() / sectorSize;
  if ( mInitObserver ) mInitObserver->setProperty( "osm_max", QVariant( cntSectors ) );

  if ( !f.open( QIODevice::ReadOnly ) )
  {
    QgsDebugMsg( "Unable to open the OSM file!" );
    return false;
  }

  QXmlInputSource source;
  source.setData( f.read( sectorSize ) );
  int sector = 1;

  QgsDebugMsg( QString( "Parsing file: %1" ).arg( osm_filename ) );
  bool res = reader.parse( &source, true );
  while ( !f.atEnd() )
  {
    if (( mInitObserver ) && ( mInitObserver->property( "osm_stop_parsing" ).toInt() == 1 ) )
    {
      QgsDebugMsg( QString( "Parsing the OSM XML was stopped." ) );
      sqlite3_exec( mDatabase, "ROLLBACK;", 0, 0, 0 );
      return false;
    }
    if (( !res ) && ( sector < cntSectors - 2 ) )
    {
      // osm file parsing failed
      if ( mInitObserver ) mInitObserver->setProperty( "osm_failure", QVariant( handler->errorString() ) );
      return false;
    }
    // parsing process can continue
    source.setData( f.read( sectorSize ) );
    if ( mInitObserver ) mInitObserver->setProperty( "osm_value", QVariant( ++sector ) );
    res = reader.parseContinue();
  }
  f.close();

  QgsDebugMsg( "Parsing complete. Result: " + QString::number( res ) );

  QgsDebugMsg( "Creating indexes..." );
  if ( mInitObserver ) mInitObserver->setProperty( "osm_status", QVariant( "Creating indexes." ) );
  createIndexes();

  sqlite3_exec( mDatabase, "COMMIT;", 0, 0, 0 );

  QgsDebugMsg( "Starting postprocessing..." );
  if (( mInitObserver ) && ( mInitObserver->property( "osm_stop_parsing" ).toInt() == 1 ) )
  {
    QgsDebugMsg( QString( "Loading the OSM data was stopped." ) );
    sqlite3_exec( mDatabase, "ROLLBACK;", 0, 0, 0 );
    return false;
  }

//    if (mInitObserver) mInitObserver->setProperty("osm_status", QVariant("Running post-parsing actions."));
  postparsing();
  QgsDebugMsg( "Postprocessing complete." );

  QgsDebugMsg( "Creating triggers..." );
  if ( mInitObserver ) mInitObserver->setProperty( "osm_status", QVariant( "Creating triggers." ) );
  createTriggers();

  if ( mInitObserver ) mInitObserver->setProperty( "osm_done", QVariant( true ) );

  // storing osm file last modified information into database

  QFile osmFile( mFileName );
  QFileInfo osmFileInfo( osmFile );

  QString cmd = "INSERT INTO meta ( key, val ) VALUES ('osm-file-last-modified','" + osmFileInfo.lastModified().toString( DATE_TIME_FMT ) + "');";
  QByteArray cmd_bytes  = cmd.toAscii();
  const char *ptr = cmd_bytes.data();

  if ( sqlite3_exec( mDatabase, ptr, 0, 0, 0 ) != SQLITE_OK )
  {
    cout << "Storing osm-file-last-modified info into database failed." << endl;
    // its not fatal situation, lets continue..
  }

  QString cmd2 = "INSERT INTO meta ( key, val ) VALUES ('osm-plugin-version','" + PLUGIN_VERSION + "');";
  QByteArray cmd_bytes2  = cmd2.toAscii();
  const char *ptr2 = cmd_bytes2.data();

  if ( sqlite3_exec( mDatabase, ptr2, 0, 0, 0 ) != SQLITE_OK )
  {
    cout << "Storing osm-plugin-version info into database failed." << endl;
    return false;
  }

  // store information got with handler into provider member variables
  xMin = handler->xMin;    // boundaries defining the area of all features
  xMax = handler->xMax;
  yMin = handler->yMin;
  yMax = handler->yMax;

  // storing boundary information into database
  QString cmd3 = QString( "INSERT INTO meta ( key, val ) VALUES ('default-area-boundaries','%1-%2-%3-%4');" )
                 .arg( xMin, 0, 'f', 20 ).arg( yMin, 0, 'f', 20 ).arg( xMax, 0, 'f', 20 ).arg( yMax, 0, 'f', 20 );
  QByteArray cmd_bytes3  = cmd3.toAscii();
  const char *ptr3 = cmd_bytes3.data();

  if ( sqlite3_exec( mDatabase, ptr3, 0, 0, 0 ) != SQLITE_OK )
  {
    cout << "Storing default area boundaries information into database failed." << endl;
    // its not critical situation
  }

  sqlite3_exec( mDatabase, "COMMIT;", 0, 0, 0 );

  if (( mInitObserver ) && ( mInitObserver->property( "osm_stop_parsing" ).toInt() == 1 ) )
  {
    QgsDebugMsg( QString( "Loading the OSM data was stopped." ) );
    sqlite3_exec( mDatabase, "ROLLBACK;", 0, 0, 0 );
    return false;
  }
  sqlite3_exec( mDatabase, "COMMIT;", 0, 0, 0 );
  return true;
}


bool QgsOSMDataProvider::createDatabaseSchema()
{
  QgsDebugMsg( "Creating database schema for OSM..." );

  const char* createTables[] =
  {
    "CREATE TABLE node ( i INTEGER PRIMARY KEY, u INTEGER DEFAULT 1, id INTEGER, lat REAL, lon REAL, timestamp VARCHAR2, user VARCHAR2, usage INTEGER DEFAULT 0, status VARCHAR2 DEFAULT 'N' );",
    "CREATE TABLE way ( i INTEGER PRIMARY KEY, u INTEGER DEFAULT 1, id INTEGER, wkb BLOB, timestamp VARCHAR2, user VARCHAR2, membercnt INTEGER DEFAULT 0, closed INTEGER, min_lat REAL, min_lon REAL, max_lat REAL, max_lon REAL, status VARCHAR2 DEFAULT 'N' );",
    "CREATE TABLE relation ( i INTEGER PRIMARY KEY, u INTEGER DEFAULT 1, id INTEGER, type VARCHAR2, timestamp VARCHAR2, user VARCHAR2, status VARCHAR2 DEFAULT 'N' );",

    "CREATE TABLE way_member ( i INTEGER PRIMARY KEY, u INTEGER DEFAULT 1, way_id INTEGER, pos_id INTEGER, node_id INTEGER );",
    "CREATE TABLE relation_member ( i INTEGER PRIMARY KEY, u INTEGER DEFAULT 1, relation_id INTEGER, pos_id INTEGER, member_id INTEGER, member_type VARCHAR2, role VARCHAR2 );",

    "CREATE TABLE tag ( i INTEGER PRIMARY KEY, u INTEGER DEFAULT 1, object_id INTEGER, object_type VARCHAR2, key VARCHAR2, val VARCHAR2 );",
    "CREATE TABLE meta ( key VARCHAR2, val VARCHAR2, PRIMARY KEY (key,val) );",

    // OSM 0.6 API requires storing version_id to each feature -> adding table version
    "CREATE TABLE version ( object_id INTEGER, object_type VARCHAR2, version_id INTEGER, PRIMARY KEY (object_id, object_type) );",
    "CREATE TABLE change_step ( change_id INTEGER PRIMARY KEY, change_type VARCHAR2, tab_name VARCHAR2, row_id INTEGER, col_name VARCHAR2, old_value VARCHAR2, new_value VARCHAR2 );"
  };

  int count = sizeof( createTables ) / sizeof( const char* );

  for ( int i = 0; i < count; i++ )
  {
    if ( sqlite3_exec( mDatabase, createTables[i], 0, 0, &mError ) != SQLITE_OK )
    {
      QgsDebugMsg( QString( "Creating table \"%1\"" ).arg( QString::fromUtf8( createTables[i] ) ) );
      return false;
    }
  }
  // database schema created successfully
  QgsDebugMsg( "Database schema for OSM was created successfully." );
  return true;
}


bool QgsOSMDataProvider::createIndexes()
{
  const char* indexes[] =
  {
    "CREATE INDEX IF NOT EXISTS main.ix_node_id ON node ( id );",
    "CREATE INDEX IF NOT EXISTS main.ix_node_us ON node ( usage,status );",
    "CREATE INDEX IF NOT EXISTS main.ix_way_id ON way ( id );",
    "CREATE INDEX IF NOT EXISTS main.ix_way_cs ON way ( closed,status );",
    "CREATE INDEX IF NOT EXISTS main.ix_wm_wid ON way_member ( way_id );",
    "CREATE INDEX IF NOT EXISTS main.ix_wm_nid ON way_member ( node_id );",
    "CREATE INDEX IF NOT EXISTS main.ix_rm_rid ON relation_member ( relation_id );",
    "CREATE INDEX IF NOT EXISTS main.ix_tag_id_type ON tag ( object_id ASC, object_type ASC );",
    "CREATE INDEX IF NOT EXISTS main.ix_version_id_type ON version ( object_id, object_type );"
  };
  int count = sizeof( indexes ) / sizeof( const char* );

  if ( mInitObserver ) mInitObserver->setProperty( "osm_max", QVariant( count ) );

  for ( int i = 0; i < count; i++ )
  {
    if ( sqlite3_exec( mDatabase, indexes[i], 0, 0, &mError ) != SQLITE_OK )
    {
      QgsDebugMsg( QString( "Creating index \"%1\" failed." ).arg( QString::fromUtf8( indexes[i] ) ) );
      // absence of index shouldn't be critical for this application -> but everything will be slow
    }
    if ( mInitObserver ) mInitObserver->setProperty( "osm_value", QVariant( i + 1 ) );
  }
  return true;
}


bool QgsOSMDataProvider::createTriggers()
{
  const char* triggers[] =
  {
    // tag table
    "create trigger if not exists main.trg_tag_oi_update after update of object_id on tag begin                                                               insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) values ('U','tag',old.i,'object_id',old.object_id,new.object_id); end;",

    "create trigger if not exists main.trg_tag_ot_update after update of object_type on tag begin                                                               insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) values ('U','tag',old.i,'object_type',old.object_type,new.object_type); end;",

    "create trigger if not exists main.trg_tag_k_update after update of key on tag begin                                                                        insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) values ('U','tag',old.i,'key',old.key,new.key); end;",

    "create trigger if not exists main.trg_tag_v_update after update of val on tag begin                                                                 insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) values ('U','tag',old.i,'val',old.val,new.val); end;",

    "create trigger if not exists main.trg_tag_insert after insert on tag begin                                                                               insert into change_step (change_type,tab_name,row_id) values ('I','tag',new.i); end;",

    "create trigger if not exists main.trg_tag_delete before delete on tag begin                                                                              insert into change_step (change_type,tab_name,row_id) values ('D','tag',old.i);                                                                      update tag set u=0 where i=old.i; select raise(ignore); end;",

    // node table
    "create trigger if not exists main.trg_node_lat_update after update of lat on node begin                                                                  insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) values ('U','node',old.i,'lat',old.lat,new.lat); end;",

    "create trigger if not exists main.trg_node_lon_update after update of lon on node begin                                                                  insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) values ('U','node',old.i,'lon',old.lon,new.lon); end;",

    "create trigger if not exists main.trg_node_t_update after update of timestamp on node begin                                                              insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) values ('U','node',old.i,'timestamp',old.timestamp,new.timestamp); end;",

    "create trigger if not exists main.trg_node_use_update after update of user on node begin                                                                 insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) values ('U','node',old.i,'user',old.user,new.user); end;",

    "create trigger if not exists main.trg_node_usa_update after update of usage on node begin                                                                insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) values ('U','node',old.i,'usage',old.usage,new.usage); end;",

    "create trigger if not exists main.trg_node_s_update after update of status on node begin                                                                 insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) values ('U','node',old.i,'status',old.status,new.status); end;",

    "create trigger if not exists main.trg_node_insert after insert on node begin                                                                             insert into change_step (change_type,tab_name,row_id) values ('I','node',new.i); end;",

    "create trigger if not exists main.trg_node_delete before delete on node begin                                                                             insert into change_step (change_type,tab_name,row_id) values ('D','node',old.i);                                                                     update node set u=0 where i=old.i; select raise(ignore); end;",

    // way table
    "create trigger if not exists main.trg_way_t_update after update of timestamp on way begin                                                                insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) values ('U','way',old.i,'timestamp',old.timestamp,new.timestamp); end;",

    "create trigger if not exists main.trg_way_u_update after update of user on way begin                                                                     insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) values ('U','way',old.i,'user',old.user,new.user); end;",

    "create trigger if not exists main.trg_way_c_update after update of closed on way begin                                                                   insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) values ('U','way',old.i,'closed',old.closed,new.closed); end;",

    "create trigger if not exists main.trg_way_s_update after update of status on way begin                                                                   insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) values ('U','way',old.i,'status',old.status,new.status); end;",

    "create trigger if not exists main.trg_way_w_update after update of wkb on way begin                                                                   insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) values ('U','way',old.i,'wkb','',''); end;",

    "create trigger if not exists main.trg_way_insert after insert on way begin                                                                               insert into change_step (change_type,tab_name,row_id) values ('I','way',new.i); end;",

    "create trigger if not exists main.trg_way_delete before delete on way begin                                                                               insert into change_step (change_type,tab_name,row_id) values ('D','way',old.i);                                                                      update way set u=0 where i=old.i; select raise(ignore); end;",

    // relation table
    "create trigger if not exists main.trg_relation_ty_update after update of type on relation begin                                                         insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) values ('U','relation',old.i,'type',old.type,new.type); end;",

    "create trigger if not exists main.trg_relation_ti_update after update of timestamp on relation begin                                                    insert into change_step  (change_type,tab_name,row_id,col_name,old_value,new_value) values ('U','relation',old.i,'timestamp',old.timestamp,new.timestamp); end;",

    "create trigger if not exists main.trg_relation_u_update after update of user on relation begin                                                          insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) values ('U','relation',old.i,'user',old.user,new.user); end;",

    "create trigger if not exists main.trg_relation_s_update after update of status on relation begin                                                        insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) values ('U','relation',old.i,'status',old.status,new.status); end;",

    "create trigger if not exists main.trg_relation_insert after insert on relation begin                                                                     insert into change_step (change_type,tab_name,row_id) values ('I','relation',new.i); end;",

    "create trigger if not exists main.trg_relation_delete before delete on relation begin                                                                     insert into change_step (change_type,tab_name,row_id) values ('D','relation',old.i);                                                                 update relation set u=0 where i=old.i; select raise(ignore); end;",

    // way_member table
    "create trigger if not exists main.trg_way_member_wi_update after update of way_id on way_member begin                                                    insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) values ('U','way_member',old.i,'way_id',old.way_id,new.way_id); end;",

    "create trigger if not exists main.trg_way_member_pi_update after update of pos_id on way_member begin                                                    insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) values ('U','way_member',old.i,'pos_id',old.pos_id,new.pos_id); end;",

    "create trigger if not exists main.trg_way_member_ni_update after update of node_id on way_member begin                                                   insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) values ('U','way_member',old.i,'node_id',old.node_id,new.node_id); end;",

    "create trigger if not exists main.trg_way_member_insert after insert on way_member begin                                                                 insert into change_step (change_type,tab_name,row_id) values ('I','way_member',new.i); end;",

    "create trigger if not exists main.trg_way_member_delete before delete on way_member begin                                                                 insert into change_step (change_type,tab_name,row_id) values ('D','way_member',old.i);                                                               update way_member set u=0 where i=old.i; select raise(ignore); end;",

    // relation_member table
    "create trigger if not exists main.trg_relation_member_ri_update after update of relation_id on relation_member begin                                     insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) values ('U','relation_member',old.i,'relation_id',old.relation_id,new.relation_id); end;",

    "create trigger if not exists main.trg_relation_member_pi_update after update of pos_id on relation_member begin                                          insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) values ('U','relation_member',old.i,'pos_id',old.pos_id,new.pos_id); end;",

    "create trigger if not exists main.trg_relation_member_mi_update after update of member_id on relation_member begin                                       insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) values ('U','relation_member',old.i,'member_id',old.member_id,new.member_id); end;",

    "create trigger if not exists main.trg_relation_member_mt_update after update of member_type on relation_member begin                                     insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) values ('U','relation_member',old.i,'member_type',old.member_type,new.member_type); end;",

    "create trigger if not exists main.trg_relation_member_r_update after update of role on relation_member begin                                             insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) values ('U','relation_member',old.i,'role',old.role,new.role); end;",

    "create trigger if not exists main.trg_relation_member_insert after insert on relation_member begin                                                       insert into change_step (change_type,tab_name,row_id) values ('I','relation_member',new.i); end;",

    "create trigger if not exists main.trg_relation_member_delete before delete on relation_member begin                                                       insert into change_step (change_type,tab_name,row_id) values ('D','relation_member',old.i);                                                               update relation_member set u=0 where i=old.i; select raise(ignore); end;"
  };

  int count = sizeof( triggers ) / sizeof( const char* );

  if ( mInitObserver ) mInitObserver->setProperty( "osm_max", QVariant( count ) );

  for ( int i = 0; i < count; i++ )
  {
    if ( sqlite3_exec( mDatabase, triggers[i], 0, 0, &mError ) != SQLITE_OK )
    {
      QgsDebugMsg( QString( "Creating trigger \"%1\" failed." ).arg( QString::fromUtf8( triggers[i] ) ) );
      return false;
    }
    if ( mInitObserver ) mInitObserver->setProperty( "osm_value", QVariant( i + 1 ) );
  }
  return true;
}


bool QgsOSMDataProvider::dropDatabaseSchema()
{
  QgsDebugMsg( "Dropping database schema for OSM..." );

  // dropping database schema -> failures of individual droppings are not fatal for the whole process;
  // the more object we will drop, the happies we will be ;)

  const char* drops[] =
  {
    // dropping indexes
    "DROP INDEX IF EXISTS main.ix_node_id;",
    "DROP INDEX IF EXISTS main.ix_node_us;",   // selecting nodes
    "DROP INDEX IF EXISTS main.ix_way_id;",
    "DROP INDEX IF EXISTS main.ix_way_cs;",    // selecting ways
    "DROP INDEX IF EXISTS main.ix_wm_wid;",
    "DROP INDEX IF EXISTS main.ix_wm_nid;",
    "DROP INDEX IF EXISTS main.ix_rm_rid;",
    "DROP INDEX IF EXISTS main.ix_tag_id_type;",
    "DROP INDEX IF EXISTS main.ix_version_id_type;",

    // dropping base tables
    "DROP TABLE node;",
    "DROP TABLE way;",
    "DROP TABLE relation;",
    "DROP TABLE way_member;",
    "DROP TABLE relation_member;",
    "DROP TABLE tag;",
    "DROP TABLE meta;",

    "DROP TABLE version;",
    "DROP TABLE change_step;"
  };
  int count = sizeof( drops ) / sizeof( const char* );

  for ( int i = 0; i < count; i++ )
  {
    if ( sqlite3_exec( mDatabase, drops[i], 0, 0, &mError ) != SQLITE_OK )
    {
      QgsDebugMsg( QString( "Dropping table \"%1\" failed." ).arg( QString::fromUtf8( drops[i] ) ) );
    }
  }

  // database schema droped successfully
  QgsDebugMsg( "Database schema for OSM dropped." );
  return true;
}


bool QgsOSMDataProvider::openDatabase()
{
  QgsDebugMsg( "Opening database." );

  QByteArray dbfn_bytes  = mDatabaseFileName.toAscii();
  const char *ptr = dbfn_bytes.data();

  // open database
  if ( sqlite3_open( ptr, &mDatabase ) != SQLITE_OK )
  {
    mError = ( char * ) "Opening SQLite3 database failed.";
    sqlite3_close( mDatabase );
    return false;
  }
  return true;
};


bool QgsOSMDataProvider::closeDatabase()
{
  QgsDebugMsg( "Closing sqlite3 database." );

  // close database
  if ( sqlite3_close( mDatabase ) != SQLITE_OK )
  {
    mError = ( char * ) "Closing SQLite3 database failed.";
    return false;
  }
  return true;
};


