/***************************************************************************
                          qgscoordinatereferencesystem.cpp

                             -------------------
    begin                : 2007
    copyright            : (C) 2007 by Gary E. Sherman
    email                : sherman@mrcc.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatereferencesystem_p.h"

#include <cmath>

#include <QDir>
#include <QTemporaryFile>
#include <QDomNode>
#include <QDomElement>
#include <QFileInfo>
#include <QRegExp>
#include <QTextStream>
#include <QFile>
#include <QRegularExpression>

#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgis.h" //const vals declared here
#include "qgslocalec.h"
#include "qgssettings.h"

#include <sqlite3.h>
#ifndef ACCEPT_USE_OF_DEPRECATED_PROJ_API_H
#define ACCEPT_USE_OF_DEPRECATED_PROJ_API_H
#endif
#include <proj_api.h>

//gdal and ogr includes (needed for == operator)
#include <ogr_srs_api.h>
#include <cpl_error.h>
#include <cpl_conv.h>
#include <cpl_csv.h>


//! The length of the string "+lat_1="
const int LAT_PREFIX_LEN = 7;

CUSTOM_CRS_VALIDATION QgsCoordinateReferenceSystem::mCustomSrsValidation = nullptr;

QReadWriteLock QgsCoordinateReferenceSystem::sSrIdCacheLock;
QHash< long, QgsCoordinateReferenceSystem > QgsCoordinateReferenceSystem::sSrIdCache;
QReadWriteLock QgsCoordinateReferenceSystem::sOgcLock;
QHash< QString, QgsCoordinateReferenceSystem > QgsCoordinateReferenceSystem::sOgcCache;
QReadWriteLock QgsCoordinateReferenceSystem::sProj4CacheLock;
QHash< QString, QgsCoordinateReferenceSystem > QgsCoordinateReferenceSystem::sProj4Cache;
QReadWriteLock QgsCoordinateReferenceSystem::sCRSWktLock;
QHash< QString, QgsCoordinateReferenceSystem > QgsCoordinateReferenceSystem::sWktCache;
QReadWriteLock QgsCoordinateReferenceSystem::sCRSSrsIdLock;
QHash< long, QgsCoordinateReferenceSystem > QgsCoordinateReferenceSystem::sSrsIdCache;
QReadWriteLock QgsCoordinateReferenceSystem::sCrsStringLock;
QHash< QString, QgsCoordinateReferenceSystem > QgsCoordinateReferenceSystem::sStringCache;

//--------------------------

QgsCoordinateReferenceSystem::QgsCoordinateReferenceSystem()
{
  d = new QgsCoordinateReferenceSystemPrivate();
}

QgsCoordinateReferenceSystem::QgsCoordinateReferenceSystem( const QString &definition )
{
  d = new QgsCoordinateReferenceSystemPrivate();
  createFromString( definition );
}

QgsCoordinateReferenceSystem::QgsCoordinateReferenceSystem( const long id, CrsType type )
{
  d = new QgsCoordinateReferenceSystemPrivate();
  createFromId( id, type );
}

QgsCoordinateReferenceSystem::QgsCoordinateReferenceSystem( const QgsCoordinateReferenceSystem &srs ) //NOLINT
  : d( srs.d )
{
}

QgsCoordinateReferenceSystem &QgsCoordinateReferenceSystem::operator=( const QgsCoordinateReferenceSystem &srs )  //NOLINT
{
  d = srs.d;
  return *this;
}

QList<long> QgsCoordinateReferenceSystem::validSrsIds()
{
  QList<long> results;
  // check both standard & user defined projection databases
  QStringList dbs = QStringList() <<  QgsApplication::srsDatabaseFilePath() << QgsApplication::qgisUserDatabaseFilePath();

  Q_FOREACH ( const QString &db, dbs )
  {
    QFileInfo myInfo( db );
    if ( !myInfo.exists() )
    {
      QgsDebugMsg( "failed : " + db + " does not exist!" );
      continue;
    }

    sqlite3_database_unique_ptr database;
    sqlite3_statement_unique_ptr statement;

    //check the db is available
    int result = openDatabase( db, database );
    if ( result != SQLITE_OK )
    {
      QgsDebugMsg( "failed : " + db + " could not be opened!" );
      continue;
    }

    QString sql = QStringLiteral( "select srs_id from tbl_srs" );
    int rc;
    statement = database.prepare( sql, rc );
    while ( true )
    {
      // this one is an infinitive loop, intended to fetch any row
      int ret = statement.step();

      if ( ret == SQLITE_DONE )
      {
        // there are no more rows to fetch - we can stop looping
        break;
      }

      if ( ret == SQLITE_ROW )
      {
        results.append( statement.columnAsInt64( 0 ) );
      }
      else
      {
        QgsMessageLog::logMessage( QObject::tr( "SQLite error: %2\nSQL: %1" ).arg( sql, sqlite3_errmsg( database.get() ) ), QObject::tr( "SpatiaLite" ) );
        break;
      }
    }
  }
  std::sort( results.begin(), results.end() );
  return results;
}

QgsCoordinateReferenceSystem QgsCoordinateReferenceSystem::fromOgcWmsCrs( const QString &ogcCrs )
{
  QgsCoordinateReferenceSystem crs;
  crs.createFromOgcWmsCrs( ogcCrs );
  return crs;
}

QgsCoordinateReferenceSystem QgsCoordinateReferenceSystem::fromEpsgId( long epsg )
{
  return fromOgcWmsCrs( "EPSG:" + QString::number( epsg ) );
}

QgsCoordinateReferenceSystem QgsCoordinateReferenceSystem::fromProj4( const QString &proj4 )
{
  QgsCoordinateReferenceSystem crs;
  crs.createFromProj4( proj4 );
  return crs;
}

QgsCoordinateReferenceSystem QgsCoordinateReferenceSystem::fromWkt( const QString &wkt )
{
  QgsCoordinateReferenceSystem crs;
  crs.createFromWkt( wkt );
  return crs;
}

QgsCoordinateReferenceSystem QgsCoordinateReferenceSystem::fromSrsId( long srsId )
{
  QgsCoordinateReferenceSystem crs;
  crs.createFromSrsId( srsId );
  return crs;
}

QgsCoordinateReferenceSystem::~QgsCoordinateReferenceSystem() //NOLINT
{
}

bool QgsCoordinateReferenceSystem::createFromId( const long id, CrsType type )
{
  bool result = false;
  switch ( type )
  {
    case InternalCrsId:
      result = createFromSrsId( id );
      break;
    case PostgisCrsId:
      result = createFromSrid( id );
      break;
    case EpsgCrsId:
      result = createFromOgcWmsCrs( QStringLiteral( "EPSG:%1" ).arg( id ) );
      break;
    default:
      //THIS IS BAD...THIS PART OF CODE SHOULD NEVER BE REACHED...
      QgsDebugMsg( QStringLiteral( "Unexpected case reached!" ) );
  };
  return result;
}

bool QgsCoordinateReferenceSystem::createFromString( const QString &definition )
{
  sCrsStringLock.lockForRead();
  QHash< QString, QgsCoordinateReferenceSystem >::const_iterator crsIt = sStringCache.constFind( definition );
  if ( crsIt != sStringCache.constEnd() )
  {
    // found a match in the cache
    *this = crsIt.value();
    sCrsStringLock.unlock();
    return true;
  }
  sCrsStringLock.unlock();

  bool result = false;
  QRegularExpression reCrsId( "^(epsg|postgis|internal|user)\\:(\\d+)$", QRegularExpression::CaseInsensitiveOption );
  QRegularExpressionMatch match = reCrsId.match( definition );
  if ( match.capturedStart() == 0 )
  {
    QString authName = match.captured( 1 ).toLower();
    CrsType type = InternalCrsId;
    if ( authName == QLatin1String( "epsg" ) )
      type = EpsgCrsId;
    if ( authName == QLatin1String( "postgis" ) )
      type = PostgisCrsId;
    long id = match.captured( 2 ).toLong();
    result = createFromId( id, type );
  }
  else
  {
    QRegularExpression reCrsStr( "^(?:(wkt|proj4)\\:)?(.+)$", QRegularExpression::CaseInsensitiveOption );
    match = reCrsStr.match( definition );
    if ( match.capturedStart() == 0 )
    {
      if ( match.captured( 1 ).compare( QLatin1String( "proj4" ), Qt::CaseInsensitive ) == 0 )
      {
        result = createFromProj4( match.captured( 2 ) );
        //TODO: createFromProj4 used to save to the user database any new CRS
        // this behavior was changed in order to separate creation and saving.
        // Not sure if it necessary to save it here, should be checked by someone
        // familiar with the code (should also give a more descriptive name to the generated CRS)
        if ( srsid() == 0 )
        {
          QString myName = QStringLiteral( " * %1 (%2)" )
                           .arg( QObject::tr( "Generated CRS", "A CRS automatically generated from layer info get this prefix for description" ),
                                 toProj4() );
          saveAsUserCrs( myName );
        }
      }
      else
      {
        result = createFromWkt( match.captured( 2 ) );
      }
    }
  }

  sCrsStringLock.lockForWrite();
  sStringCache.insert( definition, *this );
  sCrsStringLock.unlock();
  return result;
}

bool QgsCoordinateReferenceSystem::createFromUserInput( const QString &definition )
{
  QString userWkt;
  char *wkt = nullptr;
  OGRSpatialReferenceH crs = OSRNewSpatialReference( nullptr );

  // make sure towgs84 parameter is loaded if using an ESRI definition and gdal >= 1.9
  if ( definition.startsWith( QLatin1String( "ESRI::" ) ) )
  {
    setupESRIWktFix();
  }

  if ( OSRSetFromUserInput( crs, definition.toLocal8Bit().constData() ) == OGRERR_NONE )
  {
    if ( OSRExportToWkt( crs, &wkt ) == OGRERR_NONE )
    {
      userWkt = wkt;
      CPLFree( wkt );
    }
    OSRDestroySpatialReference( crs );
  }
  //QgsDebugMsg( "definition: " + definition + " wkt = " + wkt );
  return createFromWkt( userWkt );
}

void QgsCoordinateReferenceSystem::setupESRIWktFix()
{
  // make sure towgs84 parameter is loaded if gdal >= 1.9
  // this requires setting GDAL_FIX_ESRI_WKT=GEOGCS (see qgis bug #5598 and gdal bug #4673)
  const char *configOld = CPLGetConfigOption( "GDAL_FIX_ESRI_WKT", "" );
  const char *configNew = "GEOGCS";
  // only set if it was not set, to let user change the value if needed
  if ( strcmp( configOld, "" ) == 0 )
  {
    CPLSetConfigOption( "GDAL_FIX_ESRI_WKT", configNew );
    if ( strcmp( configNew, CPLGetConfigOption( "GDAL_FIX_ESRI_WKT", "" ) ) != 0 )
      QgsLogger::warning( QStringLiteral( "GDAL_FIX_ESRI_WKT could not be set to %1 : %2" )
                          .arg( configNew, CPLGetConfigOption( "GDAL_FIX_ESRI_WKT", "" ) ) );
    QgsDebugMsgLevel( QStringLiteral( "set GDAL_FIX_ESRI_WKT : %1" ).arg( configNew ), 4 );
  }
  else
  {
    QgsDebugMsgLevel( QStringLiteral( "GDAL_FIX_ESRI_WKT was already set : %1" ).arg( configNew ), 4 );
  }
}

bool QgsCoordinateReferenceSystem::createFromOgcWmsCrs( const QString &crs )
{
  sOgcLock.lockForRead();
  QHash< QString, QgsCoordinateReferenceSystem >::const_iterator crsIt = sOgcCache.constFind( crs );
  if ( crsIt != sOgcCache.constEnd() )
  {
    // found a match in the cache
    *this = crsIt.value();
    sOgcLock.unlock();
    return true;
  }
  sOgcLock.unlock();

  QString wmsCrs = crs;

  QRegExp re( "urn:ogc:def:crs:([^:]+).+([^:]+)", Qt::CaseInsensitive );
  if ( re.exactMatch( wmsCrs ) )
  {
    wmsCrs = re.cap( 1 ) + ':' + re.cap( 2 );
  }
  else
  {
    re.setPattern( QStringLiteral( "(user|custom|qgis):(\\d+)" ) );
    if ( re.exactMatch( wmsCrs ) && createFromSrsId( re.cap( 2 ).toInt() ) )
    {
      sOgcLock.lockForWrite();
      sOgcCache.insert( crs, *this );
      sOgcLock.unlock();
      return true;
    }
  }

  if ( loadFromDatabase( QgsApplication::srsDatabaseFilePath(), QStringLiteral( "lower(auth_name||':'||auth_id)" ), wmsCrs.toLower() ) )
  {
    sOgcLock.lockForWrite();
    sOgcCache.insert( crs, *this );
    sOgcLock.unlock();
    return true;
  }

  // NAD27
  if ( wmsCrs.compare( QLatin1String( "CRS:27" ), Qt::CaseInsensitive ) == 0 ||
       wmsCrs.compare( QLatin1String( "OGC:CRS27" ), Qt::CaseInsensitive ) == 0 )
  {
    // TODO: verify same axis orientation
    return createFromOgcWmsCrs( QStringLiteral( "EPSG:4267" ) );
  }

  // NAD83
  if ( wmsCrs.compare( QLatin1String( "CRS:83" ), Qt::CaseInsensitive ) == 0 ||
       wmsCrs.compare( QLatin1String( "OGC:CRS83" ), Qt::CaseInsensitive ) == 0 )
  {
    // TODO: verify same axis orientation
    return createFromOgcWmsCrs( QStringLiteral( "EPSG:4269" ) );
  }

  // WGS84
  if ( wmsCrs.compare( QLatin1String( "CRS:84" ), Qt::CaseInsensitive ) == 0 ||
       wmsCrs.compare( QLatin1String( "OGC:CRS84" ), Qt::CaseInsensitive ) == 0 )
  {
    createFromOgcWmsCrs( QStringLiteral( "EPSG:4326" ) );

    d.detach();
    d->mAxisInverted = false;
    d->mAxisInvertedDirty = false;

    sOgcLock.lockForWrite();
    sOgcCache.insert( crs, *this );
    sOgcLock.unlock();

    return d->mIsValid;
  }

  sOgcLock.lockForWrite();
  sOgcCache.insert( crs, QgsCoordinateReferenceSystem() );
  sOgcLock.unlock();
  return false;
}

// Misc helper functions -----------------------


void QgsCoordinateReferenceSystem::validate()
{
  if ( d->mIsValid )
    return;

  d.detach();

  // try to validate using custom validation routines
  if ( mCustomSrsValidation )
    mCustomSrsValidation( *this );

  if ( !d->mIsValid )
  {
    *this = QgsCoordinateReferenceSystem::fromOgcWmsCrs( GEO_EPSG_CRS_AUTHID );
  }
}

bool QgsCoordinateReferenceSystem::createFromSrid( const long id )
{
  sSrIdCacheLock.lockForRead();
  QHash< long, QgsCoordinateReferenceSystem >::const_iterator crsIt = sSrIdCache.constFind( id );
  if ( crsIt != sSrIdCache.constEnd() )
  {
    // found a match in the cache
    *this = crsIt.value();
    sSrIdCacheLock.unlock();
    return true;
  }
  sSrIdCacheLock.unlock();

  bool result = loadFromDatabase( QgsApplication::srsDatabaseFilePath(), QStringLiteral( "srid" ), QString::number( id ) );

  sSrIdCacheLock.lockForWrite();
  sSrIdCache.insert( id, *this );
  sSrIdCacheLock.unlock();

  return result;
}

bool QgsCoordinateReferenceSystem::createFromSrsId( const long id )
{
  sCRSSrsIdLock.lockForRead();
  QHash< long, QgsCoordinateReferenceSystem >::const_iterator crsIt = sSrsIdCache.constFind( id );
  if ( crsIt != sSrsIdCache.constEnd() )
  {
    // found a match in the cache
    *this = crsIt.value();
    sCRSSrsIdLock.unlock();
    return true;
  }
  sCRSSrsIdLock.unlock();

  bool result = loadFromDatabase( id < USER_CRS_START_ID ? QgsApplication::srsDatabaseFilePath() :
                                  QgsApplication::qgisUserDatabaseFilePath(),
                                  QStringLiteral( "srs_id" ), QString::number( id ) );

  sCRSSrsIdLock.lockForWrite();
  sSrsIdCache.insert( id, *this );
  sCRSSrsIdLock.unlock();

  return result;
}

bool QgsCoordinateReferenceSystem::loadFromDatabase( const QString &db, const QString &expression, const QString &value )
{
  d.detach();

  QgsDebugMsgLevel( "load CRS from " + db + " where " + expression + " is " + value, 3 );
  d->mIsValid = false;
  d->mWkt.clear();

  QFileInfo myInfo( db );
  if ( !myInfo.exists() )
  {
    QgsDebugMsg( "failed : " + db + " does not exist!" );
    return d->mIsValid;
  }

  sqlite3_database_unique_ptr database;
  sqlite3_statement_unique_ptr statement;
  int           myResult;
  //check the db is available
  myResult = openDatabase( db, database );
  if ( myResult != SQLITE_OK )
  {
    return d->mIsValid;
  }

  /*
    srs_id INTEGER PRIMARY KEY,
    description text NOT NULL,
    projection_acronym text NOT NULL,
    ellipsoid_acronym NOT NULL,
    parameters text NOT NULL,
    srid integer NOT NULL,
    auth_name varchar NOT NULL,
    auth_id integer NOT NULL,
    is_geo integer NOT NULL);
  */

  QString mySql = "select srs_id,description,projection_acronym,"
                  "ellipsoid_acronym,parameters,srid,auth_name||':'||auth_id,is_geo "
                  "from tbl_srs where " + expression + '=' + QgsSqliteUtils::quotedString( value ) + " order by deprecated";
  statement = database.prepare( mySql, myResult );
  // XXX Need to free memory from the error msg if one is set
  if ( myResult == SQLITE_OK && statement.step() == SQLITE_ROW )
  {
    d->mSrsId = statement.columnAsText( 0 ).toLong();
    d->mDescription = statement.columnAsText( 1 );
    d->mProjectionAcronym = statement.columnAsText( 2 );
    d->mEllipsoidAcronym = statement.columnAsText( 3 );
    d->mProj4 = statement.columnAsText( 4 );
    d->mSRID = statement.columnAsText( 5 ).toLong();
    d->mAuthId = statement.columnAsText( 6 );
    d->mIsGeographic = statement.columnAsText( 7 ).toInt() != 0;
    d->mAxisInvertedDirty = true;

    if ( d->mSrsId >= USER_CRS_START_ID && d->mAuthId.isEmpty() )
    {
      d->mAuthId = QStringLiteral( "USER:%1" ).arg( d->mSrsId );
    }
    else if ( d->mAuthId.startsWith( QLatin1String( "EPSG:" ), Qt::CaseInsensitive ) )
    {
      OSRDestroySpatialReference( d->mCRS );
      d->mCRS = OSRNewSpatialReference( nullptr );
      d->mIsValid = OSRSetFromUserInput( d->mCRS, d->mAuthId.toLower().toLatin1() ) == OGRERR_NONE;
      setMapUnits();
    }

    if ( !d->mIsValid )
    {
      setProj4String( d->mProj4 );
    }
  }
  else
  {
    QgsDebugMsgLevel( "failed : " + mySql, 4 );
  }
  return d->mIsValid;
}

