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
#include "qgsreadwritelocker.h"
#include "qgsruntimeprofiler.h"
#include "qgscoordinatereferencesystemregistry.h"
#include "qgscelestialbody.h"

#include <proj.h>
#include <mutex>

Q_GLOBAL_STATIC( QReadWriteLock, sEllipsoidCacheLock )
typedef QHash< QString, QgsEllipsoidUtils::EllipsoidParameters > EllipsoidParamCache;
Q_GLOBAL_STATIC( EllipsoidParamCache, sEllipsoidCache )

Q_GLOBAL_STATIC( QReadWriteLock, sDefinitionCacheLock );
typedef QList< QgsEllipsoidUtils::EllipsoidDefinition > EllipsoidDefinitionCache;
Q_GLOBAL_STATIC( EllipsoidDefinitionCache, sDefinitionCache )

static bool sDisableCache = false;

QgsEllipsoidUtils::EllipsoidParameters QgsEllipsoidUtils::ellipsoidParameters( const QString &e )
{
// maps older QGIS ellipsoid acronyms to proj acronyms/names
  static const QMap< QString, QString > sProj6EllipsoidAcronymMap
  {
    { "clrk80", "clrk80ign"  },
    {"Adrastea2000", "ESRI:107909"},
    {"Amalthea2000", "ESRI:107910"},
    {"Ananke2000", "ESRI:107911"},
    {"Ariel2000", "ESRI:107945"},
    {"Atlas2000", "ESRI:107926"},
    {"Belinda2000", "ESRI:107946"},
    {"Bianca2000", "ESRI:107947"},
    {"Callisto2000", "ESRI:107912"},
    {"Calypso2000", "ESRI:107927"},
    {"Carme2000", "ESRI:107913"},
    {"Charon2000", "ESRI:107970"},
    {"Cordelia2000", "ESRI:107948"},
    {"Cressida2000", "ESRI:107949"},
    {"Deimos2000", "ESRI:107906"},
    {"Desdemona2000", "ESRI:107950"},
    {"Despina2000", "ESRI:107961"},
    {"Dione2000", "ESRI:107928"},
    {"Elara2000", "ESRI:107914"},
    {"Enceladus2000", "ESRI:107929"},
    {"Epimetheus2000", "ESRI:107930"},
    {"Europa2000", "ESRI:107915"},
    {"Galatea2000", "ESRI:107962"},
    {"Ganymede2000", "ESRI:107916"},
    {"Helene2000", "ESRI:107931"},
    {"Himalia2000", "ESRI:107917"},
    {"Hyperion2000", "ESRI:107932"},
    {"Iapetus2000", "ESRI:107933"},
    {"Io2000", "ESRI:107918"},
    {"Janus2000", "ESRI:107934"},
    {"Juliet2000", "ESRI:107951"},
    {"Jupiter2000", "ESRI:107908"},
    {"Larissa2000", "ESRI:107963"},
    {"Leda2000", "ESRI:107919"},
    {"Lysithea2000", "ESRI:107920"},
    {"Mars2000", "ESRI:107905"},
    {"Mercury2000", "ESRI:107900"},
    {"Metis2000", "ESRI:107921"},
    {"Mimas2000", "ESRI:107935"},
    {"Miranda2000", "ESRI:107952"},
    {"Moon2000", "ESRI:107903"},
    {"Naiad2000", "ESRI:107964"},
    {"Neptune2000", "ESRI:107960"},
    {"Nereid2000", "ESRI:107965"},
    {"Oberon2000", "ESRI:107953"},
    {"Ophelia2000", "ESRI:107954"},
    {"Pan2000", "ESRI:107936"},
    {"Pandora2000", "ESRI:107937"},
    {"Pasiphae2000", "ESRI:107922"},
    {"Phobos2000", "ESRI:107907"},
    {"Phoebe2000", "ESRI:107938"},
    {"Pluto2000", "ESRI:107969"},
    {"Portia2000", "ESRI:107955"},
    {"Prometheus2000", "ESRI:107939"},
    {"Proteus2000", "ESRI:107966"},
    {"Puck2000", "ESRI:107956"},
    {"Rhea2000", "ESRI:107940"},
    {"Rosalind2000", "ESRI:107957"},
    {"Saturn2000", "ESRI:107925"},
    {"Sinope2000", "ESRI:107923"},
    {"Telesto2000", "ESRI:107941"},
    {"Tethys2000", "ESRI:107942"},
    {"Thalassa2000", "ESRI:107967"},
    {"Thebe2000", "ESRI:107924"},
    {"Titan2000", "ESRI:107943"},
    {"Titania2000", "ESRI:107958"},
    {"Triton2000", "ESRI:107968"},
    {"Umbriel2000", "ESRI:107959"},
    {"Uranus2000", "ESRI:107944"},
    {"Venus2000", "ESRI:107902"},
    {"IGNF:ELG053", "EPSG:7030"},
    {"IGNF:ELG052", "EPSG:7043"},
    {"IGNF:ELG102", "EPSG:7043"},
    {"WGS66", "ESRI:107001"},
    {"plessis", "EPSG:7027"},
    {"IGNF:ELG017", "EPSG:7027"},
    {"mod_airy", "EPSG:7002"},
    {"IGNF:ELG037", "EPSG:7019"},
    {"IGNF:ELG108", "EPSG:7036"},
    {"cape", "EPSG:7034"},
    {"IGNF:ELG010", "EPSG:7011"},
    {"IGNF:ELG003", "EPSG:7012"},
    {"IGNF:ELG004", "EPSG:7008"},
    {"GSK2011", "EPSG:1025"},
    {"airy", "EPSG:7001"},
    {"aust_SA", "EPSG:7003"},
    {"bessel", "EPSG:7004"},
    {"clrk66", "EPSG:7008"},
    {"clrk80ign", "EPSG:7011"},
    {"evrst30", "EPSG:7015"},
    {"evrstSS", "EPSG:7016"},
    {"evrst48", "EPSG:7018"},
    {"GRS80", "EPSG:7019"},
    {"helmert", "EPSG:7020"},
    {"intl", "EPSG:7022"},
    {"krass", "EPSG:7024"},
    {"NWL9D", "EPSG:7025"},
    {"WGS84", "EPSG:7030"},
    {"GRS67", "EPSG:7036"},
    {"WGS72", "EPSG:7043"},
    {"bess_nam", "EPSG:7046"},
    {"IAU76", "EPSG:7049"},
    {"sphere", "EPSG:7052"},
    {"hough", "EPSG:7053"},
    {"evrst69", "EPSG:7056"},
    {"fschr60", "ESRI:107002"},
    {"fschr68", "ESRI:107003"},
    {"fschr60m", "ESRI:107004"},
    {"walbeck", "ESRI:107007"},
    {"IGNF:ELG001", "EPSG:7022"},
    {"engelis", "EPSG:7054"},
    {"evrst56", "EPSG:7044"},
    {"SEasia", "ESRI:107004"},
    {"SGS85", "EPSG:7054"},
    {"andrae", "PROJ:ANDRAE"},
    {"clrk80", "EPSG:7034"},
    {"CPM", "PROJ:CPM"},
    {"delmbr", "PROJ:DELMBR"},
    {"Earth2000", "PROJ:EARTH2000"},
    {"kaula", "PROJ:KAULA"},
    {"lerch", "PROJ:LERCH"},
    {"MERIT", "PROJ:MERIT"},
    {"mprts", "PROJ:MPRTS"},
    {"new_intl", "PROJ:NEW_INTL"},
    {"WGS60", "PROJ:WGS60"}
  };

  QString ellipsoid = e;
  // ensure ellipsoid database is populated when first called
  static std::once_flag initialized;
  std::call_once( initialized, [ = ]
  {
    const QgsScopedRuntimeProfile profile( QObject::tr( "Initialize ellipsoids" ) );
    ( void )definitions();
  } );

  ellipsoid = sProj6EllipsoidAcronymMap.value( ellipsoid, ellipsoid ); // silently upgrade older QGIS acronyms to proj acronyms

  // check cache
  {
    const QgsReadWriteLocker locker( *sEllipsoidCacheLock(), QgsReadWriteLocker::Read );
    if ( !sDisableCache )
    {
      const QHash< QString, EllipsoidParameters >::const_iterator cacheIt = sEllipsoidCache()->constFind( ellipsoid );
      if ( cacheIt != sEllipsoidCache()->constEnd() )
      {
        // found a match in the cache
        QgsEllipsoidUtils::EllipsoidParameters params = cacheIt.value();
        return params;
      }
    }
  }

  EllipsoidParameters params;

  // Check if we have a custom projection, and set from text string.
  // Format is "PARAMETER:<semi-major axis>:<semi minor axis>
  // Numbers must be with (optional) decimal point and no other separators (C locale)
  // Distances in meters.  Flattening is calculated.
  if ( ellipsoid.startsWith( QLatin1String( "PARAMETER" ) ) )
  {
    QStringList paramList = ellipsoid.split( ':' );
    bool semiMajorOk, semiMinorOk;
    const double semiMajor = paramList[1].toDouble( & semiMajorOk );
    const double semiMinor = paramList[2].toDouble( & semiMinorOk );
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

    const QgsReadWriteLocker locker( *sEllipsoidCacheLock(), QgsReadWriteLocker::Write );
    if ( !sDisableCache )
    {
      sEllipsoidCache()->insert( ellipsoid, params );
    }
    return params;
  }
  params.valid = false;

  const QgsReadWriteLocker l( *sEllipsoidCacheLock(), QgsReadWriteLocker::Write );
  if ( !sDisableCache )
  {
    sEllipsoidCache()->insert( ellipsoid, params );
  }

  return params;
}

