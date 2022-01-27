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

#include "qgscoordinatereferencesystem_legacy_p.h"
#include "qgscoordinatereferencesystemregistry.h"
#include "qgsreadwritelocker.h"

#include <cmath>

#include <QDir>
#include <QDomNode>
#include <QDomElement>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTextStream>
#include <QFile>

#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgis.h" //const vals declared here
#include "qgslocalec.h"
#include "qgssettings.h"
#include "qgsogrutils.h"
#include "qgsdatums.h"
#include "qgsprojectionfactors.h"
#include "qgsprojoperation.h"

#include <sqlite3.h>
#include "qgsprojutils.h"
#include <proj.h>
#include <proj_experimental.h>

//gdal and ogr includes (needed for == operator)
#include <ogr_srs_api.h>
#include <cpl_error.h>
#include <cpl_conv.h>
#include <cpl_csv.h>

CUSTOM_CRS_VALIDATION QgsCoordinateReferenceSystem::sCustomSrsValidation = nullptr;

typedef QHash< long, QgsCoordinateReferenceSystem > SrIdCrsCacheHash;
typedef QHash< QString, QgsCoordinateReferenceSystem > StringCrsCacheHash;

Q_GLOBAL_STATIC( QReadWriteLock, sSrIdCacheLock )
Q_GLOBAL_STATIC( SrIdCrsCacheHash, sSrIdCache )
bool QgsCoordinateReferenceSystem::sDisableSrIdCache = false;

Q_GLOBAL_STATIC( QReadWriteLock, sOgcLock )
Q_GLOBAL_STATIC( StringCrsCacheHash, sOgcCache )
bool QgsCoordinateReferenceSystem::sDisableOgcCache = false;

Q_GLOBAL_STATIC( QReadWriteLock, sProj4CacheLock )
Q_GLOBAL_STATIC( StringCrsCacheHash, sProj4Cache )
bool QgsCoordinateReferenceSystem::sDisableProjCache = false;

Q_GLOBAL_STATIC( QReadWriteLock, sCRSWktLock )
Q_GLOBAL_STATIC( StringCrsCacheHash, sWktCache )
bool QgsCoordinateReferenceSystem::sDisableWktCache = false;

Q_GLOBAL_STATIC( QReadWriteLock, sCRSSrsIdLock )
Q_GLOBAL_STATIC( SrIdCrsCacheHash, sSrsIdCache )
bool QgsCoordinateReferenceSystem::sDisableSrsIdCache = false;

Q_GLOBAL_STATIC( QReadWriteLock, sCrsStringLock )
Q_GLOBAL_STATIC( StringCrsCacheHash, sStringCache )
bool QgsCoordinateReferenceSystem::sDisableStringCache = false;

QString getFullProjString( PJ *obj )
{
  // see https://lists.osgeo.org/pipermail/proj/2019-May/008565.html, it's not sufficient to just
  // use proj_as_proj_string
  QgsProjUtils::proj_pj_unique_ptr boundCrs( proj_crs_create_bound_crs_to_WGS84( QgsProjContext::get(), obj, nullptr ) );
  if ( boundCrs )
  {
    if ( const char *proj4src = proj_as_proj_string( QgsProjContext::get(), boundCrs.get(), PJ_PROJ_4, nullptr ) )
    {
      return QString( proj4src );
    }
  }

  return QString( proj_as_proj_string( QgsProjContext::get(), obj, PJ_PROJ_4, nullptr ) );
}
//--------------------------

QgsCoordinateReferenceSystem::QgsCoordinateReferenceSystem()
{
  static QgsCoordinateReferenceSystem nullCrs = QgsCoordinateReferenceSystem( QString() );

  d = nullCrs.d;
}

QgsCoordinateReferenceSystem::QgsCoordinateReferenceSystem( const QString &definition )
{
  d = new QgsCoordinateReferenceSystemPrivate();
  createFromString( definition );
}

QgsCoordinateReferenceSystem::QgsCoordinateReferenceSystem( const long id, CrsType type )
{
  d = new QgsCoordinateReferenceSystemPrivate();
  Q_NOWARN_DEPRECATED_PUSH
  createFromId( id, type );
  Q_NOWARN_DEPRECATED_POP
}

QgsCoordinateReferenceSystem::QgsCoordinateReferenceSystem( const QgsCoordinateReferenceSystem &srs )  //NOLINT
  : d( srs.d )
  , mValidationHint( srs.mValidationHint )
  , mNativeFormat( srs.mNativeFormat )
{
}

QgsCoordinateReferenceSystem &QgsCoordinateReferenceSystem::operator=( const QgsCoordinateReferenceSystem &srs )  //NOLINT
{
  d = srs.d;
  mValidationHint = srs.mValidationHint;
  mNativeFormat = srs.mNativeFormat;
  return *this;
}

QList<long> QgsCoordinateReferenceSystem::validSrsIds()
{
  QList<long> results;
  // check both standard & user defined projection databases
  QStringList dbs = QStringList() <<  QgsApplication::srsDatabaseFilePath() << QgsApplication::qgisUserDatabaseFilePath();

  const auto constDbs = dbs;
  for ( const QString &db : constDbs )
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
  QgsCoordinateReferenceSystem res = fromOgcWmsCrs( "EPSG:" + QString::number( epsg ) );
  if ( res.isValid() )
    return res;

  // pre proj6 builds allowed use of ESRI: codes here (e.g. 54030), so we need to keep compatibility
  res = fromOgcWmsCrs( "ESRI:" + QString::number( epsg ) );
  if ( res.isValid() )
    return res;

  return QgsCoordinateReferenceSystem();
}

QgsCoordinateReferenceSystem QgsCoordinateReferenceSystem::fromProj4( const QString &proj4 )
{
  return fromProj( proj4 );
}

QgsCoordinateReferenceSystem QgsCoordinateReferenceSystem::fromProj( const QString &proj )
{
  QgsCoordinateReferenceSystem crs;
  crs.createFromProj( proj );
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
      Q_NOWARN_DEPRECATED_PUSH
      result = createFromSrid( id );
      Q_NOWARN_DEPRECATED_POP
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
  if ( definition.isEmpty() )
    return false;

  QgsReadWriteLocker locker( *sCrsStringLock(), QgsReadWriteLocker::Read );
  if ( !sDisableStringCache )
  {
    QHash< QString, QgsCoordinateReferenceSystem >::const_iterator crsIt = sStringCache()->constFind( definition );
    if ( crsIt != sStringCache()->constEnd() )
    {
      // found a match in the cache
      *this = crsIt.value();
      return d->mIsValid;
    }
  }
  locker.unlock();

  bool result = false;
  const thread_local QRegularExpression reCrsId( QStringLiteral( "^(epsg|esri|osgeo|ignf|zangi|iau2000|postgis|internal|user)\\:(\\w+)$" ), QRegularExpression::CaseInsensitiveOption );
  QRegularExpressionMatch match = reCrsId.match( definition );
  if ( match.capturedStart() == 0 )
  {
    QString authName = match.captured( 1 ).toLower();
    if ( authName == QLatin1String( "epsg" ) )
    {
      result = createFromOgcWmsCrs( definition );
    }
    else if ( authName == QLatin1String( "postgis" ) )
    {
      const long id = match.captured( 2 ).toLong();
      Q_NOWARN_DEPRECATED_PUSH
      result = createFromSrid( id );
      Q_NOWARN_DEPRECATED_POP
    }
    else if ( authName == QLatin1String( "esri" ) || authName == QLatin1String( "osgeo" ) || authName == QLatin1String( "ignf" ) || authName == QLatin1String( "zangi" ) || authName == QLatin1String( "iau2000" ) )
    {
      result = createFromOgcWmsCrs( definition );
    }
    else
    {
      const long id = match.captured( 2 ).toLong();
      Q_NOWARN_DEPRECATED_PUSH
      result = createFromId( id, InternalCrsId );
      Q_NOWARN_DEPRECATED_POP
    }
  }
  else
  {
    const thread_local QRegularExpression reCrsStr( QStringLiteral( "^(?:(wkt|proj4|proj)\\:)?(.+)$" ), QRegularExpression::CaseInsensitiveOption );
    match = reCrsStr.match( definition );
    if ( match.capturedStart() == 0 )
    {
      if ( match.captured( 1 ).startsWith( QLatin1String( "proj" ), Qt::CaseInsensitive ) )
      {
        result = createFromProj( match.captured( 2 ) );
      }
      else
      {
        result = createFromWkt( match.captured( 2 ) );
      }
    }
  }

  locker.changeMode( QgsReadWriteLocker::Write );
  if ( !sDisableStringCache )
    sStringCache()->insert( definition, *this );
  return result;
}