bool QgsCoordinateReferenceSystem::hasAxisInverted() const
{
  if ( d->mAxisInvertedDirty )
  {
    OGRAxisOrientation orientation;
    OSRGetAxis( d->mCRS, OSRIsGeographic( d->mCRS ) ? "GEOGCS" : "PROJCS", 0, &orientation );

    // If axis orientation is unknown, try again with OSRImportFromEPSGA for EPSG crs
    if ( orientation == OAO_Other && d->mAuthId.startsWith( QLatin1String( "EPSG:" ), Qt::CaseInsensitive ) )
    {
      OGRSpatialReferenceH crs = OSRNewSpatialReference( nullptr );

      if ( OSRImportFromEPSGA( crs, d->mAuthId.midRef( 5 ).toInt() ) == OGRERR_NONE )
      {
        OSRGetAxis( crs, OSRIsGeographic( crs ) ? "GEOGCS" : "PROJCS", 0, &orientation );
      }

      OSRDestroySpatialReference( crs );
    }

    d->mAxisInverted = orientation == OAO_North;
    d->mAxisInvertedDirty = false;
  }

  return d->mAxisInverted;
}

bool QgsCoordinateReferenceSystem::createFromWkt( const QString &wkt )
{
  d.detach();

  sCRSWktLock.lockForRead();
  QHash< QString, QgsCoordinateReferenceSystem >::const_iterator crsIt = sWktCache.constFind( wkt );
  if ( crsIt != sWktCache.constEnd() )
  {
    // found a match in the cache
    *this = crsIt.value();
    sCRSWktLock.unlock();
    return true;
  }
  sCRSWktLock.unlock();

  d->mIsValid = false;
  d->mWkt.clear();
  d->mProj4.clear();

  if ( wkt.isEmpty() )
  {
    QgsDebugMsgLevel( QStringLiteral( "theWkt is uninitialized, operation failed" ), 4 );
    return d->mIsValid;
  }
  QByteArray ba = wkt.toLatin1();
  const char *pWkt = ba.data();

  OGRErr myInputResult = OSRImportFromWkt( d->mCRS, const_cast< char ** >( & pWkt ) );

  if ( myInputResult != OGRERR_NONE )
  {
    QgsDebugMsg( QStringLiteral( "\n---------------------------------------------------------------" ) );
    QgsDebugMsg( QStringLiteral( "This CRS could *** NOT *** be set from the supplied Wkt " ) );
    QgsDebugMsg( "INPUT: " + wkt );
    QgsDebugMsg( QStringLiteral( "UNUSED WKT: %1" ).arg( pWkt ) );
    QgsDebugMsg( QStringLiteral( "---------------------------------------------------------------\n" ) );

    sCRSWktLock.lockForWrite();
    sWktCache.insert( wkt, *this );
    sCRSWktLock.unlock();
    return d->mIsValid;
  }

  if ( OSRAutoIdentifyEPSG( d->mCRS ) == OGRERR_NONE )
  {
    QString authid = QStringLiteral( "%1:%2" )
                     .arg( OSRGetAuthorityName( d->mCRS, nullptr ),
                           OSRGetAuthorityCode( d->mCRS, nullptr ) );
    bool result = createFromOgcWmsCrs( authid );
    sCRSWktLock.lockForWrite();
    sWktCache.insert( wkt, *this );
    sCRSWktLock.unlock();
    return result;
  }

  // always morph from esri as it doesn't hurt anything
  // FW: Hey, that's not right!  It can screw stuff up! Disable
  //myOgrSpatialRef.morphFromESRI();

  // create the proj4 structs needed for transforming
  char *proj4src = nullptr;
  OSRExportToProj4( d->mCRS, &proj4src );

  //now that we have the proj4string, delegate to createFromProj4 so
  // that we can try to fill in the remaining class members...
  //create from Proj will set the isValidFlag
  if ( !createFromProj4( proj4src ) )
  {
    CPLFree( proj4src );

#if GDAL_VERSION_NUM < GDAL_COMPUTE_VERSION(2,5,0)
    // try fixed up version
    OSRFixup( d->mCRS );
#endif

    OSRExportToProj4( d->mCRS, &proj4src );

    createFromProj4( proj4src );
  }
  //TODO: createFromProj4 used to save to the user database any new CRS
  // this behavior was changed in order to separate creation and saving.
  // Not sure if it necessary to save it here, should be checked by someone
  // familiar with the code (should also give a more descriptive name to the generated CRS)
  if ( d->mSrsId == 0 )
  {
    QString myName = QStringLiteral( " * %1 (%2)" )
                     .arg( QObject::tr( "Generated CRS", "A CRS automatically generated from layer info get this prefix for description" ),
                           toProj4() );
    saveAsUserCrs( myName );
  }

  CPLFree( proj4src );

  sCRSWktLock.lockForWrite();
  sWktCache.insert( wkt, *this );
  sCRSWktLock.unlock();

  return d->mIsValid;
  //setMapunits will be called by createfromproj above
}

