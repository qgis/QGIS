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

#include <QString>
#include <QSet>
#include <QRegularExpression>

#if PROJ_VERSION_MAJOR>=6
#include <proj.h>
#else
#include <proj_api.h>
#endif


#if defined(USE_THREAD_LOCAL) && !defined(Q_OS_WIN)
thread_local QgsProjContext QgsProjContext::sProjContext;
#else
QThreadStorage< QgsProjContext * > QgsProjContext::sProjContext;
#endif

QgsProjContext::QgsProjContext()
{
#if PROJ_VERSION_MAJOR>=6
  mContext = proj_context_create();
#else
  mContext = pj_ctx_alloc();
#endif
}

QgsProjContext::~QgsProjContext()
{
#if PROJ_VERSION_MAJOR>=6
  // Call removeFromCacheObjectsBelongingToCurrentThread() before
  // destroying the context
  QgsCoordinateTransform::removeFromCacheObjectsBelongingToCurrentThread( mContext );
  proj_context_destroy( mContext );
#else
  pj_ctx_free( mContext );
#endif
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

#if PROJ_VERSION_MAJOR>=6
void QgsProjUtils::ProjPJDeleter::operator()( PJ *object )
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


QgsProjUtils::proj_pj_unique_ptr QgsProjUtils::crsToSingleCrs( const PJ *crs )
{
  if ( !crs )
    return nullptr;

  PJ_CONTEXT *context = QgsProjContext::get();
  switch ( proj_get_type( crs ) )
  {
    case PJ_TYPE_BOUND_CRS:
      return QgsProjUtils::proj_pj_unique_ptr( proj_get_source_crs( context, crs ) );

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

    // maybe other types to handle??

    default:
      return QgsProjUtils::proj_pj_unique_ptr( proj_clone( context, crs ) );
  }

#ifndef _MSC_VER  // unreachable
  return nullptr;
#endif
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
  static QRegularExpression sRegex( QStringLiteral( "\\+(?:nad)?grids=(.*?)\\s" ) );

  QList< QgsDatumTransform::GridDetails > grids;
  QRegularExpressionMatchIterator matches = sRegex.globalMatch( proj );
  while ( matches.hasNext() )
  {
    const QRegularExpressionMatch match = matches.next();
    const QString gridName = match.captured( 1 );
    QgsDatumTransform::GridDetails grid;
    grid.shortName = gridName;
#if PROJ_VERSION_MAJOR>6 || (PROJ_VERSION_MAJOR==6 && PROJ_VERSION_MINOR>=2)
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
#endif
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

#endif

QStringList QgsProjUtils::searchPaths()
{
#if PROJ_VERSION_MAJOR>=6
  const QString path( proj_info().searchpath );
  QStringList paths;
#if PROJ_VERSION_MINOR==1 && PROJ_VERSION_PATCH==0
  // -- see https://github.com/OSGeo/proj.4/pull/1497
  paths = path.split( ';' );
#else
#ifdef Q_OS_WIN
  paths = path.split( ';' );
#else
  paths = path.split( ':' );
#endif
#endif

  QSet<QString> existing;
  // thin out duplicates from paths -- see https://github.com/OSGeo/proj.4/pull/1498
  QStringList res;
  res.reserve( paths.count() );
  for ( const QString &p : qgis::as_const( paths ) )
  {
    if ( existing.contains( p ) )
      continue;

    existing.insert( p );
    res << p;
  }
  return res;
#else
  return QStringList();
#endif
}