bool QgsCoordinateReferenceSystem::createFromUserInput( const QString &definition )
{
  if ( definition.isEmpty() )
    return false;

  QString userWkt;
  OGRSpatialReferenceH crs = OSRNewSpatialReference( nullptr );

  if ( OSRSetFromUserInput( crs, definition.toLocal8Bit().constData() ) == OGRERR_NONE )
  {
    userWkt = QgsOgrUtils::OGRSpatialReferenceToWkt( crs );
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
  if ( crs.isEmpty() )
    return false;

  QgsReadWriteLocker locker( *sOgcLock(), QgsReadWriteLocker::Read );
  if ( !sDisableOgcCache )
  {
    QHash< QString, QgsCoordinateReferenceSystem >::const_iterator crsIt = sOgcCache()->constFind( crs );
    if ( crsIt != sOgcCache()->constEnd() )
    {
      // found a match in the cache
      *this = crsIt.value();
      return d->mIsValid;
    }
  }
  locker.unlock();

  QString wmsCrs = crs;

  thread_local const QRegularExpression re_uri( QRegularExpression::anchoredPattern( QStringLiteral( "http://www\\.opengis\\.net/def/crs/([^/]+).+/([^/]+)" ) ), QRegularExpression::CaseInsensitiveOption );
  QRegularExpressionMatch match = re_uri.match( wmsCrs );
  if ( match.hasMatch() )
  {
    wmsCrs = match.captured( 1 ) + ':' + match.captured( 2 );
  }
  else
  {
    thread_local const QRegularExpression re_urn( QRegularExpression::anchoredPattern( QStringLiteral( "urn:ogc:def:crs:([^:]+).+(?<=:)([^:]+)" ) ), QRegularExpression::CaseInsensitiveOption );
    match = re_urn.match( wmsCrs );
    if ( match.hasMatch() )
    {
      wmsCrs = match.captured( 1 ) + ':' + match.captured( 2 );
    }
    else
    {
      thread_local const QRegularExpression re_urn_custom( QRegularExpression::anchoredPattern( QStringLiteral( "(user|custom|qgis):(\\d+)" ) ), QRegularExpression::CaseInsensitiveOption );
      match = re_urn_custom.match( wmsCrs );
      if ( match.hasMatch() && createFromSrsId( match.captured( 2 ).toInt() ) )
      {
        locker.changeMode( QgsReadWriteLocker::Write );
        if ( !sDisableOgcCache )
          sOgcCache()->insert( crs, *this );
        return d->mIsValid;
      }
    }
  }

  // first chance for proj 6 - scan through legacy systems and try to use authid directly
  const QString legacyKey = wmsCrs.toLower();
  for ( auto it = sAuthIdToQgisSrsIdMap.constBegin(); it != sAuthIdToQgisSrsIdMap.constEnd(); ++it )
  {
    if ( it.key().compare( legacyKey, Qt::CaseInsensitive ) == 0 )
    {
      const QStringList parts = it.key().split( ':' );
      const QString auth = parts.at( 0 );
      const QString code = parts.at( 1 );
      if ( loadFromAuthCode( auth, code ) )
      {
        locker.changeMode( QgsReadWriteLocker::Write );
        if ( !sDisableOgcCache )
          sOgcCache()->insert( crs, *this );
        return d->mIsValid;
      }
    }
  }

  if ( loadFromDatabase( QgsApplication::srsDatabaseFilePath(), QStringLiteral( "lower(auth_name||':'||auth_id)" ), wmsCrs.toLower() ) )
  {
    locker.changeMode( QgsReadWriteLocker::Write );
    if ( !sDisableOgcCache )
      sOgcCache()->insert( crs, *this );
    return d->mIsValid;
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
    if ( loadFromDatabase( QgsApplication::srsDatabaseFilePath(), QStringLiteral( "lower(auth_name||':'||auth_id)" ), QStringLiteral( "epsg:4326" ) ) )
    {
      d->mAxisInverted = false;
      d->mAxisInvertedDirty = false;
    }

    locker.changeMode( QgsReadWriteLocker::Write );
    if ( !sDisableOgcCache )
      sOgcCache()->insert( crs, *this );

    return d->mIsValid;
  }

  locker.changeMode( QgsReadWriteLocker::Write );
  if ( !sDisableOgcCache )
    sOgcCache()->insert( crs, QgsCoordinateReferenceSystem() );
  return d->mIsValid;
}

// Misc helper functions -----------------------


void QgsCoordinateReferenceSystem::validate()
{
  if ( d->mIsValid || !sCustomSrsValidation )
    return;

  // try to validate using custom validation routines
  if ( sCustomSrsValidation )
    sCustomSrsValidation( *this );
}

bool QgsCoordinateReferenceSystem::createFromSrid( const long id )
{
  QgsReadWriteLocker locker( *sSrIdCacheLock(), QgsReadWriteLocker::Read );
  if ( !sDisableSrIdCache )
  {
    QHash< long, QgsCoordinateReferenceSystem >::const_iterator crsIt = sSrIdCache()->constFind( id );
    if ( crsIt != sSrIdCache()->constEnd() )
    {
      // found a match in the cache
      *this = crsIt.value();
      return d->mIsValid;
    }
  }
  locker.unlock();

  // first chance for proj 6 - scan through legacy systems and try to use authid directly
  for ( auto it = sAuthIdToQgisSrsIdMap.constBegin(); it != sAuthIdToQgisSrsIdMap.constEnd(); ++it )
  {
    if ( it.value().endsWith( QStringLiteral( ",%1" ).arg( id ) ) )
    {
      const QStringList parts = it.key().split( ':' );
      const QString auth = parts.at( 0 );
      const QString code = parts.at( 1 );
      if ( loadFromAuthCode( auth, code ) )
      {
        locker.changeMode( QgsReadWriteLocker::Write );
        if ( !sDisableSrIdCache )
          sSrIdCache()->insert( id, *this );

        return d->mIsValid;
      }
    }
  }

  bool result = loadFromDatabase( QgsApplication::srsDatabaseFilePath(), QStringLiteral( "srid" ), QString::number( id ) );

  locker.changeMode( QgsReadWriteLocker::Write );
  if ( !sDisableSrIdCache )
    sSrIdCache()->insert( id, *this );

  return result;
}

bool QgsCoordinateReferenceSystem::createFromSrsId( const long id )
{
  QgsReadWriteLocker locker( *sCRSSrsIdLock(), QgsReadWriteLocker::Read );
  if ( !sDisableSrsIdCache )
  {
    QHash< long, QgsCoordinateReferenceSystem >::const_iterator crsIt = sSrsIdCache()->constFind( id );
    if ( crsIt != sSrsIdCache()->constEnd() )
    {
      // found a match in the cache
      *this = crsIt.value();
      return d->mIsValid;
    }
  }
  locker.unlock();

  // first chance for proj 6 - scan through legacy systems and try to use authid directly
  for ( auto it = sAuthIdToQgisSrsIdMap.constBegin(); it != sAuthIdToQgisSrsIdMap.constEnd(); ++it )
  {
    if ( it.value().startsWith( QString::number( id ) + ',' ) )
    {
      const QStringList parts = it.key().split( ':' );
      const QString auth = parts.at( 0 );
      const QString code = parts.at( 1 );
      if ( loadFromAuthCode( auth, code ) )
      {
        locker.changeMode( QgsReadWriteLocker::Write );
        if ( !sDisableSrsIdCache )
          sSrsIdCache()->insert( id, *this );
        return d->mIsValid;
      }
    }
  }

  bool result = loadFromDatabase( id < USER_CRS_START_ID ? QgsApplication::srsDatabaseFilePath() :
                                  QgsApplication::qgisUserDatabaseFilePath(),
                                  QStringLiteral( "srs_id" ), QString::number( id ) );

  locker.changeMode( QgsReadWriteLocker::Write );
  if ( !sDisableSrsIdCache )
    sSrsIdCache()->insert( id, *this );
  return result;
}

bool QgsCoordinateReferenceSystem::loadFromDatabase( const QString &db, const QString &expression, const QString &value )
{
  d.detach();

  QgsDebugMsgLevel( "load CRS from " + db + " where " + expression + " is " + value, 3 );
  d->mIsValid = false;
  d->mWktPreferred.clear();

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
                  "ellipsoid_acronym,parameters,srid,auth_name||':'||auth_id,is_geo,wkt "
                  "from tbl_srs where " + expression + '=' + QgsSqliteUtils::quotedString( value ) + " order by deprecated";
  statement = database.prepare( mySql, myResult );
  QString wkt;
  // XXX Need to free memory from the error msg if one is set
  if ( myResult == SQLITE_OK && statement.step() == SQLITE_ROW )
  {
    d->mSrsId = statement.columnAsText( 0 ).toLong();
    d->mDescription = statement.columnAsText( 1 );
    d->mProjectionAcronym = statement.columnAsText( 2 );
    d->mEllipsoidAcronym.clear();
    d->mProj4 = statement.columnAsText( 4 );
    d->mWktPreferred.clear();
    d->mSRID = statement.columnAsText( 5 ).toLong();
    d->mAuthId = statement.columnAsText( 6 );
    d->mIsGeographic = statement.columnAsText( 7 ).toInt() != 0;
    wkt = statement.columnAsText( 8 );
    d->mAxisInvertedDirty = true;

    if ( d->mSrsId >= USER_CRS_START_ID && ( d->mAuthId.isEmpty() || d->mAuthId == QChar( ':' ) ) )
    {
      d->mAuthId = QStringLiteral( "USER:%1" ).arg( d->mSrsId );
    }
    else if ( !d->mAuthId.startsWith( QLatin1String( "USER:" ), Qt::CaseInsensitive ) )
    {
      QStringList parts = d->mAuthId.split( ':' );
      QString auth = parts.at( 0 );
      QString code = parts.at( 1 );

      {
        QgsProjUtils::proj_pj_unique_ptr crs( proj_create_from_database( QgsProjContext::get(), auth.toLatin1(), code.toLatin1(), PJ_CATEGORY_CRS, false, nullptr ) );
        d->setPj( QgsProjUtils::crsToSingleCrs( crs.get() ) );
      }

      d->mIsValid = d->hasPj();
      setMapUnits();
    }

    if ( !d->mIsValid )
    {
      if ( !wkt.isEmpty() )
      {
        setWktString( wkt );
        // set WKT string resets the description to that description embedded in the WKT, so manually overwrite this back to the
        // value from the user DB
        d->mDescription = statement.columnAsText( 1 );
      }
      else
        setProjString( d->mProj4 );
    }
  }
  else
  {
    QgsDebugMsgLevel( "failed : " + mySql, 4 );
  }
  return d->mIsValid;
}

void QgsCoordinateReferenceSystem::removeFromCacheObjectsBelongingToCurrentThread( PJ_CONTEXT *pj_context )
{
  // Not completely sure about object order destruction after main() has
  // exited. So it is safer to check sDisableCache before using sCacheLock
  // in case sCacheLock would have been destroyed before the current TLS
  // QgsProjContext object that has called us...

  if ( !sDisableSrIdCache )
  {
    QgsReadWriteLocker locker( *sSrIdCacheLock(), QgsReadWriteLocker::Write );
    if ( !sDisableSrIdCache )
    {
      for ( auto it = sSrIdCache()->begin(); it != sSrIdCache()->end(); )
      {
        auto &v = it.value();
        if ( v.d->removeObjectsBelongingToCurrentThread( pj_context ) )
          it = sSrIdCache()->erase( it );
        else
          ++it;
      }
    }
  }
  if ( !sDisableOgcCache )
  {
    QgsReadWriteLocker locker( *sOgcLock(), QgsReadWriteLocker::Write );
    if ( !sDisableOgcCache )
    {
      for ( auto it = sOgcCache()->begin(); it != sOgcCache()->end(); )
      {
        auto &v = it.value();
        if ( v.d->removeObjectsBelongingToCurrentThread( pj_context ) )
          it = sOgcCache()->erase( it );
        else
          ++it;
      }
    }
  }
  if ( !sDisableProjCache )
  {
    QgsReadWriteLocker locker( *sProj4CacheLock(), QgsReadWriteLocker::Write );
    if ( !sDisableProjCache )
    {
      for ( auto it = sProj4Cache()->begin(); it != sProj4Cache()->end(); )
      {
        auto &v = it.value();
        if ( v.d->removeObjectsBelongingToCurrentThread( pj_context ) )
          it = sProj4Cache()->erase( it );
        else
          ++it;
      }
    }
  }
  if ( !sDisableWktCache )
  {
    QgsReadWriteLocker locker( *sCRSWktLock(), QgsReadWriteLocker::Write );
    if ( !sDisableWktCache )
    {
      for ( auto it = sWktCache()->begin(); it != sWktCache()->end(); )
      {
        auto &v = it.value();
        if ( v.d->removeObjectsBelongingToCurrentThread( pj_context ) )
          it = sWktCache()->erase( it );
        else
          ++it;
      }
    }
  }
  if ( !sDisableSrsIdCache )
  {
    QgsReadWriteLocker locker( *sCRSSrsIdLock(), QgsReadWriteLocker::Write );
    if ( !sDisableSrsIdCache )
    {
      for ( auto it = sSrsIdCache()->begin(); it != sSrsIdCache()->end(); )
      {
        auto &v = it.value();
        if ( v.d->removeObjectsBelongingToCurrentThread( pj_context ) )
          it = sSrsIdCache()->erase( it );
        else
          ++it;
      }
    }
  }
  if ( !sDisableStringCache )
  {
    QgsReadWriteLocker locker( *sCrsStringLock(), QgsReadWriteLocker::Write );
    if ( !sDisableStringCache )
    {
      for ( auto it = sStringCache()->begin(); it != sStringCache()->end(); )
      {
        auto &v = it.value();
        if ( v.d->removeObjectsBelongingToCurrentThread( pj_context ) )
          it = sStringCache()->erase( it );
        else
          ++it;
      }
    }
  }
}

bool QgsCoordinateReferenceSystem::hasAxisInverted() const
{
  if ( d->mAxisInvertedDirty )
  {
    d->mAxisInverted = QgsProjUtils::axisOrderIsSwapped( d->threadLocalProjObject() );
    d->mAxisInvertedDirty = false;
  }

  return d->mAxisInverted;
}

bool QgsCoordinateReferenceSystem::createFromWkt( const QString &wkt )
{
  return createFromWktInternal( wkt, QString() );
}

bool QgsCoordinateReferenceSystem::createFromWktInternal( const QString &wkt, const QString &description )
{
  if ( wkt.isEmpty() )
    return false;

  d.detach();

  QgsReadWriteLocker locker( *sCRSWktLock(), QgsReadWriteLocker::Read );
  if ( !sDisableWktCache )
  {
    QHash< QString, QgsCoordinateReferenceSystem >::const_iterator crsIt = sWktCache()->constFind( wkt );
    if ( crsIt != sWktCache()->constEnd() )
    {
      // found a match in the cache
      *this = crsIt.value();

      if ( !description.isEmpty() && d->mDescription.isEmpty() )
      {
        // now we have a name for a previously unknown CRS! Update the cached CRS accordingly, so that we use the name from now on...
        d->mDescription = description;
        locker.changeMode( QgsReadWriteLocker::Write );
        sWktCache()->insert( wkt, *this );
      }
      return d->mIsValid;
    }
  }
  locker.unlock();

  d->mIsValid = false;
  d->mProj4.clear();
  d->mWktPreferred.clear();
  if ( wkt.isEmpty() )
  {
    QgsDebugMsgLevel( QStringLiteral( "theWkt is uninitialized, operation failed" ), 4 );
    return d->mIsValid;
  }

  // try to match against user crs
  QgsCoordinateReferenceSystem::RecordMap record = getRecord( "select * from tbl_srs where wkt=" + QgsSqliteUtils::quotedString( wkt ) + " order by deprecated" );
  if ( !record.empty() )
  {
    long srsId = record[QStringLiteral( "srs_id" )].toLong();
    if ( srsId > 0 )
    {
      createFromSrsId( srsId );
    }
  }
  else
  {
    setWktString( wkt );
    if ( !description.isEmpty() )
    {
      d->mDescription = description;
    }
    if ( d->mSrsId == 0 )
    {
      // lastly, try a tolerant match of the created proj object against all user CRSes (allowing differences in parameter order during the comparison)
      long id = matchToUserCrs();
      if ( id >= USER_CRS_START_ID )
      {
        createFromSrsId( id );
      }
    }
  }

  locker.changeMode( QgsReadWriteLocker::Write );
  if ( !sDisableWktCache )
    sWktCache()->insert( wkt, *this );

  return d->mIsValid;
  //setMapunits will be called by createfromproj above
}

bool QgsCoordinateReferenceSystem::isValid() const
{
  return d->mIsValid;
}

bool QgsCoordinateReferenceSystem::createFromProj4( const QString &proj4String )
{
  return createFromProj( proj4String );
}

bool QgsCoordinateReferenceSystem::createFromProj( const QString &projString, const bool identify )
{
  if ( projString.isEmpty() )
    return false;

  d.detach();

  if ( projString.trimmed().isEmpty() )
  {
    d->mIsValid = false;
    d->mProj4.clear();
    d->mWktPreferred.clear();
    return false;
  }

  QgsReadWriteLocker locker( *sProj4CacheLock(), QgsReadWriteLocker::Read );
  if ( !sDisableProjCache )
  {
    QHash< QString, QgsCoordinateReferenceSystem >::const_iterator crsIt = sProj4Cache()->constFind( projString );
    if ( crsIt != sProj4Cache()->constEnd() )
    {
      // found a match in the cache
      *this = crsIt.value();
      return d->mIsValid;
    }
  }
  locker.unlock();

  //
  // Examples:
  // +proj=tmerc +lat_0=0 +lon_0=-62 +k=0.999500 +x_0=400000 +y_0=0
  // +ellps=clrk80 +towgs84=-255,-15,71,0,0,0,0 +units=m +no_defs
  //
  // +proj=lcc +lat_1=46.8 +lat_0=46.8 +lon_0=2.337229166666664 +k_0=0.99987742
  // +x_0=600000 +y_0=2200000 +a=6378249.2 +b=6356515.000000472 +units=m +no_defs
  //
  QString myProj4String = projString.trimmed();
  myProj4String.remove( QStringLiteral( "+type=crs" ) );
  myProj4String = myProj4String.trimmed();

  d->mIsValid = false;
  d->mWktPreferred.clear();

  if ( identify )
  {
    // first, try to use proj to do this for us...
    const QString projCrsString = myProj4String + ( myProj4String.contains( QStringLiteral( "+type=crs" ) ) ? QString() : QStringLiteral( " +type=crs" ) );
    QgsProjUtils::proj_pj_unique_ptr crs( proj_create( QgsProjContext::get(), projCrsString.toLatin1().constData() ) );
    if ( crs )
    {
      QString authName;
      QString authCode;
      if ( QgsProjUtils::identifyCrs( crs.get(), authName, authCode, QgsProjUtils::FlagMatchBoundCrsToUnderlyingSourceCrs ) )
      {
        const QString authid = QStringLiteral( "%1:%2" ).arg( authName, authCode );
        if ( createFromOgcWmsCrs( authid ) )
        {
          locker.changeMode( QgsReadWriteLocker::Write );
          if ( !sDisableProjCache )
            sProj4Cache()->insert( projString, *this );
          return d->mIsValid;
        }
      }
    }

    // try a direct match against user crses
    QgsCoordinateReferenceSystem::RecordMap myRecord = getRecord( "select * from tbl_srs where parameters=" + QgsSqliteUtils::quotedString( myProj4String ) + " order by deprecated" );
    long id = 0;
    if ( !myRecord.empty() )
    {
      id = myRecord[QStringLiteral( "srs_id" )].toLong();
      if ( id >= USER_CRS_START_ID )
      {
        createFromSrsId( id );
      }
    }
    if ( id < USER_CRS_START_ID )
    {
      // no direct matches, so go ahead and create a new proj object based on the proj string alone.
      setProjString( myProj4String );

      // lastly, try a tolerant match of the created proj object against all user CRSes (allowing differences in parameter order during the comparison)
      id = matchToUserCrs();
      if ( id >= USER_CRS_START_ID )
      {
        createFromSrsId( id );
      }
    }
  }
  else
  {
    setProjString( myProj4String );
  }

  locker.changeMode( QgsReadWriteLocker::Write );
  if ( !sDisableProjCache )
    sProj4Cache()->insert( projString, *this );

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
      //be less fussy on proj 6 -- the db has MANY more entries!
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

QString QgsCoordinateReferenceSystem::userFriendlyIdentifier( IdentifierType type ) const
{
  QString id;
  if ( !authid().isEmpty() )
  {
    if ( type != ShortString && !description().isEmpty() )
      id = QStringLiteral( "%1 - %2" ).arg( authid(), description() );
    else
      id = authid();
  }
  else if ( !description().isEmpty() )
    id = description();
  else if ( type == ShortString )
    id = isValid() ? QObject::tr( "Custom CRS" ) : QObject::tr( "Unknown CRS" );
  else if ( !toWkt( WKT_PREFERRED ).isEmpty() )
    id = QObject::tr( "Custom CRS: %1" ).arg(
           type == MediumString ? ( toWkt( WKT_PREFERRED ).left( 50 ) + QString( QChar( 0x2026 ) ) )
           : toWkt( WKT_PREFERRED ) );
  else if ( !toProj().isEmpty() )
    id = QObject::tr( "Custom CRS: %1" ).arg( type == MediumString ? ( toProj().left( 50 ) + QString( QChar( 0x2026 ) ) )
         : toProj() );
  if ( !id.isEmpty() && !std::isnan( d->mCoordinateEpoch ) )
    id += QStringLiteral( " @ %1" ).arg( d->mCoordinateEpoch );

  return id;
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
    if ( PJ *obj = d->threadLocalProjObject() )
    {
      QgsProjUtils::proj_pj_unique_ptr ellipsoid( proj_get_ellipsoid( QgsProjContext::get(), obj ) );
      if ( ellipsoid )
      {
        const QString ellipsoidAuthName( proj_get_id_auth_name( ellipsoid.get(), 0 ) );
        const QString ellipsoidAuthCode( proj_get_id_code( ellipsoid.get(), 0 ) );
        if ( !ellipsoidAuthName.isEmpty() && !ellipsoidAuthCode.isEmpty() )
          d->mEllipsoidAcronym = QStringLiteral( "%1:%2" ).arg( ellipsoidAuthName, ellipsoidAuthCode );
        else
        {
          double semiMajor, semiMinor, invFlattening;
          int semiMinorComputed = 0;
          if ( proj_ellipsoid_get_parameters( QgsProjContext::get(), ellipsoid.get(), &semiMajor, &semiMinor, &semiMinorComputed, &invFlattening ) )
          {
            d->mEllipsoidAcronym = QStringLiteral( "PARAMETER:%1:%2" ).arg( qgsDoubleToString( semiMajor ),
                                   qgsDoubleToString( semiMinor ) );
          }
          else
          {
            d->mEllipsoidAcronym.clear();
          }
        }
      }
    }
    return d->mEllipsoidAcronym;
  }
  else
  {
    return d->mEllipsoidAcronym;
  }
}

QString QgsCoordinateReferenceSystem::toProj4() const
{
  return toProj();
}

QString QgsCoordinateReferenceSystem::toProj() const
{
  if ( !d->mIsValid )
    return QString();

  if ( d->mProj4.isEmpty() )
  {
    if ( PJ *obj = d->threadLocalProjObject() )
    {
      d->mProj4 = getFullProjString( obj );
    }
  }
  // Stray spaces at the end?
  return d->mProj4.trimmed();
}

bool QgsCoordinateReferenceSystem::isGeographic() const
{
  return d->mIsGeographic;
}

bool QgsCoordinateReferenceSystem::isDynamic() const
{
  const PJ *pj = projObject();
  if ( !pj )
    return false;

  return QgsProjUtils::isDynamic( pj );
}

QString QgsCoordinateReferenceSystem::celestialBodyName() const
{
  const PJ *pj = projObject();
  if ( !pj )
    return QString();

#if PROJ_VERSION_MAJOR>8 || (PROJ_VERSION_MAJOR==8 && PROJ_VERSION_MINOR>=1)
  PJ_CONTEXT *context = QgsProjContext::get();

  return QString( proj_get_celestial_body_name( context, pj ) );
#else
  throw QgsNotSupportedException( QStringLiteral( "Retrieving celestial body requires a QGIS build based on PROJ 8.1 or later" ) );
#endif
}

void QgsCoordinateReferenceSystem::setCoordinateEpoch( double epoch )
{
  if ( d->mCoordinateEpoch == epoch )
    return;

  // detaching clears the proj object, so we need to clone the existing one first
  QgsProjUtils::proj_pj_unique_ptr clone( proj_clone( QgsProjContext::get(), projObject() ) );
  d.detach();
  d->mCoordinateEpoch = epoch;
  d->setPj( std::move( clone ) );
}

double QgsCoordinateReferenceSystem::coordinateEpoch() const
{
  return d->mCoordinateEpoch;
}

QgsDatumEnsemble QgsCoordinateReferenceSystem::datumEnsemble() const
{
  QgsDatumEnsemble res;
  res.mValid = false;

  const PJ *pj = projObject();
  if ( !pj )
    return res;

#if PROJ_VERSION_MAJOR>=8
  PJ_CONTEXT *context = QgsProjContext::get();

  QgsProjUtils::proj_pj_unique_ptr ensemble = QgsProjUtils::crsToDatumEnsemble( pj );
  if ( !ensemble )
    return res;

  res.mValid = true;
  res.mName = QString( proj_get_name( ensemble.get() ) );
  res.mAuthority = QString( proj_get_id_auth_name( ensemble.get(), 0 ) );
  res.mCode = QString( proj_get_id_code( ensemble.get(), 0 ) );
  res.mRemarks = QString( proj_get_remarks( ensemble.get() ) );
  res.mScope = QString( proj_get_scope( ensemble.get() ) );
  res.mAccuracy = proj_datum_ensemble_get_accuracy( context, ensemble.get() );

  const int memberCount = proj_datum_ensemble_get_member_count( context, ensemble.get() );
  for ( int i = 0; i < memberCount; ++i )
  {
    QgsProjUtils::proj_pj_unique_ptr member( proj_datum_ensemble_get_member( context, ensemble.get(), i ) );
    if ( !member )
      continue;

    QgsDatumEnsembleMember details;
    details.mName = QString( proj_get_name( member.get() ) );
    details.mAuthority = QString( proj_get_id_auth_name( member.get(), 0 ) );
    details.mCode = QString( proj_get_id_code( member.get(), 0 ) );
    details.mRemarks = QString( proj_get_remarks( member.get() ) );
    details.mScope = QString( proj_get_scope( member.get() ) );

    res.mMembers << details;
  }
  return res;
#else
  throw QgsNotSupportedException( QStringLiteral( "Calculating datum ensembles requires a QGIS build based on PROJ 8.0 or later" ) );
#endif
}

QgsProjectionFactors QgsCoordinateReferenceSystem::factors( const QgsPoint &point ) const
{
  QgsProjectionFactors res;

  // we have to make a transformation object corresponding to the crs
  QString projString = toProj();
  projString.replace( QLatin1String( "+type=crs" ), QString() );

  QgsProjUtils::proj_pj_unique_ptr transformation( proj_create( QgsProjContext::get(), projString.toUtf8().constData() ) );
  if ( !transformation )
    return res;

  PJ_COORD coord = proj_coord( 0, 0, 0, HUGE_VAL );
  coord.uv.u = point.x() * M_PI / 180.0;
  coord.uv.v = point.y() * M_PI / 180.0;

  proj_errno_reset( transformation.get() );
  const PJ_FACTORS pjFactors = proj_factors( transformation.get(), coord );
  if ( proj_errno( transformation.get() ) )
  {
    return res;
  }

  res.mIsValid = true;
  res.mMeridionalScale = pjFactors.meridional_scale;
  res.mParallelScale = pjFactors.parallel_scale;
  res.mArealScale = pjFactors.areal_scale;
  res.mAngularDistortion = pjFactors.angular_distortion;
  res.mMeridianParallelAngle = pjFactors.meridian_parallel_angle * 180 / M_PI;
  res.mMeridianConvergence = pjFactors.meridian_convergence * 180 / M_PI;
  res.mTissotSemimajor = pjFactors.tissot_semimajor;
  res.mTissotSemiminor = pjFactors.tissot_semiminor;
  res.mDxDlam = pjFactors.dx_dlam;
  res.mDxDphi = pjFactors.dx_dphi;
  res.mDyDlam = pjFactors.dy_dlam;
  res.mDyDphi = pjFactors.dy_dphi;
  return res;
}

QgsProjOperation QgsCoordinateReferenceSystem::operation() const
{
  QgsProjOperation res;

  // we have to make a transformation object corresponding to the crs
  QString projString = toProj();
  projString.replace( QLatin1String( "+type=crs" ), QString() );

  QgsProjUtils::proj_pj_unique_ptr transformation( proj_create( QgsProjContext::get(), projString.toUtf8().constData() ) );
  if ( !transformation )
    return res;

  PJ_PROJ_INFO info = proj_pj_info( transformation.get() );

  if ( info.id )
  {
    return QgsApplication::coordinateReferenceSystemRegistry()->projOperations().value( QString( info.id ) );
  }

  return res;
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

  PJ *obj = d->threadLocalProjObject();
  if ( !obj )
    return QgsRectangle();

  double westLon = 0;
  double southLat = 0;
  double eastLon = 0;
  double northLat = 0;

  if ( !proj_get_area_of_use( QgsProjContext::get(), obj,
                              &westLon, &southLat, &eastLon, &northLat, nullptr ) )
    return QgsRectangle();


  // don't use the constructor which normalizes!
  QgsRectangle rect;
  rect.setXMinimum( westLon );
  rect.setYMinimum( southLat );
  rect.setXMaximum( eastLon );
  rect.setYMaximum( northLat );
  return rect;
}

void QgsCoordinateReferenceSystem::updateDefinition()
{
  if ( !d->mIsValid )
    return;

  if ( d->mSrsId >= USER_CRS_START_ID )
  {
    // user CRS, so update to new definition
    createFromSrsId( d->mSrsId );
  }
  else
  {
    // nothing to do -- only user CRS definitions can be changed
  }
}

void QgsCoordinateReferenceSystem::setProjString( const QString &proj4String )
{
  d.detach();
  d->mProj4 = proj4String;
  d->mWktPreferred.clear();

  QgsLocaleNumC l;
  QString trimmed = proj4String.trimmed();

  trimmed += QLatin1String( " +type=crs" );
  PJ_CONTEXT *ctx = QgsProjContext::get();

  {
    d->setPj( QgsProjUtils::proj_pj_unique_ptr( proj_create( ctx, trimmed.toLatin1().constData() ) ) );
  }

  if ( !d->hasPj() )
  {
#ifdef QGISDEBUG
    const int errNo = proj_context_errno( ctx );
    QgsDebugMsg( QStringLiteral( "proj string rejected: %1" ).arg( proj_errno_string( errNo ) ) );
#endif
    d->mIsValid = false;
  }
  else
  {
    d->mEllipsoidAcronym.clear();
    d->mIsValid = true;
  }

  setMapUnits();
}

bool QgsCoordinateReferenceSystem::setWktString( const QString &wkt )
{
  bool res = false;
  d->mIsValid = false;
  d->mWktPreferred.clear();

  PROJ_STRING_LIST warnings = nullptr;
  PROJ_STRING_LIST grammerErrors = nullptr;
  {
    d->setPj( QgsProjUtils::proj_pj_unique_ptr( proj_create_from_wkt( QgsProjContext::get(), wkt.toLatin1().constData(), nullptr, &warnings, &grammerErrors ) ) );
  }

  res = d->hasPj();
  if ( !res )
  {
    QgsDebugMsg( QStringLiteral( "\n---------------------------------------------------------------" ) );
    QgsDebugMsg( QStringLiteral( "This CRS could *** NOT *** be set from the supplied Wkt " ) );
    QgsDebugMsg( "INPUT: " + wkt );
    for ( auto iter = warnings; iter && *iter; ++iter )
      QgsDebugMsg( *iter );
    for ( auto iter = grammerErrors; iter && *iter; ++iter )
      QgsDebugMsg( *iter );
    QgsDebugMsg( QStringLiteral( "---------------------------------------------------------------\n" ) );
  }
  proj_string_list_destroy( warnings );
  proj_string_list_destroy( grammerErrors );

  QgsReadWriteLocker locker( *sProj4CacheLock(), QgsReadWriteLocker::Unlocked );
  if ( !res )
  {
    locker.changeMode( QgsReadWriteLocker::Write );
    if ( !sDisableWktCache )
      sWktCache()->insert( wkt, *this );
    return d->mIsValid;
  }

  if ( d->hasPj() )
  {
    // try 1 - maybe we can directly grab the auth name and code from the crs already?
    QString authName( proj_get_id_auth_name( d->threadLocalProjObject(), 0 ) );
    QString authCode( proj_get_id_code( d->threadLocalProjObject(), 0 ) );

    if ( authName.isEmpty() || authCode.isEmpty() )
    {
      // try 2, use proj's identify method and see if there's a nice candidate we can use
      QgsProjUtils::identifyCrs( d->threadLocalProjObject(), authName, authCode );
    }

    if ( !authName.isEmpty() && !authCode.isEmpty() )
    {
      if ( loadFromAuthCode( authName, authCode ) )
      {
        locker.changeMode( QgsReadWriteLocker::Write );
        if ( !sDisableWktCache )
          sWktCache()->insert( wkt, *this );
        return d->mIsValid;
      }
    }
    else
    {
      // Still a valid CRS, just not a known one
      d->mIsValid = true;
      d->mDescription = QString( proj_get_name( d->threadLocalProjObject() ) );
    }
    setMapUnits();
  }

  return d->mIsValid;
}

void QgsCoordinateReferenceSystem::setMapUnits()
{
  if ( !d->mIsValid )
  {
    d->mMapUnits = QgsUnitTypes::DistanceUnknownUnit;
    return;
  }

  if ( !d->hasPj() )
  {
    d->mMapUnits = QgsUnitTypes::DistanceUnknownUnit;
    return;
  }

  PJ_CONTEXT *context = QgsProjContext::get();
  QgsProjUtils::proj_pj_unique_ptr crs( QgsProjUtils::crsToSingleCrs( d->threadLocalProjObject() ) );
  QgsProjUtils::proj_pj_unique_ptr coordinateSystem( proj_crs_get_coordinate_system( context, crs.get() ) );
  if ( !coordinateSystem )
  {
    d->mMapUnits = QgsUnitTypes::DistanceUnknownUnit;
    return;
  }

  const int axisCount = proj_cs_get_axis_count( context, coordinateSystem.get() );
  if ( axisCount > 0 )
  {
    const char *outUnitName = nullptr;
    // Read only first axis
    proj_cs_get_axis_info( context, coordinateSystem.get(), 0,
                           nullptr,
                           nullptr,
                           nullptr,
                           nullptr,
                           &outUnitName,
                           nullptr,
                           nullptr );

    const QString unitName( outUnitName );

    // proj unit names are freeform -- they differ from authority to authority :(
    // see https://lists.osgeo.org/pipermail/proj/2019-April/008444.html
    if ( unitName.compare( QLatin1String( "degree" ), Qt::CaseInsensitive ) == 0 ||
         unitName.compare( QLatin1String( "degree minute second" ), Qt::CaseInsensitive ) == 0 ||
         unitName.compare( QLatin1String( "degree minute second hemisphere" ), Qt::CaseInsensitive ) == 0 ||
         unitName.compare( QLatin1String( "degree minute" ), Qt::CaseInsensitive ) == 0 ||
         unitName.compare( QLatin1String( "degree hemisphere" ), Qt::CaseInsensitive ) == 0 ||
         unitName.compare( QLatin1String( "degree minute hemisphere" ), Qt::CaseInsensitive ) == 0 ||
         unitName.compare( QLatin1String( "hemisphere degree" ), Qt::CaseInsensitive ) == 0 ||
         unitName.compare( QLatin1String( "hemisphere degree minute" ), Qt::CaseInsensitive ) == 0 ||
         unitName.compare( QLatin1String( "hemisphere degree minute second" ), Qt::CaseInsensitive ) == 0 ||
         unitName.compare( QLatin1String( "degree (supplier to define representation)" ), Qt::CaseInsensitive ) == 0 )
      d->mMapUnits = QgsUnitTypes::DistanceDegrees;
    else if ( unitName.compare( QLatin1String( "metre" ), Qt::CaseInsensitive ) == 0
              || unitName.compare( QLatin1String( "m" ), Qt::CaseInsensitive ) == 0
              || unitName.compare( QLatin1String( "meter" ), Qt::CaseInsensitive ) == 0 )
      d->mMapUnits = QgsUnitTypes::DistanceMeters;
    // we don't differentiate between these, suck it imperial users!
    else if ( unitName.compare( QLatin1String( "US survey foot" ), Qt::CaseInsensitive ) == 0 ||
              unitName.compare( QLatin1String( "foot" ), Qt::CaseInsensitive ) == 0 )
      d->mMapUnits = QgsUnitTypes::DistanceFeet;
    else if ( unitName.compare( QLatin1String( "kilometre" ), Qt::CaseInsensitive ) == 0 )  //#spellok
      d->mMapUnits = QgsUnitTypes::DistanceKilometers;
    else if ( unitName.compare( QLatin1String( "centimetre" ), Qt::CaseInsensitive ) == 0 )  //#spellok
      d->mMapUnits = QgsUnitTypes::DistanceCentimeters;
    else if ( unitName.compare( QLatin1String( "millimetre" ), Qt::CaseInsensitive ) == 0 )  //#spellok
      d->mMapUnits = QgsUnitTypes::DistanceMillimeters;
    else if ( unitName.compare( QLatin1String( "Statute mile" ), Qt::CaseInsensitive ) == 0 )
      d->mMapUnits = QgsUnitTypes::DistanceMiles;
    else if ( unitName.compare( QLatin1String( "nautical mile" ), Qt::CaseInsensitive ) == 0 )
      d->mMapUnits = QgsUnitTypes::DistanceNauticalMiles;
    else if ( unitName.compare( QLatin1String( "yard" ), Qt::CaseInsensitive ) == 0 )
      d->mMapUnits = QgsUnitTypes::DistanceYards;
    // TODO - maybe more values to handle here?
    else
      d->mMapUnits = QgsUnitTypes::DistanceUnknownUnit;
    return;
  }
  else
  {
    d->mMapUnits = QgsUnitTypes::DistanceUnknownUnit;
    return;
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
      if ( toProj() == myProj4String.trimmed() )
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
      if ( toProj() == myProj4String.trimmed() )
      {
        return mySrsId.toLong();
      }
    }
  }

  return 0;
}