bool QgsCoordinateReferenceSystem::isValid() const
{
  return d->mIsValid;
}

bool QgsCoordinateReferenceSystem::createFromProj4( const QString &proj4String )
{
  d.detach();

  sProj4CacheLock.lockForRead();
  QHash< QString, QgsCoordinateReferenceSystem >::const_iterator crsIt = sProj4Cache.constFind( proj4String );
  if ( crsIt != sProj4Cache.constEnd() )
  {
    // found a match in the cache
    *this = crsIt.value();
    sProj4CacheLock.unlock();
    return true;
  }
  sProj4CacheLock.unlock();

  //
  // Examples:
  // +proj=tmerc +lat_0=0 +lon_0=-62 +k=0.999500 +x_0=400000 +y_0=0
  // +ellps=clrk80 +towgs84=-255,-15,71,0,0,0,0 +units=m +no_defs
  //
  // +proj=lcc +lat_1=46.8 +lat_0=46.8 +lon_0=2.337229166666664 +k_0=0.99987742
  // +x_0=600000 +y_0=2200000 +a=6378249.2 +b=6356515.000000472 +units=m +no_defs
  //
  QString myProj4String = proj4String.trimmed();
  d->mIsValid = false;
  d->mWkt.clear();

  QRegExp myProjRegExp( "\\+proj=(\\S+)" );
  int myStart = myProjRegExp.indexIn( myProj4String );
  if ( myStart == -1 )
  {
    sProj4CacheLock.lockForWrite();
    sProj4Cache.insert( proj4String, *this );
    sProj4CacheLock.unlock();

    return d->mIsValid;
  }

  d->mProjectionAcronym = myProjRegExp.cap( 1 );

  QRegExp myEllipseRegExp( "\\+ellps=(\\S+)" );
  myStart = myEllipseRegExp.indexIn( myProj4String );
  if ( myStart == -1 )
  {
    d->mEllipsoidAcronym.clear();
  }
  else
  {
    d->mEllipsoidAcronym = myEllipseRegExp.cap( 1 );
  }

  QRegExp myAxisRegExp( "\\+a=(\\S+)" );
  myStart = myAxisRegExp.indexIn( myProj4String );

  long mySrsId = 0;
  QgsCoordinateReferenceSystem::RecordMap myRecord;

  /*
   * We try to match the proj string to and srsid using the following logic:
   * - perform a whole text search on proj4 string (if not null)
   */
  myRecord = getRecord( "select * from tbl_srs where parameters=" + QgsSqliteUtils::quotedString( myProj4String ) + " order by deprecated" );
  if ( myRecord.empty() )
  {
    // Ticket #722 - aaronr
    // Check if we can swap the lat_1 and lat_2 params (if they exist) to see if we match...
    // First we check for lat_1 and lat_2
    QRegExp myLat1RegExp( "\\+lat_1=\\S+" );
    QRegExp myLat2RegExp( "\\+lat_2=\\S+" );
    int myStart1 = 0;
    int myLength1 = 0;
    int myStart2 = 0;
    int myLength2 = 0;
    QString lat1Str;
    QString lat2Str;
    myStart1 = myLat1RegExp.indexIn( myProj4String, myStart1 );
    myStart2 = myLat2RegExp.indexIn( myProj4String, myStart2 );
    if ( myStart1 != -1 && myStart2 != -1 )
    {
      myLength1 = myLat1RegExp.matchedLength();
      myLength2 = myLat2RegExp.matchedLength();
      lat1Str = myProj4String.mid( myStart1 + LAT_PREFIX_LEN, myLength1 - LAT_PREFIX_LEN );
      lat2Str = myProj4String.mid( myStart2 + LAT_PREFIX_LEN, myLength2 - LAT_PREFIX_LEN );
    }
    // If we found the lat_1 and lat_2 we need to swap and check to see if we can find it...
    if ( !lat1Str.isEmpty() && !lat2Str.isEmpty() )
    {
      // Make our new string to check...
      QString proj4StringModified = myProj4String;
      // First just swap in the lat_2 value for lat_1 value
      proj4StringModified.replace( myStart1 + LAT_PREFIX_LEN, myLength1 - LAT_PREFIX_LEN, lat2Str );
      // Now we have to find the lat_2 location again since it has potentially moved...
      myStart2 = 0;
      myStart2 = myLat2RegExp.indexIn( proj4String, myStart2 );
      proj4StringModified.replace( myStart2 + LAT_PREFIX_LEN, myLength2 - LAT_PREFIX_LEN, lat1Str );
      QgsDebugMsgLevel( QStringLiteral( "trying proj4string match with swapped lat_1,lat_2" ), 4 );
      myRecord = getRecord( "select * from tbl_srs where parameters=" + QgsSqliteUtils::quotedString( proj4StringModified.trimmed() ) + " order by deprecated" );
    }
  }

  if ( myRecord.empty() )
  {
    // match all parameters individually:
    // - order of parameters doesn't matter
    // - found definition may have more parameters (like +towgs84 in GDAL)
    // - retry without datum, if no match is found (looks like +datum<>WGS84 was dropped in GDAL)

    QString sql = QStringLiteral( "SELECT * FROM tbl_srs WHERE " );
    QString delim;
    QString datum;

    // split on spaces followed by a plus sign (+) to deal
    // also with parameters containing spaces (e.g. +nadgrids)
    // make sure result is trimmed (#5598)
    QStringList myParams;
    Q_FOREACH ( const QString &param, myProj4String.split( QRegExp( "\\s+(?=\\+)" ), QString::SkipEmptyParts ) )
    {
      QString arg = QStringLiteral( "' '||parameters||' ' LIKE %1" ).arg( QgsSqliteUtils::quotedString( QStringLiteral( "% %1 %" ).arg( param.trimmed() ) ) );
      if ( param.startsWith( QLatin1String( "+datum=" ) ) )
      {
        datum = arg;
      }
      else
      {
        sql += delim + arg;
        delim = QStringLiteral( " AND " );
        myParams << param.trimmed();
      }
    }

    if ( !datum.isEmpty() )
    {
      myRecord = getRecord( sql + delim + datum + " order by deprecated" );
    }

    if ( myRecord.empty() )
    {
      // datum might have disappeared in definition - retry without it
      myRecord = getRecord( sql + " order by deprecated" );
    }

    if ( !myRecord.empty() )
    {
      // Bugfix 8487 : test param lists are equal, except for +datum
      QStringList foundParams;
      Q_FOREACH ( const QString &param, myRecord["parameters"].split( QRegExp( "\\s+(?=\\+)" ), QString::SkipEmptyParts ) )
      {
        if ( !param.startsWith( QLatin1String( "+datum=" ) ) )
          foundParams << param.trimmed();
      }

      myParams.sort();
      foundParams.sort();

      if ( myParams != foundParams )
      {
        myRecord.clear();
      }
    }
  }

  if ( !myRecord.empty() )
  {
    mySrsId = myRecord[QStringLiteral( "srs_id" )].toLong();
    if ( mySrsId > 0 )
    {
      createFromSrsId( mySrsId );
    }
  }
  else
  {
    // Last ditch attempt to piece together what we know of the projection to find a match...
    setProj4String( myProj4String );
    mySrsId = findMatchingProj();
    if ( mySrsId > 0 )
    {
      createFromSrsId( mySrsId );
    }
    else
    {
      d->mIsValid = false;
    }
  }

  // if we failed to look up the projection in database, don't worry. we can still use it :)
  if ( !d->mIsValid )
  {
    QgsDebugMsgLevel( QStringLiteral( "Projection is not found in databases." ), 4 );
    //setProj4String will set mIsValidFlag to true if there is no issue
    setProj4String( myProj4String );
  }

  sProj4CacheLock.lockForWrite();
  sProj4Cache.insert( proj4String, *this );
  sProj4CacheLock.unlock();

  return d->mIsValid;
}

