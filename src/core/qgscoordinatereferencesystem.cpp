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

#include "qgscoordinatereferencesystem_legacy.h"
#include "qgsreadwritelocker.h"

#include <cmath>

#include <QDir>
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
#include "qgsogrutils.h"

#include <sqlite3.h>
#if PROJ_VERSION_MAJOR>=6
#include "qgsprojutils.h"
#include <proj.h>
#include <proj_experimental.h>
#else
#include <proj_api.h>
#endif

//gdal and ogr includes (needed for == operator)
#include <ogr_srs_api.h>
#include <cpl_error.h>
#include <cpl_conv.h>
#include <cpl_csv.h>


#if PROJ_VERSION_MAJOR<6
//! The length of the string "+lat_1="
const int LAT_PREFIX_LEN = 7;
#endif

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

#if PROJ_VERSION_MAJOR>=6
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
#endif
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
  Q_NOWARN_DEPRECATED_PUSH
  createFromId( id, type );
  Q_NOWARN_DEPRECATED_POP
}

QgsCoordinateReferenceSystem::QgsCoordinateReferenceSystem( const QgsCoordinateReferenceSystem &srs )  //NOLINT
  : d( srs.d )
  , mValidationHint( srs.mValidationHint )
{
}

QgsCoordinateReferenceSystem &QgsCoordinateReferenceSystem::operator=( const QgsCoordinateReferenceSystem &srs )  //NOLINT
{
  d = srs.d;
  mValidationHint = srs.mValidationHint;
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
      result = createFromPostgisSrid( id );
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
      return true;
    }
  }
  locker.unlock();

  bool result = false;
  QRegularExpression reCrsId( QStringLiteral( "^(epsg|esri|osgeo|ignf|zangi|iau2000|postgis|internal|user)\\:(\\w+)$" ), QRegularExpression::CaseInsensitiveOption );
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
      result = createFromPostgisSrid( id );
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
    QRegularExpression reCrsStr( "^(?:(wkt|proj4|proj)\\:)?(.+)$", QRegularExpression::CaseInsensitiveOption );
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

#if PROJ_VERSION_MAJOR<6
  // make sure towgs84 parameter is loaded if using an ESRI definition and gdal >= 1.9
  if ( definition.startsWith( QLatin1String( "ESRI::" ) ) )
  {
    Q_NOWARN_DEPRECATED_PUSH
    setupESRIWktFix();
    Q_NOWARN_DEPRECATED_POP
  }
#endif

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
      return true;
    }
  }
  locker.unlock();

  QString wmsCrs = crs;

  QRegExp re_uri( "http://www\\.opengis\\.net/def/crs/([^/]+).+/([^/]+)", Qt::CaseInsensitive );
  QRegExp re_urn( "urn:ogc:def:crs:([^:]+).+([^:]+)", Qt::CaseInsensitive );
  if ( re_uri.exactMatch( wmsCrs ) )
  {
    wmsCrs = re_uri.cap( 1 ) + ':' + re_uri.cap( 2 );
  }
  else if ( re_urn.exactMatch( wmsCrs ) )
  {
    wmsCrs = re_urn.cap( 1 ) + ':' + re_urn.cap( 2 );
  }
  else
  {
    re_urn.setPattern( QStringLiteral( "(user|custom|qgis):(\\d+)" ) );
    if ( re_urn.exactMatch( wmsCrs ) && createFromSrsId( re_urn.cap( 2 ).toInt() ) )
    {
      locker.changeMode( QgsReadWriteLocker::Write );
      if ( !sDisableOgcCache )
        sOgcCache()->insert( crs, *this );
      return true;
    }
  }

#if PROJ_VERSION_MAJOR>=6
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
        return true;
      }
    }
  }
#endif

  if ( loadFromDatabase( QgsApplication::srsDatabaseFilePath(), QStringLiteral( "lower(auth_name||':'||auth_id)" ), wmsCrs.toLower() ) )
  {
    locker.changeMode( QgsReadWriteLocker::Write );
    if ( !sDisableOgcCache )
      sOgcCache()->insert( crs, *this );
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
  return false;
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
  return createFromPostgisSrid( id );
}

