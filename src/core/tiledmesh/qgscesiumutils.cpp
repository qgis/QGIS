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

QgsBox3d QgsCesiumUtils::parseRegion( const QVariantList &region )
{
  if ( region.size() != 6 )
    return QgsBox3d();

  // The region property is an array of six numbers that define the bounding geographic region with
  // latitude, longitude, and height coordinates with the order [west, south, east, north, minimum height, maximum height].
  bool ok = false;
  const double west = region.at( 0 ).toDouble( &ok );
  if ( !ok )
    return QgsBox3d();
  const double south = region.at( 1 ).toDouble( &ok );
  if ( !ok )
    return QgsBox3d();
  const double east = region.at( 2 ).toDouble( &ok );
  if ( !ok )
    return QgsBox3d();
  const double north = region.at( 3 ).toDouble( &ok );
  if ( !ok )
    return QgsBox3d();
  const double minHeight = region.at( 4 ).toDouble( &ok );
  if ( !ok )
    return QgsBox3d();
  const double maxHeight = region.at( 5 ).toDouble( &ok );
  if ( !ok )
    return QgsBox3d();

  return QgsBox3d( west, south, minHeight, east, north, maxHeight );
}