//private method meant for internal use by this class only
QgsCoordinateReferenceSystem::RecordMap QgsCoordinateReferenceSystem::getRecord( const QString &sql )
{
  QString myDatabaseFileName;
  QgsCoordinateReferenceSystem::RecordMap myMap;
  QString myFieldName;
  QString myFieldValue;
  sqlite3_database_unique_ptr database;
  sqlite3_statement_unique_ptr statement;
  int           myResult;

  // Get the full path name to the sqlite3 spatial reference database.
  myDatabaseFileName = QgsApplication::srsDatabaseFilePath();
  QFileInfo myInfo( myDatabaseFileName );
  if ( !myInfo.exists() )
  {
    QgsDebugMsg( "failed : " + myDatabaseFileName + " does not exist!" );
    return myMap;
  }

  //check the db is available
  myResult = openDatabase( myDatabaseFileName, database );
  if ( myResult != SQLITE_OK )
  {
    return myMap;
  }

  statement = database.prepare( sql, myResult );
  // XXX Need to free memory from the error msg if one is set
  if ( myResult == SQLITE_OK && statement.step() == SQLITE_ROW )
  {
    int myColumnCount = statement.columnCount();
    //loop through each column in the record adding its expression name and value to the map
    for ( int myColNo = 0; myColNo < myColumnCount; myColNo++ )
    {
      myFieldName = statement.columnName( myColNo );
      myFieldValue = statement.columnAsText( myColNo );
      myMap[myFieldName] = myFieldValue;
    }
    if ( statement.step() != SQLITE_DONE )
    {
      QgsDebugMsgLevel( QStringLiteral( "Multiple records found in srs.db" ), 4 );
      myMap.clear();
    }
  }
  else
  {
    QgsDebugMsgLevel( "failed :  " + sql, 4 );
  }

  if ( myMap.empty() )
  {
    myDatabaseFileName = QgsApplication::qgisUserDatabaseFilePath();
    QFileInfo myFileInfo;
    myFileInfo.setFile( myDatabaseFileName );
    if ( !myFileInfo.exists() )
    {
      QgsDebugMsg( QStringLiteral( "user qgis.db not found" ) );
      return myMap;
    }

    //check the db is available
    myResult = openDatabase( myDatabaseFileName, database );
    if ( myResult != SQLITE_OK )
    {
      return myMap;
    }

    statement = database.prepare( sql, myResult );
    // XXX Need to free memory from the error msg if one is set
    if ( myResult == SQLITE_OK && statement.step() == SQLITE_ROW )
    {
      int myColumnCount = statement.columnCount();
      //loop through each column in the record adding its field name and value to the map
      for ( int myColNo = 0; myColNo < myColumnCount; myColNo++ )
      {
        myFieldName = statement.columnName( myColNo );
        myFieldValue = statement.columnAsText( myColNo );
        myMap[myFieldName] = myFieldValue;
      }

      if ( statement.step() != SQLITE_DONE )
      {
        QgsDebugMsgLevel( QStringLiteral( "Multiple records found in srs.db" ), 4 );
        myMap.clear();
      }
    }
    else
    {
      QgsDebugMsgLevel( "failed :  " + sql, 4 );
    }
  }
  return myMap;
}

// Accessors -----------------------------------

long QgsCoordinateReferenceSystem::srsid() const
{
  return d->mSrsId;
}

long QgsCoordinateReferenceSystem::postgisSrid() const
{
  return d->mSRID;
}

QString QgsCoordinateReferenceSystem::authid() const
{
  return d->mAuthId;
}

QString QgsCoordinateReferenceSystem::description() const
{
  if ( d->mDescription.isNull() )
  {
    return QString();
  }
  else
  {
    return d->mDescription;
  }
}

QString QgsCoordinateReferenceSystem::projectionAcronym() const
{
  if ( d->mProjectionAcronym.isNull() )
  {
    return QString();
  }
  else
  {
    return d->mProjectionAcronym;
  }
}

QString QgsCoordinateReferenceSystem::ellipsoidAcronym() const
{
  if ( d->mEllipsoidAcronym.isNull() )
  {
    return QString();
  }
  else
  {
    return d->mEllipsoidAcronym;
  }
}

QString QgsCoordinateReferenceSystem::toProj4() const
{
  if ( !d->mIsValid )
    return QString();

  if ( d->mProj4.isEmpty() )
  {
    char *proj4src = nullptr;
    OSRExportToProj4( d->mCRS, &proj4src );
    d->mProj4 = proj4src;
    CPLFree( proj4src );
  }
  // Stray spaces at the end?
  return d->mProj4.trimmed();
}

bool QgsCoordinateReferenceSystem::isGeographic() const
{
  return d->mIsGeographic;
}

QgsUnitTypes::DistanceUnit QgsCoordinateReferenceSystem::mapUnits() const
{
  if ( !d->mIsValid )
    return QgsUnitTypes::DistanceUnknownUnit;

  return d->mMapUnits;
}

QgsRectangle QgsCoordinateReferenceSystem::bounds() const
{
  if ( !d->mIsValid )
    return QgsRectangle();

  //check the db is available
  QString databaseFileName = QgsApplication::srsDatabaseFilePath();

  sqlite3_database_unique_ptr database;
  sqlite3_statement_unique_ptr statement;

  int result = openDatabase( databaseFileName, database );
  if ( result != SQLITE_OK )
  {
    return QgsRectangle();
  }

  QString sql = QStringLiteral( "select west_bound_lon, north_bound_lat, east_bound_lon, south_bound_lat from tbl_bounds "
                                "where srid=%1" )
                .arg( d->mSRID );
  statement = database.prepare( sql, result );

  QgsRectangle rect;
  if ( result == SQLITE_OK )
  {
    if ( statement.step() == SQLITE_ROW )
    {
      double west = statement.columnAsDouble( 0 );
      double north = statement.columnAsDouble( 1 );
      double east = statement.columnAsDouble( 2 );
      double south = statement.columnAsDouble( 3 );

      rect.setXMinimum( west );
      rect.setYMinimum( south );
      rect.setXMaximum( east );
      rect.setYMaximum( north );
    }
  }

  return rect;
}


// Mutators -----------------------------------


void QgsCoordinateReferenceSystem::setInternalId( long srsId )
{
  d.detach();
  d->mSrsId = srsId;
}
void QgsCoordinateReferenceSystem::setAuthId( const QString &authId )
{
  d.detach();
  d->mAuthId = authId;
}
void QgsCoordinateReferenceSystem::setSrid( long srid )
{
  d.detach();
  d->mSRID = srid;
}
void QgsCoordinateReferenceSystem::setDescription( const QString &description )
{
  d.detach();
  d->mDescription = description;
}
void QgsCoordinateReferenceSystem::setProj4String( const QString &proj4String )
{
  d.detach();
  d->mProj4 = proj4String;

  QgsLocaleNumC l;

  OSRDestroySpatialReference( d->mCRS );
  d->mCRS = OSRNewSpatialReference( nullptr );
  d->mIsValid = OSRImportFromProj4( d->mCRS, proj4String.trimmed().toLatin1().constData() ) == OGRERR_NONE;
  // OSRImportFromProj4() may accept strings that are not valid proj.4 strings,
  // e.g if they lack a +ellps parameter, it will automatically add +ellps=WGS84, but as
  // we use the original mProj4 with QgsCoordinateTransform, it will fail to initialize
  // so better detect it now.
  projCtx pContext = pj_ctx_alloc();
  projPJ proj = pj_init_plus_ctx( pContext, proj4String.trimmed().toLatin1().constData() );
  if ( !proj )
  {
    QgsDebugMsgLevel( QStringLiteral( "proj.4 string rejected by pj_init_plus_ctx()" ), 4 );
    d->mIsValid = false;
  }
  else
  {
    pj_free( proj );
  }
  pj_ctx_free( pContext );
  d->mWkt.clear();
  setMapUnits();
}

void QgsCoordinateReferenceSystem::setGeographicFlag( bool geoFlag )
{
  d.detach();
  d->mIsGeographic = geoFlag;
}
void QgsCoordinateReferenceSystem::setEpsg( long epsg )
{
  d.detach();
  d->mAuthId = QStringLiteral( "EPSG:%1" ).arg( epsg );
}
void  QgsCoordinateReferenceSystem::setProjectionAcronym( const QString &projectionAcronym )
{
  d.detach();
  d->mProjectionAcronym = projectionAcronym;
}
void  QgsCoordinateReferenceSystem::setEllipsoidAcronym( const QString &ellipsoidAcronym )
{
  d.detach();
  d->mEllipsoidAcronym = ellipsoidAcronym;
}

void QgsCoordinateReferenceSystem::setMapUnits()
{
  d.detach();
  if ( !d->mIsValid )
  {
    d->mMapUnits = QgsUnitTypes::DistanceUnknownUnit;
    return;
  }

  char *unitName = nullptr;

#if GDAL_VERSION_NUM < GDAL_COMPUTE_VERSION(2,5,0)
  // Of interest to us is that this call adds in a unit parameter if
  // one doesn't already exist.
  OSRFixup( d->mCRS );
#endif

  if ( OSRIsProjected( d->mCRS ) )
  {
    double toMeter = OSRGetLinearUnits( d->mCRS, &unitName );
    QString unit( unitName );

    // If the units parameter was created during the Fixup() call
    // above, the name of the units is likely to be 'unknown'. Try to
    // do better than that ... (but perhaps ogr should be enhanced to
    // do this instead?).

    static const double FEET_TO_METER = 0.3048;
    static const double SMALL_NUM = 1e-3;

    if ( std::fabs( toMeter - FEET_TO_METER ) < SMALL_NUM )
      unit = QStringLiteral( "Foot" );

    if ( qgsDoubleNear( toMeter, 1.0 ) ) //Unit name for meters would be "metre"
      d->mMapUnits = QgsUnitTypes::DistanceMeters;
    else if ( unit == QLatin1String( "Foot" ) )
      d->mMapUnits = QgsUnitTypes::DistanceFeet;
    else
    {
      d->mMapUnits = QgsUnitTypes::DistanceUnknownUnit;
    }
  }
  else
  {
    OSRGetAngularUnits( d->mCRS, &unitName );
    QString unit( unitName );
    if ( unit == QLatin1String( "degree" ) )
      d->mMapUnits = QgsUnitTypes::DistanceDegrees;
    else
    {
      d->mMapUnits = QgsUnitTypes::DistanceUnknownUnit;
    }
  }
}


long QgsCoordinateReferenceSystem::findMatchingProj()
{
  if ( d->mEllipsoidAcronym.isNull() || d->mProjectionAcronym.isNull()
       || !d->mIsValid )
  {
    QgsDebugMsgLevel( "QgsCoordinateReferenceSystem::findMatchingProj will only "
                      "work if prj acr ellipsoid acr and proj4string are set"
                      " and the current projection is valid!", 4 );
    return 0;
  }

  sqlite3_database_unique_ptr database;
  sqlite3_statement_unique_ptr statement;
  int myResult;

  // Set up the query to retrieve the projection information
  // needed to populate the list
  QString mySql = QString( "select srs_id,parameters from tbl_srs where "
                           "projection_acronym=%1 and ellipsoid_acronym=%2 order by deprecated" )
                  .arg( QgsSqliteUtils::quotedString( d->mProjectionAcronym ),
                        QgsSqliteUtils::quotedString( d->mEllipsoidAcronym ) );
  // Get the full path name to the sqlite3 spatial reference database.
  QString myDatabaseFileName = QgsApplication::srsDatabaseFilePath();

  //check the db is available
  myResult = openDatabase( myDatabaseFileName, database );
  if ( myResult != SQLITE_OK )
  {
    return 0;
  }

  statement = database.prepare( mySql, myResult );
  if ( myResult == SQLITE_OK )
  {

    while ( statement.step() == SQLITE_ROW )
    {
      QString mySrsId = statement.columnAsText( 0 );
      QString myProj4String = statement.columnAsText( 1 );
      if ( toProj4() == myProj4String.trimmed() )
      {
        return mySrsId.toLong();
      }
    }
  }

  //
  // Try the users db now
  //

  myDatabaseFileName = QgsApplication::qgisUserDatabaseFilePath();
  //check the db is available
  myResult = openDatabase( myDatabaseFileName, database );
  if ( myResult != SQLITE_OK )
  {
    return 0;
  }

  statement = database.prepare( mySql, myResult );

  if ( myResult == SQLITE_OK )
  {
    while ( statement.step() == SQLITE_ROW )
    {
      QString mySrsId = statement.columnAsText( 0 );
      QString myProj4String = statement.columnAsText( 1 );
      if ( toProj4() == myProj4String.trimmed() )
      {
        return mySrsId.toLong();
      }
    }
  }

  return 0;
}

