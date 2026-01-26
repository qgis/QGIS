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
  // crs may be a compound crs, so get just the horizontal component first
  const QgsCoordinateReferenceSystem horizontalCrs = crs.horizontalCrs();
  if ( !horizontalCrs.isValid() )
    return Qgis::CoordinateOrder::XY;

  const QList< Qgis::CrsAxisDirection > axisList = horizontalCrs.axisOrdering();
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

QString QgsCoordinateReferenceSystemUtils::crsTypeToString( Qgis::CrsType type )
{
  switch ( type )
  {
    case Qgis::CrsType::Unknown:
      return QObject::tr( "Unknown" );
    case Qgis::CrsType::Other:
      return QObject::tr( "Other" );
    case Qgis::CrsType::Geodetic:
      return QObject::tr( "Geodetic" );
    case Qgis::CrsType::Geocentric:
      return QObject::tr( "Geocentric" );
    case Qgis::CrsType::Geographic2d:
      return QObject::tr( "Geographic (2D)" );
    case Qgis::CrsType::Geographic3d:
      return QObject::tr( "Geographic (3D)" );
    case Qgis::CrsType::Vertical:
      return QObject::tr( "Vertical" );
    case Qgis::CrsType::Projected:
      return QObject::tr( "Projected" );
    case Qgis::CrsType::Compound:
      return QObject::tr( "Compound" );
    case Qgis::CrsType::Temporal:
      return QObject::tr( "Temporal" );
    case Qgis::CrsType::Engineering:
      return QObject::tr( "Engineering" );
    case Qgis::CrsType::Bound:
      return QObject::tr( "Bound" );
    case Qgis::CrsType::DerivedProjected:
      return QObject::tr( "Derived projected" );
  }
  return QString();
}

