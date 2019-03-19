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
#include <QCollator>
#include "qgsprojutils.h"

#if PROJ_VERSION_MAJOR>=6
#include <proj.h>
#include <mutex>
#endif

QReadWriteLock QgsEllipsoidUtils::sEllipsoidCacheLock;
QHash< QString, QgsEllipsoidUtils::EllipsoidParameters > QgsEllipsoidUtils::sEllipsoidCache;
QReadWriteLock QgsEllipsoidUtils::sDefinitionCacheLock;
QList< QgsEllipsoidUtils::EllipsoidDefinition > QgsEllipsoidUtils::sDefinitionCache;

QgsEllipsoidUtils::EllipsoidParameters QgsEllipsoidUtils::ellipsoidParameters( const QString &ellipsoid )
{
#if PROJ_VERSION_MAJOR >= 6
  // ensure ellipsoid database is populated when first called
  static std::once_flag initialized;
  std::call_once( initialized, [ = ]
  {
    ( void )definitions();
  } );
#endif

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

#if PROJ_VERSION_MAJOR< 6
  // cache miss - get from database
  // NOT REQUIRED FOR PROJ >= 6 -- we populate known types once by calling definitions() above

  QString radius, parameter2;
  //
  // SQLITE3 stuff - get parameters for selected ellipsoid
  //
  sqlite3_database_unique_ptr database;
  sqlite3_statement_unique_ptr statement;
  // Continue with PROJ list of ellipsoids.

  //check the db is available
  int result = database.open_v2( QgsApplication::srsDatabaseFilePath(), SQLITE_OPEN_READONLY, nullptr );
  if ( result )
  {
    QgsMessageLog::logMessage( QObject::tr( "Can not open srs database (%1): %2" ).arg( QgsApplication::srsDatabaseFilePath(), database.errorMessage() ) );
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
    QgsDebugMsg( QStringLiteral( "setEllipsoid: no row in tbl_ellipsoid for acronym '%1'" ).arg( ellipsoid ) );
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
    QgsDebugMsg( QStringLiteral( "setEllipsoid: wrong format of radius field: '%1'" ).arg( radius ) );
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
    QgsDebugMsg( QStringLiteral( "setEllipsoid: wrong format of parameter2 field: '%1'" ).arg( parameter2 ) );
    params.valid = false;
    sEllipsoidCacheLock.lockForWrite();
    sEllipsoidCache.insert( ellipsoid, params );
    sEllipsoidCacheLock.unlock();
    return params;
  }

  QgsDebugMsgLevel( QStringLiteral( "setEllipsoid: a=%1, b=%2, 1/f=%3" ).arg( params.semiMajor ).arg( params.semiMinor ).arg( params.inverseFlattening ), 4 );


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
#else
  params.valid = false;
  sEllipsoidCacheLock.lockForWrite();
  sEllipsoidCache.insert( ellipsoid, params );
  sEllipsoidCacheLock.unlock();
  return params;
#endif
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
  QList<QgsEllipsoidUtils::EllipsoidDefinition> defs;

#if PROJ_VERSION_MAJOR>=6
  sEllipsoidCacheLock.lockForWrite();

  PJ_CONTEXT *context = QgsProjContext::get();
  PROJ_STRING_LIST authorities = proj_get_authorities_from_database( context );
  PROJ_STRING_LIST authoritiesIt = authorities;
  while ( char *authority = *authoritiesIt )
  {
    QgsDebugMsg( QString( authority ) );
    PROJ_STRING_LIST codes = proj_get_codes_from_database( context, authority, PJ_TYPE_ELLIPSOID, 0 );
    PROJ_STRING_LIST codesIt = codes;
    while ( char *code = *codesIt )
    {
      EllipsoidDefinition def;

      QgsDebugMsg( QString( code ) );

      PJ *ellipsoid = proj_create_from_database( context, authority, code, PJ_CATEGORY_ELLIPSOID, 0, nullptr );

      QgsDebugMsg( QString( proj_get_name( ellipsoid ) ) );
      def.description = QString( proj_get_name( ellipsoid ) ) ;

      double semiMajor, semiMinor, invFlattening;
      int semiMinorComputed;
      if ( proj_ellipsoid_get_parameters( context, ellipsoid, &semiMajor, &semiMinor, &semiMinorComputed, &invFlattening ) == 0 )
      {
        def.parameters.semiMajor = semiMajor;
        def.parameters.semiMinor = semiMinor;
        def.parameters.inverseFlattening = invFlattening;
      }
      else
      {
        def.parameters.valid = false;
      }

      defs << def;

      //    sEllipsoidCache.insert( QString( ellipsoid->id ), def.parameters );


      codesIt++;
    }
    proj_string_list_destroy( codes );

    authoritiesIt++;
  }
  proj_string_list_destroy( authorities );

#if 0
  // use proj to get ellipsoids
  const PJ_ELLPS *ellipsoid = proj_list_ellps();
  while ( ellipsoid->name )
  {
    EllipsoidDefinition def;
    def.acronym = ellipsoid->id ;
    def.description = ellipsoid->name;
    QgsDebugMsg( QString( ellipsoid->id ) );
    QgsDebugMsg( QString( ellipsoid->name ) );
    const QString majorString( ellipsoid->major );
    def.parameters.semiMajor = majorString.midRef( 2 ).toDouble();
    const QString minorString( ellipsoid->ell );
    if ( minorString.startsWith( 'b' ) )
    {
      // b= style
      def.parameters.semiMinor = minorString.midRef( 2 ).toDouble();
      def.parameters.inverseFlattening = def.parameters.semiMajor / ( def.parameters.semiMajor - def.parameters.semiMinor );
    }
    else
    {
      // rf= style
      def.parameters.inverseFlattening = minorString.midRef( 2 ).toDouble();
      def.parameters.semiMinor = def.parameters.semiMajor  * ( 1 - def.parameters.inverseFlattening );
    }

    QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromProj4( QStringLiteral( "+proj=longlat +ellps=%1 +no_defs" ).arg( def.acronym ) );
    if ( crs.srsid() == 0 )
    {
      //TODO: createFromProj4 used to save to the user database any new CRS
      // this behavior was changed in order to separate creation and saving.
      // Not sure if it necessary to save it here, should be checked by someone
      // familiar with the code (should also give a more descriptive name to the generated CRS)
      QString name = QStringLiteral( " * %1 (%2)" )
                     .arg( QObject::tr( "Generated CRS", "A CRS automatically generated from layer info get this prefix for description" ),
                           crs.toProj4() );
      crs.saveAsUserCrs( name );
    }
    def.parameters.crs = crs;

    defs << def;

    sEllipsoidCache.insert( QString( ellipsoid->id ), def.parameters );

    ellipsoid++;
  }
#endif
  sEllipsoidCacheLock.unlock();


#else
  sqlite3_database_unique_ptr database;
  sqlite3_statement_unique_ptr statement;
  int result;

  //check the db is available
  result = database.open_v2( QgsApplication::srsDatabaseFilePath(), SQLITE_OPEN_READONLY, nullptr );
  if ( result )
  {
    QgsDebugMsg( QStringLiteral( "Can't open database: %1" ).arg( database.errorMessage() ) );
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

#endif

  QCollator collator;
  collator.setCaseSensitivity( Qt::CaseInsensitive );
  std::sort( defs.begin(), defs.end(), [&collator]( const EllipsoidDefinition & a, const EllipsoidDefinition & b )
  {
    return collator.compare( a.description, b.description ) < 0;
  } );
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