bool QgsCoordinateReferenceSystem::operator==( const QgsCoordinateReferenceSystem &srs ) const
{
  return ( !d->mIsValid && !srs.d->mIsValid ) ||
         ( d->mIsValid && srs.d->mIsValid && srs.authid() == authid() );
}

bool QgsCoordinateReferenceSystem::operator!=( const QgsCoordinateReferenceSystem &srs ) const
{
  return  !( *this == srs );
}

QString QgsCoordinateReferenceSystem::toWkt() const
{
  if ( d->mWkt.isEmpty() )
  {
    char *wkt = nullptr;
    if ( OSRExportToWkt( d->mCRS, &wkt ) == OGRERR_NONE )
    {
      d->mWkt = wkt;
      CPLFree( wkt );
    }
  }
  return d->mWkt;
}

bool QgsCoordinateReferenceSystem::readXml( const QDomNode &node )
{
  d.detach();
  bool result = true;
  QDomNode srsNode  = node.namedItem( QStringLiteral( "spatialrefsys" ) );

  if ( ! srsNode.isNull() )
  {
    bool initialized = false;

    long srsid = srsNode.namedItem( QStringLiteral( "srsid" ) ).toElement().text().toLong();

    QDomNode myNode;

    if ( srsid < USER_CRS_START_ID )
    {
      myNode = srsNode.namedItem( QStringLiteral( "authid" ) );
      if ( !myNode.isNull() )
      {
        operator=( QgsCoordinateReferenceSystem::fromOgcWmsCrs( myNode.toElement().text() ) );
        if ( isValid() )
        {
          initialized = true;
        }
      }

      if ( !initialized )
      {
        myNode = srsNode.namedItem( QStringLiteral( "epsg" ) );
        if ( !myNode.isNull() )
        {
          operator=( QgsCoordinateReferenceSystem::fromEpsgId( myNode.toElement().text().toLong() ) );
          if ( isValid() )
          {
            initialized = true;
          }
        }
      }
    }

    if ( !initialized )
    {
      myNode = srsNode.namedItem( QStringLiteral( "proj4" ) );

      if ( !createFromProj4( myNode.toElement().text() ) )
      {
        // Setting from elements one by one

        myNode = srsNode.namedItem( QStringLiteral( "proj4" ) );
        setProj4String( myNode.toElement().text() );

        myNode = srsNode.namedItem( QStringLiteral( "srsid" ) );
        setInternalId( myNode.toElement().text().toLong() );

        myNode = srsNode.namedItem( QStringLiteral( "srid" ) );
        setSrid( myNode.toElement().text().toLong() );

        myNode = srsNode.namedItem( QStringLiteral( "authid" ) );
        setAuthId( myNode.toElement().text() );

        myNode = srsNode.namedItem( QStringLiteral( "description" ) );
        setDescription( myNode.toElement().text() );

        myNode = srsNode.namedItem( QStringLiteral( "projectionacronym" ) );
        setProjectionAcronym( myNode.toElement().text() );

        myNode = srsNode.namedItem( QStringLiteral( "ellipsoidacronym" ) );
        setEllipsoidAcronym( myNode.toElement().text() );

        myNode = srsNode.namedItem( QStringLiteral( "geographicflag" ) );
        if ( myNode.toElement().text().compare( QLatin1String( "true" ) ) )
        {
          setGeographicFlag( true );
        }
        else
        {
          setGeographicFlag( false );
        }

        //make sure the map units have been set
        setMapUnits();
      }
      //TODO: createFromProj4 used to save to the user database any new CRS
      // this behavior was changed in order to separate creation and saving.
      // Not sure if it necessary to save it here, should be checked by someone
      // familiar with the code (should also give a more descriptive name to the generated CRS)
      if ( d->mSrsId == 0 )
      {
        QString myName = QStringLiteral( " * %1 (%2)" )
                         .arg( QObject::tr( "Generated CRS", "A CRS automatically generated from layer info get this prefix for description" ),
                               toProj4() );
        saveAsUserCrs( myName );
      }

    }
  }
  else
  {
    // Return empty CRS if none was found in the XML.
    d = new QgsCoordinateReferenceSystemPrivate();
    result = false;
  }
  return result;
}

bool QgsCoordinateReferenceSystem::writeXml( QDomNode &node, QDomDocument &doc ) const
{

  QDomElement myLayerNode = node.toElement();
  QDomElement mySrsElement  = doc.createElement( QStringLiteral( "spatialrefsys" ) );

  QDomElement myProj4Element  = doc.createElement( QStringLiteral( "proj4" ) );
  myProj4Element.appendChild( doc.createTextNode( toProj4() ) );
  mySrsElement.appendChild( myProj4Element );

  QDomElement mySrsIdElement  = doc.createElement( QStringLiteral( "srsid" ) );
  mySrsIdElement.appendChild( doc.createTextNode( QString::number( srsid() ) ) );
  mySrsElement.appendChild( mySrsIdElement );

  QDomElement mySridElement  = doc.createElement( QStringLiteral( "srid" ) );
  mySridElement.appendChild( doc.createTextNode( QString::number( postgisSrid() ) ) );
  mySrsElement.appendChild( mySridElement );

  QDomElement myEpsgElement  = doc.createElement( QStringLiteral( "authid" ) );
  myEpsgElement.appendChild( doc.createTextNode( authid() ) );
  mySrsElement.appendChild( myEpsgElement );

  QDomElement myDescriptionElement  = doc.createElement( QStringLiteral( "description" ) );
  myDescriptionElement.appendChild( doc.createTextNode( description() ) );
  mySrsElement.appendChild( myDescriptionElement );

  QDomElement myProjectionAcronymElement  = doc.createElement( QStringLiteral( "projectionacronym" ) );
  myProjectionAcronymElement.appendChild( doc.createTextNode( projectionAcronym() ) );
  mySrsElement.appendChild( myProjectionAcronymElement );

  QDomElement myEllipsoidAcronymElement  = doc.createElement( QStringLiteral( "ellipsoidacronym" ) );
  myEllipsoidAcronymElement.appendChild( doc.createTextNode( ellipsoidAcronym() ) );
  mySrsElement.appendChild( myEllipsoidAcronymElement );

  QDomElement myGeographicFlagElement  = doc.createElement( QStringLiteral( "geographicflag" ) );
  QString myGeoFlagText = QStringLiteral( "false" );
  if ( isGeographic() )
  {
    myGeoFlagText = QStringLiteral( "true" );
  }

  myGeographicFlagElement.appendChild( doc.createTextNode( myGeoFlagText ) );
  mySrsElement.appendChild( myGeographicFlagElement );

  myLayerNode.appendChild( mySrsElement );

  return true;
}



//
// Static helper methods below this point only please!
//


// Returns the whole proj4 string for the selected srsid
//this is a static method! NOTE I've made it private for now to reduce API clutter TS
QString QgsCoordinateReferenceSystem::proj4FromSrsId( const int srsId )
{
  QString myDatabaseFileName;
  QString myProjString;
  QString mySql = QStringLiteral( "select parameters from tbl_srs where srs_id = %1 order by deprecated" ).arg( srsId );

  //
  // Determine if this is a user projection or a system on
  // user projection defs all have srs_id >= 100000
  //
  if ( srsId >= USER_CRS_START_ID )
  {
    myDatabaseFileName = QgsApplication::qgisUserDatabaseFilePath();
    QFileInfo myFileInfo;
    myFileInfo.setFile( myDatabaseFileName );
    if ( !myFileInfo.exists() ) //its unlikely that this condition will ever be reached
    {
      QgsDebugMsg( QStringLiteral( "users qgis.db not found" ) );
      return QString();
    }
  }
  else //must be  a system projection then
  {
    myDatabaseFileName = QgsApplication::srsDatabaseFilePath();
  }

  sqlite3_database_unique_ptr database;
  sqlite3_statement_unique_ptr statement;

  int rc;
  rc = openDatabase( myDatabaseFileName, database );
  if ( rc )
  {
    return QString();
  }

  statement = database.prepare( mySql, rc );

  if ( rc == SQLITE_OK )
  {
    if ( statement.step() == SQLITE_ROW )
    {
      myProjString = statement.columnAsText( 0 );
    }
  }

  return myProjString;
}

int QgsCoordinateReferenceSystem::openDatabase( const QString &path, sqlite3_database_unique_ptr &database, bool readonly )
{
  int myResult;
  if ( readonly )
    myResult = database.open_v2( path, SQLITE_OPEN_READONLY, nullptr );
  else
    myResult = database.open( path );

  if ( myResult != SQLITE_OK )
  {
    QgsDebugMsg( "Can't open database: " + database.errorMessage() );
    // XXX This will likely never happen since on open, sqlite creates the
    //     database if it does not exist.
    // ... unfortunately it happens on Windows
    QgsMessageLog::logMessage( QObject::tr( "Could not open CRS database %1\nError(%2): %3" )
                               .arg( path )
                               .arg( myResult )
                               .arg( database.errorMessage() ), QObject::tr( "CRS" ) );
  }
  return myResult;
}

void QgsCoordinateReferenceSystem::setCustomCrsValidation( CUSTOM_CRS_VALIDATION f )
{
  mCustomSrsValidation = f;
}

CUSTOM_CRS_VALIDATION QgsCoordinateReferenceSystem::customCrsValidation()
{
  return mCustomSrsValidation;
}

void QgsCoordinateReferenceSystem::debugPrint()
{
  QgsDebugMsg( QStringLiteral( "***SpatialRefSystem***" ) );
  QgsDebugMsg( "* Valid : " + ( d->mIsValid ? QString( "true" ) : QString( "false" ) ) );
  QgsDebugMsg( "* SrsId : " + QString::number( d->mSrsId ) );
  QgsDebugMsg( "* Proj4 : " + toProj4() );
  QgsDebugMsg( "* WKT   : " + toWkt() );
  QgsDebugMsg( "* Desc. : " + d->mDescription );
  if ( mapUnits() == QgsUnitTypes::DistanceMeters )
  {
    QgsDebugMsg( QStringLiteral( "* Units : meters" ) );
  }
  else if ( mapUnits() == QgsUnitTypes::DistanceFeet )
  {
    QgsDebugMsg( QStringLiteral( "* Units : feet" ) );
  }
  else if ( mapUnits() == QgsUnitTypes::DistanceDegrees )
  {
    QgsDebugMsg( QStringLiteral( "* Units : degrees" ) );
  }
}

