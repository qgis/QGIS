/***************************************************************************
                         qgscesiumutils.cpp
                         --------------------
    begin                : July 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ******************************************************************
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscesiumutils.h"
#include "nlohmann/json.hpp"
#include "qgsjsonutils.h"
#include "qgssphere.h"
#include "qgsorientedboundingbox.h"

QgsBox3d QgsCesiumUtils::parseRegion( const json &region )
{
  try
  {
    const double west = region[0].get<double>();
    const double south = region[1].get<double>();
    const double east = region[2].get<double>();
    const double north = region[3].get<double>();
    double minHeight = region[4].get<double>();
    double maxHeight = region[5].get<double>();
    return QgsBox3d( west, south, minHeight, east, north, maxHeight );
  }
  catch ( nlohmann::json::exception & )
  {
    return QgsBox3d();
  }
}

QgsBox3d QgsCesiumUtils::parseRegion( const QVariantList &region )
{
  if ( region.size() != 6 )
    return QgsBox3d();

  return parseRegion( QgsJsonUtils::jsonFromVariant( region ) );
}

QgsOrientedBoundingBox QgsCesiumUtils::parseBox( const json &box )
{
  if ( box.size() != 12 )
    return QgsOrientedBoundingBox();

  try
  {
    QgsOrientedBoundingBox res;
    for ( int i = 0; i < 3; ++i )
    {
      res.mCenter[i] = box[i].get<double>();
    }
    for ( int i = 0; i < 9; ++i )
    {
      res.mHalfAxes[i] = box[i + 3].get<double>();
    }
    return res;
  }
  catch ( nlohmann::json::exception & )
  {
    return QgsOrientedBoundingBox();
  }
}

QgsOrientedBoundingBox QgsCesiumUtils::parseBox( const QVariantList &box )
{
  if ( box.size() != 12 )
    return QgsOrientedBoundingBox();

  return parseBox( QgsJsonUtils::jsonFromVariant( box ) );
}

QgsSphere QgsCesiumUtils::parseSphere( const json &sphere )
{
  if ( sphere.size() != 4 )
    return QgsSphere();

  try
  {
    const double centerX = sphere[0].get<double>();
    const double centerY = sphere[1].get<double>();
    const double centerZ = sphere[2].get<double>();
    const double radius = sphere[3].get<double>();
    return QgsSphere( centerX, centerY, centerZ, radius );
  }
  catch ( nlohmann::json::exception & )
  {
    return QgsSphere();
  }
}

QgsSphere QgsCesiumUtils::parseSphere( const QVariantList &sphere )
{
  if ( sphere.size() != 4 )
    return QgsSphere();

  return parseSphere( QgsJsonUtils::jsonFromVariant( sphere ) );
}