QString QgsCoordinateReferenceSystemUtils::translateProjection( const QString &projection )
{
  if ( projection == "adams_ws2"_L1 )
    return QObject::tr( "Adams World in a Square II" );
  if ( projection == "aea"_L1 )
    return QObject::tr( "Albers Equal Area" );
  if ( projection == "aeqd"_L1 )
    return QObject::tr( "Azimuthal Equidistant" );
  if ( projection == "airy"_L1 )
    return QObject::tr( "Airy" );
  if ( projection == "aitoff"_L1 )
    return QObject::tr( "Aitoff" );
  if ( projection == "alsk"_L1 )
    return QObject::tr( "Modified Stererographics of Alaska" );
  if ( projection == "apian"_L1 )
    return QObject::tr( "Apian Globular I" );
  if ( projection == "august"_L1 )
    return QObject::tr( "August Epicycloidal" );
  if ( projection == "bacon"_L1 )
    return QObject::tr( "Bacon Globular" );
  if ( projection == "bipc"_L1 )
    return QObject::tr( "Bipolar Conic of Western Hemisphere" );
  if ( projection == "boggs"_L1 )
    return QObject::tr( "Boggs Eumorphic" );
  if ( projection == "bonne"_L1 )
    return QObject::tr( "Bonne (Werner lat_1=90)" );
  if ( projection == "cass"_L1 )
    return QObject::tr( "Cassini" );
  if ( projection == "cc"_L1 )
    return QObject::tr( "Central Cylindrical" );
  if ( projection == "cea"_L1 )
    return QObject::tr( "Equal Area Cylindrical" );
  if ( projection == "chamb"_L1 )
    return QObject::tr( "Chamberlin Trimetric" );
  if ( projection == "col_urban"_L1 )
    return QObject::tr( "Colombia Urban" );
  if ( projection == "collg"_L1 )
    return QObject::tr( "Collignon" );
  if ( projection == "comill"_L1 )
    return QObject::tr( "Compact Miller" );
  if ( projection == "crast"_L1 )
    return QObject::tr( "Craster Parabolic (Putnins P4)" );
  if ( projection == "denoy"_L1 )
    return QObject::tr( "Denoyer Semi-Elliptical" );
  if ( projection == "eck1"_L1 )
    return QObject::tr( "Eckert I" );
  if ( projection == "eck2"_L1 )
    return QObject::tr( "Eckert II" );
  if ( projection == "eck3"_L1 )
    return QObject::tr( "Eckert III" );
  if ( projection == "eck4"_L1 )
    return QObject::tr( "Eckert IV" );
  if ( projection == "eck5"_L1 )
    return QObject::tr( "Eckert V" );
  if ( projection == "eck6"_L1 )
    return QObject::tr( "Eckert VI" );
  if ( projection == "eqc"_L1 )
    return QObject::tr( "Equidistant Cylindrical (Plate Carrée)" );
  if ( projection == "eqdc"_L1 )
    return QObject::tr( "Equidistant Conic" );
  if ( projection == "eqearth"_L1 )
    return QObject::tr( "Equal Earth" );
  if ( projection == "euler"_L1 )
    return QObject::tr( "Euler" );
  if ( projection == "fahey"_L1 )
    return QObject::tr( "Fahey" );
  if ( projection == "fouc"_L1 )
    return QObject::tr( "Foucaut" );
  if ( projection == "fouc_s"_L1 )
    return QObject::tr( "Foucaut Sinusoidal" );
  if ( projection == "gall"_L1 )
    return QObject::tr( "Gall (Gall Stereographic)" );
  if ( projection == "geocent"_L1 )
    return QObject::tr( "Geocentric" );
  if ( projection == "geos"_L1 )
    return QObject::tr( "Geostationary Satellite View" );
  if ( projection == "gins8"_L1 )
    return QObject::tr( "Ginsburg VIII (TsNIIGAiK)" );
  if ( projection == "gn_sinu"_L1 )
    return QObject::tr( "General Sinusoidal Series" );
  if ( projection == "gnom"_L1 )
    return QObject::tr( "Gnomonic" );
  if ( projection == "goode"_L1 )
    return QObject::tr( "Goode Homolosine" );
  if ( projection == "gs48"_L1 )
    return QObject::tr( "Modified Stererographics of 48 U.S." );
  if ( projection == "gs50"_L1 )
    return QObject::tr( "Modified Stererographics of 50 U.S." );
  if ( projection == "hammer"_L1 )
    return QObject::tr( "Hammer & Eckert-Greifendorff" );
  if ( projection == "hatano"_L1 )
    return QObject::tr( "Hatano Asymmetrical Equal Area" );
  if ( projection == "igh"_L1 )
    return QObject::tr( "Interrupted Goode Homolosine" );
  if ( projection == "igh_o"_L1 )
    return QObject::tr( "Interrupted Goode Homolosine (Oceanic View)" );
  if ( projection == "imw_p"_L1 )
    return QObject::tr( "International Map of the World Polyconic" );
  if ( projection == "kav5"_L1 )
    return QObject::tr( "Kavraisky V" );
  if ( projection == "kav7"_L1 )
    return QObject::tr( "Kavraisky VII" );
  if ( projection == "krovak"_L1 )
    return QObject::tr( "Krovak" );
  if ( projection == "labrd"_L1 )
    return QObject::tr( "Laborde" );
  if ( projection == "laea"_L1 )
    return QObject::tr( "Lambert Azimuthal Equal Area" );
  if ( projection == "lagrng"_L1 )
    return QObject::tr( "Lagrange" );
  if ( projection == "larr"_L1 )
    return QObject::tr( "Larrivee" );
  if ( projection == "lask"_L1 )
    return QObject::tr( "Laskowski" );
  if ( projection == "longlat"_L1 )
    return QObject::tr( "Long/lat (Geodetic Alias)" );
  if ( projection == "latlong"_L1 )
    return QObject::tr( "Lat/long (Geodetic Alias)" );
  if ( projection == "lcc"_L1 )
    return QObject::tr( "Lambert Conformal Conic" );
  if ( projection == "lcca"_L1 )
    return QObject::tr( "Lambert Conformal Conic Alternative" );
  if ( projection == "leac"_L1 )
    return QObject::tr( "Lambert Equal Area Conic" );
  if ( projection == "lee_os"_L1 )
    return QObject::tr( "Lee Oblated Stereographic" );
  if ( projection == "loxim"_L1 )
    return QObject::tr( "Loximuthal" );
  if ( projection == "lsat"_L1 ) //#spellok
    return QObject::tr( "Space Oblique for LANDSAT" );
  if ( projection == "mbt_s"_L1 )
    return QObject::tr( "McBryde-Thomas Flat-Polar Sine (No. 1)" );
  if ( projection == "mbt_fps"_L1 )
    return QObject::tr( "McBryde-Thomas Flat-Pole Sine (No. 2)" );
  if ( projection == "mbtfpp"_L1 )
    return QObject::tr( "McBride-Thomas Flat-Polar Parabolic" );
  if ( projection == "mbtfpq"_L1 )
    return QObject::tr( "McBryde-Thomas Flat-Polar Quartic" );
  if ( projection == "mbtfps"_L1 )
    return QObject::tr( "McBryde-Thomas Flat-Polar Sinusoidal" );
  if ( projection == "merc"_L1 )
    return QObject::tr( "Mercator" );
  if ( projection == "mil_os"_L1 )
    return QObject::tr( "Miller Oblated Stereographic" );
  if ( projection == "mill"_L1 )
    return QObject::tr( "Miller Cylindrical" );
  if ( projection == "mod_krovak"_L1 )
    return QObject::tr( "Modified Krovak" );
  if ( projection == "moll"_L1 )
    return QObject::tr( "Mollweide" );
  if ( projection == "murd1"_L1 )
    return QObject::tr( "Murdoch I" );
  if ( projection == "murd2"_L1 )
    return QObject::tr( "Murdoch II" );
  if ( projection == "murd3"_L1 )
    return QObject::tr( "Murdoch III" );
  if ( projection == "natearth"_L1 )
    return QObject::tr( "Natural Earth" );
  if ( projection == "natearth2"_L1 )
    return QObject::tr( "Natural Earth II" );
  if ( projection == "nell"_L1 )
    return QObject::tr( "Nell" );
  if ( projection == "nell_h"_L1 )
    return QObject::tr( "Nell-Hammer" );
  if ( projection == "nicol"_L1 )
    return QObject::tr( "Nicolosi Globular" );
  if ( projection == "nsper"_L1 )
    return QObject::tr( "Near-sided Perspective" );
  if ( projection == "nzmg"_L1 )
    return QObject::tr( "New Zealand Map Grid" );
  if ( projection == "ob_tran"_L1 )
    return QObject::tr( "General Oblique Transformation" );
  if ( projection == "ocea"_L1 )
    return QObject::tr( "Oblique Cylindrical Equal Area" );
  if ( projection == "oea"_L1 )
    return QObject::tr( "Oblated Equal Area" );
  if ( projection == "omerc"_L1 )
    return QObject::tr( "Oblique Mercator" );
  if ( projection == "ortel"_L1 )
    return QObject::tr( "Ortelius Oval" );
  if ( projection == "ortho"_L1 )
    return QObject::tr( "Orthographic" );
  if ( projection == "patterson"_L1 )
    return QObject::tr( "Patterson" );
  if ( projection == "pconic"_L1 )
    return QObject::tr( "Perspective Conic" );
  if ( projection == "peirce_q"_L1 )
    return QObject::tr( "Peirce Quincuncial" );
  if ( projection == "poly"_L1 )
    return QObject::tr( "Polyconic (American)" );
  if ( projection == "putp1"_L1 )
    return QObject::tr( "Putnins P1" );
  if ( projection == "putp2"_L1 )
    return QObject::tr( "Putnins P2" );
  if ( projection == "putp3"_L1 )
    return QObject::tr( "Putnins P3" );
  if ( projection == "putp3p"_L1 )
    return QObject::tr( "Putnins P3'" );
  if ( projection == "putp4p"_L1 )
    return QObject::tr( "Putnins P4'" );
  if ( projection == "putp5"_L1 )
    return QObject::tr( "Putnins P5" );
  if ( projection == "putp5p"_L1 )
    return QObject::tr( "Putnins P5'" );
  if ( projection == "putp6"_L1 )
    return QObject::tr( "Putnins P6" );
  if ( projection == "putp6p"_L1 )
    return QObject::tr( "Putnins P6'" );
  if ( projection == "qua_aut"_L1 )
    return QObject::tr( "Quartic Authalic" );
  if ( projection == "robin"_L1 )
    return QObject::tr( "Robinson" );
  if ( projection == "rouss"_L1 )
    return QObject::tr( "Roussilhe Stereographic" );
  if ( projection == "rpoly"_L1 )
    return QObject::tr( "Rectangular Polyconic" );
  if ( projection == "sinu"_L1 )
    return QObject::tr( "Sinusoidal (Sanson-Flamsteed)" );
  if ( projection == "spilhaus"_L1 )
    return QObject::tr( "Spilhaus" );
  if ( projection == "somerc"_L1 )
    return QObject::tr( "Swiss Oblique Mercator" );
  if ( projection == "stere"_L1 )
    return QObject::tr( "Stereographic" );
  if ( projection == "sterea"_L1 )
    return QObject::tr( "Oblique Stereographic Alternative" );
  if ( projection == "tcc"_L1 )
    return QObject::tr( "Transverse Central Cylindrical" );
  if ( projection == "tcea"_L1 )
    return QObject::tr( "Transverse Cylindrical Equal Area" );
  if ( projection == "times"_L1 )
    return QObject::tr( "Times" );
  if ( projection == "tissot"_L1 )
    return QObject::tr( "Tissot" );
  if ( projection == "tmerc"_L1 )
    return QObject::tr( "Transverse Mercator" );
  if ( projection == "tpeqd"_L1 )
    return QObject::tr( "Two Point Equidistant" );
  if ( projection == "tpers"_L1 )
    return QObject::tr( "Tilted Perspective" );
  if ( projection == "ups"_L1 )
    return QObject::tr( "Universal Polar Stereographic" );
  if ( projection == "urm5"_L1 )
    return QObject::tr( "Urmaev V" );
  if ( projection == "urmfps"_L1 )
    return QObject::tr( "Urmaev Flat-Polar Sinusoidal" );
  if ( projection == "utm"_L1 )
    return QObject::tr( "Universal Transverse Mercator (UTM)" );
  if ( projection == "vandg"_L1 )
    return QObject::tr( "van der Grinten (I)" );
  if ( projection == "vandg2"_L1 )
    return QObject::tr( "van der Grinten II" );
  if ( projection == "vandg3"_L1 )
    return QObject::tr( "van der Grinten III" );
  if ( projection == "vandg4"_L1 )
    return QObject::tr( "van der Grinten IV" );
  if ( projection == "vitk1"_L1 )
    return QObject::tr( "Vitkovsky I" );
  if ( projection == "wag1"_L1 )
    return QObject::tr( "Wagner I (Kavraisky VI)" );
  if ( projection == "wag2"_L1 )
    return QObject::tr( "Wagner II" );
  if ( projection == "wag3"_L1 )
    return QObject::tr( "Wagner III" );
  if ( projection == "wag4"_L1 )
    return QObject::tr( "Wagner IV" );
  if ( projection == "wag5"_L1 )
    return QObject::tr( "Wagner V" );
  if ( projection == "wag6"_L1 )
    return QObject::tr( "Wagner VI" );
  if ( projection == "wag7"_L1 )
    return QObject::tr( "Wagner VII" );
  if ( projection == "weren"_L1 )
    return QObject::tr( "Werenskiold I" );
  if ( projection == "wink1"_L1 )
    return QObject::tr( "Winkel I" );
  if ( projection == "wink2"_L1 )
    return QObject::tr( "Winkel II" );
  if ( projection == "wintri"_L1 )
    return QObject::tr( "Winkel Tripel" );
  if ( projection == "gstmerc"_L1 )
    return QObject::tr( "Gauss-Schreiber" );
  return QString();
}