void QgsCoordinateReferenceSystem::setValidationHint( const QString &html )
{
  d.detach();
  d->mValidationHint = html;
}

QString QgsCoordinateReferenceSystem::validationHint()
{
  return d->mValidationHint;
}

/// Copied from QgsCustomProjectionDialog ///
/// Please refactor into SQL handler !!!  ///

long QgsCoordinateReferenceSystem::saveAsUserCrs( const QString &name )
{
  if ( !d->mIsValid )
  {
    QgsDebugMsgLevel( QStringLiteral( "Can't save an invalid CRS!" ), 4 );
    return -1;
  }

  QString mySql;

  QString proj4String = d->mProj4;
  if ( proj4String.isEmpty() )
  {
    proj4String = toProj4();
  }

  // ellipsoid acroynym column is incorrect marked as not null in many crs database instances,
  // hack around this by using an empty string instead
  const QString quotedEllipsoidString = ellipsoidAcronym().isNull() ? "''" : QgsSqliteUtils::quotedString( ellipsoidAcronym() );

  //if this is the first record we need to ensure that its srs_id is 10000. For
  //any rec after that sqlite3 will take care of the autonumbering
  //this was done to support sqlite 3.0 as it does not yet support
  //the autoinc related system tables.
  if ( getRecordCount() == 0 )
  {
    mySql = "insert into tbl_srs (srs_id,description,projection_acronym,ellipsoid_acronym,parameters,is_geo) values ("
            + QString::number( USER_CRS_START_ID )
            + ',' + QgsSqliteUtils::quotedString( name )
            + ',' + QgsSqliteUtils::quotedString( projectionAcronym() )
            + ',' + quotedEllipsoidString
            + ',' + QgsSqliteUtils::quotedString( toProj4() )
            + ",0)"; // <-- is_geo shamelessly hard coded for now
  }
  else
  {
    mySql = "insert into tbl_srs (description,projection_acronym,ellipsoid_acronym,parameters,is_geo) values ("
            + QgsSqliteUtils::quotedString( name )
            + ',' + QgsSqliteUtils::quotedString( projectionAcronym() )
            + ',' + quotedEllipsoidString
            + ',' + QgsSqliteUtils::quotedString( toProj4() )
            + ",0)"; // <-- is_geo shamelessly hard coded for now
  }
  sqlite3_database_unique_ptr database;
  sqlite3_statement_unique_ptr statement;
  int           myResult;
  //check the db is available
  myResult = database.open( QgsApplication::qgisUserDatabaseFilePath() );
  if ( myResult != SQLITE_OK )
  {
    QgsDebugMsg( QStringLiteral( "Can't open or create database %1: %2" )
                 .arg( QgsApplication::qgisUserDatabaseFilePath(),
                       database.errorMessage() ) );
    return false;
  }
  statement = database.prepare( mySql, myResult );

  qint64 returnId;
  if ( myResult == SQLITE_OK && statement.step() == SQLITE_DONE )
  {
    QgsMessageLog::logMessage( QObject::tr( "Saved user CRS [%1]" ).arg( toProj4() ), QObject::tr( "CRS" ) );

    returnId = sqlite3_last_insert_rowid( database.get() );
    setInternalId( returnId );
    if ( authid().isEmpty() )
      setAuthId( QStringLiteral( "USER:%1" ).arg( returnId ) );
    setDescription( name );

    //We add the just created user CRS to the list of recently used CRS
    QgsSettings settings;
    //QStringList recentProjections = settings.value( "/UI/recentProjections" ).toStringList();
    QStringList projectionsProj4 = settings.value( QStringLiteral( "UI/recentProjectionsProj4" ) ).toStringList();
    QStringList projectionsAuthId = settings.value( QStringLiteral( "UI/recentProjectionsAuthId" ) ).toStringList();
    //recentProjections.append();
    //settings.setValue( "/UI/recentProjections", recentProjections );
    projectionsProj4.append( toProj4() );
    projectionsAuthId.append( authid() );
    settings.setValue( QStringLiteral( "UI/recentProjectionsProj4" ), projectionsProj4 );
    settings.setValue( QStringLiteral( "UI/recentProjectionsAuthId" ), projectionsAuthId );

  }
  else
    returnId = -1;

  invalidateCache();
  return returnId;
}

long QgsCoordinateReferenceSystem::getRecordCount()
{
  sqlite3_database_unique_ptr database;
  sqlite3_statement_unique_ptr statement;
  int           myResult;
  long          myRecordCount = 0;
  //check the db is available
  myResult = database.open_v2( QgsApplication::qgisUserDatabaseFilePath(), SQLITE_OPEN_READONLY, nullptr );
  if ( myResult != SQLITE_OK )
  {
    QgsDebugMsg( QStringLiteral( "Can't open database: %1" ).arg( database.errorMessage() ) );
    return 0;
  }
  // Set up the query to retrieve the projection information needed to populate the ELLIPSOID list
  QString mySql = QStringLiteral( "select count(*) from tbl_srs" );
  statement = database.prepare( mySql, myResult );
  if ( myResult == SQLITE_OK )
  {
    if ( statement.step() == SQLITE_ROW )
    {
      QString myRecordCountString = statement.columnAsText( 0 );
      myRecordCount = myRecordCountString.toLong();
    }
  }
  return myRecordCount;
}

// adapted from gdal/ogr/ogr_srs_dict.cpp
bool QgsCoordinateReferenceSystem::loadWkts( QHash<int, QString> &wkts, const char *filename )
{
  QgsDebugMsgLevel( QStringLiteral( "Loading %1" ).arg( filename ), 4 );
  const char *pszFilename = CPLFindFile( "gdal", filename );
  if ( !pszFilename )
    return false;

  QFile csv( pszFilename );
  if ( !csv.open( QIODevice::ReadOnly ) )
    return false;

  QTextStream lines( &csv );

  for ( ;; )
  {
    QString line = lines.readLine();
    if ( line.isNull() )
      break;

    if ( line.trimmed().isEmpty() || line.startsWith( '#' ) )
    {
      continue;
    }
    else if ( line.startsWith( QLatin1String( "include " ) ) )
    {
      if ( !loadWkts( wkts, line.mid( 8 ).toUtf8() ) )
        break;
    }
    else
    {
      int pos = line.indexOf( ',' );
      if ( pos < 0 )
        return false;

      bool ok;
      int epsg = line.leftRef( pos ).toInt( &ok );
      if ( !ok )
        return false;

      wkts.insert( epsg, line.mid( pos + 1 ) );
    }
  }

  csv.close();

  return true;
}

bool QgsCoordinateReferenceSystem::loadIds( QHash<int, QString> &wkts )
{
  OGRSpatialReferenceH crs = OSRNewSpatialReference( nullptr );

  Q_FOREACH ( const QString &csv, QStringList() << "gcs.csv" << "pcs.csv" << "vertcs.csv" << "compdcs.csv" << "geoccs.csv" )
  {
    QString filename = CPLFindFile( "gdal", csv.toUtf8() );

    QFile f( filename );
    if ( !f.open( QIODevice::ReadOnly ) )
      continue;

    QTextStream lines( &f );
    int l = 0, n = 0;

    lines.readLine();
    for ( ;; )
    {
      l++;
      QString line = lines.readLine();
      if ( line.isNull() )
        break;

      if ( line.trimmed().isEmpty() )
        continue;

      int pos = line.indexOf( ',' );
      if ( pos < 0 )
      {
        qWarning( "No id found in: %s", qPrintable( line ) );
        continue;
      }

      bool ok;
      int epsg = line.leftRef( pos ).toInt( &ok );
      if ( !ok )
      {
        qWarning( "No valid id found in: %s", qPrintable( line ) );
        continue;
      }

      // some CRS are known to fail (see http://trac.osgeo.org/gdal/ticket/2900)
      if ( epsg == 2218 || epsg == 2221 || epsg == 2296 || epsg == 2297 || epsg == 2298 || epsg == 2299 || epsg == 2300 || epsg == 2301 || epsg == 2302 ||
           epsg == 2303 || epsg == 2304 || epsg == 2305 || epsg == 2306 || epsg == 2307 || epsg == 2963 || epsg == 2985 || epsg == 2986 || epsg == 3052 ||
           epsg == 3053 || epsg == 3139 || epsg == 3144 || epsg == 3145 || epsg == 3173 || epsg == 3295 || epsg == 3993 || epsg == 4087 || epsg == 4088 ||
           epsg == 5017 || epsg == 5221 || epsg == 5224 || epsg == 5225 || epsg == 5514 || epsg == 5515 || epsg == 5516 || epsg == 5819 || epsg == 5820 ||
           epsg == 5821 || epsg == 6200 || epsg == 6201 || epsg == 6202 || epsg == 6244 || epsg == 6245 || epsg == 6246 || epsg == 6247 || epsg == 6248 ||
           epsg == 6249 || epsg == 6250 || epsg == 6251 || epsg == 6252 || epsg == 6253 || epsg == 6254 || epsg == 6255 || epsg == 6256 || epsg == 6257 ||
           epsg == 6258 || epsg == 6259 || epsg == 6260 || epsg == 6261 || epsg == 6262 || epsg == 6263 || epsg == 6264 || epsg == 6265 || epsg == 6266 ||
           epsg == 6267 || epsg == 6268 || epsg == 6269 || epsg == 6270 || epsg == 6271 || epsg == 6272 || epsg == 6273 || epsg == 6274 || epsg == 6275 ||
           epsg == 6966 || epsg == 7082 || epsg == 32600 || epsg == 32663 || epsg == 32700 )
        continue;

      if ( OSRImportFromEPSG( crs, epsg ) != OGRERR_NONE )
      {
        qDebug( "EPSG %d: not imported", epsg );
        continue;
      }

      char *wkt = nullptr;
      if ( OSRExportToWkt( crs, &wkt ) != OGRERR_NONE )
      {
        qWarning( "EPSG %d: not exported to WKT", epsg );
        continue;
      }

      wkts.insert( epsg, wkt );
      n++;

      CPLFree( wkt );
    }

    f.close();

    QgsDebugMsgLevel( QStringLiteral( "Loaded %1/%2 from %3" ).arg( QString::number( n ), QString::number( l ), filename.toUtf8().constData() ), 4 );
  }

  OSRDestroySpatialReference( crs );

  return true;
}

