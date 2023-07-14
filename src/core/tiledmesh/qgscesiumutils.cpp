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

  return QgsOrientedBoundingBox::fromJson( box );
}

QgsOrientedBoundingBox QgsCesiumUtils::parseBox( const QVariantList &box )
{
  if ( box.size() != 12 )
    return QgsOrientedBoundingBox();

  return QgsOrientedBoundingBox::fromJson( QgsJsonUtils::jsonFromVariant( box ) );
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

//
// QgsOrientedBoundingBox
//

QgsOrientedBoundingBox QgsOrientedBoundingBox::fromJson( const json &json )
{
  try
  {
    QgsOrientedBoundingBox res;
    for ( int i = 0; i < 3; ++i )
    {
      res.mCenter[i] = json[i].get<double>();
    }
    for ( int i = 0; i < 9; ++i )
    {
      res.mHalfAxes[i] = json[i + 3].get<double>();
    }
    return res;
  }
  catch ( nlohmann::json::exception & )
  {
    return QgsOrientedBoundingBox();
  }
}

QgsOrientedBoundingBox::QgsOrientedBoundingBox() = default;

QgsOrientedBoundingBox::QgsOrientedBoundingBox( const QList<double> &center, QList<double> &halfAxes )
{
  if ( center.size() == 3 )
  {
    mCenter[0] = center.at( 0 );
    mCenter[1] = center.at( 1 );
    mCenter[2] = center.at( 2 );
  }
  if ( halfAxes.size() == 9 )
  {
    for ( int i = 0; i < 9; ++i )
    {
      mHalfAxes[i] = halfAxes.at( i );
    }
  }
}

bool QgsOrientedBoundingBox::isNull() const
{
  return std::isnan( mCenter[0] ) || std::isnan( mCenter[1] ) || std::isnan( mCenter[2] );
}

QList< double > QgsOrientedBoundingBox::halfAxesList() const
{
  QList< double > res;
  res.reserve( 9 );
  for ( int i = 0; i < 9; ++i )
  {
    res.append( mHalfAxes[i] );
  }
  return res;
}

QgsBox3d QgsOrientedBoundingBox::extent() const
{
  const double extent[3]
  {
    std::fabs( mHalfAxes[0] ) + std::fabs( mHalfAxes[3] ) + std::fabs( mHalfAxes[6] ),
    std::fabs( mHalfAxes[1] ) + std::fabs( mHalfAxes[4] ) + std::fabs( mHalfAxes[7] ),
    std::fabs( mHalfAxes[2] ) + std::fabs( mHalfAxes[5] ) + std::fabs( mHalfAxes[8] ),
  };

  const double minX = mCenter[0] - extent[0];
  const double maxX = mCenter[0] + extent[0];
  const double minY = mCenter[1] - extent[1];
  const double maxY = mCenter[1] + extent[1];
  const double minZ = mCenter[2] - extent[2];
  const double maxZ = mCenter[2] + extent[2];

  return QgsBox3d( minX, minY, minZ, maxX, maxY, maxZ );
}