QList<QgsEllipsoidUtils::EllipsoidDefinition> QgsEllipsoidUtils::definitions()
{
  QgsReadWriteLocker defLocker( *sDefinitionCacheLock(), QgsReadWriteLocker::Read );
  if ( !sDefinitionCache()->isEmpty() )
  {
    return *sDefinitionCache();
  }
  defLocker.changeMode( QgsReadWriteLocker::Write );

  QList<QgsEllipsoidUtils::EllipsoidDefinition> defs;

  QgsReadWriteLocker locker( *sEllipsoidCacheLock(), QgsReadWriteLocker::Write );

  PJ_CONTEXT *context = QgsProjContext::get();
  if ( PROJ_STRING_LIST authorities = proj_get_authorities_from_database( context ) )
  {
    PROJ_STRING_LIST authoritiesIt = authorities;
    while ( char *authority = *authoritiesIt )
    {
      if ( PROJ_STRING_LIST codes = proj_get_codes_from_database( context, authority, PJ_TYPE_ELLIPSOID, 0 ) )
      {
        PROJ_STRING_LIST codesIt = codes;
        while ( char *code = *codesIt )
        {
          const QgsProjUtils::proj_pj_unique_ptr ellipsoid( proj_create_from_database( context, authority, code, PJ_CATEGORY_ELLIPSOID, 0, nullptr ) );
          if ( ellipsoid.get() )
          {
            EllipsoidDefinition def;
            QString name = QString( proj_get_name( ellipsoid.get() ) );
            def.acronym = QStringLiteral( "%1:%2" ).arg( authority, code );
            name.replace( '_', ' ' );
            def.description = QStringLiteral( "%1 (%2:%3)" ).arg( name, authority, code );

#if PROJ_VERSION_MAJOR>8 || (PROJ_VERSION_MAJOR==8 && PROJ_VERSION_MINOR>=1)
            def.celestialBodyName = proj_get_celestial_body_name( context, ellipsoid.get() );
#endif

            double semiMajor, semiMinor, invFlattening;
            int semiMinorComputed = 0;
            if ( proj_ellipsoid_get_parameters( context, ellipsoid.get(), &semiMajor, &semiMinor, &semiMinorComputed, &invFlattening ) )
            {
              def.parameters.semiMajor = semiMajor;
              def.parameters.semiMinor = semiMinor;
              def.parameters.inverseFlattening = invFlattening;
              if ( !semiMinorComputed )
                def.parameters.crs.createFromProj( QStringLiteral( "+proj=longlat +a=%1 +b=%2 +no_defs +type=crs" ).arg( def.parameters.semiMajor, 0, 'g', 17 ).arg( def.parameters.semiMinor, 0, 'g', 17 ), false );
              else if ( !qgsDoubleNear( def.parameters.inverseFlattening, 0.0 ) )
                def.parameters.crs.createFromProj( QStringLiteral( "+proj=longlat +a=%1 +rf=%2 +no_defs +type=crs" ).arg( def.parameters.semiMajor, 0, 'g', 17 ).arg( def.parameters.inverseFlattening, 0, 'g', 17 ), false );
              else
                def.parameters.crs.createFromProj( QStringLiteral( "+proj=longlat +a=%1 +no_defs +type=crs" ).arg( def.parameters.semiMajor, 0, 'g', 17 ), false );
            }
            else
            {
              def.parameters.valid = false;
            }

            defs << def;
            if ( !sDisableCache )
            {
              sEllipsoidCache()->insert( def.acronym, def.parameters );
            }
          }

          codesIt++;
        }
        proj_string_list_destroy( codes );
      }

      authoritiesIt++;
    }
    proj_string_list_destroy( authorities );
  }
  locker.unlock();

  QCollator collator;
  collator.setCaseSensitivity( Qt::CaseInsensitive );
  std::sort( defs.begin(), defs.end(), [&collator]( const EllipsoidDefinition & a, const EllipsoidDefinition & b )
  {
    return collator.compare( a.description, b.description ) < 0;
  } );
  if ( !sDisableCache )
  {
    *sDefinitionCache() = defs;
  }

  return defs;
}

QStringList QgsEllipsoidUtils::acronyms()
{
  QStringList result;
  const QList<QgsEllipsoidUtils::EllipsoidDefinition> defs = definitions();
  result.reserve( defs.size() );
  for ( const QgsEllipsoidUtils::EllipsoidDefinition &def : defs )
  {
    result << def.acronym;
  }
  return result;
}

QList<QgsCelestialBody> QgsEllipsoidUtils::celestialBodies()
{
  return QgsApplication::coordinateReferenceSystemRegistry()->celestialBodies();
}

void QgsEllipsoidUtils::invalidateCache( bool disableCache )
{
  const QgsReadWriteLocker locker1( *sEllipsoidCacheLock(), QgsReadWriteLocker::Write );
  const QgsReadWriteLocker locker2( *sDefinitionCacheLock(), QgsReadWriteLocker::Write );

  if ( !sDisableCache )
  {
    if ( disableCache )
      sDisableCache = true;
    sEllipsoidCache()->clear();
    sDefinitionCache()->clear();
  }
}