bool QgsCoordinateReferenceSystem::createFromPostgisSrid( const long id )
{
  QgsReadWriteLocker locker( *sSrIdCacheLock(), QgsReadWriteLocker::Read );
  if ( !sDisableSrIdCache )
  {
    QHash< long, QgsCoordinateReferenceSystem >::const_iterator crsIt = sSrIdCache()->constFind( id );
    if ( crsIt != sSrIdCache()->constEnd() )
    {
      // found a match in the cache
      *this = crsIt.value();
      return true;
    }
  }
  locker.unlock();

#if PROJ_VERSION_MAJOR>=6
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

        return true;
      }
    }
  }
#endif

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
      return true;
    }
  }
  locker.unlock();

#if PROJ_VERSION_MAJOR>=6
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
        return true;
      }
    }
  }
#endif

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
#if PROJ_VERSION_MAJOR>=6
    d->mEllipsoidAcronym.clear();
#else
    d->mEllipsoidAcronym = statement.columnAsText( 3 );
#endif
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
#if PROJ_VERSION_MAJOR>=6
      QStringList parts = d->mAuthId.split( ':' );
      QString auth = parts.at( 0 );
      QString code = parts.at( 1 );

      {
        QgsProjUtils::proj_pj_unique_ptr crs( proj_create_from_database( QgsProjContext::get(), auth.toLatin1(), code.toLatin1(), PJ_CATEGORY_CRS, false, nullptr ) );
        d->setPj( QgsProjUtils::crsToSingleCrs( crs.get() ) );
      }

      d->mIsValid = d->hasPj();
#else
      OSRDestroySpatialReference( d->mCRS );
      d->mCRS = OSRNewSpatialReference( nullptr );
      d->mIsValid = OSRSetFromUserInput( d->mCRS, d->mAuthId.toLower().toLatin1() ) == OGRERR_NONE;