int QgsCoordinateReferenceSystem::syncDatabase()
{
  setlocale( LC_ALL, "C" );
  QString dbFilePath = QgsApplication::srsDatabaseFilePath();
  syncDatumTransform( dbFilePath );

  int inserted = 0, updated = 0, deleted = 0, errors = 0;

  QgsDebugMsgLevel( QStringLiteral( "Load srs db from: %1" ).arg( QgsApplication::srsDatabaseFilePath().toLocal8Bit().constData() ), 4 );

  sqlite3_database_unique_ptr database;
  if ( database.open( dbFilePath ) != SQLITE_OK )
  {
    QgsDebugMsg( QStringLiteral( "Could not open database: %1 (%2)\n" ).arg( QgsApplication::srsDatabaseFilePath(), database.errorMessage() ) );
    return -1;
  }

  if ( sqlite3_exec( database.get(), "BEGIN TRANSACTION", nullptr, nullptr, nullptr ) != SQLITE_OK )
  {
    QgsDebugMsg( QStringLiteral( "Could not begin transaction: %1 (%2)\n" ).arg( QgsApplication::srsDatabaseFilePath(), database.errorMessage() ) );
    return -1;
  }

  // fix up database, if not done already //
  if ( sqlite3_exec( database.get(), "alter table tbl_srs add noupdate boolean", nullptr, nullptr, nullptr ) == SQLITE_OK )
    ( void )sqlite3_exec( database.get(), "update tbl_srs set noupdate=(auth_name='EPSG' and auth_id in (5513,5514,5221,2065,102067,4156,4818))", nullptr, nullptr, nullptr );

  ( void )sqlite3_exec( database.get(), "UPDATE tbl_srs SET srid=141001 WHERE srid=41001 AND auth_name='OSGEO' AND auth_id='41001'", nullptr, nullptr, nullptr );

  OGRSpatialReferenceH crs = nullptr;
  sqlite3_statement_unique_ptr statement;
  int result;
  char *errMsg = nullptr;

  QString proj4;
  QString sql;
  QHash<int, QString> wkts;
  loadIds( wkts );
  loadWkts( wkts, "epsg.wkt" );

  QgsDebugMsgLevel( QStringLiteral( "%1 WKTs loaded" ).arg( wkts.count() ), 4 );

  for ( QHash<int, QString>::const_iterator it = wkts.constBegin(); it != wkts.constEnd(); ++it )
  {
    QByteArray ba( it.value().toUtf8() );
    char *psz = ba.data();

    if ( crs )
      OSRDestroySpatialReference( crs );
    crs = nullptr;
    crs = OSRNewSpatialReference( nullptr );

    OGRErr ogrErr = OSRImportFromWkt( crs, &psz );
    if ( ogrErr != OGRERR_NONE )
      continue;

    if ( OSRExportToProj4( crs, &psz ) != OGRERR_NONE )
    {
      CPLFree( psz );
      continue;
    }

    proj4 = psz;
    proj4 = proj4.trimmed();

    CPLFree( psz );

    if ( proj4.isEmpty() )
      continue;

    QString name( OSRIsGeographic( crs ) ? OSRGetAttrValue( crs, "GEOGCS", 0 ) :
                  OSRIsGeocentric( crs ) ? OSRGetAttrValue( crs, "GEOCCS", 0 ) :
                  OSRGetAttrValue( crs, "PROJCS", 0 ) );
    if ( name.isEmpty() )
      name = QObject::tr( "Imported from GDAL" );

    bool deprecated = name.contains( QLatin1Literal( "(deprecated)" ) );

    sql = QStringLiteral( "SELECT parameters,description,deprecated,noupdate FROM tbl_srs WHERE auth_name='EPSG' AND auth_id='%1'" ).arg( it.key() );
    statement = database.prepare( sql, result );
    if ( result != SQLITE_OK )
    {
      QgsDebugMsg( QStringLiteral( "Could not prepare: %1 [%2]\n" ).arg( sql, database.errorMessage() ) );
      continue;
    }

    QString srsProj4;
    QString srsDesc;
    bool srsDeprecated = deprecated;
    if ( statement.step() == SQLITE_ROW )
    {
      srsProj4 = statement.columnAsText( 0 );
      srsDesc = statement.columnAsText( 1 );
      srsDeprecated = statement.columnAsText( 2 ).toInt() != 0;

      if ( statement.columnAsText( 3 ).toInt() != 0 )
      {
        continue;
      }
    }

    if ( !srsProj4.isEmpty() || !srsDesc.isEmpty() )
    {
      if ( proj4 != srsProj4 || name != srsDesc || deprecated != srsDeprecated )
      {
        errMsg = nullptr;
        sql = QStringLiteral( "UPDATE tbl_srs SET parameters=%1,description=%2,deprecated=%3 WHERE auth_name='EPSG' AND auth_id=%4" )
              .arg( QgsSqliteUtils::quotedString( proj4 ) )
              .arg( QgsSqliteUtils::quotedString( name ) )
              .arg( deprecated ? 1 : 0 )
              .arg( it.key() );

        if ( sqlite3_exec( database.get(), sql.toUtf8(), nullptr, nullptr, &errMsg ) != SQLITE_OK )
        {
          QgsDebugMsg( QStringLiteral( "Could not execute: %1 [%2/%3]\n" ).arg(
                         sql,
                         database.errorMessage(),
                         errMsg ? errMsg : "(unknown error)" ) );
          if ( errMsg )
            sqlite3_free( errMsg );
          errors++;
        }
        else
        {
          updated++;
        }
      }
    }
    else
    {
      QRegExp projRegExp( "\\+proj=(\\S+)" );
      if ( projRegExp.indexIn( proj4 ) < 0 )
      {
        QgsDebugMsgLevel( QStringLiteral( "EPSG %1: no +proj argument found [%2]" ).arg( it.key() ).arg( proj4 ), 4 );
        continue;
      }

      QRegExp ellipseRegExp( "\\+ellps=(\\S+)" );
      QString ellps;
      if ( ellipseRegExp.indexIn( proj4 ) >= 0 )
      {
        ellps = ellipseRegExp.cap( 1 );
      }
      else
      {
        // satisfy not null constraint on ellipsoid_acronym field
        // possibly we should drop the constraint, yet the crses with missing ellipsoid_acronym are malformed
        // and will result in oddities within other areas of QGIS (e.g. project ellipsoid won't be correctly
        // set for these CRSes). Better just hack around and make the constraint happy for now,
        // and hope that the definitions get corrected in future.
        ellps = "";
      }

      sql = QStringLiteral( "INSERT INTO tbl_srs(description,projection_acronym,ellipsoid_acronym,parameters,srid,auth_name,auth_id,is_geo,deprecated) VALUES (%1,%2,%3,%4,%5,'EPSG',%5,%6,%7)" )
            .arg( QgsSqliteUtils::quotedString( name ),
                  QgsSqliteUtils::quotedString( projRegExp.cap( 1 ) ),
                  QgsSqliteUtils::quotedString( ellps ),
                  QgsSqliteUtils::quotedString( proj4 ) )
            .arg( it.key() )
            .arg( OSRIsGeographic( crs ) )
            .arg( deprecated ? 1 : 0 );

      errMsg = nullptr;
      if ( sqlite3_exec( database.get(), sql.toUtf8(), nullptr, nullptr, &errMsg ) == SQLITE_OK )
      {
        inserted++;
      }
      else
      {
        qCritical( "Could not execute: %s [%s/%s]\n",
                   sql.toLocal8Bit().constData(),
                   sqlite3_errmsg( database.get() ),
                   errMsg ? errMsg : "(unknown error)" );
        errors++;

        if ( errMsg )
          sqlite3_free( errMsg );
      }
    }
  }

  if ( crs )
    OSRDestroySpatialReference( crs );
  crs = nullptr;

  sql = QStringLiteral( "DELETE FROM tbl_srs WHERE auth_name='EPSG' AND NOT auth_id IN (" );
  QString delim;
  QHash<int, QString>::const_iterator it = wkts.constBegin();
  for ( ; it != wkts.constEnd(); ++it )
  {
    sql += delim + QString::number( it.key() );
    delim = ',';
  }
  sql += QLatin1String( ") AND NOT noupdate" );

  if ( sqlite3_exec( database.get(), sql.toUtf8(), nullptr, nullptr, nullptr ) == SQLITE_OK )
  {
    deleted = sqlite3_changes( database.get() );
  }
  else
  {
    errors++;
    qCritical( "Could not execute: %s [%s]\n",
               sql.toLocal8Bit().constData(),
               sqlite3_errmsg( database.get() ) );
  }

  projCtx pContext = pj_ctx_alloc();

#if !defined(PJ_VERSION) || PJ_VERSION!=470
  sql = QStringLiteral( "select auth_name,auth_id,parameters from tbl_srs WHERE auth_name<>'EPSG' AND NOT deprecated AND NOT noupdate" );
  statement = database.prepare( sql, result );
  if ( result == SQLITE_OK )
  {
    while ( statement.step()  == SQLITE_ROW )
    {
      QString auth_name = statement.columnAsText( 0 );
      QString auth_id   = statement.columnAsText( 1 );
      QString params    = statement.columnAsText( 2 );

      QString input = QStringLiteral( "+init=%1:%2" ).arg( auth_name.toLower(), auth_id );
      projPJ pj = pj_init_plus_ctx( pContext, input.toLatin1() );
      if ( !pj )
      {
        input = QStringLiteral( "+init=%1:%2" ).arg( auth_name.toUpper(), auth_id );
        pj = pj_init_plus_ctx( pContext, input.toLatin1() );
      }

      if ( pj )
      {
        char *def = pj_get_def( pj, 0 );
        if ( def )
        {
          proj4 = def;
          pj_dalloc( def );

          input.prepend( ' ' ).append( ' ' );
          if ( proj4.startsWith( input ) )
          {
            proj4 = proj4.mid( input.size() );
            proj4 = proj4.trimmed();
          }

          if ( proj4 != params )
          {
            sql = QStringLiteral( "UPDATE tbl_srs SET parameters=%1 WHERE auth_name=%2 AND auth_id=%3" )
                  .arg( QgsSqliteUtils::quotedString( proj4 ),
                        QgsSqliteUtils::quotedString( auth_name ),
                        QgsSqliteUtils::quotedString( auth_id ) );

            if ( sqlite3_exec( database.get(), sql.toUtf8(), nullptr, nullptr, &errMsg ) == SQLITE_OK )
            {
              updated++;
            }
            else
            {
              qCritical( "Could not execute: %s [%s/%s]\n",
                         sql.toLocal8Bit().constData(),
                         sqlite3_errmsg( database.get() ),
                         errMsg ? errMsg : "(unknown error)" );
              if ( errMsg )
                sqlite3_free( errMsg );
              errors++;
            }
          }
        }
        else
        {
          QgsDebugMsgLevel( QStringLiteral( "could not retrieve proj string for %1 from PROJ" ).arg( input ), 4 );
        }
      }
      else
      {
        QgsDebugMsgLevel( QStringLiteral( "could not retrieve crs for %1 from PROJ" ).arg( input ), 3 );
      }

      pj_free( pj );
    }
  }
  else
  {
    errors++;
    QgsDebugMsg( QStringLiteral( "Could not execute: %1 [%2]\n" ).arg(
                   sql,
                   sqlite3_errmsg( database.get() ) ) );
  }
#endif

  pj_ctx_free( pContext );

  if ( sqlite3_exec( database.get(), "COMMIT", nullptr, nullptr, nullptr ) != SQLITE_OK )
  {
    QgsDebugMsg( QStringLiteral( "Could not commit transaction: %1 [%2]\n" ).arg(
                   QgsApplication::srsDatabaseFilePath(),
                   sqlite3_errmsg( database.get() ) )
               );
    return -1;
  }

  Q_UNUSED( deleted );
  QgsDebugMsgLevel( QStringLiteral( "CRS update (inserted:%1 updated:%2 deleted:%3 errors:%4)" ).arg( QString::number( inserted ), QString::number( updated ), QString::number( deleted ), QString::number( errors ) ), 4 );

  if ( errors > 0 )
    return -errors;
  else
    return updated + inserted;
}

