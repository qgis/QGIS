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
#include "osmfeatureiterator.h"

#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsapplication.h"

#include <QFileInfo>
#include <QDateTime>
#include <QByteArray>

static const QString TEXT_PROVIDER_KEY = "osm";
static const QString TEXT_PROVIDER_DESCRIPTION = "Open Street Map data provider";
static const QString DATE_TIME_FMT = "dd.MM.yyyy HH:mm:ss";
static const QString PROVIDER_VERSION = "0.5.1";

// supported attributes
const char* QgsOSMDataProvider::attr[] = { "timestamp", "user", "tags" };



QgsOSMDataProvider::QgsOSMDataProvider( QString uri )
    : QgsVectorDataProvider( uri )
    , mActiveIterator( 0 )
{
  QgsDebugMsg( "Initializing provider: " + uri );

  mValid = false;

  // set the selection rectangle to null
  mDatabase = NULL;
  mInitObserver = NULL;
  mFeatureType = PointType;    // default feature type ~ point

  // set default boundaries
  mExtent = QgsRectangle( -DEFAULT_EXTENT, -DEFAULT_EXTENT, DEFAULT_EXTENT, DEFAULT_EXTENT );

  // get the filename and other parameters from the URI
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
      {
        mFeatureType = LineType;
      }
      else if ( propValue == "point" )
      {
        mFeatureType = PointType;
      }
      else if ( propValue == "polygon" )
      {
        mFeatureType = PolygonType;
      }
      else
      {
        QgsDebugMsg( "Unknown feature type: " + propValue );
      }
    }
    if ( propName == "observer" )
    {
      // remove observer from the URI
      // (because otherwise it would be saved into project file and would cause crashes)
      QString newProps;
      foreach ( QString p , props )
      {
        if ( !p.startsWith( "observer" ) )
        {
          if ( !newProps.isEmpty() )
            newProps += "&";
          newProps += p;
        }
      }
      QString newUri = uri.left( fileNameEnd + 1 ) + newProps;
      setDataSourceUri( newUri );

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

  // set up attributes depending on the feature type - same attributes for both point and way type so far
  mAttributeFields.append( QgsField( attr[TimestampAttr], QVariant::String, "string" ) );
  mAttributeFields.append( QgsField( attr[UserAttr], QVariant::String, "string" ) );
  mAttributeFields.append( QgsField( attr[TagAttr], QVariant::String, "string" ) );

  // add custom attributes - these were chosen by user through OSM plugin
  for ( int tagId = 0; tagId < mCustomTagsList.count(); ++tagId )
  {
    mAttributeFields.append( QgsField( mCustomTagsList[tagId], QVariant::String, "string" ) );
  }

  // get source file name and database file name
  mFileName = uri.left( fileNameEnd );
  mDatabaseFileName = mFileName + ".db";
  QFile dbFile( mDatabaseFileName );
  QFile osmFile( mFileName );
  bool databaseExists = dbFile.exists();

  // open database and create database schema for OSM data if necessary
  // (find out if such database already exists; if not create it)
  if ( !openDatabase() )
  {
    QgsDebugMsg( "Opening sqlite3 database failed, OSM provider cannot be constructed." );
    closeDatabase();
    return;
  }

  // flag determining if OSM file parsing is necessary
  bool shouldParse = true;

  if ( mFeatureType != PolygonType )
    shouldParse = false;

  // test if db file that belongs to source OSM file already exists and if it has the right version
  if ( databaseExists && isDatabaseCompatibleWithInput( mFileName ) && isDatabaseCompatibleWithProvider() )
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
      // stop the osmprovider construction!
      return;
    }
  }
  else
  {
    // no OSM file parsing was done, we must find out default area boundaries from database meta information
    char sqlSelectBoundary[] = "SELECT val FROM meta WHERE key='default-area-boundaries';";
    sqlite3_stmt *stmtSelectBoundary;
    if ( sqlite3_prepare_v2( mDatabase, sqlSelectBoundary, sizeof( sqlSelectBoundary ), &stmtSelectBoundary, 0 ) != SQLITE_OK )
    {
      QgsDebugMsg( "Getting default area boundary failed." );
      // don't worry, we just let default values in xMax, yMax, xMin and yMin variables
    }
    else
    {
      if ( sqlite3_step( stmtSelectBoundary ) != SQLITE_ROW )
      {
        QgsDebugMsg( "Getting default area boundary failed." );
        // don't worry again, we just let default values in boundary variables
      }
      else
      {
        const unsigned char *boundaries_char = sqlite3_column_text( stmtSelectBoundary, 0 );
        QString boundaries(( const char * ) boundaries_char );

        // boundaries should be string in following format: "xMin:yMin:xMax:yMax"
        QStringList parts = boundaries.split( QChar( ':' ) );
        if ( parts.count() == 4 )
        {
          mExtent.setXMinimum( parts[0].toDouble() );
          mExtent.setYMinimum( parts[1].toDouble() );
          mExtent.setXMaximum( parts[2].toDouble() );
          mExtent.setYMaximum( parts[3].toDouble() );
        }
        else
        {
          QgsDebugMsg( "Default area boundary has invalid format." );
        }
      }
    }

    // destroy database statement
    sqlite3_finalize( stmtSelectBoundary );
  }


  // finally OSM provider is initialized and considered to be valid!
  mValid = true;
}


