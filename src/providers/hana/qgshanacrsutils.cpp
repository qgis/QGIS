/***************************************************************************
   qgshanacrsutils.cpp
   --------------------------------------
   Date      : 13-10-2019
   Copyright : (C) SAP SE
   Author    : Maxim Rylov
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#include "qgshanacrsutils.h"
#include "qgshanaexception.h"

#include <proj.h>
#include "qgsprojutils.h"

double QgsHanaCrsUtils::getAngularUnits( const QgsCoordinateReferenceSystem &crs )
{
  auto throwUnableToGetAngularUnits = []()
  {
    throw QgsHanaException( "Unable to retrieve angular units from a spatial reference system" );
  };

  PJ *pjCrs = crs.projObject();
  if ( !pjCrs )
    throwUnableToGetAngularUnits();
  PJ_CONTEXT *context = QgsProjContext::get();
  const QgsProjUtils::proj_pj_unique_ptr pjCoordinateSystem( proj_crs_get_coordinate_system( context, pjCrs ) );
  if ( !pjCoordinateSystem )
    throwUnableToGetAngularUnits();

  const int axisCount = proj_cs_get_axis_count( context, pjCoordinateSystem.get() );
  if ( axisCount <= 0 )
    throwUnableToGetAngularUnits();

  double factor;
  const bool ret = proj_cs_get_axis_info( context, pjCoordinateSystem.get(), 0,
                                          nullptr,
                                          nullptr,
                                          nullptr,
                                          &factor,
                                          nullptr,
                                          nullptr,
                                          nullptr );
  if ( !ret )
    throwUnableToGetAngularUnits();
  return factor;
}

bool QgsHanaCrsUtils::identifyCrs( const QgsCoordinateReferenceSystem &crs, QString &name, long &srid )
{
  QString authName;
  QString authCode;

  QStringList sl = crs.authid().split( ':' );
  if ( sl.length() != 2 )
    return false;
  authName = sl[0];
  authCode = sl[1];

  if ( authName.isEmpty() )
    return false;

  bool ok;
  const long id = authCode.toLong( &ok );
  if ( !ok )
    return false;

  name = authName;
  srid = id;
  return true;
}