bool QgsCoordinateReferenceSystem::syncDatumTransform( const QString &dbPath )
{
  const char *filename = CSVFilename( "datum_shift.csv" );
  FILE *fp = VSIFOpen( filename, "rb" );
  if ( !fp )
  {
    return false;
  }

  char **fieldnames = CSVReadParseLine( fp );

  // "SEQ_KEY","COORD_OP_CODE","SOURCE_CRS_CODE","TARGET_CRS_CODE","REMARKS","COORD_OP_SCOPE","AREA_OF_USE_CODE","AREA_SOUTH_BOUND_LAT","AREA_NORTH_BOUND_LAT","AREA_WEST_BOUND_LON","AREA_EAST_BOUND_LON","SHOW_OPERATION","DEPRECATED","COORD_OP_METHOD_CODE","DX","DY","DZ","RX","RY","RZ","DS","PREFERRED"

  struct
  {
    const char *src; //skip-init-check
    const char *dst; //skip-init-check
    int idx;
  } map[] =
  {
    // { "SEQ_KEY", "", -1 },
    { "SOURCE_CRS_CODE", "source_crs_code", -1 },
    { "TARGET_CRS_CODE", "target_crs_code", -1 },
    { "REMARKS", "remarks", -1 },
    { "COORD_OP_SCOPE", "scope", -1 },
    { "AREA_OF_USE_CODE", "area_of_use_code", -1 },
    // { "AREA_SOUTH_BOUND_LAT", "", -1 },
    // { "AREA_NORTH_BOUND_LAT", "", -1 },
    // { "AREA_WEST_BOUND_LON", "", -1 },
    // { "AREA_EAST_BOUND_LON", "", -1 },
    // { "SHOW_OPERATION", "", -1 },
    { "DEPRECATED", "deprecated", -1 },
    { "COORD_OP_METHOD_CODE", "coord_op_method_code", -1 },
    { "DX", "p1", -1 },
    { "DY", "p2", -1 },
    { "DZ", "p3", -1 },
    { "RX", "p4", -1 },
    { "RY", "p5", -1 },
    { "RZ", "p6", -1 },
    { "DS", "p7", -1 },
    { "PREFERRED", "preferred", -1 },
    { "COORD_OP_CODE", "coord_op_code", -1 },
  };

  QString update = QStringLiteral( "UPDATE tbl_datum_transform SET " );
  QString insert, values;

  int n = CSLCount( fieldnames );

  int idxid = -1, idxrx = -1, idxry = -1, idxrz = -1, idxmcode = -1;
  for ( unsigned int i = 0; i < sizeof( map ) / sizeof( *map ); i++ )
  {
    bool last = i == sizeof( map ) / sizeof( *map ) - 1;

    map[i].idx = CSLFindString( fieldnames, map[i].src );
    if ( map[i].idx < 0 )
    {
      qWarning( "field %s not found", map[i].src );
      CSLDestroy( fieldnames );
      fclose( fp );
      return false;
    }

    if ( strcmp( map[i].src, "COORD_OP_CODE" ) == 0 )
      idxid = i;
    if ( strcmp( map[i].src, "RX" ) == 0 )
      idxrx = i;
    if ( strcmp( map[i].src, "RY" ) == 0 )
      idxry = i;
    if ( strcmp( map[i].src, "RZ" ) == 0 )
      idxrz = i;
    if ( strcmp( map[i].src, "COORD_OP_METHOD_CODE" ) == 0 )
      idxmcode = i;

    if ( i > 0 )
    {
      insert += ',';
      values += ',';

      if ( last )
      {
        update += QLatin1String( " WHERE " );
      }
      else
      {
        update += ',';
      }
    }

    update += QStringLiteral( "%1=%%2" ).arg( map[i].dst ).arg( i + 1 );

    insert += map[i].dst;
    values += QStringLiteral( "%%1" ).arg( i + 1 );
  }

  insert = "INSERT INTO tbl_datum_transform(" + insert + ") VALUES (" + values + ')';

  CSLDestroy( fieldnames );

  Q_ASSERT( idxid >= 0 );
  Q_ASSERT( idxrx >= 0 );
  Q_ASSERT( idxry >= 0 );
  Q_ASSERT( idxrz >= 0 );

  sqlite3_database_unique_ptr database;
  int openResult = database.open( dbPath );
  if ( openResult != SQLITE_OK )
  {
    fclose( fp );
    return false;
  }

  if ( sqlite3_exec( database.get(), "BEGIN TRANSACTION", nullptr, nullptr, nullptr ) != SQLITE_OK )
  {
    qCritical( "Could not begin transaction: %s [%s]\n", QgsApplication::srsDatabaseFilePath().toLocal8Bit().constData(), sqlite3_errmsg( database.get() ) );
    fclose( fp );
    return false;
  }

  QStringList v;
  v.reserve( sizeof( map ) / sizeof( *map ) );

  for ( ;; )
  {
    char **values = CSVReadParseLine( fp );
    if ( !values )
      break;

    v.clear();

    if ( CSLCount( values ) == 0 )
    {
      CSLDestroy( values );
      break;
    }

    if ( CSLCount( values ) < n )
    {
      qWarning( "Only %d columns", CSLCount( values ) );
      CSLDestroy( values );
      continue;
    }

    for ( unsigned int i = 0; i < sizeof( map ) / sizeof( *map ); i++ )
    {
      int idx = map[i].idx;
      Q_ASSERT( idx != -1 );
      Q_ASSERT( idx < n );
      v.insert( i, *values[ idx ] ? QgsSqliteUtils::quotedString( values[idx] ) : QStringLiteral( "NULL" ) );
    }
    CSLDestroy( values );

    //switch sign of rotation parameters. See http://trac.osgeo.org/proj/wiki/GenParms#towgs84-DatumtransformationtoWGS84
    if ( v.at( idxmcode ).compare( QLatin1String( "'9607'" ) ) == 0 )
    {
      v[ idxmcode ] = QStringLiteral( "'9606'" );
      v[ idxrx ] = '\'' + qgsDoubleToString( -( v[ idxrx ].remove( '\'' ).toDouble() ) ) + '\'';
      v[ idxry ] = '\'' + qgsDoubleToString( -( v[ idxry ].remove( '\'' ).toDouble() ) ) + '\'';
      v[ idxrz ] = '\'' + qgsDoubleToString( -( v[ idxrz ].remove( '\'' ).toDouble() ) ) + '\'';
    }

    //entry already in db?
    sqlite3_statement_unique_ptr statement;
    QString cOpCode;
    QString sql = QStringLiteral( "SELECT coord_op_code FROM tbl_datum_transform WHERE coord_op_code=%1" ).arg( v[ idxid ] );
    int prepareRes;
    statement = database.prepare( sql, prepareRes );
    if ( prepareRes != SQLITE_OK )
      continue;

    if ( statement.step() == SQLITE_ROW )
    {
      cOpCode = statement.columnAsText( 0 );
    }

    sql = cOpCode.isEmpty() ? insert : update;
    for ( int i = 0; i < v.size(); i++ )
    {
      sql = sql.arg( v[i] );
    }

    if ( sqlite3_exec( database.get(), sql.toUtf8(), nullptr, nullptr, nullptr ) != SQLITE_OK )
    {
      qCritical( "SQL: %s", sql.toUtf8().constData() );
      qCritical( "Error: %s", sqlite3_errmsg( database.get() ) );
    }
  }

  if ( sqlite3_exec( database.get(), "COMMIT", nullptr, nullptr, nullptr ) != SQLITE_OK )
  {
    QgsDebugMsg( QStringLiteral( "Could not commit transaction: %1 [%2]\n" ).arg( QgsApplication::srsDatabaseFilePath(), sqlite3_errmsg( database.get() ) ) );
    return false;
  }

  return true;
}

QString QgsCoordinateReferenceSystem::geographicCrsAuthId() const
{
  if ( isGeographic() )
  {
    return d->mAuthId;
  }
  else if ( d->mCRS )
  {
    return OSRGetAuthorityName( d->mCRS, "GEOGCS" ) + QStringLiteral( ":" ) + OSRGetAuthorityCode( d->mCRS, "GEOGCS" );
  }
  else
  {
    return QString();
  }
}

QStringList QgsCoordinateReferenceSystem::recentProjections()
{
  QStringList projections;

  // Read settings from persistent storage
  QgsSettings settings;
  projections = settings.value( QStringLiteral( "UI/recentProjections" ) ).toStringList();
  /*** The reading (above) of internal id from persistent storage should be removed sometime in the future */
  /*** This is kept now for backwards compatibility */

  QStringList projectionsProj4  = settings.value( QStringLiteral( "UI/recentProjectionsProj4" ) ).toStringList();
  QStringList projectionsAuthId = settings.value( QStringLiteral( "UI/recentProjectionsAuthId" ) ).toStringList();
  if ( projectionsAuthId.size() >= projections.size() )
  {
    // We had saved state with AuthId and Proj4. Use that instead
    // to find out the crs id
    projections.clear();
    for ( int i = 0; i <  projectionsAuthId.size(); i++ )
    {
      // Create a crs from the EPSG
      QgsCoordinateReferenceSystem crs;
      crs.createFromOgcWmsCrs( projectionsAuthId.at( i ) );
      if ( ! crs.isValid() )
      {
        // Couldn't create from EPSG, try the Proj4 string instead
        if ( i >= projectionsProj4.size() || !crs.createFromProj4( projectionsProj4.at( i ) ) )
        {
          // No? Skip this entry
          continue;
        }
        //If the CRS can be created but do not correspond to a CRS in the database, skip it (for example a deleted custom CRS)
        if ( crs.srsid() == 0 )
        {
          continue;
        }
      }
      projections << QString::number( crs.srsid() );
    }
  }
  return projections;
}

void QgsCoordinateReferenceSystem::invalidateCache()
{
  sSrIdCacheLock.lockForWrite();
  sSrIdCache.clear();
  sSrIdCacheLock.unlock();
  sOgcLock.lockForWrite();
  sOgcCache.clear();
  sOgcLock.unlock();
  sProj4CacheLock.lockForWrite();
  sProj4Cache.clear();
  sProj4CacheLock.unlock();
  sCRSWktLock.lockForWrite();
  sWktCache.clear();
  sCRSWktLock.unlock();
  sCRSSrsIdLock.lockForWrite();
  sSrsIdCache.clear();
  sCRSSrsIdLock.unlock();
  sCrsStringLock.lockForWrite();
  sStringCache.clear();
  sCrsStringLock.unlock();
}