#endif
      setMapUnits();
    }

    if ( !d->mIsValid )
    {
      if ( !wkt.isEmpty() )
        setWktString( wkt, false );
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

#if PROJ_VERSION_MAJOR>=6
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
#endif

bool QgsCoordinateReferenceSystem::hasAxisInverted() const
{
  if ( d->mAxisInvertedDirty )
  {
#if PROJ_VERSION_MAJOR>=6
    d->mAxisInverted = QgsProjUtils::axisOrderIsSwapped( d->threadLocalProjObject() );
#else
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
#endif
    d->mAxisInvertedDirty = false;
  }

  return d->mAxisInverted;
}

bool QgsCoordinateReferenceSystem::createFromWkt( const QString &wkt )
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
      return true;
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
    if ( d->mSrsId == 0 )
    {
#if PROJ_VERSION_MAJOR>=6
      // lastly, try a tolerant match of the created proj object against all user CRSes (allowing differences in parameter order during the comparison)
      long id = matchToUserCrs();
      if ( id >= USER_CRS_START_ID )
      {
        createFromSrsId( id );
      }
#endif
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
      return true;
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
#if PROJ_VERSION_MAJOR>=6
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
          return true;
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

#else
  Q_UNUSED( identify )

  QRegExp myProjRegExp( "\\+proj=(\\S+)" );
  int myStart = myProjRegExp.indexIn( myProj4String );
  if ( myStart == -1 )
  {
    locker.changeMode( QgsReadWriteLocker::Write );
    if ( !sDisableProjCache )
      sProj4Cache()->insert( projString, *this );

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
      myStart2 = myLat2RegExp.indexIn( projString, myStart2 );
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
    const QRegExp regExp( "\\s+(?=\\+)" );
    {
      const auto constSplit = myProj4String.split( regExp, QString::SkipEmptyParts );
      for ( const QString &param : constSplit )
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
      const auto constSplit = myRecord["parameters"].split( regExp, QString::SkipEmptyParts );
      for ( const QString &param : constSplit )
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
    setProjString( myProj4String );
    Q_NOWARN_DEPRECATED_PUSH
    mySrsId = findMatchingProj();
    Q_NOWARN_DEPRECATED_POP
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
    setProjString( myProj4String );
  }
#endif

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
#if PROJ_VERSION_MAJOR<6
      myMap.clear();
#endif
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
  if ( !authid().isEmpty() )
  {
    if ( type != ShortString && !description().isEmpty() )
      return QStringLiteral( "%1 - %2" ).arg( authid(), description() );
    return authid();
  }
  else if ( !description().isEmpty() )
    return description();
  else if ( type == ShortString )
    return QObject::tr( "Unknown CRS" );
  else if ( !toWkt( WKT_PREFERRED ).isEmpty() )
    return QObject::tr( "Unknown CRS: %1" ).arg(
             type == MediumString ? ( toWkt( WKT_PREFERRED ).left( 50 ) + QString( QChar( 0x2026 ) ) )
             : toWkt( WKT_PREFERRED ) );
  else if ( !toProj().isEmpty() )
    return QObject::tr( "Unknown CRS: %1" ).arg( type == MediumString ? ( toProj().left( 50 ) + QString( QChar( 0x2026 ) ) )
           : toProj() );
  else
    return QString();
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
#if PROJ_VERSION_MAJOR>=6
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
#else
    return QString();

#endif
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
#if PROJ_VERSION_MAJOR>=6
    if ( PJ *obj = d->threadLocalProjObject() )
    {
      d->mProj4 = getFullProjString( obj );
    }
#else
    char *proj4src = nullptr;
    OSRExportToProj4( d->mCRS, &proj4src );
    d->mProj4 = proj4src;
    CPLFree( proj4src );
#endif
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

#if PROJ_VERSION_MAJOR>=6
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

#else
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
#endif
}

void QgsCoordinateReferenceSystem::setProjString( const QString &proj4String )
{
  d.detach();
  d->mProj4 = proj4String;
  d->mWktPreferred.clear();

  QgsLocaleNumC l;
  QString trimmed = proj4String.trimmed();

#if PROJ_VERSION_MAJOR>=6
  trimmed += QStringLiteral( " +type=crs" );
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
#else
  OSRDestroySpatialReference( d->mCRS );
  d->mCRS = OSRNewSpatialReference( nullptr );
  d->mIsValid = OSRImportFromProj4( d->mCRS, trimmed.toLatin1().constData() ) == OGRERR_NONE;

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
#endif

  setMapUnits();
}

bool QgsCoordinateReferenceSystem::setWktString( const QString &wkt, bool allowProjFallback )
{
  bool res = false;
  d->mIsValid = false;
  d->mWktPreferred.clear();

#if PROJ_VERSION_MAJOR>=6
  // TODO - remove allowProjFallback when we require proj 6+ to build
  ( void )allowProjFallback;

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
#else
  QByteArray ba = wkt.toLatin1();
  const char *pWkt = ba.data();

  OGRErr myInputResult = OSRImportFromWkt( d->mCRS, const_cast< char ** >( & pWkt ) );
  res = myInputResult == OGRERR_NONE;
  if ( !res )
  {
    QgsDebugMsg( QStringLiteral( "\n---------------------------------------------------------------" ) );
    QgsDebugMsg( QStringLiteral( "This CRS could *** NOT *** be set from the supplied Wkt " ) );
    QgsDebugMsg( "INPUT: " + wkt );
    QgsDebugMsg( QStringLiteral( "UNUSED WKT: %1" ).arg( pWkt ) );
    QgsDebugMsg( QStringLiteral( "---------------------------------------------------------------\n" ) );
  }
  else
  {
    d->mIsValid = true;
  }
#endif

  QgsReadWriteLocker locker( *sProj4CacheLock(), QgsReadWriteLocker::Unlocked );
  if ( !res )
  {
    locker.changeMode( QgsReadWriteLocker::Write );
    if ( !sDisableWktCache )
      sWktCache()->insert( wkt, *this );
    return d->mIsValid;
  }

#if PROJ_VERSION_MAJOR>=6
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
        return true;
      }
    }
    else
    {
      // Still a valid CRS, just not a known one
      d->mIsValid = true;
    }
    setMapUnits();
  }
