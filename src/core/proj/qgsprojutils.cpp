/***************************************************************************
                             qgsprojutils.h
                             -------------------
    begin                : March 2019
    copyright            : (C) 2019 by Nyall Dawson
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
#include "qgsprojutils.h"
#include "qgis.h"
#include "qgscoordinatetransform.h"
#include "qgsexception.h"
#include <QString>
#include <QSet>
#include <QRegularExpression>
#include <QDate>

#include <proj.h>
#include <proj_experimental.h>

#if defined(USE_THREAD_LOCAL) && !defined(Q_OS_WIN)
thread_local QgsProjContext QgsProjContext::sProjContext;
#else
QThreadStorage< QgsProjContext * > QgsProjContext::sProjContext;
#endif

QgsProjContext::QgsProjContext()
{
  mContext = proj_context_create();
}

QgsProjContext::~QgsProjContext()
{
  // Call removeFromCacheObjectsBelongingToCurrentThread() before
  // destroying the context
  QgsCoordinateTransform::removeFromCacheObjectsBelongingToCurrentThread( mContext );
  QgsCoordinateReferenceSystem::removeFromCacheObjectsBelongingToCurrentThread( mContext );
  proj_context_destroy( mContext );
}

PJ_CONTEXT *QgsProjContext::get()
{
#if defined(USE_THREAD_LOCAL) && !defined(Q_OS_WIN)
  return sProjContext.mContext;
#else
  PJ_CONTEXT *pContext = nullptr;
  if ( sProjContext.hasLocalData() )
  {
    pContext = sProjContext.localData()->mContext;
  }
  else
  {
    sProjContext.setLocalData( new QgsProjContext() );
    pContext = sProjContext.localData()->mContext;
  }
  return pContext;
#endif
}

void QgsProjUtils::ProjPJDeleter::operator()( PJ *object ) const
{
  proj_destroy( object );
}

bool QgsProjUtils::usesAngularUnit( const QString &projDef )
{
  const QString crsDef = QStringLiteral( "%1 +type=crs" ).arg( projDef );
  PJ_CONTEXT *context = QgsProjContext::get();
  QgsProjUtils::proj_pj_unique_ptr projSingleOperation( proj_create( context, crsDef.toUtf8().constData() ) );
  if ( !projSingleOperation )
    return false;

  QgsProjUtils::proj_pj_unique_ptr coordinateSystem( proj_crs_get_coordinate_system( context, projSingleOperation.get() ) );
  if ( !coordinateSystem )
    return false;

  const int axisCount = proj_cs_get_axis_count( context, coordinateSystem.get() );
  if ( axisCount > 0 )
  {
    const char *outUnitAuthName = nullptr;
    const char *outUnitAuthCode = nullptr;
    // Read only first axis
    proj_cs_get_axis_info( context, coordinateSystem.get(), 0,
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
      if ( proj_uom_get_info_from_database( context, outUnitAuthName, outUnitAuthCode, nullptr, nullptr, &unitCategory ) )
      {
        return QString( unitCategory ).compare( QLatin1String( "angular" ), Qt::CaseInsensitive ) == 0;
      }
    }
  }
  return false;
}

bool QgsProjUtils::axisOrderIsSwapped( const PJ *crs )
{
  //ported from https://github.com/pramsey/postgis/blob/7ecf6839c57a838e2c8540001a3cd35b78a730db/liblwgeom/lwgeom_transform.c#L299
  if ( !crs )
    return false;

  PJ_CONTEXT *context = QgsProjContext::get();
  QgsProjUtils::proj_pj_unique_ptr pjCs( proj_crs_get_coordinate_system( context, crs ) );
  if ( !pjCs )
    return false;

  const int axisCount = proj_cs_get_axis_count( context, pjCs.get() );
  if ( axisCount > 0 )
  {
    const char *outDirection = nullptr;
    // Read only first axis, see if it is degrees / north

    proj_cs_get_axis_info( context, pjCs.get(), 0,
                           nullptr,
                           nullptr,
                           &outDirection,
                           nullptr,
                           nullptr,
                           nullptr,
                           nullptr
                         );
    return QString( outDirection ).compare( QLatin1String( "north" ), Qt::CaseInsensitive ) == 0;
  }
  return false;
}

bool QgsProjUtils::isDynamic( const PJ *crs )
{
  // ported from GDAL OGRSpatialReference::IsDynamic()
  bool isDynamic = false;
  PJ_CONTEXT *context = QgsProjContext::get();

  // prefer horizontal crs if possible
  proj_pj_unique_ptr candidate = crsToHorizontalCrs( crs );
  if ( !crs )
    candidate = unboundCrs( crs );

  proj_pj_unique_ptr datum( candidate ? proj_crs_get_datum( context, candidate.get() ) : nullptr );
  if ( datum )
  {
    const PJ_TYPE type = proj_get_type( datum.get() );
    isDynamic = type == PJ_TYPE_DYNAMIC_GEODETIC_REFERENCE_FRAME ||
                type == PJ_TYPE_DYNAMIC_VERTICAL_REFERENCE_FRAME;
    if ( !isDynamic )
    {
      const QString authName( proj_get_id_auth_name( datum.get(), 0 ) );
      const QString code( proj_get_id_code( datum.get(), 0 ) );
      if ( authName == QLatin1String( "EPSG" ) && code == QLatin1String( "6326" ) )
      {
        isDynamic = true;
      }
    }
  }
  else
  {
    proj_pj_unique_ptr ensemble( candidate ? proj_crs_get_datum_ensemble( context, candidate.get() ) : nullptr );
    if ( ensemble )
    {
      proj_pj_unique_ptr member( proj_datum_ensemble_get_member( context, ensemble.get(), 0 ) );
      if ( member )
      {
        const PJ_TYPE type = proj_get_type( member.get() );
        isDynamic = type == PJ_TYPE_DYNAMIC_GEODETIC_REFERENCE_FRAME ||
                    type == PJ_TYPE_DYNAMIC_VERTICAL_REFERENCE_FRAME;
      }
    }
  }
  return isDynamic;
}

QgsProjUtils::proj_pj_unique_ptr QgsProjUtils::crsToHorizontalCrs( const PJ *crs )
{
  if ( !crs )
    return nullptr;

  PJ_CONTEXT *context = QgsProjContext::get();
  switch ( proj_get_type( crs ) )
  {
    case PJ_TYPE_COMPOUND_CRS:
    {
      int i = 0;
      QgsProjUtils::proj_pj_unique_ptr res( proj_crs_get_sub_crs( context, crs, i ) );
      while ( res && ( proj_get_type( res.get() ) == PJ_TYPE_VERTICAL_CRS || proj_get_type( res.get() ) == PJ_TYPE_TEMPORAL_CRS ) )
      {
        i++;
        res.reset( proj_crs_get_sub_crs( context, crs, i ) );
      }
      return res;
    }

    case PJ_TYPE_VERTICAL_CRS:
      return nullptr;

    // maybe other types to handle??

    default:
      return unboundCrs( crs );
  }

#ifndef _MSC_VER  // unreachable
  return nullptr;
#endif
}

QgsProjUtils::proj_pj_unique_ptr QgsProjUtils::crsToVerticalCrs( const PJ *crs )
{
  if ( !crs )
    return nullptr;

  PJ_CONTEXT *context = QgsProjContext::get();
  switch ( proj_get_type( crs ) )
  {
    case PJ_TYPE_COMPOUND_CRS:
    {
      int i = 0;
      QgsProjUtils::proj_pj_unique_ptr res( proj_crs_get_sub_crs( context, crs, i ) );
      while ( res && ( proj_get_type( res.get() ) != PJ_TYPE_VERTICAL_CRS ) )
      {
        i++;
        res.reset( proj_crs_get_sub_crs( context, crs, i ) );
      }
      return res;
    }

    case PJ_TYPE_VERTICAL_CRS:
      return QgsProjUtils::proj_pj_unique_ptr( proj_clone( context, crs ) );

    // maybe other types to handle??

    default:
      return nullptr;
  }

  BUILTIN_UNREACHABLE
}

QgsProjUtils::proj_pj_unique_ptr QgsProjUtils::unboundCrs( const PJ *crs )
{
  if ( !crs )
    return nullptr;

  PJ_CONTEXT *context = QgsProjContext::get();
  switch ( proj_get_type( crs ) )
  {
    case PJ_TYPE_BOUND_CRS:
      return QgsProjUtils::proj_pj_unique_ptr( proj_get_source_crs( context, crs ) );

    // maybe other types to handle??

    default:
      return QgsProjUtils::proj_pj_unique_ptr( proj_clone( context, crs ) );
  }

#ifndef _MSC_VER  // unreachable
  return nullptr;
#endif
}

QgsProjUtils::proj_pj_unique_ptr QgsProjUtils::crsToDatumEnsemble( const PJ *crs )
{
  if ( !crs )
    return nullptr;

#if PROJ_VERSION_MAJOR>=8
  PJ_CONTEXT *context = QgsProjContext::get();
  QgsProjUtils::proj_pj_unique_ptr candidate = crsToHorizontalCrs( crs );
  if ( !candidate ) // purely vertical CRS
    candidate = unboundCrs( crs );

  if ( !candidate )
    return nullptr;

  return QgsProjUtils::proj_pj_unique_ptr( proj_crs_get_datum_ensemble( context, candidate.get() ) );
#else
  throw QgsNotSupportedException( QObject::tr( "Calculating datum ensembles requires a QGIS build based on PROJ 8.0 or later" ) );
#endif
}

QgsProjUtils::proj_pj_unique_ptr QgsProjUtils::createCompoundCrs( const PJ *horizontalCrs, const PJ *verticalCrs )
{
  if ( !horizontalCrs || !verticalCrs )
    return nullptr;

  // const cast here is for compatibility with proj < 9.5
  return QgsProjUtils::proj_pj_unique_ptr( proj_create_compound_crs( QgsProjContext::get(),
         nullptr,
         const_cast< PJ *>( horizontalCrs ),
         const_cast< PJ * >( verticalCrs ) ) );
}

bool QgsProjUtils::identifyCrs( const PJ *crs, QString &authName, QString &authCode, IdentifyFlags flags )
{
  authName.clear();
  authCode.clear();

  if ( !crs )
    return false;

  int *confidence = nullptr;
  if ( PJ_OBJ_LIST *crsList = proj_identify( QgsProjContext::get(), crs, nullptr, nullptr, &confidence ) )
  {
    const int count = proj_list_get_count( crsList );
    int bestConfidence = 0;
    QgsProjUtils::proj_pj_unique_ptr matchedCrs;
    for ( int i = 0; i < count; ++i )
    {
      if ( confidence[i] >= bestConfidence )
      {
        QgsProjUtils::proj_pj_unique_ptr candidateCrs( proj_list_get( QgsProjContext::get(), crsList, i ) );
        switch ( proj_get_type( candidateCrs.get() ) )
        {
          case PJ_TYPE_BOUND_CRS:
            // proj_identify also matches bound CRSes to the source CRS. But they are not the same as the source CRS, so we don't
            // consider them a candidate for a match here (depending on the identify flags, that is!)
            if ( flags & FlagMatchBoundCrsToUnderlyingSourceCrs )
              break;
            else
              continue;

          default:
            break;
        }

        candidateCrs = QgsProjUtils::unboundCrs( candidateCrs.get() );
        const QString authName( proj_get_id_auth_name( candidateCrs.get(), 0 ) );
        // if a match is identical confidence, we prefer EPSG codes for compatibility with earlier qgis conversions
        if ( confidence[i] > bestConfidence || ( confidence[i] == bestConfidence && authName == QLatin1String( "EPSG" ) ) )
        {
          bestConfidence = confidence[i];
          matchedCrs = std::move( candidateCrs );
        }
      }
    }
    proj_list_destroy( crsList );
    proj_int_list_destroy( confidence );
    if ( matchedCrs && bestConfidence >= 70 )
    {
      authName = QString( proj_get_id_auth_name( matchedCrs.get(), 0 ) );
      authCode = QString( proj_get_id_code( matchedCrs.get(), 0 ) );
    }
  }
  return !authName.isEmpty() && !authCode.isEmpty();
}

bool QgsProjUtils::coordinateOperationIsAvailable( const QString &projDef )
{
  if ( projDef.isEmpty() )
    return true;

  PJ_CONTEXT *context = QgsProjContext::get();
  QgsProjUtils::proj_pj_unique_ptr coordinateOperation( proj_create( context, projDef.toUtf8().constData() ) );
  if ( !coordinateOperation )
    return false;

  return static_cast< bool >( proj_coordoperation_is_instantiable( context, coordinateOperation.get() ) );
}

QList<QgsDatumTransform::GridDetails> QgsProjUtils::gridsUsed( const QString &proj )
{
  const thread_local QRegularExpression regex( QStringLiteral( "\\+(?:nad)?grids=(.*?)\\s" ) );

  QList< QgsDatumTransform::GridDetails > grids;
  QRegularExpressionMatchIterator matches = regex.globalMatch( proj );
  while ( matches.hasNext() )
  {
    const QRegularExpressionMatch match = matches.next();
    const QString gridName = match.captured( 1 );
    QgsDatumTransform::GridDetails grid;
    grid.shortName = gridName;
    const char *fullName = nullptr;
    const char *packageName = nullptr;
    const char *url = nullptr;
    int directDownload = 0;
    int openLicense = 0;
    int available = 0;
    proj_grid_get_info_from_database( QgsProjContext::get(), gridName.toUtf8().constData(), &fullName, &packageName, &url, &directDownload, &openLicense, &available );
    grid.fullName = QString( fullName );
    grid.packageName = QString( packageName );
    grid.url = QString( url );
    grid.directDownload = directDownload;
    grid.openLicense = openLicense;
    grid.isAvailable = available;
    grids.append( grid );
  }
  return grids;
}

#if 0
QStringList QgsProjUtils::nonAvailableGrids( const QString &projDef )
{
  if ( projDef.isEmpty() )
    return QStringList();

  PJ_CONTEXT *context = QgsProjContext::get();
  QgsProjUtils::proj_pj_unique_ptr op( proj_create( context, projDef.toUtf8().constData() ) ); < ---- - this always fails if grids are missing
  if ( !op )
      return QStringList();

  QStringList res;
  for ( int j = 0; j < proj_coordoperation_get_grid_used_count( context, op.get() ); ++j )
  {
    const char *shortName = nullptr;
    int isAvailable = 0;
    proj_coordoperation_get_grid_used( context, op.get(), j, &shortName, nullptr, nullptr, nullptr, nullptr, nullptr, &isAvailable );
    if ( !isAvailable )
      res << QString( shortName );
  }
  return res;
}
#endif

int QgsProjUtils::projVersionMajor()
{
  return PROJ_VERSION_MAJOR;
}

int QgsProjUtils::projVersionMinor()
{
  return PROJ_VERSION_MINOR;
}

QString QgsProjUtils::epsgRegistryVersion()
{
  PJ_CONTEXT *context = QgsProjContext::get();
  const char *version = proj_context_get_database_metadata( context, "EPSG.VERSION" );
  return QString( version );
}

QDate QgsProjUtils::epsgRegistryDate()
{
  PJ_CONTEXT *context = QgsProjContext::get();
  const char *date = proj_context_get_database_metadata( context, "EPSG.DATE" );
  return QDate::fromString( date, Qt::DateFormat::ISODate );
}

QString QgsProjUtils::esriDatabaseVersion()
{
  PJ_CONTEXT *context = QgsProjContext::get();
  const char *version = proj_context_get_database_metadata( context, "ESRI.VERSION" );
  return QString( version );
}

QDate QgsProjUtils::esriDatabaseDate()
{
  PJ_CONTEXT *context = QgsProjContext::get();
  const char *date = proj_context_get_database_metadata( context, "ESRI.DATE" );
  return QDate::fromString( date, Qt::DateFormat::ISODate );
}

QString QgsProjUtils::ignfDatabaseVersion()
{
  PJ_CONTEXT *context = QgsProjContext::get();
  const char *version = proj_context_get_database_metadata( context, "IGNF.VERSION" );
  return QString( version );
}

QDate QgsProjUtils::ignfDatabaseDate()
{
  PJ_CONTEXT *context = QgsProjContext::get();
  const char *date = proj_context_get_database_metadata( context, "IGNF.DATE" );
  return QDate::fromString( date, Qt::DateFormat::ISODate );
}

QStringList QgsProjUtils::searchPaths()
{
  const QString path( proj_info().searchpath );
  QStringList paths;
#ifdef Q_OS_WIN
  paths = path.split( ';' );
#else
  paths = path.split( ':' );
#endif

  QSet<QString> existing;
  // thin out duplicates from paths -- see https://github.com/OSGeo/proj.4/pull/1498
  QStringList res;
  res.reserve( paths.count() );
  for ( const QString &p : std::as_const( paths ) )
  {
    if ( existing.contains( p ) )
      continue;

    existing.insert( p );
    res << p;
  }
  return res;
}
