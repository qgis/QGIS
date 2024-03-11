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
  if ( projection == QLatin1String( "adams_ws2" ) )
    return QObject::tr( "Adams World in a Square II" );
  if ( projection == QLatin1String( "aea" ) )
    return QObject::tr( "Albers Equal Area" );
  if ( projection == QLatin1String( "aeqd" ) )
    return QObject::tr( "Azimuthal Equidistant" );
  if ( projection == QLatin1String( "airy" ) )
    return QObject::tr( "Airy" );
  if ( projection == QLatin1String( "aitoff" ) )
    return QObject::tr( "Aitoff" );
  if ( projection == QLatin1String( "alsk" ) )
    return QObject::tr( "Modified Stererographics of Alaska" );
  if ( projection == QLatin1String( "apian" ) )
    return QObject::tr( "Apian Globular I" );
  if ( projection == QLatin1String( "august" ) )
    return QObject::tr( "August Epicycloidal" );
  if ( projection == QLatin1String( "bacon" ) )
    return QObject::tr( "Bacon Globular" );
  if ( projection == QLatin1String( "bipc" ) )
    return QObject::tr( "Bipolar Conic of Western Hemisphere" );
  if ( projection == QLatin1String( "boggs" ) )
    return QObject::tr( "Boggs Eumorphic" );
  if ( projection == QLatin1String( "bonne" ) )
    return QObject::tr( "Bonne (Werner lat_1=90)" );
  if ( projection == QLatin1String( "cass" ) )
    return QObject::tr( "Cassini" );
  if ( projection == QLatin1String( "cc" ) )
    return QObject::tr( "Central Cylindrical" );
  if ( projection == QLatin1String( "cea" ) )
    return QObject::tr( "Equal Area Cylindrical" );
  if ( projection == QLatin1String( "chamb" ) )
    return QObject::tr( "Chamberlin Trimetric" );
  if ( projection == QLatin1String( "col_urban" ) )
    return QObject::tr( "Colombia Urban" );
  if ( projection == QLatin1String( "collg" ) )
    return QObject::tr( "Collignon" );
  if ( projection == QLatin1String( "comill" ) )
    return QObject::tr( "Compact Miller" );
  if ( projection == QLatin1String( "crast" ) )
    return QObject::tr( "Craster Parabolic (Putnins P4)" );
  if ( projection == QLatin1String( "denoy" ) )
    return QObject::tr( "Denoyer Semi-Elliptical" );
  if ( projection == QLatin1String( "eck1" ) )
    return QObject::tr( "Eckert I" );
  if ( projection == QLatin1String( "eck2" ) )
    return QObject::tr( "Eckert II" );
  if ( projection == QLatin1String( "eck3" ) )
    return QObject::tr( "Eckert III" );
  if ( projection == QLatin1String( "eck4" ) )
    return QObject::tr( "Eckert IV" );
  if ( projection == QLatin1String( "eck5" ) )
    return QObject::tr( "Eckert V" );
  if ( projection == QLatin1String( "eck6" ) )
    return QObject::tr( "Eckert VI" );
  if ( projection == QLatin1String( "eqc" ) )
    return QObject::tr( "Equidistant Cylindrical (Plate Carrée)" );
  if ( projection == QLatin1String( "eqdc" ) )
    return QObject::tr( "Equidistant Conic" );
  if ( projection == QLatin1String( "eqearth" ) )
    return QObject::tr( "Equal Earth" );
  if ( projection == QLatin1String( "euler" ) )
    return QObject::tr( "Euler" );
  if ( projection == QLatin1String( "fahey" ) )
    return QObject::tr( "Fahey" );
  if ( projection == QLatin1String( "fouc" ) )
    return QObject::tr( "Foucaut" );
  if ( projection == QLatin1String( "fouc_s" ) )
    return QObject::tr( "Foucaut Sinusoidal" );
  if ( projection == QLatin1String( "gall" ) )
    return QObject::tr( "Gall (Gall Stereographic)" );
  if ( projection == QLatin1String( "geocent" ) )
    return QObject::tr( "Geocentric" );
  if ( projection == QLatin1String( "geos" ) )
    return QObject::tr( "Geostationary Satellite View" );
  if ( projection == QLatin1String( "gins8" ) )
    return QObject::tr( "Ginsburg VIII (TsNIIGAiK)" );
  if ( projection == QLatin1String( "gn_sinu" ) )
    return QObject::tr( "General Sinusoidal Series" );
  if ( projection == QLatin1String( "gnom" ) )
    return QObject::tr( "Gnomonic" );
  if ( projection == QLatin1String( "goode" ) )
    return QObject::tr( "Goode Homolosine" );
  if ( projection == QLatin1String( "gs48" ) )
    return QObject::tr( "Modified Stererographics of 48 U.S." );
  if ( projection == QLatin1String( "gs50" ) )
    return QObject::tr( "Modified Stererographics of 50 U.S." );
  if ( projection == QLatin1String( "hammer" ) )
    return QObject::tr( "Hammer & Eckert-Greifendorff" );
  if ( projection == QLatin1String( "hatano" ) )
    return QObject::tr( "Hatano Asymmetrical Equal Area" );
  if ( projection == QLatin1String( "igh" ) )
    return QObject::tr( "Interrupted Goode Homolosine" );
  if ( projection == QLatin1String( "igh_o" ) )
    return QObject::tr( "Interrupted Goode Homolosine (Oceanic View)" );
  if ( projection == QLatin1String( "imw_p" ) )
    return QObject::tr( "International Map of the World Polyconic" );
  if ( projection == QLatin1String( "kav5" ) )
    return QObject::tr( "Kavraisky V" );
  if ( projection == QLatin1String( "kav7" ) )
    return QObject::tr( "Kavraisky VII" );
  if ( projection == QLatin1String( "krovak" ) )
    return QObject::tr( "Krovak" );
  if ( projection == QLatin1String( "labrd" ) )
    return QObject::tr( "Laborde" );
  if ( projection == QLatin1String( "laea" ) )
    return QObject::tr( "Lambert Azimuthal Equal Area" );
  if ( projection == QLatin1String( "lagrng" ) )
    return QObject::tr( "Lagrange" );
  if ( projection == QLatin1String( "larr" ) )
    return QObject::tr( "Larrivee" );
  if ( projection == QLatin1String( "lask" ) )
    return QObject::tr( "Laskowski" );
  if ( projection == QLatin1String( "longlat" ) )
    return QObject::tr( "Long/lat (Geodetic Alias)" );
  if ( projection == QLatin1String( "latlong" ) )
    return QObject::tr( "Lat/long (Geodetic Alias)" );
  if ( projection == QLatin1String( "lcc" ) )
    return QObject::tr( "Lambert Conformal Conic" );
  if ( projection == QLatin1String( "lcca" ) )
    return QObject::tr( "Lambert Conformal Conic Alternative" );
  if ( projection == QLatin1String( "leac" ) )
    return QObject::tr( "Lambert Equal Area Conic" );
  if ( projection == QLatin1String( "lee_os" ) )
    return QObject::tr( "Lee Oblated Stereographic" );
  if ( projection == QLatin1String( "loxim" ) )
    return QObject::tr( "Loximuthal" );
  if ( projection == QLatin1String( "lsat" ) ) //#spellok
    return QObject::tr( "Space Oblique for LANDSAT" );
  if ( projection == QLatin1String( "mbt_s" ) )
    return QObject::tr( "McBryde-Thomas Flat-Polar Sine (No. 1)" );
  if ( projection == QLatin1String( "mbt_fps" ) )
    return QObject::tr( "McBryde-Thomas Flat-Pole Sine (No. 2)" );
  if ( projection == QLatin1String( "mbtfpp" ) )
    return QObject::tr( "McBride-Thomas Flat-Polar Parabolic" );
  if ( projection == QLatin1String( "mbtfpq" ) )
    return QObject::tr( "McBryde-Thomas Flat-Polar Quartic" );
  if ( projection == QLatin1String( "mbtfps" ) )
    return QObject::tr( "McBryde-Thomas Flat-Polar Sinusoidal" );
  if ( projection == QLatin1String( "merc" ) )
    return QObject::tr( "Mercator" );
  if ( projection == QLatin1String( "mil_os" ) )
    return QObject::tr( "Miller Oblated Stereographic" );
  if ( projection == QLatin1String( "mill" ) )
    return QObject::tr( "Miller Cylindrical" );
  if ( projection == QLatin1String( "mod_krovak" ) )
    return QObject::tr( "Modified Krovak" );
  if ( projection == QLatin1String( "moll" ) )
    return QObject::tr( "Mollweide" );
  if ( projection == QLatin1String( "murd1" ) )
    return QObject::tr( "Murdoch I" );
  if ( projection == QLatin1String( "murd2" ) )
    return QObject::tr( "Murdoch II" );
  if ( projection == QLatin1String( "murd3" ) )
    return QObject::tr( "Murdoch III" );
  if ( projection == QLatin1String( "natearth" ) )
    return QObject::tr( "Natural Earth" );
  if ( projection == QLatin1String( "natearth2" ) )
    return QObject::tr( "Natural Earth II" );
  if ( projection == QLatin1String( "nell" ) )
    return QObject::tr( "Nell" );
  if ( projection == QLatin1String( "nell_h" ) )
    return QObject::tr( "Nell-Hammer" );
  if ( projection == QLatin1String( "nicol" ) )
    return QObject::tr( "Nicolosi Globular" );
  if ( projection == QLatin1String( "nsper" ) )
    return QObject::tr( "Near-sided Perspective" );
  if ( projection == QLatin1String( "nzmg" ) )
    return QObject::tr( "New Zealand Map Grid" );
  if ( projection == QLatin1String( "ob_tran" ) )
    return QObject::tr( "General Oblique Transformation" );
  if ( projection == QLatin1String( "ocea" ) )
    return QObject::tr( "Oblique Cylindrical Equal Area" );
  if ( projection == QLatin1String( "oea" ) )
    return QObject::tr( "Oblated Equal Area" );
  if ( projection == QLatin1String( "omerc" ) )
    return QObject::tr( "Oblique Mercator" );
  if ( projection == QLatin1String( "ortel" ) )
    return QObject::tr( "Ortelius Oval" );
  if ( projection == QLatin1String( "ortho" ) )
    return QObject::tr( "Orthographic" );
  if ( projection == QLatin1String( "patterson" ) )
    return QObject::tr( "Patterson" );
  if ( projection == QLatin1String( "pconic" ) )
    return QObject::tr( "Perspective Conic" );
  if ( projection == QLatin1String( "peirce_q" ) )
    return QObject::tr( "Peirce Quincuncial" );
  if ( projection == QLatin1String( "poly" ) )
    return QObject::tr( "Polyconic (American)" );
  if ( projection == QLatin1String( "putp1" ) )
    return QObject::tr( "Putnins P1" );
  if ( projection == QLatin1String( "putp2" ) )
    return QObject::tr( "Putnins P2" );
  if ( projection == QLatin1String( "putp3" ) )
    return QObject::tr( "Putnins P3" );
  if ( projection == QLatin1String( "putp3p" ) )
    return QObject::tr( "Putnins P3'" );
  if ( projection == QLatin1String( "putp4p" ) )
    return QObject::tr( "Putnins P4'" );
  if ( projection == QLatin1String( "putp5" ) )
    return QObject::tr( "Putnins P5" );
  if ( projection == QLatin1String( "putp5p" ) )
    return QObject::tr( "Putnins P5'" );
  if ( projection == QLatin1String( "putp6" ) )
    return QObject::tr( "Putnins P6" );
  if ( projection == QLatin1String( "putp6p" ) )
    return QObject::tr( "Putnins P6'" );
  if ( projection == QLatin1String( "qua_aut" ) )
    return QObject::tr( "Quartic Authalic" );
  if ( projection == QLatin1String( "robin" ) )
    return QObject::tr( "Robinson" );
  if ( projection == QLatin1String( "rouss" ) )
    return QObject::tr( "Roussilhe Stereographic" );
  if ( projection == QLatin1String( "rpoly" ) )
    return QObject::tr( "Rectangular Polyconic" );
  if ( projection == QLatin1String( "sinu" ) )
    return QObject::tr( "Sinusoidal (Sanson-Flamsteed)" );
  if ( projection == QLatin1String( "somerc" ) )
    return QObject::tr( "Swiss Oblique Mercator" );
  if ( projection == QLatin1String( "stere" ) )
    return QObject::tr( "Stereographic" );
  if ( projection == QLatin1String( "sterea" ) )
    return QObject::tr( "Oblique Stereographic Alternative" );
  if ( projection == QLatin1String( "tcc" ) )
    return QObject::tr( "Transverse Central Cylindrical" );
  if ( projection == QLatin1String( "tcea" ) )
    return QObject::tr( "Transverse Cylindrical Equal Area" );
  if ( projection == QLatin1String( "times" ) )
    return QObject::tr( "Times" );
  if ( projection == QLatin1String( "tissot" ) )
    return QObject::tr( "Tissot" );
  if ( projection == QLatin1String( "tmerc" ) )
    return QObject::tr( "Transverse Mercator" );
  if ( projection == QLatin1String( "tpeqd" ) )
    return QObject::tr( "Two Point Equidistant" );
  if ( projection == QLatin1String( "tpers" ) )
    return QObject::tr( "Tilted Perspective" );
  if ( projection == QLatin1String( "ups" ) )
    return QObject::tr( "Universal Polar Stereographic" );
  if ( projection == QLatin1String( "urm5" ) )
    return QObject::tr( "Urmaev V" );
  if ( projection == QLatin1String( "urmfps" ) )
    return QObject::tr( "Urmaev Flat-Polar Sinusoidal" );
  if ( projection == QLatin1String( "utm" ) )
    return QObject::tr( "Universal Transverse Mercator (UTM)" );
  if ( projection == QLatin1String( "vandg" ) )
    return QObject::tr( "van der Grinten (I)" );
  if ( projection == QLatin1String( "vandg2" ) )
    return QObject::tr( "van der Grinten II" );
  if ( projection == QLatin1String( "vandg3" ) )
    return QObject::tr( "van der Grinten III" );
  if ( projection == QLatin1String( "vandg4" ) )
    return QObject::tr( "van der Grinten IV" );
  if ( projection == QLatin1String( "vitk1" ) )
    return QObject::tr( "Vitkovsky I" );
  if ( projection == QLatin1String( "wag1" ) )
    return QObject::tr( "Wagner I (Kavraisky VI)" );
  if ( projection == QLatin1String( "wag2" ) )
    return QObject::tr( "Wagner II" );
  if ( projection == QLatin1String( "wag3" ) )
    return QObject::tr( "Wagner III" );
  if ( projection == QLatin1String( "wag4" ) )
    return QObject::tr( "Wagner IV" );
  if ( projection == QLatin1String( "wag5" ) )
    return QObject::tr( "Wagner V" );
  if ( projection == QLatin1String( "wag6" ) )
    return QObject::tr( "Wagner VI" );
  if ( projection == QLatin1String( "wag7" ) )
    return QObject::tr( "Wagner VII" );
  if ( projection == QLatin1String( "weren" ) )
    return QObject::tr( "Werenskiold I" );
  if ( projection == QLatin1String( "wink1" ) )
    return QObject::tr( "Winkel I" );
  if ( projection == QLatin1String( "wink2" ) )
    return QObject::tr( "Winkel II" );
  if ( projection == QLatin1String( "wintri" ) )
    return QObject::tr( "Winkel Tripel" );
  if ( projection == QLatin1String( "gstmerc" ) )
    return QObject::tr( "Gauss-Schreiber" );
  return QString();
}