#else
  if ( OSRAutoIdentifyEPSG( d->mCRS ) == OGRERR_NONE )
  {
    QString authid = QStringLiteral( "%1:%2" )
                     .arg( OSRGetAuthorityName( d->mCRS, nullptr ),
                           OSRGetAuthorityCode( d->mCRS, nullptr ) );
    bool result = createFromOgcWmsCrs( authid );
    locker.changeMode( QgsReadWriteLocker::Write );
    if ( !sDisableWktCache )
      sWktCache()->insert( wkt, *this );
    return result;
  }
#endif

  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // WARNING - wkt to proj conversion is lossy, hence we DON'T DO THIS on proj 6
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#if PROJ_VERSION_MAJOR<6
  if ( allowProjFallback )
  {
    d->mIsValid = false;
    // create the proj4 structs needed for transforming
    char *proj4src = nullptr;
    OSRExportToProj4( d->mCRS, &proj4src );

    //now that we have the proj4string, delegate to createFromProj so
    // that we can try to fill in the remaining class members...
    //create from Proj will set the isValidFlag
    if ( !createFromProj( proj4src ) )
    {
      CPLFree( proj4src );

#if GDAL_VERSION_NUM < GDAL_COMPUTE_VERSION(2,5,0)
      // try fixed up version
      OSRFixup( d->mCRS );
#endif

      OSRExportToProj4( d->mCRS, &proj4src );

      createFromProj( proj4src );
    }
    CPLFree( proj4src );
  }
  else if ( d->mIsValid )
  {
    setMapUnits();
  }
#endif
  return d->mIsValid;
}

void QgsCoordinateReferenceSystem::setMapUnits()
{
  if ( !d->mIsValid )
  {
    d->mMapUnits = QgsUnitTypes::DistanceUnknownUnit;
    return;
  }

#if PROJ_VERSION_MAJOR<6
#if GDAL_VERSION_NUM < GDAL_COMPUTE_VERSION(2,5,0)
  // Of interest to us is that this call adds in a unit parameter if
  // one doesn't already exist.
  OSRFixup( d->mCRS );
#endif
#endif

#if PROJ_VERSION_MAJOR>=6
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

#else
  char *unitName = nullptr;

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
#endif
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

  if ( ( !d->mAuthId.isEmpty() || !srs.d->mAuthId.isEmpty() ) )
    return d->mAuthId == srs.d->mAuthId;

  return toWkt( WKT_PREFERRED ) == srs.toWkt( WKT_PREFERRED );
}

bool QgsCoordinateReferenceSystem::operator!=( const QgsCoordinateReferenceSystem &srs ) const
{
  return  !( *this == srs );
}