bool QgsCoordinateReferenceSystem::operator==( const QgsCoordinateReferenceSystem &srs ) const
{
  // shortcut
  if ( d == srs.d )
    return true;

  if ( !d->mIsValid && !srs.d->mIsValid )
    return true;

  if ( !d->mIsValid || !srs.d->mIsValid )
    return false;

  if ( !qgsNanCompatibleEquals( d->mCoordinateEpoch, srs.d->mCoordinateEpoch ) )
    return false;

  const bool isUser = d->mSrsId >= USER_CRS_START_ID;
  const bool otherIsUser = srs.d->mSrsId >= USER_CRS_START_ID;
  if ( isUser != otherIsUser )
    return false;

  // we can't directly compare authid for user crses -- the actual definition of these may have changed
  if ( !isUser && ( !d->mAuthId.isEmpty() || !srs.d->mAuthId.isEmpty() ) )
    return d->mAuthId == srs.d->mAuthId;

  return toWkt( WKT_PREFERRED ) == srs.toWkt( WKT_PREFERRED );
}

bool QgsCoordinateReferenceSystem::operator!=( const QgsCoordinateReferenceSystem &srs ) const
{
  return  !( *this == srs );
}

QString QgsCoordinateReferenceSystem::toWkt( WktVariant variant, bool multiline, int indentationWidth ) const
{
  if ( PJ *obj = d->threadLocalProjObject() )
  {
    const bool isDefaultPreferredFormat = variant == WKT_PREFERRED && !multiline;
    if ( isDefaultPreferredFormat && !d->mWktPreferred.isEmpty() )
    {
      // can use cached value
      return d->mWktPreferred;
    }

    PJ_WKT_TYPE type = PJ_WKT1_GDAL;
    switch ( variant )
    {
      case WKT1_GDAL:
        type = PJ_WKT1_GDAL;
        break;
      case WKT1_ESRI:
        type = PJ_WKT1_ESRI;
        break;
      case WKT2_2015:
        type = PJ_WKT2_2015;
        break;
      case WKT2_2015_SIMPLIFIED:
        type = PJ_WKT2_2015_SIMPLIFIED;
        break;
      case WKT2_2019:
        type = PJ_WKT2_2019;
        break;
      case WKT2_2019_SIMPLIFIED:
        type = PJ_WKT2_2019_SIMPLIFIED;
        break;
    }

    const QByteArray multiLineOption = QStringLiteral( "MULTILINE=%1" ).arg( multiline ? QStringLiteral( "YES" ) : QStringLiteral( "NO" ) ).toLocal8Bit();
    const QByteArray indentatationWidthOption = QStringLiteral( "INDENTATION_WIDTH=%1" ).arg( multiline ? QString::number( indentationWidth ) : QStringLiteral( "0" ) ).toLocal8Bit();
    const char *const options[] = {multiLineOption.constData(), indentatationWidthOption.constData(), nullptr};
    QString res = proj_as_wkt( QgsProjContext::get(), obj, type, options );

    if ( isDefaultPreferredFormat )
    {
      // cache result for later use
      d->mWktPreferred = res;
    }

    return res;
  }
  return QString();
}