QgsOSMDataProvider::~QgsOSMDataProvider()
{
  if ( mActiveIterator )
    mActiveIterator->close();

  // close opened sqlite3 database
  if ( mDatabase )
  {
    closeDatabase();
  }
}


bool QgsOSMDataProvider::isDatabaseCompatibleWithInput( QString mFileName )
{
  QFile osmFile( mFileName );
  QFileInfo osmFileInfo( osmFile );
  QDateTime mOsmFileLastModif = osmFileInfo.lastModified();

  char sqlSelectLastModif[] = "SELECT val FROM meta WHERE key='osm-file-last-modified';";
  sqlite3_stmt *stmtSelectLastModif;

  if ( sqlite3_prepare_v2( mDatabase, sqlSelectLastModif, sizeof( sqlSelectLastModif ), &stmtSelectLastModif, 0 ) == SQLITE_OK )
  {
    if ( sqlite3_step( stmtSelectLastModif ) == SQLITE_ROW )
    {
      QString oldOsmLastModifString = ( const char * ) sqlite3_column_text( stmtSelectLastModif, 0 );
      QDateTime oldOsmFileLastModif = QDateTime::fromString( oldOsmLastModifString, DATE_TIME_FMT );

      // each OSM database schema carry info on last-modified of file from which database was created;
      // if value equals to last-modified of current input file then DB file belongs to current input file
      // (in such case we say that "database is compatible with input")
      if ( mOsmFileLastModif.toTime_t() == oldOsmFileLastModif.toTime_t() )
      {
        sqlite3_finalize( stmtSelectLastModif );
        return true;
      }
    }
  }
  // destroy database statement
  sqlite3_finalize( stmtSelectLastModif );
  return false;
}


bool QgsOSMDataProvider::isDatabaseCompatibleWithProvider()
{
  char sqlSelectProviderVer[] = "SELECT val FROM meta WHERE key='osm-provider-version';";
  sqlite3_stmt *stmtSelectProviderVer;

  if ( sqlite3_prepare_v2( mDatabase, sqlSelectProviderVer, sizeof( sqlSelectProviderVer ), &stmtSelectProviderVer, 0 ) == SQLITE_OK )
  {
    if ( sqlite3_step( stmtSelectProviderVer ) == SQLITE_ROW )
    {
      QString osmProviderVersion = ( const char * ) sqlite3_column_text( stmtSelectProviderVer, 0 );

      // each OSM database schema carry info on version of QGIS OSM plugin from which database was created;
      // this provider must be of the same version to be able to manage OSM data correctly
      if ( osmProviderVersion == PROVIDER_VERSION )
      {
        sqlite3_finalize( stmtSelectProviderVer );
        return true;
      }
    }
  }
  // destroy database statement
  sqlite3_finalize( stmtSelectProviderVer );
  return false;
}


QString QgsOSMDataProvider::storageType() const
{
  // just return string reprezenting data storage type
  return tr( "Open Street Map format" );
}



int QgsOSMDataProvider::wayMemberCount( int wayId )
{
  // prepare select: get count of all the WAY members
  char sqlWayMemberCnt[] = "SELECT count(n.id) FROM way_member wm, node n WHERE wm.way_id=? AND wm.node_id=n.id AND wm.u=1 AND n.u=1;";
  sqlite3_stmt *stmtWayMemberCnt;

  if ( sqlite3_prepare_v2( mDatabase, sqlWayMemberCnt, sizeof( sqlWayMemberCnt ), &stmtWayMemberCnt, 0 ) != SQLITE_OK )
  {
    QgsDebugMsg( "sqlite3 statement for selecting waymember count - prepare failed." );
    sqlite3_finalize( stmtWayMemberCnt );
    return -1;
  }

  // bind the way identifier
  sqlite3_bind_int( stmtWayMemberCnt, 1, wayId );

  if ( sqlite3_step( stmtWayMemberCnt ) != SQLITE_ROW )
  {
    QgsDebugMsg( "Cannot get number of way members." );
    sqlite3_finalize( stmtWayMemberCnt );
    return -1;
  }

  // getting the result
  int wayMemberCnt = sqlite3_column_int( stmtWayMemberCnt, 0 );
  // destroy database statement
  sqlite3_finalize( stmtWayMemberCnt );
  return wayMemberCnt;
}


