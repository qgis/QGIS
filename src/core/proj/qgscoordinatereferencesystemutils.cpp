/***************************************************************************
                             qgscoordinatereferencesystemutils.h
                             -------------------
    begin                : April 2022
    copyright            : (C) 202 by Nyall Dawson
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
#include "qgscoordinatereferencesystemutils.h"
#include "qgscoordinatereferencesystem.h"

Qgis::CoordinateOrder QgsCoordinateReferenceSystemUtils::defaultCoordinateOrderForCrs( const QgsCoordinateReferenceSystem &crs )
{
  const QList< Qgis::CrsAxisDirection > axisList = crs.axisOrdering();
  if ( axisList.size() < 2 )
    return Qgis::CoordinateOrder::XY;

  for ( Qgis::CrsAxisDirection axis : axisList )
  {
    // we're trying to map all the different possible values to just XY or YX, so excuse the coarseness!
    switch ( axis )
    {
      case Qgis::CrsAxisDirection::North:
      case Qgis::CrsAxisDirection::NorthNorthEast:
      case Qgis::CrsAxisDirection::SouthSouthEast:
      case Qgis::CrsAxisDirection::South:
      case Qgis::CrsAxisDirection::SouthSouthWest:
      case Qgis::CrsAxisDirection::NorthNorthWest:
      case Qgis::CrsAxisDirection::GeocentricY:
      case Qgis::CrsAxisDirection::DisplayUp:
      case Qgis::CrsAxisDirection::DisplayDown:
        return Qgis::CoordinateOrder::YX;

      case Qgis::CrsAxisDirection::NorthEast:
      case Qgis::CrsAxisDirection::EastNorthEast:
      case Qgis::CrsAxisDirection::East:
      case Qgis::CrsAxisDirection::EastSouthEast:
      case Qgis::CrsAxisDirection::SouthEast:
      case Qgis::CrsAxisDirection::SouthWest:
      case Qgis::CrsAxisDirection::WestSouthWest:
      case Qgis::CrsAxisDirection::West:
      case Qgis::CrsAxisDirection::WestNorthWest:
      case Qgis::CrsAxisDirection::NorthWest:
      case Qgis::CrsAxisDirection::GeocentricX:
      case Qgis::CrsAxisDirection::DisplayRight:
      case Qgis::CrsAxisDirection::DisplayLeft:
        return Qgis::CoordinateOrder::XY;

      case Qgis::CrsAxisDirection::GeocentricZ:
      case Qgis::CrsAxisDirection::Up:
      case Qgis::CrsAxisDirection::Down:
      case Qgis::CrsAxisDirection::Forward:
      case Qgis::CrsAxisDirection::Aft:
      case Qgis::CrsAxisDirection::Port:
      case Qgis::CrsAxisDirection::Starboard:
      case Qgis::CrsAxisDirection::Clockwise:
      case Qgis::CrsAxisDirection::CounterClockwise:
      case Qgis::CrsAxisDirection::ColumnPositive:
      case Qgis::CrsAxisDirection::ColumnNegative:
      case Qgis::CrsAxisDirection::RowPositive:
      case Qgis::CrsAxisDirection::RowNegative:
      case Qgis::CrsAxisDirection::Future:
      case Qgis::CrsAxisDirection::Past:
      case Qgis::CrsAxisDirection::Towards:
      case Qgis::CrsAxisDirection::AwayFrom:
      case Qgis::CrsAxisDirection::Unspecified:
        break;
    }
  }

  return Qgis::CoordinateOrder::XY;
}