bool QgsCoordinateReferenceSystem::readXml( const QDomNode &node )
{
  d.detach();
  bool result = true;
  QDomNode srsNode = node.namedItem( QStringLiteral( "spatialrefsys" ) );

  if ( ! srsNode.isNull() )
  {
    bool initialized = false;

    bool ok = false;
    long srsid = srsNode.namedItem( QStringLiteral( "srsid" ) ).toElement().text().toLong( &ok );

    QDomNode node;

    if ( ok && srsid > 0 && srsid < USER_CRS_START_ID )
    {
      node = srsNode.namedItem( QStringLiteral( "authid" ) );
      if ( !node.isNull() )
      {
        createFromOgcWmsCrs( node.toElement().text() );
        if ( isValid() )
        {
          initialized = true;
        }
      }

      if ( !initialized )
      {
        node = srsNode.namedItem( QStringLiteral( "epsg" ) );
        if ( !node.isNull() )
        {
          operator=( QgsCoordinateReferenceSystem::fromEpsgId( node.toElement().text().toLong() ) );
          if ( isValid() )
          {
            initialized = true;
          }
        }
      }
    }

    // if wkt is present, prefer that since it's lossless (unlike proj4 strings)
    if ( !initialized )
    {
      // before doing anything, we grab and set the stored CRS name (description).
      // this way if the stored CRS doesn't match anything available locally (i.e. from Proj's db
      // or the user's custom CRS list), then we will correctly show the CRS with its original
      // name (instead of just "custom crs")
      const QString description = srsNode.namedItem( QStringLiteral( "description" ) ).toElement().text();

      const QString wkt = srsNode.namedItem( QStringLiteral( "wkt" ) ).toElement().text();
      initialized = createFromWktInternal( wkt, description );
    }

    if ( !initialized )
    {
      node = srsNode.namedItem( QStringLiteral( "proj4" ) );
      const QString proj4 = node.toElement().text();
      initialized = createFromProj( proj4 );
    }

    if ( !initialized )
    {
      // Setting from elements one by one
      node = srsNode.namedItem( QStringLiteral( "proj4" ) );
      const QString proj4 = node.toElement().text();
      if ( !proj4.trimmed().isEmpty() )
        setProjString( node.toElement().text() );

      node = srsNode.namedItem( QStringLiteral( "srsid" ) );
      d->mSrsId = node.toElement().text().toLong();

      node = srsNode.namedItem( QStringLiteral( "srid" ) );
      d->mSRID = node.toElement().text().toLong();

      node = srsNode.namedItem( QStringLiteral( "authid" ) );
      d->mAuthId = node.toElement().text();

      node = srsNode.namedItem( QStringLiteral( "description" ) );
      d->mDescription = node.toElement().text();

      node = srsNode.namedItem( QStringLiteral( "projectionacronym" ) );
      d->mProjectionAcronym = node.toElement().text();

      node = srsNode.namedItem( QStringLiteral( "ellipsoidacronym" ) );
      d->mEllipsoidAcronym = node.toElement().text();

      node = srsNode.namedItem( QStringLiteral( "geographicflag" ) );
      d->mIsGeographic = node.toElement().text() == QLatin1String( "true" );

      d->mWktPreferred.clear();

      //make sure the map units have been set
      setMapUnits();
    }

    const QString epoch = srsNode.toElement().attribute( QStringLiteral( "coordinateEpoch" ) );
    if ( !epoch.isEmpty() )
    {
      bool epochOk = false;
      d->mCoordinateEpoch = epoch.toDouble( &epochOk );
      if ( !epochOk )
        d->mCoordinateEpoch = std::numeric_limits< double >::quiet_NaN();
    }
    else
    {
      d->mCoordinateEpoch = std::numeric_limits< double >::quiet_NaN();
    }

    mNativeFormat = qgsEnumKeyToValue<Qgis::CrsDefinitionFormat>( srsNode.toElement().attribute( QStringLiteral( "nativeFormat" ) ), Qgis::CrsDefinitionFormat::Wkt );
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
  QDomElement layerNode = node.toElement();
  QDomElement srsElement = doc.createElement( QStringLiteral( "spatialrefsys" ) );

  srsElement.setAttribute( QStringLiteral( "nativeFormat" ), qgsEnumValueToKey<Qgis::CrsDefinitionFormat>( mNativeFormat ) );

  if ( std::isfinite( d->mCoordinateEpoch ) )
  {
    srsElement.setAttribute( QStringLiteral( "coordinateEpoch" ), d->mCoordinateEpoch );
  }

  QDomElement wktElement = doc.createElement( QStringLiteral( "wkt" ) );
  wktElement.appendChild( doc.createTextNode( toWkt( WKT_PREFERRED ) ) );
  srsElement.appendChild( wktElement );

  QDomElement proj4Element = doc.createElement( QStringLiteral( "proj4" ) );
  proj4Element.appendChild( doc.createTextNode( toProj() ) );
  srsElement.appendChild( proj4Element );

  QDomElement srsIdElement = doc.createElement( QStringLiteral( "srsid" ) );
  srsIdElement.appendChild( doc.createTextNode( QString::number( srsid() ) ) );
  srsElement.appendChild( srsIdElement );

  QDomElement sridElement = doc.createElement( QStringLiteral( "srid" ) );
  sridElement.appendChild( doc.createTextNode( QString::number( postgisSrid() ) ) );
  srsElement.appendChild( sridElement );

  QDomElement authidElement = doc.createElement( QStringLiteral( "authid" ) );
  authidElement.appendChild( doc.createTextNode( authid() ) );
  srsElement.appendChild( authidElement );

  QDomElement descriptionElement = doc.createElement( QStringLiteral( "description" ) );
  descriptionElement.appendChild( doc.createTextNode( description() ) );
  srsElement.appendChild( descriptionElement );

  QDomElement projectionAcronymElement = doc.createElement( QStringLiteral( "projectionacronym" ) );
  projectionAcronymElement.appendChild( doc.createTextNode( projectionAcronym() ) );
  srsElement.appendChild( projectionAcronymElement );

  QDomElement ellipsoidAcronymElement = doc.createElement( QStringLiteral( "ellipsoidacronym" ) );
  ellipsoidAcronymElement.appendChild( doc.createTextNode( ellipsoidAcronym() ) );
  srsElement.appendChild( ellipsoidAcronymElement );

  QDomElement geographicFlagElement = doc.createElement( QStringLiteral( "geographicflag" ) );
  QString geoFlagText = QStringLiteral( "false" );
  if ( isGeographic() )
  {
    geoFlagText = QStringLiteral( "true" );
  }

  geographicFlagElement.appendChild( doc.createTextNode( geoFlagText ) );
  srsElement.appendChild( geographicFlagElement );

  layerNode.appendChild( srsElement );

  return true;
}

//
// Static helper methods below this point only please!
//


// Returns the whole proj4 string for the selected srsid
//this is a static method! NOTE I've made it private for now to reduce API clutter TS
QString QgsCoordinateReferenceSystem::projFromSrsId( const int srsId )
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
  sCustomSrsValidation = f;
}

