/***************************************************************************
  qgsellipsoidutils.cpp
 ----------------------
  Date                 : April 2017
  Copyright            : (C) 2017 by Nyall Dawson
  email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsellipsoidutils.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include <sqlite3.h>

QReadWriteLock QgsEllipsoidUtils::sEllipsoidCacheLock;
QHash< QString, QgsEllipsoidUtils::EllipsoidParameters > QgsEllipsoidUtils::sEllipsoidCache;
QReadWriteLock QgsEllipsoidUtils::sDefinitionCacheLock;
QList< QgsEllipsoidUtils::EllipsoidDefinition > QgsEllipsoidUtils::sDefinitionCache;

QgsEllipsoidUtils::EllipsoidParameters QgsEllipsoidUtils::ellipsoidParameters( const QString &ellipsoid )
{
  // check cache
  sEllipsoidCacheLock.lockForRead();
  QHash< QString, EllipsoidParameters >::const_iterator cacheIt = sEllipsoidCache.constFind( ellipsoid );
  if ( cacheIt != sEllipsoidCache.constEnd() )
  {
    // found a match in the cache
    QgsEllipsoidUtils::EllipsoidParameters params = cacheIt.value();
    sEllipsoidCacheLock.unlock();
    return params;
  }
  sEllipsoidCacheLock.unlock();

  EllipsoidParameters params;

  // Check if we have a custom projection, and set from text string.
  // Format is "PARAMETER:<semi-major axis>:<semi minor axis>
  // Numbers must be with (optional) decimal point and no other separators (C locale)
  // Distances in meters.  Flattening is calculated.
  if ( ellipsoid.startsWith( QLatin1String( "PARAMETER" ) ) )
  {
    QStringList paramList = ellipsoid.split( ':' );
    bool semiMajorOk, semiMinorOk;
    double semiMajor = paramList[1].toDouble( & semiMajorOk );
    double semiMinor = paramList[2].toDouble( & semiMinorOk );
    if ( semiMajorOk && semiMinorOk )
    {
      params.semiMajor = semiMajor;
      params.semiMinor = semiMinor;
      params.inverseFlattening = semiMajor / ( semiMajor - semiMinor );
      params.useCustomParameters = true;
    }
    else
    {
      params.valid = false;
    }

    sEllipsoidCacheLock.lockForWrite();
    sEllipsoidCache.insert( ellipsoid, params );
    sEllipsoidCacheLock.unlock();
    return params;
  }

  // cache miss - get from database

  QString radius, parameter2;
  //
  // SQLITE3 stuff - get parameters for selected ellipsoid
  //
  sqlite3_database_unique_ptr database;
  sqlite3_statement_unique_ptr statement;
  // Continue with PROJ.4 list of ellipsoids.

  //check the db is available
  int result = database.open_v2( QgsApplication::srsDatabaseFilePath(), SQLITE_OPEN_READONLY, nullptr );
  if ( result )
  {
    QgsMessageLog::logMessage( QObject::tr( "Can't open database: %1" ).arg( database.errorMessage() ) );
    // XXX This will likely never happen since on open, sqlite creates the
    //     database if it does not exist.
    return params;
  }
  // Set up the query to retrieve the projection information needed to populate the ELLIPSOID list
  QString sql = "select radius, parameter2 from tbl_ellipsoid where acronym='" + ellipsoid + '\'';
  statement = database.prepare( sql, result );
  // XXX Need to free memory from the error msg if one is set
  if ( result == SQLITE_OK )
  {
    if ( statement.step() == SQLITE_ROW )
    {
      radius = statement.columnAsText( 0 );
      parameter2 = statement.columnAsText( 1 );
    }
  }
  // row for this ellipsoid wasn't found?
  if ( radius.isEmpty() || parameter2.isEmpty() )
  {
    QgsDebugMsg( QString( "setEllipsoid: no row in tbl_ellipsoid for acronym '%1'" ).arg( ellipsoid ) );
    params.valid = false;
    sEllipsoidCacheLock.lockForWrite();
    sEllipsoidCache.insert( ellipsoid, params );
    sEllipsoidCacheLock.unlock();
    return params;
  }

  // get major semiaxis
  if ( radius.left( 2 ) == QLatin1String( "a=" ) )
    params.semiMajor = radius.midRef( 2 ).toDouble();
  else
  {
    QgsDebugMsg( QString( "setEllipsoid: wrong format of radius field: '%1'" ).arg( radius ) );
    params.valid = false;
    sEllipsoidCacheLock.lockForWrite();
    sEllipsoidCache.insert( ellipsoid, params );
    sEllipsoidCacheLock.unlock();
    return params;
  }

  // get second parameter
  // one of values 'b' or 'f' is in field parameter2
  // second one must be computed using formula: invf = a/(a-b)
  if ( parameter2.left( 2 ) == QLatin1String( "b=" ) )
  {
    params.semiMinor = parameter2.midRef( 2 ).toDouble();
    params.inverseFlattening = params.semiMajor / ( params.semiMajor - params.semiMinor );
  }
  else if ( parameter2.left( 3 ) == QLatin1String( "rf=" ) )
  {
    params.inverseFlattening = parameter2.midRef( 3 ).toDouble();
    params.semiMinor = params.semiMajor - ( params.semiMajor / params.inverseFlattening );
  }
  else
  {
    QgsDebugMsg( QString( "setEllipsoid: wrong format of parameter2 field: '%1'" ).arg( parameter2 ) );
    params.valid = false;
    sEllipsoidCacheLock.lockForWrite();
    sEllipsoidCache.insert( ellipsoid, params );
    sEllipsoidCacheLock.unlock();
    return params;
  }

  QgsDebugMsgLevel( QString( "setEllipsoid: a=%1, b=%2, 1/f=%3" ).arg( params.semiMajor ).arg( params.semiMinor ).arg( params.inverseFlattening ), 4 );


  // get spatial ref system for ellipsoid
  QString proj4 = "+proj=longlat +ellps=" + ellipsoid + " +no_defs";
  QgsCoordinateReferenceSystem destCRS = QgsCoordinateReferenceSystem::fromProj4( proj4 );
  //TODO: createFromProj4 used to save to the user database any new CRS
  // this behavior was changed in order to separate creation and saving.
  // Not sure if it necessary to save it here, should be checked by someone
  // familiar with the code (should also give a more descriptive name to the generated CRS)
  if ( destCRS.srsid() == 0 )
  {
    QString name = QStringLiteral( " * %1 (%2)" )
                   .arg( QObject::tr( "Generated CRS", "A CRS automatically generated from layer info get this prefix for description" ),
                         destCRS.toProj4() );
    destCRS.saveAsUserCrs( name );
  }
  //

  // set transformation from project CRS to ellipsoid coordinates
  params.crs = destCRS;

  sEllipsoidCacheLock.lockForWrite();
  sEllipsoidCache.insert( ellipsoid, params );
  sEllipsoidCacheLock.unlock();
  return params;
}

QList<QgsEllipsoidUtils::EllipsoidDefinition> QgsEllipsoidUtils::definitions()
{
  sDefinitionCacheLock.lockForRead();
  if ( !sDefinitionCache.isEmpty() )
  {
    QList<QgsEllipsoidUtils::EllipsoidDefinition> defs = sDefinitionCache;
    sDefinitionCacheLock.unlock();
    return defs;
  }
  sDefinitionCacheLock.unlock();

  sDefinitionCacheLock.lockForWrite();
  sqlite3_database_unique_ptr database;
  sqlite3_statement_unique_ptr statement;
  int result;

  QList<QgsEllipsoidUtils::EllipsoidDefinition> defs;

  //check the db is available
  result = database.open_v2( QgsApplication::srsDatabaseFilePath(), SQLITE_OPEN_READONLY, nullptr );
  if ( result )
  {
    QgsDebugMsg( QString( "Can't open database: %1" ).arg( database.errorMessage() ) );
    // XXX This will likely never happen since on open, sqlite creates the
    //     database if it does not exist.
    Q_ASSERT( result == 0 );
  }

  // Set up the query to retrieve the projection information needed to populate the ELLIPSOID list
  QString sql = QStringLiteral( "select acronym, name from tbl_ellipsoid order by name" );
  statement = database.prepare( sql, result );

  if ( result == SQLITE_OK )
  {
    while ( statement.step() == SQLITE_ROW )
    {
      EllipsoidDefinition def;
      def.acronym = statement.columnAsText( 0 );
      def.description = statement.columnAsText( 1 );

      // use ellipsoidParameters so that result is cached
      def.parameters = ellipsoidParameters( def.acronym );

      defs << def;
    }
  }

  sDefinitionCache = defs;
  sDefinitionCacheLock.unlock();

  return defs;
}

QStringList QgsEllipsoidUtils::acronyms()
{
  QStringList result;
  Q_FOREACH ( const QgsEllipsoidUtils::EllipsoidDefinition &def, definitions() )
  {
    result << def.acronym;
  }
  return result;
}
