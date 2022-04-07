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

QString QgsCoordinateReferenceSystemUtils::axisDirectionToAbbreviatedString( Qgis::CrsAxisDirection axis )
{
  switch ( axis )
  {
    case Qgis::CrsAxisDirection::North:
      return QObject::tr( "N", "axis" );
    case Qgis::CrsAxisDirection::NorthNorthEast:
      return QObject::tr( "NNE", "axis" );
    case Qgis::CrsAxisDirection::SouthSouthEast:
      return QObject::tr( "SSE", "axis" );
    case Qgis::CrsAxisDirection::South:
      return QObject::tr( "S", "axis" );
    case Qgis::CrsAxisDirection::SouthSouthWest:
      return QObject::tr( "SSW", "axis" );
    case Qgis::CrsAxisDirection::NorthNorthWest:
      return QObject::tr( "NNW", "axis" );
    case Qgis::CrsAxisDirection::GeocentricY:
      return QObject::tr( "Y", "axis" );
    case Qgis::CrsAxisDirection::DisplayUp:
      return QObject::tr( "Up", "axis" );
    case Qgis::CrsAxisDirection::DisplayDown:
      return QObject::tr( "Down", "axis" );
    case Qgis::CrsAxisDirection::NorthEast:
      return QObject::tr( "NE", "axis" );
    case Qgis::CrsAxisDirection::EastNorthEast:
      return QObject::tr( "ENE", "axis" );
    case Qgis::CrsAxisDirection::East:
      return QObject::tr( "E", "axis" );
    case Qgis::CrsAxisDirection::EastSouthEast:
      return QObject::tr( "ESE", "axis" );
    case Qgis::CrsAxisDirection::SouthEast:
      return QObject::tr( "SE", "axis" );
    case Qgis::CrsAxisDirection::SouthWest:
      return QObject::tr( "SW", "axis" );
    case Qgis::CrsAxisDirection::WestSouthWest:
      return QObject::tr( "WSW", "axis" );
    case Qgis::CrsAxisDirection::West:
      return QObject::tr( "W", "axis" );
    case Qgis::CrsAxisDirection::WestNorthWest:
      return QObject::tr( "WNW", "axis" );
    case Qgis::CrsAxisDirection::NorthWest:
      return QObject::tr( "NW", "axis" );
    case Qgis::CrsAxisDirection::GeocentricX:
      return QObject::tr( "X", "axis" );
    case Qgis::CrsAxisDirection::DisplayRight:
      return QObject::tr( "Disp. R", "axis" );
    case Qgis::CrsAxisDirection::DisplayLeft:
      return QObject::tr( "Disp. L", "axis" );
    case Qgis::CrsAxisDirection::GeocentricZ:
      return QObject::tr( "Z", "axis" );
    case Qgis::CrsAxisDirection::Up:
      return QObject::tr( "U", "axis" );
    case Qgis::CrsAxisDirection::Down:
      return QObject::tr( "D", "axis" );
    case Qgis::CrsAxisDirection::Forward:
      return QObject::tr( "F", "axis" );
    case Qgis::CrsAxisDirection::Aft:
      return QObject::tr( "A", "axis" );
    case Qgis::CrsAxisDirection::Port:
      return QObject::tr( "P", "axis" );
    case Qgis::CrsAxisDirection::Starboard:
      return QObject::tr( "STBD", "axis" );
    case Qgis::CrsAxisDirection::Clockwise:
      return QObject::tr( "CW", "axis" );
    case Qgis::CrsAxisDirection::CounterClockwise:
      return QObject::tr( "CCW", "axis" );
    case Qgis::CrsAxisDirection::ColumnPositive:
      return QObject::tr( "C+", "axis" );
    case Qgis::CrsAxisDirection::ColumnNegative:
      return QObject::tr( "C-", "axis" );
    case Qgis::CrsAxisDirection::RowPositive:
      return QObject::tr( "R+", "axis" );
    case Qgis::CrsAxisDirection::RowNegative:
      return QObject::tr( "R-", "axis" );
    case Qgis::CrsAxisDirection::Future:
      return QObject::tr( "F", "axis" );
    case Qgis::CrsAxisDirection::Past:
      return QObject::tr( "P", "axis" );
    case Qgis::CrsAxisDirection::Towards:
      return QObject::tr( "T", "axis" );
    case Qgis::CrsAxisDirection::AwayFrom:
      return QObject::tr( "AF", "axis" );
    case Qgis::CrsAxisDirection::Unspecified:
      break;
  }

  return QString();
}