CUSTOM_CRS_VALIDATION QgsCoordinateReferenceSystem::customCrsValidation()
{
  return sCustomSrsValidation;
}

void QgsCoordinateReferenceSystem::debugPrint()
{
  QgsDebugMsg( QStringLiteral( "***SpatialRefSystem***" ) );
  QgsDebugMsg( "* Valid : " + ( d->mIsValid ? QString( "true" ) : QString( "false" ) ) );
  QgsDebugMsg( "* SrsId : " + QString::number( d->mSrsId ) );
  QgsDebugMsg( "* Proj4 : " + toProj() );
  QgsDebugMsg( "* WKT   : " + toWkt( WKT_PREFERRED ) );
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
  mValidationHint = html;
}

QString QgsCoordinateReferenceSystem::validationHint()
{
  return mValidationHint;
}

long QgsCoordinateReferenceSystem::saveAsUserCrs( const QString &name, Qgis::CrsDefinitionFormat nativeFormat )
{
  return QgsApplication::coordinateReferenceSystemRegistry()->addUserCrs( *this, name, nativeFormat );
}

void QgsCoordinateReferenceSystem::setNativeFormat( Qgis::CrsDefinitionFormat format )
{
  mNativeFormat = format;
}

Qgis::CrsDefinitionFormat QgsCoordinateReferenceSystem::nativeFormat() const
{
  return mNativeFormat;
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

bool testIsGeographic( PJ *crs )
{
  PJ_CONTEXT *pjContext = QgsProjContext::get();
  bool isGeographic = false;
  QgsProjUtils::proj_pj_unique_ptr coordinateSystem( proj_crs_get_coordinate_system( pjContext, crs ) );
  if ( coordinateSystem )
  {
    const int axisCount = proj_cs_get_axis_count( pjContext, coordinateSystem.get() );
    if ( axisCount > 0 )
    {
      const char *outUnitAuthName = nullptr;
      const char *outUnitAuthCode = nullptr;
      // Read only first axis
      proj_cs_get_axis_info( pjContext, coordinateSystem.get(), 0,
                             nullptr,
                             nullptr,
                             nullptr,
                             nullptr,
                             nullptr,
                             &outUnitAuthName,
                             &outUnitAuthCode );

      if ( outUnitAuthName && outUnitAuthCode )
      {
        const char *unitCategory = nullptr;
        if ( proj_uom_get_info_from_database( pjContext, outUnitAuthName, outUnitAuthCode, nullptr, nullptr, &unitCategory ) )
        {
          isGeographic = QString( unitCategory ).compare( QLatin1String( "angular" ), Qt::CaseInsensitive ) == 0;
        }
      }
    }
  }
  return isGeographic;
}

void getOperationAndEllipsoidFromProjString( const QString &proj, QString &operation, QString &ellipsoid )
{
  thread_local const QRegularExpression projRegExp( QStringLiteral( "\\+proj=(\\S+)" ) );
  const QRegularExpressionMatch projMatch = projRegExp.match( proj );
  if ( !projMatch.hasMatch() )
  {
    QgsDebugMsgLevel( QStringLiteral( "no +proj argument found [%2]" ).arg( proj ), 2 );
    return;
  }
  operation = projMatch.captured( 1 );

  const QRegularExpressionMatch ellipseMatch = projRegExp.match( proj );
  if ( ellipseMatch.hasMatch() )
  {
    ellipsoid = ellipseMatch.captured( 1 );
  }
  else
  {
    // satisfy not null constraint on ellipsoid_acronym field
    // possibly we should drop the constraint, yet the crses with missing ellipsoid_acronym are malformed
    // and will result in oddities within other areas of QGIS (e.g. project ellipsoid won't be correctly
    // set for these CRSes). Better just hack around and make the constraint happy for now,
    // and hope that the definitions get corrected in future.
    ellipsoid = "";
  }
}


bool QgsCoordinateReferenceSystem::loadFromAuthCode( const QString &auth, const QString &code )
{
  d.detach();
  d->mIsValid = false;
  d->mWktPreferred.clear();

  PJ_CONTEXT *pjContext = QgsProjContext::get();
  QgsProjUtils::proj_pj_unique_ptr crs( proj_create_from_database( pjContext, auth.toUtf8().constData(), code.toUtf8().constData(), PJ_CATEGORY_CRS, false, nullptr ) );
  if ( !crs )
  {
    return false;
  }

  switch ( proj_get_type( crs.get() ) )
  {
    case PJ_TYPE_VERTICAL_CRS:
      return false;

    default:
      break;
  }

  crs = QgsProjUtils::crsToSingleCrs( crs.get() );

  QString proj4 = getFullProjString( crs.get() );
  proj4.replace( QLatin1String( "+type=crs" ), QString() );
  proj4 = proj4.trimmed();

  d->mIsValid = true;
  d->mProj4 = proj4;
  d->mWktPreferred.clear();
  d->mDescription = QString( proj_get_name( crs.get() ) );
  d->mAuthId = QStringLiteral( "%1:%2" ).arg( auth, code );
  d->mIsGeographic = testIsGeographic( crs.get() );
  d->mAxisInvertedDirty = true;
  QString operation;
  QString ellipsoid;
  getOperationAndEllipsoidFromProjString( proj4, operation, ellipsoid );
  d->mProjectionAcronym = operation;
  d->mEllipsoidAcronym.clear();
  d->setPj( std::move( crs ) );

  const QString dbVals = sAuthIdToQgisSrsIdMap.value( QStringLiteral( "%1:%2" ).arg( auth, code ).toUpper() );
  if ( !dbVals.isEmpty() )
  {
    const QStringList parts = dbVals.split( ',' );
    d->mSrsId = parts.at( 0 ).toInt();
    d->mSRID = parts.at( 1 ).toInt();
  }

  setMapUnits();

  return true;
}

QList<long> QgsCoordinateReferenceSystem::userSrsIds()
{
  QList<long> results;
  // check user defined projection database
  const QString db = QgsApplication::qgisUserDatabaseFilePath();

  QFileInfo myInfo( db );
  if ( !myInfo.exists() )
  {
    QgsDebugMsg( "failed : " + db + " does not exist!" );
    return results;
  }

  sqlite3_database_unique_ptr database;
  sqlite3_statement_unique_ptr statement;

  //check the db is available
  int result = openDatabase( db, database );
  if ( result != SQLITE_OK )
  {
    QgsDebugMsg( "failed : " + db + " could not be opened!" );
    return results;
  }

  QString sql = QStringLiteral( "select srs_id from tbl_srs where srs_id >= %1" ).arg( USER_CRS_START_ID );
  int rc;
  statement = database.prepare( sql, rc );
  while ( true )
  {
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

  return results;
}

long QgsCoordinateReferenceSystem::matchToUserCrs() const
{
  PJ *obj = d->threadLocalProjObject();
  if ( !obj )
    return 0;

  const QList< long > ids = userSrsIds();
  for ( long id : ids )
  {
    QgsCoordinateReferenceSystem candidate = QgsCoordinateReferenceSystem::fromSrsId( id );
    if ( candidate.projObject() && proj_is_equivalent_to( obj, candidate.projObject(), PJ_COMP_EQUIVALENT ) )
    {
      return id;
    }
  }
  return 0;
}

static void sync_db_proj_logger( void * /* user_data */, int level, const char *message )
{
#ifndef QGISDEBUG
  Q_UNUSED( message )
#endif
  if ( level == PJ_LOG_ERROR )
  {
    QgsDebugMsgLevel( QStringLiteral( "PROJ: %1" ).arg( message ), 2 );
  }
  else if ( level == PJ_LOG_DEBUG )
  {
    QgsDebugMsgLevel( QStringLiteral( "PROJ: %1" ).arg( message ), 3 );
  }
}

int QgsCoordinateReferenceSystem::syncDatabase()
{
  setlocale( LC_ALL, "C" );
  QString dbFilePath = QgsApplication::srsDatabaseFilePath();

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

  sqlite3_statement_unique_ptr statement;
  int result;
  char *errMsg = nullptr;

  if ( sqlite3_exec( database.get(), "create table tbl_info (proj_major INT, proj_minor INT, proj_patch INT)", nullptr, nullptr, nullptr ) == SQLITE_OK )
  {
    QString sql = QStringLiteral( "INSERT INTO tbl_info(proj_major, proj_minor, proj_patch) VALUES (%1, %2,%3)" )
                  .arg( QString::number( PROJ_VERSION_MAJOR ),
                        QString::number( PROJ_VERSION_MINOR ),
                        QString::number( PROJ_VERSION_PATCH ) );
    if ( sqlite3_exec( database.get(), sql.toUtf8(), nullptr, nullptr, &errMsg ) != SQLITE_OK )
    {
      QgsDebugMsg( QStringLiteral( "Could not execute: %1 [%2/%3]\n" ).arg(
                     sql,
                     database.errorMessage(),
                     errMsg ? errMsg : "(unknown error)" ) );
      if ( errMsg )
        sqlite3_free( errMsg );
      return -1;
    }
  }
  else
  {
    // retrieve last update details
    QString sql = QStringLiteral( "SELECT proj_major, proj_minor, proj_patch FROM tbl_info" );
    statement = database.prepare( sql, result );
    if ( result != SQLITE_OK )
    {
      QgsDebugMsg( QStringLiteral( "Could not prepare: %1 [%2]\n" ).arg( sql, database.errorMessage() ) );
      return -1;
    }
    if ( statement.step() == SQLITE_ROW )
    {
      int major = statement.columnAsInt64( 0 );
      int minor = statement.columnAsInt64( 1 );
      int patch = statement.columnAsInt64( 2 );
      if ( major == PROJ_VERSION_MAJOR && minor == PROJ_VERSION_MINOR && patch == PROJ_VERSION_PATCH )
        // yay, nothing to do!
        return 0;
    }
    else
    {
      QgsDebugMsg( QStringLiteral( "Could not retrieve previous CRS sync PROJ version number" ) );
      return -1;
    }
  }

  PJ_CONTEXT *pjContext = QgsProjContext::get();
  // silence proj warnings
  proj_log_func( pjContext, nullptr, sync_db_proj_logger );

  PROJ_STRING_LIST authorities = proj_get_authorities_from_database( pjContext );

  int nextSrsId = 63561;
  int nextSrId = 520003561;
  for ( auto authIter = authorities; authIter && *authIter; ++authIter )
  {
    const QString authority( *authIter );
    QgsDebugMsgLevel( QStringLiteral( "Loading authority '%1'" ).arg( authority ), 2 );
    PROJ_STRING_LIST codes = proj_get_codes_from_database( pjContext, *authIter, PJ_TYPE_CRS, true );

    QStringList allCodes;

    for ( auto codesIter = codes; codesIter && *codesIter; ++codesIter )
    {
      const QString code( *codesIter );
      allCodes << QgsSqliteUtils::quotedString( code );
      QgsDebugMsgLevel( QStringLiteral( "Loading code '%1'" ).arg( code ), 4 );
      QgsProjUtils::proj_pj_unique_ptr crs( proj_create_from_database( pjContext, *authIter, *codesIter, PJ_CATEGORY_CRS, false, nullptr ) );
      if ( !crs )
      {
        QgsDebugMsg( QStringLiteral( "Could not load '%1:%2'" ).arg( authority, code ) );
        continue;
      }

      switch ( proj_get_type( crs.get() ) )
      {
        case PJ_TYPE_VERTICAL_CRS: // don't need these in the CRS db
          continue;

        default:
          break;
      }

      crs = QgsProjUtils::crsToSingleCrs( crs.get() );

      QString proj4 = getFullProjString( crs.get() );
      proj4.replace( QLatin1String( "+type=crs" ), QString() );
      proj4 = proj4.trimmed();

      if ( proj4.isEmpty() )
      {
        QgsDebugMsgLevel( QStringLiteral( "No proj4 for '%1:%2'" ).arg( authority, code ), 2 );
        // satisfy not null constraint
        proj4 = "";
      }

      const bool deprecated = proj_is_deprecated( crs.get() );
      const QString name( proj_get_name( crs.get() ) );

      QString sql = QStringLiteral( "SELECT parameters,description,deprecated FROM tbl_srs WHERE auth_name='%1' AND auth_id='%2'" ).arg( authority, code );
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
      }

      if ( !srsProj4.isEmpty() || !srsDesc.isEmpty() )
      {
        if ( proj4 != srsProj4 || name != srsDesc || deprecated != srsDeprecated )
        {
          errMsg = nullptr;
          sql = QStringLiteral( "UPDATE tbl_srs SET parameters=%1,description=%2,deprecated=%3 WHERE auth_name=%4 AND auth_id=%5" )
                .arg( QgsSqliteUtils::quotedString( proj4 ) )
                .arg( QgsSqliteUtils::quotedString( name ) )
                .arg( deprecated ? 1 : 0 )
                .arg( QgsSqliteUtils::quotedString( authority ), QgsSqliteUtils::quotedString( code ) );

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
        // there's a not-null contraint on these columns, so we must use empty strings instead
        QString operation = "";
        QString ellps = "";
        getOperationAndEllipsoidFromProjString( proj4, operation, ellps );
        const bool isGeographic = testIsGeographic( crs.get() );

        // work out srid and srsid
        const QString dbVals = sAuthIdToQgisSrsIdMap.value( QStringLiteral( "%1:%2" ).arg( authority, code ) );
        QString srsId;
        QString srId;
        if ( !dbVals.isEmpty() )
        {
          const QStringList parts = dbVals.split( ',' );
          srsId = parts.at( 0 );
          srId = parts.at( 1 );
        }
        if ( srId.isEmpty() )
        {
          srId = QString::number( nextSrId );
          nextSrId++;
        }
        if ( srsId.isEmpty() )
        {
          srsId = QString::number( nextSrsId );
          nextSrsId++;
        }

        if ( !srsId.isEmpty() )
        {
          sql = QStringLiteral( "INSERT INTO tbl_srs(srs_id, description,projection_acronym,ellipsoid_acronym,parameters,srid,auth_name,auth_id,is_geo,deprecated) VALUES (%1, %2,%3,%4,%5,%6,%7,%8,%9,%10)" )
                .arg( srsId )
                .arg( QgsSqliteUtils::quotedString( name ),
                      QgsSqliteUtils::quotedString( operation ),
                      QgsSqliteUtils::quotedString( ellps ),
                      QgsSqliteUtils::quotedString( proj4 ) )
                .arg( srId )
                .arg( QgsSqliteUtils::quotedString( authority ) )
                .arg( QgsSqliteUtils::quotedString( code ) )
                .arg( isGeographic ? 1 : 0 )
                .arg( deprecated ? 1 : 0 );
        }
        else
        {
          sql = QStringLiteral( "INSERT INTO tbl_srs(description,projection_acronym,ellipsoid_acronym,parameters,srid,auth_name,auth_id,is_geo,deprecated) VALUES (%2,%3,%4,%5,%6,%7,%8,%9,%10)" )
                .arg( QgsSqliteUtils::quotedString( name ),
                      QgsSqliteUtils::quotedString( operation ),
                      QgsSqliteUtils::quotedString( ellps ),
                      QgsSqliteUtils::quotedString( proj4 ) )
                .arg( srId )
                .arg( QgsSqliteUtils::quotedString( authority ) )
                .arg( QgsSqliteUtils::quotedString( code ) )
                .arg( isGeographic ? 1 : 0 )
                .arg( deprecated ? 1 : 0 );
        }

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

    proj_string_list_destroy( codes );

    const QString sql = QStringLiteral( "DELETE FROM tbl_srs WHERE auth_name='%1' AND NOT auth_id IN (%2)" ).arg( authority, allCodes.join( ',' ) );
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

  }
  proj_string_list_destroy( authorities );

  QString sql = QStringLiteral( "UPDATE tbl_info set proj_major=%1,proj_minor=%2,proj_patch=%3" )
                .arg( QString::number( PROJ_VERSION_MAJOR ),
                      QString::number( PROJ_VERSION_MINOR ),
                      QString::number( PROJ_VERSION_PATCH ) );
  if ( sqlite3_exec( database.get(), sql.toUtf8(), nullptr, nullptr, &errMsg ) != SQLITE_OK )
  {
    QgsDebugMsg( QStringLiteral( "Could not execute: %1 [%2/%3]\n" ).arg(
                   sql,
                   database.errorMessage(),
                   errMsg ? errMsg : "(unknown error)" ) );
    if ( errMsg )
      sqlite3_free( errMsg );
    return -1;
  }

  if ( sqlite3_exec( database.get(), "COMMIT", nullptr, nullptr, nullptr ) != SQLITE_OK )
  {
    QgsDebugMsg( QStringLiteral( "Could not commit transaction: %1 [%2]\n" ).arg(
                   QgsApplication::srsDatabaseFilePath(),
                   sqlite3_errmsg( database.get() ) )
               );
    return -1;
  }

#ifdef QGISDEBUG
  QgsDebugMsgLevel( QStringLiteral( "CRS update (inserted:%1 updated:%2 deleted:%3 errors:%4)" ).arg( inserted ).arg( updated ).arg( deleted ).arg( errors ), 4 );
#else
  Q_UNUSED( deleted )
#endif

  if ( errors > 0 )
    return -errors;
  else
    return updated + inserted;
}

const QHash<QString, QgsCoordinateReferenceSystem> &QgsCoordinateReferenceSystem::stringCache()
{
  return *sStringCache();
}

const QHash<QString, QgsCoordinateReferenceSystem> &QgsCoordinateReferenceSystem::projCache()
{
  return *sProj4Cache();
}

const QHash<QString, QgsCoordinateReferenceSystem> &QgsCoordinateReferenceSystem::ogcCache()
{
  return *sOgcCache();
}

const QHash<QString, QgsCoordinateReferenceSystem> &QgsCoordinateReferenceSystem::wktCache()
{
  return *sWktCache();
}

const QHash<long, QgsCoordinateReferenceSystem> &QgsCoordinateReferenceSystem::srIdCache()
{
  return *sSrIdCache();
}

const QHash<long, QgsCoordinateReferenceSystem> &QgsCoordinateReferenceSystem::srsIdCache()
{
  return *sSrsIdCache();
}

QgsCoordinateReferenceSystem QgsCoordinateReferenceSystem::toGeographicCrs() const
{
  if ( isGeographic() )
  {
    return *this;
  }

  if ( PJ *obj = d->threadLocalProjObject() )
  {
    PJ_CONTEXT *pjContext = QgsProjContext::get();
    QgsProjUtils::proj_pj_unique_ptr geoCrs( proj_crs_get_geodetic_crs( pjContext, obj ) );
    if ( !geoCrs )
      return QgsCoordinateReferenceSystem();

    if ( !testIsGeographic( geoCrs.get() ) )
      return QgsCoordinateReferenceSystem();

    QgsProjUtils::proj_pj_unique_ptr normalized( proj_normalize_for_visualization( pjContext, geoCrs.get() ) );
    if ( !normalized )
      return QgsCoordinateReferenceSystem();

    return QgsCoordinateReferenceSystem::fromProjObject( normalized.get() );
  }
  else
  {
    return QgsCoordinateReferenceSystem();
  }
}

QString QgsCoordinateReferenceSystem::geographicCrsAuthId() const
{
  if ( isGeographic() )
  {
    return d->mAuthId;
  }
  else if ( PJ *obj = d->threadLocalProjObject() )
  {
    QgsProjUtils::proj_pj_unique_ptr geoCrs( proj_crs_get_geodetic_crs( QgsProjContext::get(), obj ) );
    return geoCrs ? QStringLiteral( "%1:%2" ).arg( proj_get_id_auth_name( geoCrs.get(), 0 ), proj_get_id_code( geoCrs.get(), 0 ) ) : QString();
  }
  else
  {
    return QString();
  }
}

PJ *QgsCoordinateReferenceSystem::projObject() const
{
  return d->threadLocalProjObject();
}

QgsCoordinateReferenceSystem QgsCoordinateReferenceSystem::fromProjObject( PJ *object )
{
  QgsCoordinateReferenceSystem crs;
  crs.createFromProjObject( object );
  return crs;
}

bool QgsCoordinateReferenceSystem::createFromProjObject( PJ *object )
{
  d.detach();
  d->mIsValid = false;
  d->mProj4.clear();
  d->mWktPreferred.clear();

  if ( !object )
  {
    return false;
  }

  switch ( proj_get_type( object ) )
  {
    case PJ_TYPE_GEODETIC_CRS:
    case PJ_TYPE_GEOCENTRIC_CRS:
    case PJ_TYPE_GEOGRAPHIC_CRS:
    case PJ_TYPE_GEOGRAPHIC_2D_CRS:
    case PJ_TYPE_GEOGRAPHIC_3D_CRS:
    case PJ_TYPE_VERTICAL_CRS:
    case PJ_TYPE_PROJECTED_CRS:
    case PJ_TYPE_COMPOUND_CRS:
    case PJ_TYPE_TEMPORAL_CRS:
    case PJ_TYPE_ENGINEERING_CRS:
    case PJ_TYPE_BOUND_CRS:
    case PJ_TYPE_OTHER_CRS:
      break;

    default:
      return false;
  }

  d->setPj( QgsProjUtils::crsToSingleCrs( object ) );

  if ( !d->hasPj() )
  {
    return d->mIsValid;
  }
  else
  {
    // maybe we can directly grab the auth name and code from the crs
    const QString authName( proj_get_id_auth_name( d->threadLocalProjObject(), 0 ) );
    const QString authCode( proj_get_id_code( d->threadLocalProjObject(), 0 ) );
    if ( !authName.isEmpty() && !authCode.isEmpty() && loadFromAuthCode( authName, authCode ) )
    {
      return d->mIsValid;
    }
    else
    {
      // Still a valid CRS, just not a known one
      d->mIsValid = true;
      d->mDescription = QString( proj_get_name( d->threadLocalProjObject() ) );
      setMapUnits();
    }
  }

  return d->mIsValid;
}

QStringList QgsCoordinateReferenceSystem::recentProjections()
{
  QStringList projections;
  const QList<QgsCoordinateReferenceSystem> res = recentCoordinateReferenceSystems();
  projections.reserve( res.size() );
  for ( const QgsCoordinateReferenceSystem &crs : res )
  {
    projections << QString::number( crs.srsid() );
  }
  return projections;
}

QList<QgsCoordinateReferenceSystem> QgsCoordinateReferenceSystem::recentCoordinateReferenceSystems()
{
  QList<QgsCoordinateReferenceSystem> res;

  // Read settings from persistent storage
  QgsSettings settings;
  QStringList projectionsProj4  = settings.value( QStringLiteral( "UI/recentProjectionsProj4" ) ).toStringList();
  QStringList projectionsWkt = settings.value( QStringLiteral( "UI/recentProjectionsWkt" ) ).toStringList();
  QStringList projectionsAuthId = settings.value( QStringLiteral( "UI/recentProjectionsAuthId" ) ).toStringList();
  int max = std::max( projectionsAuthId.size(), std::max( projectionsProj4.size(), projectionsWkt.size() ) );
  res.reserve( max );
  for ( int i = 0; i < max; ++i )
  {
    const QString proj = projectionsProj4.value( i );
    const QString wkt = projectionsWkt.value( i );
    const QString authid = projectionsAuthId.value( i );

    QgsCoordinateReferenceSystem crs;
    if ( !authid.isEmpty() )
      crs = QgsCoordinateReferenceSystem( authid );
    if ( !crs.isValid() && !wkt.isEmpty() )
      crs.createFromWkt( wkt );
    if ( !crs.isValid() && !proj.isEmpty() )
      crs.createFromProj( wkt );

    if ( crs.isValid() )
      res << crs;
  }
  return res;
}

void QgsCoordinateReferenceSystem::pushRecentCoordinateReferenceSystem( const QgsCoordinateReferenceSystem &crs )
{
  // we only want saved and standard CRSes in the recent list
  if ( crs.srsid() == 0 || !crs.isValid() )
    return;

  QList<QgsCoordinateReferenceSystem> recent = recentCoordinateReferenceSystems();
  recent.removeAll( crs );
  recent.insert( 0, crs );

  // trim to max 30 items
  recent = recent.mid( 0, 30 );
  QStringList authids;
  authids.reserve( recent.size() );
  QStringList proj;
  proj.reserve( recent.size() );
  QStringList wkt;
  wkt.reserve( recent.size() );
  for ( const QgsCoordinateReferenceSystem &c : std::as_const( recent ) )
  {
    authids << c.authid();
    proj << c.toProj();
    wkt << c.toWkt( WKT_PREFERRED );
  }

  QgsSettings settings;
  settings.setValue( QStringLiteral( "UI/recentProjectionsAuthId" ), authids );
  settings.setValue( QStringLiteral( "UI/recentProjectionsWkt" ), wkt );
  settings.setValue( QStringLiteral( "UI/recentProjectionsProj4" ), proj );
}

void QgsCoordinateReferenceSystem::invalidateCache( bool disableCache )
{
  sSrIdCacheLock()->lockForWrite();
  if ( !sDisableSrIdCache )
  {
    if ( disableCache )
      sDisableSrIdCache = true;
    sSrIdCache()->clear();
  }
  sSrIdCacheLock()->unlock();

  sOgcLock()->lockForWrite();
  if ( !sDisableOgcCache )
  {
    if ( disableCache )
      sDisableOgcCache = true;
    sOgcCache()->clear();
  }
  sOgcLock()->unlock();

  sProj4CacheLock()->lockForWrite();
  if ( !sDisableProjCache )
  {
    if ( disableCache )
      sDisableProjCache = true;
    sProj4Cache()->clear();
  }
  sProj4CacheLock()->unlock();

  sCRSWktLock()->lockForWrite();
  if ( !sDisableWktCache )
  {
    if ( disableCache )
      sDisableWktCache = true;
    sWktCache()->clear();
  }
  sCRSWktLock()->unlock();

  sCRSSrsIdLock()->lockForWrite();
  if ( !sDisableSrsIdCache )
  {
    if ( disableCache )
      sDisableSrsIdCache = true;
    sSrsIdCache()->clear();
  }
  sCRSSrsIdLock()->unlock();

  sCrsStringLock()->lockForWrite();
  if ( !sDisableStringCache )
  {
    if ( disableCache )
      sDisableStringCache = true;
    sStringCache()->clear();
  }
  sCrsStringLock()->unlock();
}

// invalid < regular < user
bool operator> ( const QgsCoordinateReferenceSystem &c1, const QgsCoordinateReferenceSystem &c2 )
{
  if ( c1.d == c2.d )
    return false;

  if ( !c1.d->mIsValid && !c2.d->mIsValid )
    return false;

  if ( !c1.d->mIsValid && c2.d->mIsValid )
    return false;

  if ( c1.d->mIsValid && !c2.d->mIsValid )
    return true;

  const bool c1IsUser = c1.d->mSrsId >= USER_CRS_START_ID;
  const bool c2IsUser = c2.d->mSrsId >= USER_CRS_START_ID;

  if ( c1IsUser && !c2IsUser )
    return true;

  if ( !c1IsUser && c2IsUser )
    return false;

  if ( !c1IsUser && !c2IsUser )
  {
    if ( c1.d->mAuthId != c2.d->mAuthId )
      return c1.d->mAuthId > c2.d->mAuthId;
  }
  else
  {
    const QString wkt1 = c1.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED );
    const QString wkt2 = c2.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED );
    if ( wkt1 != wkt2 )
      return wkt1 > wkt2;
  }

  if ( c1.d->mCoordinateEpoch == c2.d->mCoordinateEpoch )
    return false;

  if ( std::isnan( c1.d->mCoordinateEpoch ) && std::isnan( c2.d->mCoordinateEpoch ) )
    return false;

  if ( std::isnan( c1.d->mCoordinateEpoch ) && !std::isnan( c2.d->mCoordinateEpoch ) )
    return false;

  if ( !std::isnan( c1.d->mCoordinateEpoch ) && std::isnan( c2.d->mCoordinateEpoch ) )
    return true;

  return c1.d->mCoordinateEpoch > c2.d->mCoordinateEpoch;
}

bool operator< ( const QgsCoordinateReferenceSystem &c1, const QgsCoordinateReferenceSystem &c2 )
{
  if ( c1.d == c2.d )
    return false;

  if ( !c1.d->mIsValid && !c2.d->mIsValid )
    return false;

  if ( c1.d->mIsValid && !c2.d->mIsValid )
    return false;

  if ( !c1.d->mIsValid && c2.d->mIsValid )
    return true;

  const bool c1IsUser = c1.d->mSrsId >= USER_CRS_START_ID;
  const bool c2IsUser = c2.d->mSrsId >= USER_CRS_START_ID;

  if ( !c1IsUser && c2IsUser )
    return true;

  if ( c1IsUser && !c2IsUser )
    return false;

  if ( !c1IsUser && !c2IsUser )
  {
    if ( c1.d->mAuthId != c2.d->mAuthId )
      return c1.d->mAuthId < c2.d->mAuthId;
  }
  else
  {
    const QString wkt1 = c1.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED );
    const QString wkt2 = c2.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED );
    if ( wkt1 != wkt2 )
      return wkt1 < wkt2;
  }

  if ( c1.d->mCoordinateEpoch == c2.d->mCoordinateEpoch )
    return false;

  if ( std::isnan( c1.d->mCoordinateEpoch ) && std::isnan( c2.d->mCoordinateEpoch ) )
    return false;

  if ( !std::isnan( c1.d->mCoordinateEpoch ) && std::isnan( c2.d->mCoordinateEpoch ) )
    return false;

  if ( std::isnan( c1.d->mCoordinateEpoch ) && !std::isnan( c2.d->mCoordinateEpoch ) )
    return true;

  return c1.d->mCoordinateEpoch < c2.d->mCoordinateEpoch;
}

bool operator>= ( const QgsCoordinateReferenceSystem &c1, const QgsCoordinateReferenceSystem &c2 )
{
  return !( c1 < c2 );
}
bool operator<= ( const QgsCoordinateReferenceSystem &c1, const QgsCoordinateReferenceSystem &c2 )
{
  return !( c1 > c2 );
}
