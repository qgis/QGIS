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
#include "qgscoordinatereferencesystem_p.h"
#include "qgscoordinatetransform.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgssqliteutils.h"

#include <sqlite3.h>

QgsCoordinateReferenceSystemRegistry::QgsCoordinateReferenceSystemRegistry( QObject *parent )
  : QObject( parent )
{

}

long QgsCoordinateReferenceSystemRegistry::addUserCrs( const QgsCoordinateReferenceSystem &crs, const QString &name, QgsCoordinateReferenceSystem::Format nativeFormat )
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
  QString wktString = crs.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED );

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
            + ( nativeFormat == QgsCoordinateReferenceSystem::FormatWkt ? QgsSqliteUtils::quotedString( wktString ) : QStringLiteral( "''" ) )
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
            + ( nativeFormat == QgsCoordinateReferenceSystem::FormatWkt ? QgsSqliteUtils::quotedString( wktString ) : QStringLiteral( "''" ) )
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
    QgsMessageLog::logMessage( QObject::tr( "Saved user CRS [%1]" ).arg( crs.toProj() ), QObject::tr( "CRS" ) );

    returnId = sqlite3_last_insert_rowid( database.get() );
    crs.d->mSrsId = returnId;
    crs.d->mAuthId = QStringLiteral( "USER:%1" ).arg( returnId );
    crs.d->mDescription = name;
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

bool QgsCoordinateReferenceSystemRegistry::updateUserCrs( long id, const QgsCoordinateReferenceSystem &crs, const QString &name, QgsCoordinateReferenceSystem::Format nativeFormat )
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
                      + ",wkt=" + ( nativeFormat == QgsCoordinateReferenceSystem::FormatWkt ? QgsSqliteUtils::quotedString( crs.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED, false ) ) : QStringLiteral( "''" ) )
                      + " where srs_id=" + QgsSqliteUtils::quotedString( QString::number( id ) )
                      ;

  sqlite3_database_unique_ptr database;
  //check the db is available
  int myResult = database.open( QgsApplication::qgisUserDatabaseFilePath() );
  if ( myResult != SQLITE_OK )
  {
    QgsDebugMsg( QStringLiteral( "Can't open or create database %1: %2" )
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

  QgsCoordinateReferenceSystem::invalidateCache();
  QgsCoordinateTransform::invalidateCache();

  if ( res )
  {
    emit userCrsChanged( crs.d->mAuthId );
    emit crsDefinitionsChanged();
  }

  return res;
}