QString QgsCoordinateReferenceSystem::toWkt( WktVariant variant, bool multiline, int indentationWidth ) const
{
#if PROJ_VERSION_MAJOR>=6
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
#else
  Q_UNUSED( variant )
  Q_UNUSED( multiline )
  Q_UNUSED( indentationWidth )
  char *wkt = nullptr;
  QString stringWkt;
  if ( OSRExportToWkt( d->mCRS, &wkt ) == OGRERR_NONE )
  {
    stringWkt = wkt;
    CPLFree( wkt );
  }
  return stringWkt;
#endif
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
      const QString wkt = srsNode.namedItem( QStringLiteral( "wkt" ) ).toElement().text();
      initialized = createFromWkt( wkt );
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
      d->mIsGeographic = node.toElement().text().compare( QLatin1String( "true" ) );

      d->mWktPreferred.clear();

      //make sure the map units have been set
      setMapUnits();
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
  QDomElement layerNode = node.toElement();
  QDomElement srsElement = doc.createElement( QStringLiteral( "spatialrefsys" ) );

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

/// Copied from QgsCustomProjectionDialog ///
/// Please refactor into SQL handler !!!  ///

long QgsCoordinateReferenceSystem::saveAsUserCrs( const QString &name, Format nativeFormat )
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
    proj4String = toProj();
  }
  QString wktString = toWkt( WKT_PREFERRED );

  // ellipsoid acroynym column is incorrectly marked as not null in many crs database instances,
  // hack around this by using an empty string instead
  const QString quotedEllipsoidString = ellipsoidAcronym().isNull() ? QStringLiteral( "''" ) : QgsSqliteUtils::quotedString( ellipsoidAcronym() );

  //if this is the first record we need to ensure that its srs_id is 10000. For
  //any rec after that sqlite3 will take care of the autonumbering
  //this was done to support sqlite 3.0 as it does not yet support
  //the autoinc related system tables.
  if ( getRecordCount() == 0 )
  {
    mySql = "insert into tbl_srs (srs_id,description,projection_acronym,ellipsoid_acronym,parameters,is_geo,wkt) values ("
            + QString::number( USER_CRS_START_ID )
            + ',' + QgsSqliteUtils::quotedString( name )
            + ',' + ( !d->mProjectionAcronym.isEmpty() ? QgsSqliteUtils::quotedString( d->mProjectionAcronym ) : QStringLiteral( "''" ) )
            + ',' + quotedEllipsoidString
            + ',' + ( !proj4String.isEmpty() ? QgsSqliteUtils::quotedString( proj4String ) : QStringLiteral( "''" ) )
            + ",0,"  // <-- is_geo shamelessly hard coded for now
            + ( nativeFormat == FormatWkt ? QgsSqliteUtils::quotedString( wktString ) : QStringLiteral( "''" ) )
            + ')';
  }
  else
  {
    mySql = "insert into tbl_srs (description,projection_acronym,ellipsoid_acronym,parameters,is_geo,wkt) values ("
            + QgsSqliteUtils::quotedString( name )
            + ',' + ( !d->mProjectionAcronym.isEmpty() ? QgsSqliteUtils::quotedString( d->mProjectionAcronym ) : QStringLiteral( "''" ) )
            + ',' + quotedEllipsoidString
            + ',' + ( !proj4String.isEmpty() ? QgsSqliteUtils::quotedString( proj4String ) : QStringLiteral( "''" ) )
            + ",0,"  // <-- is_geo shamelessly hard coded for now
            + ( nativeFormat == FormatWkt ? QgsSqliteUtils::quotedString( wktString ) : QStringLiteral( "''" ) )
            + ')';
  }
  sqlite3_database_unique_ptr database;
  sqlite3_statement_unique_ptr statement;
  //check the db is available
  int myResult = database.open( QgsApplication::qgisUserDatabaseFilePath() );
  if ( myResult != SQLITE_OK )
  {
    QgsDebugMsg( QStringLiteral( "Can't open or create database %1: %2" )
                 .arg( QgsApplication::qgisUserDatabaseFilePath(),
                       database.errorMessage() ) );
    return false;
  }
  statement = database.prepare( mySql, myResult );

  qint64 returnId = -1;
  if ( myResult == SQLITE_OK && statement.step() == SQLITE_DONE )
  {
    QgsMessageLog::logMessage( QObject::tr( "Saved user CRS [%1]" ).arg( toProj() ), QObject::tr( "CRS" ) );

    returnId = sqlite3_last_insert_rowid( database.get() );
    d->mSrsId = returnId;
    if ( authid().isEmpty() )
      d->mAuthId = QStringLiteral( "USER:%1" ).arg( returnId );
    d->mDescription = name;
  }

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

