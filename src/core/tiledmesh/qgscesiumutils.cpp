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