QgsRectangle QgsOSMDataProvider::extent()
{
  return mExtent;
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
    sqlite3_prepare_v2( mDatabase, "SELECT count(*) FROM way w WHERE w.closed=0 AND w.status<>'R' AND w.u=1", -1, &countStmt, 0 );
  else if ( mFeatureType == PolygonType )
    sqlite3_prepare_v2( mDatabase, "SELECT count(*) FROM way w WHERE w.closed=1 AND w.status<>'R' AND w.u=1", -1, &countStmt, 0 );
  else return -1;

  if ( sqlite3_step( countStmt ) == SQLITE_ROW )
    cnt = sqlite3_column_int( countStmt, 0 );

  sqlite3_finalize( countStmt );
  return cnt;
}


QgsFeatureIterator QgsOSMDataProvider::getFeatures( const QgsFeatureRequest& request )
{
  return QgsFeatureIterator( new QgsOSMFeatureIterator( this, request ) );
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


const QgsFields & QgsOSMDataProvider::fields() const
{
  return mAttributeFields;
}


QgsCoordinateReferenceSystem QgsOSMDataProvider::crs()
{
  return QgsCoordinateReferenceSystem( GEOSRID, QgsCoordinateReferenceSystem::PostgisCrsId ); // use WGS84
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

    memset(( *geo ), 0, 9 + 16 * memberCnt );

    ( *geo )[0] = QgsApplication::endian();
    ( *geo )[( *geo )[0] == QgsApplication::NDR ? 1 : 4] = QGis::WKBLineString;
    memcpy(( *geo ) + 5, &memberCnt, 4 );

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

      memcpy(( *geo ) + 9 + 16 * i, &selLon, sizeof( double ) );
      memcpy(( *geo ) + 9 + 16 * i + 8, &selLat, sizeof( double ) );
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
    memset(( *geo ), 0, 13 + 16 * memberCnt );
    ( *geo )[0] = QgsApplication::endian();
    ( *geo )[( *geo )[0] == QgsApplication::NDR ? 1 : 4] = QGis::WKBPolygon;
    memcpy(( *geo ) + 5, &ringsCnt, 4 );
    memcpy(( *geo ) + 9, &memberCnt, 4 );

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

      memcpy(( *geo ) + 13 + 16 * i, &selLon, sizeof( double ) );
      memcpy(( *geo ) + 13 + 16 * i + 8, &selLat, sizeof( double ) );

      if ( firstLat == -1000.0 )
        firstLat = selLat;
      if ( firstLon == -1000.0 )
        firstLon = selLon;
      i++;
    }
    // add last polygon line
    memcpy(( *geo ) + 13 + 16 * i, &firstLon, sizeof( double ) );
    memcpy(( *geo ) + 13 + 16 * i + 8, &firstLat, sizeof( double ) );

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
    QgsDebugMsg( "Storing osm-file-last-modified info into database failed." );
    // its not fatal situation, lets continue..
  }

  QString cmd2 = "INSERT INTO meta ( key, val ) VALUES ('osm-provider-version','" + PROVIDER_VERSION + "');";
  QByteArray cmd_bytes2  = cmd2.toAscii();
  const char *ptr2 = cmd_bytes2.data();

  if ( sqlite3_exec( mDatabase, ptr2, 0, 0, 0 ) != SQLITE_OK )
  {
    QgsDebugMsg( "Storing osm-provider-version info into database failed." );
    return false;
  }

  // store information got with handler into provider member variables
  // boundaries defining the area of all features
  mExtent = QgsRectangle( handler->xMin, handler->yMin, handler->xMax, handler->yMax );

  // storing boundary information into database
  QString cmd3 = QString( "INSERT INTO meta ( key, val ) VALUES ('default-area-boundaries','%1:%2:%3:%4');" )
                 .arg( mExtent.xMinimum(), 0, 'f', 10 ).arg( mExtent.yMinimum(), 0, 'f', 10 )
                 .arg( mExtent.xMaximum(), 0, 'f', 10 ).arg( mExtent.yMaximum(), 0, 'f', 10 );
  QByteArray cmd_bytes3  = cmd3.toAscii();
  const char *ptr3 = cmd_bytes3.data();

  if ( sqlite3_exec( mDatabase, ptr3, 0, 0, 0 ) != SQLITE_OK )
  {
    QgsDebugMsg( "Storing default area boundaries information into database failed." );
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
    "create trigger if not exists main.trg_tag_oi_update after update of object_id on tag begin "
    "insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) "
    "    values ('U','tag',old.i,'object_id',old.object_id,new.object_id); "
    "end;",

    "create trigger if not exists main.trg_tag_ot_update after update of object_type on tag begin "
    "insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) "
    "    values ('U','tag',old.i,'object_type',old.object_type,new.object_type); "
    "end;",

    "create trigger if not exists main.trg_tag_k_update after update of key on tag begin "
    "insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) "
    "    values ('U','tag',old.i,'key',old.key,new.key); "
    "end;",

    "create trigger if not exists main.trg_tag_v_update after update of val on tag begin "
    "insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) "
    "    values ('U','tag',old.i,'val',old.val,new.val); "
    "end;",

    "create trigger if not exists main.trg_tag_insert after insert on tag begin "
    "insert into change_step (change_type,tab_name,row_id) values ('I','tag',new.i); "
    "end;",

    "create trigger if not exists main.trg_tag_delete before delete on tag begin "
    "insert into change_step (change_type,tab_name,row_id) values ('D','tag',old.i); "
    "update tag set u=0 where i=old.i; "
    "select raise(ignore); "
    "end;",

    // node table
    "create trigger if not exists main.trg_node_lat_update after update of lat on node begin "
    "insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) "
    "    values ('U','node',old.i,'lat',old.lat,new.lat); "
    "end;",

    "create trigger if not exists main.trg_node_lon_update after update of lon on node begin "
    "insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) "
    "    values ('U','node',old.i,'lon',old.lon,new.lon); "
    "end;",

    "create trigger if not exists main.trg_node_t_update after update of timestamp on node begin "
    "insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) "
    "    values ('U','node',old.i,'timestamp',old.timestamp,new.timestamp); "
    "end;",

    "create trigger if not exists main.trg_node_use_update after update of user on node begin "
    "insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) "
    "    values ('U','node',old.i,'user',old.user,new.user); "
    "end;",

    "create trigger if not exists main.trg_node_usa_update after update of usage on node begin "
    "insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) "
    "    values ('U','node',old.i,'usage',old.usage,new.usage); "
    "end;",

    "create trigger if not exists main.trg_node_s_update after update of status on node begin "
    "insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) "
    "    values ('U','node',old.i,'status',old.status,new.status); "
    "end;",

    "create trigger if not exists main.trg_node_insert after insert on node begin "
    "insert into change_step (change_type,tab_name,row_id) values ('I','node',new.i); "
    "end;",

    "create trigger if not exists main.trg_node_delete before delete on node begin "
    "insert into change_step (change_type,tab_name,row_id) values ('D','node',old.i); "
    "update node set u=0 where i=old.i; "
    "select raise(ignore); "
    "end;",

    // way table
    "create trigger if not exists main.trg_way_t_update after update of timestamp on way begin "
    "insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) "
    "    values ('U','way',old.i,'timestamp',old.timestamp,new.timestamp); "
    "end;",

    "create trigger if not exists main.trg_way_u_update after update of user on way begin "
    "insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) "
    "    values ('U','way',old.i,'user',old.user,new.user); "
    "end;",

    "create trigger if not exists main.trg_way_c_update after update of closed on way begin "
    "insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) "
    "    values ('U','way',old.i,'closed',old.closed,new.closed); "
    "end;",

    "create trigger if not exists main.trg_way_s_update after update of status on way begin "
    "insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) "
    "  values ('U','way',old.i,'status',old.status,new.status); "
    "end;",

    "create trigger if not exists main.trg_way_w_update after update of wkb on way begin "
    "insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) "
    "    values ('U','way',old.i,'wkb','',''); "
    "end;",

    "create trigger if not exists main.trg_way_insert after insert on way begin "
    "insert into change_step (change_type,tab_name,row_id) values ('I','way',new.i); "
    "end;",

    "create trigger if not exists main.trg_way_delete before delete on way begin "
    "insert into change_step (change_type,tab_name,row_id) values ('D','way',old.i); "
    "update way set u=0 where i=old.i; "
    "select raise(ignore); "
    "end;",

    // relation table
    "create trigger if not exists main.trg_relation_ty_update after update of type on relation begin "
    "insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) "
    "    values ('U','relation',old.i,'type',old.type,new.type); "
    "end;",

    "create trigger if not exists main.trg_relation_ti_update after update of timestamp on relation begin "
    "insert into change_step  (change_type,tab_name,row_id,col_name,old_value,new_value) "
    "    values ('U','relation',old.i,'timestamp',old.timestamp,new.timestamp); "
    "end;",

    "create trigger if not exists main.trg_relation_u_update after update of user on relation begin "
    "insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) "
    "values ('U','relation',old.i,'user',old.user,new.user); "
    "end;",

    "create trigger if not exists main.trg_relation_s_update after update of status on relation begin "
    "insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) "
    "    values ('U','relation',old.i,'status',old.status,new.status); "
    "end;",

    "create trigger if not exists main.trg_relation_insert after insert on relation begin "
    "insert into change_step (change_type,tab_name,row_id) values ('I','relation',new.i); "
    "end;",

    "create trigger if not exists main.trg_relation_delete before delete on relation begin "
    "insert into change_step (change_type,tab_name,row_id) values ('D','relation',old.i); "
    "update relation set u=0 where i=old.i; "
    "select raise(ignore); "
    "end;",

    // way_member table
    "create trigger if not exists main.trg_way_member_wi_update after update of way_id on way_member begin "
    "insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) "
    "    values ('U','way_member',old.i,'way_id',old.way_id,new.way_id); "
    "end;",

    "create trigger if not exists main.trg_way_member_pi_update after update of pos_id on way_member begin "
    "insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) "
    "    values ('U','way_member',old.i,'pos_id',old.pos_id,new.pos_id); "
    "end;",

    "create trigger if not exists main.trg_way_member_ni_update after update of node_id on way_member begin "
    "insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) "
    "    values ('U','way_member',old.i,'node_id',old.node_id,new.node_id); "
    "end;",

    "create trigger if not exists main.trg_way_member_insert after insert on way_member begin "
    "insert into change_step (change_type,tab_name,row_id) values ('I','way_member',new.i); "
    "end;",

    "create trigger if not exists main.trg_way_member_delete before delete on way_member begin "
    "insert into change_step (change_type,tab_name,row_id) values ('D','way_member',old.i); "
    "update way_member set u=0 where i=old.i; "
    "select raise(ignore); "
    "end;",

    // relation_member table
    "create trigger if not exists main.trg_relation_member_ri_update after update of relation_id on relation_member begin "
    "insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) "
    "    values ('U','relation_member',old.i,'relation_id',old.relation_id,new.relation_id); "
    "end;",

    "create trigger if not exists main.trg_relation_member_pi_update after update of pos_id on relation_member begin "
    "insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) "
    "    values ('U','relation_member',old.i,'pos_id',old.pos_id,new.pos_id); "
    "end;",

    "create trigger if not exists main.trg_relation_member_mi_update after update of member_id on relation_member begin "
    "insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) "
    "    values ('U','relation_member',old.i,'member_id',old.member_id,new.member_id); "
    "end;",

    "create trigger if not exists main.trg_relation_member_mt_update after update of member_type on relation_member begin "
    "insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) "
    "    values ('U','relation_member',old.i,'member_type',old.member_type,new.member_type); "
    "end;",

    "create trigger if not exists main.trg_relation_member_r_update after update of role on relation_member begin "
    "insert into change_step (change_type,tab_name,row_id,col_name,old_value,new_value) "
    "values ('U','relation_member',old.i,'role',old.role,new.role); "
    "end;",

    "create trigger if not exists main.trg_relation_member_insert after insert on relation_member begin "
    "insert into change_step (change_type,tab_name,row_id) values ('I','relation_member',new.i); "
    "end;",

    "create trigger if not exists main.trg_relation_member_delete before delete on relation_member begin "
    "insert into change_step (change_type,tab_name,row_id) values ('D','relation_member',old.i); "
    "update relation_member set u=0 where i=old.i; "
    "select raise(ignore); "
    "end;"
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

  // open database
  if ( sqlite3_open( mDatabaseFileName.toUtf8().data(), &mDatabase ) != SQLITE_OK )
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
  mDatabase = NULL;
  return true;
};

void QgsOSMDataProvider::setRenderer( QgsVectorLayer *layer )
{
  layer->setRenderer( new OsmRenderer( layer->geometryType(), mStyleFileName ) );
}
