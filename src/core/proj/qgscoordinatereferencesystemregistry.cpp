/***************************************************************************
                             qgscoordinatereferencesystemregistry.cpp
                             -------------------
    begin                : January 2021
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscoordinatereferencesystemregistry.h"
#include "moc_qgscoordinatereferencesystemregistry.cpp"
#include "qgscoordinatereferencesystem_p.h"
#include "qgscoordinatetransform.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgssqliteutils.h"
#include "qgscelestialbody.h"
#include "qgsprojutils.h"
#include "qgsruntimeprofiler.h"
#include "qgsexception.h"
#include "qgsprojoperation.h"
#include "qgssettings.h"

#include <QFileInfo>
#include <sqlite3.h>
#include <mutex>
#include <proj.h>

QgsCoordinateReferenceSystemRegistry::QgsCoordinateReferenceSystemRegistry( QObject *parent )
  : QObject( parent )
{

}

QgsCoordinateReferenceSystemRegistry::~QgsCoordinateReferenceSystemRegistry() = default;

QList<QgsCoordinateReferenceSystemRegistry::UserCrsDetails> QgsCoordinateReferenceSystemRegistry::userCrsList() const
{
  QList<QgsCoordinateReferenceSystemRegistry::UserCrsDetails> res;

  //Setup connection to the existing custom CRS database:
  sqlite3_database_unique_ptr database;
  //check the db is available
  int result = database.open_v2( QgsApplication::qgisUserDatabaseFilePath(), SQLITE_OPEN_READONLY, nullptr );
  if ( result != SQLITE_OK )
  {
    QgsDebugError( QStringLiteral( "Can't open database: %1" ).arg( database.errorMessage() ) );
    return res;
  }

  const QString sql = QStringLiteral( "select srs_id,description,parameters, wkt from tbl_srs" );
  QgsDebugMsgLevel( QStringLiteral( "Query to populate existing list:%1" ).arg( sql ), 4 );
  sqlite3_statement_unique_ptr preparedStatement = database.prepare( sql, result );
  if ( result == SQLITE_OK )
  {
    const QgsCoordinateReferenceSystem crs;
    while ( preparedStatement.step() == SQLITE_ROW )
    {
      UserCrsDetails details;
      details.id = preparedStatement.columnAsText( 0 ).toLong();
      details.name = preparedStatement.columnAsText( 1 );
      details.proj = preparedStatement.columnAsText( 2 );
      details.wkt = preparedStatement.columnAsText( 3 );

      if ( !details.wkt.isEmpty() )
        details.crs.createFromWkt( details.wkt );
      else
        details.crs.createFromProj( details.proj );

      res << details;
    }
  }
  return res;
}

long QgsCoordinateReferenceSystemRegistry::addUserCrs( const QgsCoordinateReferenceSystem &crs, const QString &name, Qgis::CrsDefinitionFormat nativeFormat )
{
  if ( !crs.isValid() )
  {
    QgsDebugMsgLevel( QStringLiteral( "Can't save an invalid CRS!" ), 4 );
    return -1;
  }

  QString mySql;

  QString proj4String = crs.d->mProj4;
  if ( proj4String.isEmpty() )
  {
    proj4String = crs.toProj();
  }
  const QString wktString = crs.toWkt( Qgis::CrsWktVariant::Preferred );

  // ellipsoid acroynym column is incorrectly marked as not null in many crs database instances,
  // hack around this by using an empty string instead
  const QString quotedEllipsoidString = crs.ellipsoidAcronym().isNull() ? QStringLiteral( "''" ) : QgsSqliteUtils::quotedString( crs.ellipsoidAcronym() );

  //if this is the first record we need to ensure that its srs_id is 10000. For
  //any rec after that sqlite3 will take care of the autonumbering
  //this was done to support sqlite 3.0 as it does not yet support
  //the autoinc related system tables.
  if ( QgsCoordinateReferenceSystem::getRecordCount() == 0 )
  {
    mySql = "insert into tbl_srs (srs_id,description,projection_acronym,ellipsoid_acronym,parameters,is_geo,wkt) values ("
            + QString::number( USER_CRS_START_ID )
            + ',' + QgsSqliteUtils::quotedString( name )
            + ',' + ( !crs.d->mProjectionAcronym.isEmpty() ? QgsSqliteUtils::quotedString( crs.d->mProjectionAcronym ) : QStringLiteral( "''" ) )
            + ',' + quotedEllipsoidString
            + ',' + ( !proj4String.isEmpty() ? QgsSqliteUtils::quotedString( proj4String ) : QStringLiteral( "''" ) )
            + ",0,"  // <-- is_geo shamelessly hard coded for now
            + ( nativeFormat == Qgis::CrsDefinitionFormat::Wkt ? QgsSqliteUtils::quotedString( wktString ) : QStringLiteral( "''" ) )
            + ')';
  }
  else
  {
    mySql = "insert into tbl_srs (description,projection_acronym,ellipsoid_acronym,parameters,is_geo,wkt) values ("
            + QgsSqliteUtils::quotedString( name )
            + ',' + ( !crs.d->mProjectionAcronym.isEmpty() ? QgsSqliteUtils::quotedString( crs.d->mProjectionAcronym ) : QStringLiteral( "''" ) )
            + ',' + quotedEllipsoidString
            + ',' + ( !proj4String.isEmpty() ? QgsSqliteUtils::quotedString( proj4String ) : QStringLiteral( "''" ) )
            + ",0,"  // <-- is_geo shamelessly hard coded for now
            + ( nativeFormat == Qgis::CrsDefinitionFormat::Wkt ? QgsSqliteUtils::quotedString( wktString ) : QStringLiteral( "''" ) )
            + ')';
  }
  sqlite3_database_unique_ptr database;
  sqlite3_statement_unique_ptr statement;
  //check the db is available
  int myResult = database.open( QgsApplication::qgisUserDatabaseFilePath() );
  if ( myResult != SQLITE_OK )
  {
    QgsDebugError( QStringLiteral( "Can't open or create database %1: %2" )
                   .arg( QgsApplication::qgisUserDatabaseFilePath(),
                         database.errorMessage() ) );
    return false;
  }
  statement = database.prepare( mySql, myResult );

  qint64 returnId = -1;
  if ( myResult == SQLITE_OK && statement.step() == SQLITE_DONE )
  {
    QgsMessageLog::logMessage( QObject::tr( "Saved user CRS [%1]" ).arg( crs.toProj() ), QObject::tr( "CRS" ) );

    returnId = sqlite3_last_insert_rowid( database.get() );
    crs.d->mSrsId = returnId;
    crs.d->mAuthId = QStringLiteral( "USER:%1" ).arg( returnId );
    crs.d->mDescription = name;
  }

  if ( returnId != -1 )
  {
    // If we have a projection acronym not in the user db previously, add it.
    // This is a must, or else we can't select it from the vw_srs table.
    // Actually, add it always and let the SQL PRIMARY KEY remove duplicates.
    insertProjection( crs.projectionAcronym() );
  }

  QgsCoordinateReferenceSystem::invalidateCache();
  QgsCoordinateTransform::invalidateCache();

  if ( returnId != -1 )
  {
    emit userCrsAdded( crs.d->mAuthId );
    emit crsDefinitionsChanged();
  }

  return returnId;
}

bool QgsCoordinateReferenceSystemRegistry::updateUserCrs( long id, const QgsCoordinateReferenceSystem &crs, const QString &name, Qgis::CrsDefinitionFormat nativeFormat )
{
  if ( !crs.isValid() )
  {
    QgsDebugMsgLevel( QStringLiteral( "Can't save an invalid CRS!" ), 4 );
    return false;
  }

  const QString sql = "update tbl_srs set description="
                      + QgsSqliteUtils::quotedString( name )
                      + ",projection_acronym=" + ( !crs.projectionAcronym().isEmpty() ? QgsSqliteUtils::quotedString( crs.projectionAcronym() ) : QStringLiteral( "''" ) )
                      + ",ellipsoid_acronym=" + ( !crs.ellipsoidAcronym().isEmpty() ? QgsSqliteUtils::quotedString( crs.ellipsoidAcronym() ) : QStringLiteral( "''" ) )
                      + ",parameters=" + ( !crs.toProj().isEmpty() ? QgsSqliteUtils::quotedString( crs.toProj() ) : QStringLiteral( "''" ) )
                      + ",is_geo=0" // <--shamelessly hard coded for now
                      + ",wkt=" + ( nativeFormat == Qgis::CrsDefinitionFormat::Wkt ? QgsSqliteUtils::quotedString( crs.toWkt( Qgis::CrsWktVariant::Preferred, false ) ) : QStringLiteral( "''" ) )
                      + " where srs_id=" + QgsSqliteUtils::quotedString( QString::number( id ) )
                      ;

  sqlite3_database_unique_ptr database;
  //check the db is available
  const int myResult = database.open( QgsApplication::qgisUserDatabaseFilePath() );
  if ( myResult != SQLITE_OK )
  {
    QgsDebugError( QStringLiteral( "Can't open or create database %1: %2" )
                   .arg( QgsApplication::qgisUserDatabaseFilePath(),
                         database.errorMessage() ) );
    return false;
  }

  bool res = true;
  QString errorMessage;
  if ( database.exec( sql, errorMessage ) != SQLITE_OK )
  {
    QgsMessageLog::logMessage( QObject::tr( "Error saving user CRS [%1]: %2" ).arg( crs.toProj(), errorMessage ), QObject::tr( "CRS" ) );
    res = false;
  }
  else
  {
    const int changed = sqlite3_changes( database.get() );
    if ( changed )
    {
      QgsMessageLog::logMessage( QObject::tr( "Saved user CRS [%1]" ).arg( crs.toProj() ), QObject::tr( "CRS" ) );
    }
    else
    {
      QgsMessageLog::logMessage( QObject::tr( "Error saving user CRS [%1]: No matching ID found in database" ).arg( crs.toProj() ), QObject::tr( "CRS" ) );
      res = false;
    }
  }

  if ( res )
  {
    // If we have a projection acronym not in the user db previously, add it.
    // This is a must, or else we can't select it from the vw_srs table.
    // Actually, add it always and let the SQL PRIMARY KEY remove duplicates.
    insertProjection( crs.projectionAcronym() );
  }

  QgsCoordinateReferenceSystem::invalidateCache();
  QgsCoordinateTransform::invalidateCache();

  if ( res )
  {
    emit userCrsChanged( QStringLiteral( "USER:%1" ).arg( id ) );
    emit crsDefinitionsChanged();
  }

  return res;
}

bool QgsCoordinateReferenceSystemRegistry::removeUserCrs( long id )
{
  sqlite3_database_unique_ptr database;

  const QString sql = "delete from tbl_srs where srs_id=" + QgsSqliteUtils::quotedString( QString::number( id ) );
  QgsDebugMsgLevel( sql, 4 );
  //check the db is available
  int result = database.open( QgsApplication::qgisUserDatabaseFilePath() );
  if ( result != SQLITE_OK )
  {
    QgsDebugError( QStringLiteral( "Can't open database: %1 \n please notify QGIS developers of this error \n %2 (file name) " ).arg( database.errorMessage(),
                   QgsApplication::qgisUserDatabaseFilePath() ) );
    return false;
  }

  bool res = true;
  {
    sqlite3_statement_unique_ptr preparedStatement = database.prepare( sql, result );
    if ( result != SQLITE_OK || preparedStatement.step() != SQLITE_DONE )
    {
      QgsDebugError( QStringLiteral( "failed to remove custom CRS from database: %1 [%2]" ).arg( sql, database.errorMessage() ) );
      res = false;
    }
    else
    {
      const int changed = sqlite3_changes( database.get() );
      if ( changed )
      {
        QgsMessageLog::logMessage( QObject::tr( "Removed user CRS [%1]" ).arg( id ), QObject::tr( "CRS" ) );
      }
      else
      {
        QgsMessageLog::logMessage( QObject::tr( "Error removing user CRS [%1]: No matching ID found in database" ).arg( id ), QObject::tr( "CRS" ) );
        res = false;
      }
    }
  }

  QgsCoordinateReferenceSystem::invalidateCache();
  QgsCoordinateTransform::invalidateCache();

  if ( res )
  {
    emit userCrsRemoved( id );
    emit crsDefinitionsChanged();
  }

  return res;
}


bool QgsCoordinateReferenceSystemRegistry::insertProjection( const QString &projectionAcronym )
{
  sqlite3_database_unique_ptr database;
  sqlite3_database_unique_ptr srsDatabase;
  QString sql;
  //check the db is available
  int result = database.open( QgsApplication::qgisUserDatabaseFilePath() );
  if ( result != SQLITE_OK )
  {
    QgsDebugError( QStringLiteral( "Can't open database: %1 \n please notify  QGIS developers of this error \n %2 (file name) " ).arg( database.errorMessage(),
                   QgsApplication::qgisUserDatabaseFilePath() ) );
    return false;
  }
  int srsResult = srsDatabase.open( QgsApplication::srsDatabaseFilePath() );
  if ( result != SQLITE_OK )
  {
    QgsDebugError( QStringLiteral( "Can't open database %1 [%2]" ).arg( QgsApplication::srsDatabaseFilePath(),
                   srsDatabase.errorMessage() ) );
    return false;
  }

  // Set up the query to retrieve the projection information needed to populate the PROJECTION list
  const QString srsSql = "select acronym,name,notes,parameters from tbl_projection where acronym=" + QgsSqliteUtils::quotedString( projectionAcronym );

  sqlite3_statement_unique_ptr srsPreparedStatement = srsDatabase.prepare( srsSql, srsResult );
  if ( srsResult == SQLITE_OK )
  {
    if ( srsPreparedStatement.step() == SQLITE_ROW )
    {
      QgsDebugMsgLevel( QStringLiteral( "Trying to insert projection" ), 4 );
      // We have the result from system srs.db. Now insert into user db.
      sql = "insert into tbl_projection(acronym,name,notes,parameters) values ("
            + QgsSqliteUtils::quotedString( srsPreparedStatement.columnAsText( 0 ) )
            + ',' + QgsSqliteUtils::quotedString( srsPreparedStatement.columnAsText( 1 ) )
            + ',' + QgsSqliteUtils::quotedString( srsPreparedStatement.columnAsText( 2 ) )
            + ',' + QgsSqliteUtils::quotedString( srsPreparedStatement.columnAsText( 3 ) )
            + ')';
      sqlite3_statement_unique_ptr preparedStatement = database.prepare( sql, result );
      if ( result != SQLITE_OK || preparedStatement.step() != SQLITE_DONE )
      {
        QgsDebugError( QStringLiteral( "Could not insert projection into database: %1 [%2]" ).arg( sql, database.errorMessage() ) );
        return false;
      }
    }
  }
  else
  {
    QgsDebugError( QStringLiteral( "prepare failed: %1 [%2]" ).arg( srsSql, srsDatabase.errorMessage() ) );
    return false;
  }

  return true;
}

QMap<QString, QgsProjOperation> QgsCoordinateReferenceSystemRegistry::projOperations() const
{
  static std::once_flag initialized;
  static QMap< QString, QgsProjOperation > sProjOperations;
  std::call_once( initialized, []
  {
    const QgsScopedRuntimeProfile profile( QObject::tr( "Initialize PROJ operations" ) );

    const PJ_OPERATIONS *operation = proj_list_operations();
    while ( operation && operation->id )
    {
      QgsProjOperation value;
      value.mValid = true;
      value.mId = QString( operation->id );

      const QString description( *operation->descr );
      const QStringList descriptionParts = description.split( QStringLiteral( "\n\t" ) );
      value.mDescription = descriptionParts.value( 0 );
      value.mDetails = descriptionParts.mid( 1 ).join( '\n' );

      sProjOperations.insert( value.id(), value );

      operation++;
    }
  } );

  return sProjOperations;
}

QList< QgsCelestialBody> QgsCoordinateReferenceSystemRegistry::celestialBodies() const
{
#if PROJ_VERSION_MAJOR>8 || (PROJ_VERSION_MAJOR==8 && PROJ_VERSION_MINOR>=1)
  static QList< QgsCelestialBody > sCelestialBodies;
  static std::once_flag initialized;
  std::call_once( initialized, []
  {
    QgsScopedRuntimeProfile profile( QObject::tr( "Initialize celestial bodies" ) );

    PJ_CONTEXT *context = QgsProjContext::get();

    int resultCount = 0;
    PROJ_CELESTIAL_BODY_INFO **list = proj_get_celestial_body_list_from_database( context, nullptr, &resultCount );
    sCelestialBodies.reserve( resultCount );
    for ( int i = 0; i < resultCount; i++ )
    {
      const PROJ_CELESTIAL_BODY_INFO *info = list[ i ];
      if ( !info )
        break;

      QgsCelestialBody body;
      body.mValid = true;
      body.mAuthority = QString( info->auth_name );
      body.mName = QString( info->name );

      sCelestialBodies << body;
    }
    proj_celestial_body_list_destroy( list );
  } );

  return sCelestialBodies;
#else
  throw QgsNotSupportedException( QObject::tr( "Retrieving celestial bodies requires a QGIS build based on PROJ 8.1 or later" ) );
#endif
}

QSet<QString> QgsCoordinateReferenceSystemRegistry::authorities() const
{
  static QSet< QString > sKnownAuthorities;
  static std::once_flag initialized;
  std::call_once( initialized, []
  {
    QgsScopedRuntimeProfile profile( QObject::tr( "Initialize authorities" ) );

    PJ_CONTEXT *pjContext = QgsProjContext::get();
    PROJ_STRING_LIST authorities = proj_get_authorities_from_database( pjContext );

    for ( auto authIter = authorities; authIter && *authIter; ++authIter )
    {
      const QString authority( *authIter );
      sKnownAuthorities.insert( authority.toLower() );
    }

    proj_string_list_destroy( authorities );
  } );

  return sKnownAuthorities;
}

QList<QgsCrsDbRecord> QgsCoordinateReferenceSystemRegistry::crsDbRecords() const
{
  QgsReadWriteLocker locker( mCrsDbRecordsLock, QgsReadWriteLocker::Read );
  if ( mCrsDbRecordsPopulated )
    return mCrsDbRecords;

  locker.changeMode( QgsReadWriteLocker::Write );

  const QString srsDatabaseFileName = QgsApplication::srsDatabaseFilePath();
  if ( QFileInfo::exists( srsDatabaseFileName ) )
  {
    // open the database containing the spatial reference data, and do a one-time read
    sqlite3_database_unique_ptr database;
    int result = database.open_v2( srsDatabaseFileName, SQLITE_OPEN_READONLY, nullptr );
    if ( result == SQLITE_OK )
    {
      const QString sql = QStringLiteral( "SELECT description, srs_id, auth_name, auth_id, projection_acronym, deprecated, srs_type FROM tbl_srs" );
      sqlite3_statement_unique_ptr preparedStatement = database.prepare( sql, result );
      if ( result == SQLITE_OK )
      {
        while ( preparedStatement.step() == SQLITE_ROW )
        {
          QgsCrsDbRecord record;
          record.description = preparedStatement.columnAsText( 0 );
          record.srsId = preparedStatement.columnAsText( 1 );
          record.authName = preparedStatement.columnAsText( 2 );
          record.authId = preparedStatement.columnAsText( 3 );
          record.projectionAcronym = preparedStatement.columnAsText( 4 );
          record.deprecated = preparedStatement.columnAsText( 5 ).toInt();
          record.type = qgsEnumKeyToValue( preparedStatement.columnAsText( 6 ), Qgis::CrsType::Unknown );
          mCrsDbRecords.append( record );
        }
      }
    }
  }

  mCrsDbRecordsPopulated = true;
  return mCrsDbRecords;
}

QList<QgsCoordinateReferenceSystem> QgsCoordinateReferenceSystemRegistry::recentCrs()
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

void QgsCoordinateReferenceSystemRegistry::clearRecent()
{
  QgsSettings settings;
  settings.remove( QStringLiteral( "UI/recentProjectionsAuthId" ) );
  settings.remove( QStringLiteral( "UI/recentProjectionsWkt" ) );
  settings.remove( QStringLiteral( "UI/recentProjectionsProj4" ) );

  emit recentCrsCleared();
}

void QgsCoordinateReferenceSystemRegistry::pushRecent( const QgsCoordinateReferenceSystem &crs )
{
  // we only want saved and standard CRSes in the recent list
  if ( crs.srsid() == 0 || !crs.isValid() )
    return;

  QList<QgsCoordinateReferenceSystem> recent = recentCrs();
  recent.removeAll( crs );
  recent.insert( 0, crs );

  auto hasVertical = []( const QgsCoordinateReferenceSystem & crs )
  {
    switch ( crs.type() )
    {
      case Qgis::CrsType::Unknown:
      case Qgis::CrsType::Geodetic:
      case Qgis::CrsType::Geocentric:
      case Qgis::CrsType::Geographic2d:
      case Qgis::CrsType::Projected:
      case Qgis::CrsType::Temporal:
      case Qgis::CrsType::Engineering:
      case Qgis::CrsType::Bound:
      case Qgis::CrsType::Other:
      case Qgis::CrsType::DerivedProjected:
        return false;

      case Qgis::CrsType::Geographic3d:
      case Qgis::CrsType::Vertical:
      case Qgis::CrsType::Compound:
        return true;
    }
    BUILTIN_UNREACHABLE
  };
  QList<QgsCoordinateReferenceSystem> recentSameType;
  std::copy_if( recent.begin(), recent.end(), std::back_inserter( recentSameType ), [crs, &hasVertical]( const QgsCoordinateReferenceSystem & it )
  {
    return hasVertical( it ) == hasVertical( crs );
  } );

  // trim to max 30 items of the same type
  const QList<QgsCoordinateReferenceSystem> toTrim = recentSameType.mid( 30 );
  for ( const QgsCoordinateReferenceSystem &crsTrimmed : toTrim )
  {
    recent.removeOne( crsTrimmed );
    emit recentCrsRemoved( crsTrimmed );
  }

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
    wkt << c.toWkt( Qgis::CrsWktVariant::Preferred );
  }

  QgsSettings settings;
  settings.setValue( QStringLiteral( "UI/recentProjectionsAuthId" ), authids );
  settings.setValue( QStringLiteral( "UI/recentProjectionsWkt" ), wkt );
  settings.setValue( QStringLiteral( "UI/recentProjectionsProj4" ), proj );

  emit recentCrsPushed( crs );
}

void QgsCoordinateReferenceSystemRegistry::removeRecent( const QgsCoordinateReferenceSystem &crs )
{
  if ( crs.srsid() == 0 || !crs.isValid() )
    return;

  QList<QgsCoordinateReferenceSystem> recent = recentCrs();
  recent.removeAll( crs );
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
    wkt << c.toWkt( Qgis::CrsWktVariant::Preferred );
  }
  QgsSettings settings;
  settings.setValue( QStringLiteral( "UI/recentProjectionsAuthId" ), authids );
  settings.setValue( QStringLiteral( "UI/recentProjectionsWkt" ), wkt );
  settings.setValue( QStringLiteral( "UI/recentProjectionsProj4" ), proj );

  emit recentCrsRemoved( crs );
}