#if PROJ_VERSION_MAJOR>=6
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
  QRegExp projRegExp( "\\+proj=(\\S+)" );
  if ( projRegExp.indexIn( proj ) < 0 )
  {
    QgsDebugMsgLevel( QStringLiteral( "no +proj argument found [%2]" ).arg( proj ), 2 );
    return;
  }
  operation = projRegExp.cap( 1 );

  QRegExp ellipseRegExp( "\\+(?:ellps|datum)=(\\S+)" );
  QString ellps;
  if ( ellipseRegExp.indexIn( proj ) >= 0 )
  {
    ellipsoid = ellipseRegExp.cap( 1 );
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
  proj4.replace( QStringLiteral( "+type=crs" ), QString() );
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
  QString srsId;
  QString srId;
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
#endif

#if PROJ_VERSION_MAJOR<6
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

  static const QStringList csvs { QStringList() << QStringLiteral( "gcs.csv" ) << QStringLiteral( "pcs.csv" ) << QStringLiteral( "vertcs.csv" ) << QStringLiteral( "compdcs.csv" ) << QStringLiteral( "geoccs.csv" ) };
  for ( const QString &csv : csvs )
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
#endif

#if PROJ_VERSION_MAJOR>=6
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
#endif

int QgsCoordinateReferenceSystem::syncDatabase()
{
  setlocale( LC_ALL, "C" );
  QString dbFilePath = QgsApplication::srsDatabaseFilePath();

#if PROJ_VERSION_MAJOR<6
  syncDatumTransform( dbFilePath );
#endif

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

#if PROJ_VERSION_MAJOR<6
// fix up database, if not done already //
  if ( sqlite3_exec( database.get(), "alter table tbl_srs add noupdate boolean", nullptr, nullptr, nullptr ) == SQLITE_OK )
    ( void )sqlite3_exec( database.get(), "update tbl_srs set noupdate=(auth_name='EPSG' and auth_id in (5513,5514,5221,2065,102067,4156,4818))", nullptr, nullptr, nullptr );

  ( void )sqlite3_exec( database.get(), "UPDATE tbl_srs SET srid=141001 WHERE srid=41001 AND auth_name='OSGEO' AND auth_id='41001'", nullptr, nullptr, nullptr );
#else
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
#endif


#if PROJ_VERSION_MAJOR>=6
  PJ_CONTEXT *pjContext = QgsProjContext::get();
  // silence proj warnings
  proj_log_func( pjContext, nullptr, sync_db_proj_logger );

  PROJ_STRING_LIST authorities = proj_get_authorities_from_database( pjContext );

  int nextSrsId = 63321;
  int nextSrId = 520003321;
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
      proj4.replace( QStringLiteral( "+type=crs" ), QString() );
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

#else

  OGRSpatialReferenceH crs = nullptr;

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

    bool deprecated = name.contains( QLatin1String( "(deprecated)" ) );

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

#endif

#if PROJ_VERSION_MAJOR>=6
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
#endif

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

#if PROJ_VERSION_MAJOR<6
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
  QString insert;
  const int n = CSLCount( fieldnames );
  int idxid = -1, idxrx = -1, idxry = -1, idxrz = -1, idxmcode = -1;

  {
    QString values;

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

    Q_ASSERT( idxid >= 0 );
    Q_ASSERT( idxrx >= 0 );
    Q_ASSERT( idxry >= 0 );
    Q_ASSERT( idxrz >= 0 );
  }

  CSLDestroy( fieldnames );

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
#endif

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

QString QgsCoordinateReferenceSystem::geographicCrsAuthId() const
{
  if ( isGeographic() )
  {
    return d->mAuthId;
  }
#if PROJ_VERSION_MAJOR>=6
  else if ( PJ *obj = d->threadLocalProjObject() )
  {
    QgsProjUtils::proj_pj_unique_ptr geoCrs( proj_crs_get_geodetic_crs( QgsProjContext::get(), obj ) );
    return geoCrs ? QStringLiteral( "%1:%2" ).arg( proj_get_id_auth_name( geoCrs.get(), 0 ), proj_get_id_code( geoCrs.get(), 0 ) ) : QString();
  }
#else
  else if ( d->mCRS )
  {
    return OSRGetAuthorityName( d->mCRS, "GEOGCS" ) + QStringLiteral( ":" ) + OSRGetAuthorityCode( d->mCRS, "GEOGCS" );
  }
#endif
  else
  {
    return QString();
  }
}

#if PROJ_VERSION_MAJOR>=6
PJ *QgsCoordinateReferenceSystem::projObject() const
{
  return d->threadLocalProjObject();
}
#endif

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

  // trim to max 10 items
  recent = recent.mid( 0, 10 );
  QStringList authids;
  authids.reserve( recent.size() );
  QStringList proj;
  proj.reserve( recent.size() );
  QStringList wkt;
  wkt.reserve( recent.size() );
  for ( const QgsCoordinateReferenceSystem &c : qgis::as_const( recent ) )
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
