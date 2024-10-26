/***************************************************************************
                         qgsunittypes.cpp
                         --------------
    begin                : February 2016
    copyright            : (C) 2016 by Nyall Dawson
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

#include "qgsunittypes.h"
#include "moc_qgsunittypes.cpp"
#include "qgis.h"

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in test_qgsunittypes.py.
 * See details in QEP #17
 ****************************************************************************/

QString QgsUnitTypes::encodeUnitType( Qgis::UnitType type )
{
  switch ( type )
  {
    case Qgis::UnitType::Distance:
      return QStringLiteral( "distance" );

    case Qgis::UnitType::Area:
      return QStringLiteral( "area" );

    case Qgis::UnitType::Volume:
      return QStringLiteral( "volume" );

    case Qgis::UnitType::Temporal:
      return QStringLiteral( "temporal" );

    case Qgis::UnitType::Unknown:
      return QStringLiteral( "<unknown>" );

  }
  return QString();
}

Qgis::UnitType QgsUnitTypes::decodeUnitType( const QString &string, bool *ok )
{
  const QString normalized = string.trimmed().toLower();

  if ( ok )
    *ok = true;

  if ( normalized == encodeUnitType( Qgis::UnitType::Distance ) )
    return Qgis::UnitType::Distance;
  if ( normalized == encodeUnitType( Qgis::UnitType::Area ) )
    return Qgis::UnitType::Area;
  if ( normalized == encodeUnitType( Qgis::UnitType::Volume ) )
    return Qgis::UnitType::Volume;
  if ( normalized == encodeUnitType( Qgis::UnitType::Temporal ) )
    return Qgis::UnitType::Temporal;
  if ( normalized == encodeUnitType( Qgis::UnitType::Unknown ) )
    return Qgis::UnitType::Unknown;

  if ( ok )
    *ok = false;

  return Qgis::UnitType::Unknown;
}

Qgis::DistanceUnitType QgsUnitTypes::unitType( Qgis::DistanceUnit unit )
{
  switch ( unit )
  {
    case Qgis::DistanceUnit::Meters:
    case Qgis::DistanceUnit::Feet:
    case Qgis::DistanceUnit::NauticalMiles:
    case Qgis::DistanceUnit::Yards:
    case Qgis::DistanceUnit::Miles:
    case Qgis::DistanceUnit::Kilometers:
    case Qgis::DistanceUnit::Centimeters:
    case Qgis::DistanceUnit::Millimeters:
    case Qgis::DistanceUnit::Inches:
    case Qgis::DistanceUnit::ChainsInternational:
    case Qgis::DistanceUnit::ChainsBritishBenoit1895A:
    case Qgis::DistanceUnit::ChainsBritishBenoit1895B:
    case Qgis::DistanceUnit::ChainsBritishSears1922Truncated:
    case Qgis::DistanceUnit::ChainsBritishSears1922:
    case Qgis::DistanceUnit::ChainsClarkes:
    case Qgis::DistanceUnit::ChainsUSSurvey:
    case Qgis::DistanceUnit::FeetBritish1865:
    case Qgis::DistanceUnit::FeetBritish1936:
    case Qgis::DistanceUnit::FeetBritishBenoit1895A:
    case Qgis::DistanceUnit::FeetBritishBenoit1895B:
    case Qgis::DistanceUnit::FeetBritishSears1922Truncated:
    case Qgis::DistanceUnit::FeetBritishSears1922:
    case Qgis::DistanceUnit::FeetClarkes:
    case Qgis::DistanceUnit::FeetGoldCoast:
    case Qgis::DistanceUnit::FeetIndian:
    case Qgis::DistanceUnit::FeetIndian1937:
    case Qgis::DistanceUnit::FeetIndian1962:
    case Qgis::DistanceUnit::FeetIndian1975:
    case Qgis::DistanceUnit::FeetUSSurvey:
    case Qgis::DistanceUnit::LinksInternational:
    case Qgis::DistanceUnit::LinksBritishBenoit1895A:
    case Qgis::DistanceUnit::LinksBritishBenoit1895B:
    case Qgis::DistanceUnit::LinksBritishSears1922Truncated:
    case Qgis::DistanceUnit::LinksBritishSears1922:
    case Qgis::DistanceUnit::LinksClarkes:
    case Qgis::DistanceUnit::LinksUSSurvey:
    case Qgis::DistanceUnit::YardsBritishBenoit1895A:
    case Qgis::DistanceUnit::YardsBritishBenoit1895B:
    case Qgis::DistanceUnit::YardsBritishSears1922Truncated:
    case Qgis::DistanceUnit::YardsBritishSears1922:
    case Qgis::DistanceUnit::YardsClarkes:
    case Qgis::DistanceUnit::YardsIndian:
    case Qgis::DistanceUnit::YardsIndian1937:
    case Qgis::DistanceUnit::YardsIndian1962:
    case Qgis::DistanceUnit::YardsIndian1975:
    case Qgis::DistanceUnit::MilesUSSurvey:
    case Qgis::DistanceUnit::Fathoms:
    case Qgis::DistanceUnit::MetersGermanLegal:
      return Qgis::DistanceUnitType::Standard;

    case Qgis::DistanceUnit::Degrees:
      return Qgis::DistanceUnitType::Geographic;

    case Qgis::DistanceUnit::Unknown:
      return Qgis::DistanceUnitType::Unknown;
  }
  return Qgis::DistanceUnitType::Unknown;
}

Qgis::DistanceUnitType QgsUnitTypes::unitType( Qgis::AreaUnit unit )
{
  switch ( unit )
  {
    case Qgis::AreaUnit::SquareMeters:
    case Qgis::AreaUnit::SquareKilometers:
    case Qgis::AreaUnit::SquareFeet:
    case Qgis::AreaUnit::SquareYards:
    case Qgis::AreaUnit::SquareMiles:
    case Qgis::AreaUnit::Hectares:
    case Qgis::AreaUnit::Acres:
    case Qgis::AreaUnit::SquareNauticalMiles:
    case Qgis::AreaUnit::SquareCentimeters:
    case Qgis::AreaUnit::SquareMillimeters:
    case Qgis::AreaUnit::SquareInches:
      return Qgis::DistanceUnitType::Standard;

    case Qgis::AreaUnit::SquareDegrees:
      return Qgis::DistanceUnitType::Geographic;

    case Qgis::AreaUnit::Unknown:
      return Qgis::DistanceUnitType::Unknown;
  }

  return Qgis::DistanceUnitType::Unknown;
}

QString QgsUnitTypes::encodeUnit( Qgis::DistanceUnit unit )
{
  switch ( unit )
  {
    case Qgis::DistanceUnit::Meters:
      return QStringLiteral( "meters" );

    case Qgis::DistanceUnit::Kilometers:
      return QStringLiteral( "km" );

    case Qgis::DistanceUnit::Feet:
      return QStringLiteral( "feet" );

    case Qgis::DistanceUnit::Yards:
      return QStringLiteral( "yd" );

    case Qgis::DistanceUnit::Miles:
      return QStringLiteral( "mi" );

    case Qgis::DistanceUnit::Degrees:
      return QStringLiteral( "degrees" );

    case Qgis::DistanceUnit::Unknown:
      return QStringLiteral( "<unknown>" );

    case Qgis::DistanceUnit::NauticalMiles:
      return QStringLiteral( "nautical miles" );

    case Qgis::DistanceUnit::Centimeters:
      return QStringLiteral( "cm" );

    case Qgis::DistanceUnit::Millimeters:
      return QStringLiteral( "mm" );

    case Qgis::DistanceUnit::Inches:
      return QStringLiteral( "in" );

    case Qgis::DistanceUnit::ChainsInternational:
      return QStringLiteral( "chain" );

    case Qgis::DistanceUnit::ChainsBritishBenoit1895A:
      return QStringLiteral( "chain british benoit b1895a" );

    case Qgis::DistanceUnit::ChainsBritishBenoit1895B:
      return QStringLiteral( "chain british benoit b1895b" );

    case Qgis::DistanceUnit::ChainsBritishSears1922Truncated:
      return QStringLiteral( "chain british sears 1922 truncated" );

    case Qgis::DistanceUnit::ChainsBritishSears1922:
      return QStringLiteral( "chain british sears 1922" );

    case Qgis::DistanceUnit::ChainsClarkes:
      return QStringLiteral( "chain clarkes" );

    case Qgis::DistanceUnit::ChainsUSSurvey:
      return QStringLiteral( "chain us survey" );

    case Qgis::DistanceUnit::FeetBritish1865:
      return QStringLiteral( "feet british 1865" );

    case Qgis::DistanceUnit::FeetBritish1936:
      return QStringLiteral( "feet british 1936" );

    case Qgis::DistanceUnit::FeetBritishBenoit1895A:
      return QStringLiteral( "feet british benoit 1895a" );

    case Qgis::DistanceUnit::FeetBritishBenoit1895B:
      return QStringLiteral( "feet british benoit 1895b" );

    case Qgis::DistanceUnit::FeetBritishSears1922Truncated:
      return QStringLiteral( "feet british sears 1922 truncated" );

    case Qgis::DistanceUnit::FeetBritishSears1922:
      return QStringLiteral( "feet british sears 1922" );

    case Qgis::DistanceUnit::FeetClarkes:
      return QStringLiteral( "feet clarkes" );

    case Qgis::DistanceUnit::FeetGoldCoast:
      return QStringLiteral( "feet gold coast" );

    case Qgis::DistanceUnit::FeetIndian:
      return QStringLiteral( "feet indian" );
    case Qgis::DistanceUnit::FeetIndian1937:
      return QStringLiteral( "feet indian 1937" );

    case Qgis::DistanceUnit::FeetIndian1962:
      return QStringLiteral( "feet indian 1962" );

    case Qgis::DistanceUnit::FeetIndian1975:
      return QStringLiteral( "feet indian 1975" );

    case Qgis::DistanceUnit::FeetUSSurvey:
      return QStringLiteral( "feet us survey" );

    case Qgis::DistanceUnit::LinksInternational:
      return QStringLiteral( "links" );

    case Qgis::DistanceUnit::LinksBritishBenoit1895A:
      return QStringLiteral( "links british benoit 1895a" );

    case Qgis::DistanceUnit::LinksBritishBenoit1895B:
      return QStringLiteral( "links british benoit 1895b" );

    case Qgis::DistanceUnit::LinksBritishSears1922Truncated:
      return QStringLiteral( "links british sears 1922 truncated" );

    case Qgis::DistanceUnit::LinksBritishSears1922:
      return QStringLiteral( "links british sears 1922" );

    case Qgis::DistanceUnit::LinksClarkes:
      return QStringLiteral( "links clarkes" );

    case Qgis::DistanceUnit::LinksUSSurvey:
      return QStringLiteral( "links us survey" );

    case Qgis::DistanceUnit::YardsBritishBenoit1895A:
      return QStringLiteral( "yards british benoit 1895a" );

    case Qgis::DistanceUnit::YardsBritishBenoit1895B:
      return QStringLiteral( "yards british benoit 1895b" );

    case Qgis::DistanceUnit::YardsBritishSears1922Truncated:
      return QStringLiteral( "yards british sears 1922 truncated" );

    case Qgis::DistanceUnit::YardsBritishSears1922:
      return QStringLiteral( "yards british sears 1922" );

    case Qgis::DistanceUnit::YardsClarkes:
      return QStringLiteral( "yards clarkes" );

    case Qgis::DistanceUnit::YardsIndian:
      return QStringLiteral( "yards indian" );

    case Qgis::DistanceUnit::YardsIndian1937:
      return QStringLiteral( "yards indian 1937" );

    case Qgis::DistanceUnit::YardsIndian1962:
      return QStringLiteral( "yards indian 1962" );

    case Qgis::DistanceUnit::YardsIndian1975:
      return QStringLiteral( "yards indian 1975" );

    case Qgis::DistanceUnit::MilesUSSurvey:
      return QStringLiteral( "miles us survey" );

    case Qgis::DistanceUnit::Fathoms:
      return QStringLiteral( "yards fathom" );

    case Qgis::DistanceUnit::MetersGermanLegal:
      return QStringLiteral( "german legal meters" );
  }
  return QString();
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in test_qgsunittypes.py.
 * See details in QEP #17
 ****************************************************************************/

Qgis::DistanceUnit QgsUnitTypes::decodeDistanceUnit( const QString &string, bool *ok )
{
  const QString normalized = string.trimmed().toLower();

  if ( ok )
    *ok = true;

  for ( const Qgis::DistanceUnit unit :
        {
          Qgis::DistanceUnit::Meters,
          Qgis::DistanceUnit::Feet,
          Qgis::DistanceUnit::Degrees,
          Qgis::DistanceUnit::NauticalMiles,
          Qgis::DistanceUnit::Kilometers,
          Qgis::DistanceUnit::Yards,
          Qgis::DistanceUnit::Miles,
          Qgis::DistanceUnit::Centimeters,
          Qgis::DistanceUnit::Millimeters,
          Qgis::DistanceUnit::Inches,
          Qgis::DistanceUnit::Unknown,

          Qgis::DistanceUnit::ChainsInternational,
          Qgis::DistanceUnit::ChainsBritishBenoit1895A,
          Qgis::DistanceUnit::ChainsBritishBenoit1895B,
          Qgis::DistanceUnit::ChainsBritishSears1922Truncated,
          Qgis::DistanceUnit::ChainsBritishSears1922,
          Qgis::DistanceUnit::ChainsClarkes,
          Qgis::DistanceUnit::ChainsUSSurvey,
          Qgis::DistanceUnit::FeetBritish1865,
          Qgis::DistanceUnit::FeetBritish1936,
          Qgis::DistanceUnit::FeetBritishBenoit1895A,
          Qgis::DistanceUnit::FeetBritishBenoit1895B,
          Qgis::DistanceUnit::FeetBritishSears1922Truncated,
          Qgis::DistanceUnit::FeetBritishSears1922,
          Qgis::DistanceUnit::FeetClarkes,
          Qgis::DistanceUnit::FeetGoldCoast,
          Qgis::DistanceUnit::FeetIndian,
          Qgis::DistanceUnit::FeetIndian1937,
          Qgis::DistanceUnit::FeetIndian1962,
          Qgis::DistanceUnit::FeetIndian1975,
          Qgis::DistanceUnit::FeetUSSurvey,
          Qgis::DistanceUnit::LinksInternational,
          Qgis::DistanceUnit::LinksBritishBenoit1895A,
          Qgis::DistanceUnit::LinksBritishBenoit1895B,
          Qgis::DistanceUnit::LinksBritishSears1922Truncated,
          Qgis::DistanceUnit::LinksBritishSears1922,
          Qgis::DistanceUnit::LinksClarkes,
          Qgis::DistanceUnit::LinksUSSurvey,
          Qgis::DistanceUnit::YardsBritishBenoit1895A,
          Qgis::DistanceUnit::YardsBritishBenoit1895B,
          Qgis::DistanceUnit::YardsBritishSears1922Truncated,
          Qgis::DistanceUnit::YardsBritishSears1922,
          Qgis::DistanceUnit::YardsClarkes,
          Qgis::DistanceUnit::YardsIndian,
          Qgis::DistanceUnit::YardsIndian1937,
          Qgis::DistanceUnit::YardsIndian1962,
          Qgis::DistanceUnit::YardsIndian1975,
          Qgis::DistanceUnit::MilesUSSurvey,
          Qgis::DistanceUnit::Fathoms,
          Qgis::DistanceUnit::MetersGermanLegal,
        } )
  {
    if ( normalized == encodeUnit( unit ) )
      return unit;
  }
  if ( ok )
    *ok = false;

  return Qgis::DistanceUnit::Unknown;
}

QString QgsUnitTypes::toString( Qgis::DistanceUnit unit )
{
  switch ( unit )
  {
    case Qgis::DistanceUnit::Meters:
      return QObject::tr( "meters", "distance" );

    case Qgis::DistanceUnit::Kilometers:
      return QObject::tr( "kilometers", "distance" );

    case Qgis::DistanceUnit::Feet:
      return QObject::tr( "feet", "distance" );

    case Qgis::DistanceUnit::Yards:
      return QObject::tr( "yards", "distance" );

    case Qgis::DistanceUnit::Miles:
      return QObject::tr( "miles", "distance" );

    case Qgis::DistanceUnit::Degrees:
      return QObject::tr( "degrees", "distance" );

    case Qgis::DistanceUnit::Centimeters:
      return QObject::tr( "centimeters", "distance" );

    case Qgis::DistanceUnit::Millimeters:
      return QObject::tr( "millimeters", "distance" );

    case Qgis::DistanceUnit::Inches:
      return QObject::tr( "inches", "distance" );

    case Qgis::DistanceUnit::Unknown:
      return QObject::tr( "<unknown>", "distance" );

    case Qgis::DistanceUnit::NauticalMiles:
      return QObject::tr( "nautical miles", "distance" );

    case Qgis::DistanceUnit::ChainsInternational:
      return QObject::tr( "chains (international)", "distance" );

    case Qgis::DistanceUnit::ChainsBritishBenoit1895A:
      return QObject::tr( "chains (British, Benoit 1895 A)", "distance" );

    case Qgis::DistanceUnit::ChainsBritishBenoit1895B:
      return QObject::tr( "chains (British, Benoit 1895 B)", "distance" );

    case Qgis::DistanceUnit::ChainsBritishSears1922Truncated:
      return QObject::tr( "chains (British, Sears 1922 truncated)", "distance" );

    case Qgis::DistanceUnit::ChainsBritishSears1922:
      return QObject::tr( "chains (British, Sears 1922)", "distance" );

    case Qgis::DistanceUnit::ChainsClarkes:
      return QObject::tr( "chains (Clarke's)", "distance" );

    case Qgis::DistanceUnit::ChainsUSSurvey:
      return QObject::tr( "chains (US survey)", "distance" );

    case Qgis::DistanceUnit::FeetBritish1865:
      return QObject::tr( "feet (British, 1865)", "distance" );

    case Qgis::DistanceUnit::FeetBritish1936:
      return QObject::tr( "feet (British, 1936)", "distance" );

    case Qgis::DistanceUnit::FeetBritishBenoit1895A:
      return QObject::tr( "feet (British, Benoit 1895 A)", "distance" );

    case Qgis::DistanceUnit::FeetBritishBenoit1895B:
      return QObject::tr( "feet (British, Benoit 1895 B)", "distance" );

    case Qgis::DistanceUnit::FeetBritishSears1922Truncated:
      return QObject::tr( "feet (British, Sears 1922 truncated)", "distance" );

    case Qgis::DistanceUnit::FeetBritishSears1922:
      return QObject::tr( "feet (British, Sears 1922)", "distance" );

    case Qgis::DistanceUnit::FeetClarkes:
      return QObject::tr( "feet (Clarke's)", "distance" );

    case Qgis::DistanceUnit::FeetGoldCoast:
      return QObject::tr( "feet (Gold Coast)", "distance" );

    case Qgis::DistanceUnit::FeetIndian:
      return QObject::tr( "feet (Indian)", "distance" );

    case Qgis::DistanceUnit::FeetIndian1937:
      return QObject::tr( "feet (Indian 1937)", "distance" );

    case Qgis::DistanceUnit::FeetIndian1962:
      return QObject::tr( "feet (Indian 1962)", "distance" );

    case Qgis::DistanceUnit::FeetIndian1975:
      return QObject::tr( "feet (Indian 1975)", "distance" );

    case Qgis::DistanceUnit::FeetUSSurvey:
      return QObject::tr( "feet (US survey)", "distance" );

    case Qgis::DistanceUnit::LinksInternational:
      return QObject::tr( "links", "distance" );

    case Qgis::DistanceUnit::LinksBritishBenoit1895A:
      return QObject::tr( "links (British, Benoit 1895 A)", "distance" );

    case Qgis::DistanceUnit::LinksBritishBenoit1895B:
      return QObject::tr( "links (British, Benoit 1895 B)", "distance" );

    case Qgis::DistanceUnit::LinksBritishSears1922Truncated:
      return QObject::tr( "links (British, Sears 1922 truncated)", "distance" );

    case Qgis::DistanceUnit::LinksBritishSears1922:
      return QObject::tr( "links (British, Sears 1922)", "distance" );

    case Qgis::DistanceUnit::LinksClarkes:
      return QObject::tr( "links (Clarke's)", "distance" );

    case Qgis::DistanceUnit::LinksUSSurvey:
      return QObject::tr( "links (US survey)", "distance" );

    case Qgis::DistanceUnit::YardsBritishBenoit1895A:
      return QObject::tr( "yards (British, Benoit 1895 A)", "distance" );

    case Qgis::DistanceUnit::YardsBritishBenoit1895B:
      return QObject::tr( "yards (British, Benoit 1895 B)", "distance" );

    case Qgis::DistanceUnit::YardsBritishSears1922Truncated:
      return QObject::tr( "yards (British, Sears 1922 truncated)", "distance" );

    case Qgis::DistanceUnit::YardsBritishSears1922:
      return QObject::tr( "yards (British, Sears 1922)", "distance" );

    case Qgis::DistanceUnit::YardsClarkes:
      return QObject::tr( "yards (Clarke's)", "distance" );

    case Qgis::DistanceUnit::YardsIndian:
      return QObject::tr( "yards (Indian)", "distance" );

    case Qgis::DistanceUnit::YardsIndian1937:
      return QObject::tr( "yards (Indian 1937)", "distance" );

    case Qgis::DistanceUnit::YardsIndian1962:
      return QObject::tr( "yards (Indian 1962)", "distance" );

    case Qgis::DistanceUnit::YardsIndian1975:
      return QObject::tr( "yards (Indian 1975)", "distance" );

    case Qgis::DistanceUnit::MilesUSSurvey:
      return QObject::tr( "miles (US survey)", "distance" );

    case Qgis::DistanceUnit::Fathoms:
      return QObject::tr( "fathoms", "distance" );

    case Qgis::DistanceUnit::MetersGermanLegal:
      return QObject::tr( "meters (German legal)", "distance" );
  }
  return QString();
}

QString QgsUnitTypes::toAbbreviatedString( Qgis::RenderUnit unit )
{
  switch ( unit )
  {
    case Qgis::RenderUnit::Millimeters:
      return QObject::tr( "mm", "render" );

    case Qgis::RenderUnit::MapUnits:
      return QObject::tr( "map units", "render" );

    case Qgis::RenderUnit::Pixels:
      return QObject::tr( "px", "render" );

    case Qgis::RenderUnit::Percentage:
      return QObject::tr( "%", "render" );

    case Qgis::RenderUnit::Points:
      return QObject::tr( "pt", "render" );

    case Qgis::RenderUnit::Inches:
      return QObject::tr( "in", "render" );

    case Qgis::RenderUnit::Unknown:
      return QObject::tr( "unknown", "render" );

    case Qgis::RenderUnit::MetersInMapUnits:
      return QObject::tr( "m", "render" );

  }

  return QString();
}

QString QgsUnitTypes::toAbbreviatedString( Qgis::DistanceUnit unit )
{
  switch ( unit )
  {
    case Qgis::DistanceUnit::Meters:
      return QObject::tr( "m", "distance" );

    case Qgis::DistanceUnit::Kilometers:
      return QObject::tr( "km", "distance" );

    case Qgis::DistanceUnit::Feet:
    case Qgis::DistanceUnit::FeetBritish1865:
    case Qgis::DistanceUnit::FeetBritish1936:
    case Qgis::DistanceUnit::FeetBritishBenoit1895A:
    case Qgis::DistanceUnit::FeetBritishBenoit1895B:
    case Qgis::DistanceUnit::FeetBritishSears1922Truncated:
    case Qgis::DistanceUnit::FeetBritishSears1922:
    case Qgis::DistanceUnit::FeetClarkes:
    case Qgis::DistanceUnit::FeetGoldCoast:
    case Qgis::DistanceUnit::FeetIndian:
    case Qgis::DistanceUnit::FeetIndian1937:
    case Qgis::DistanceUnit::FeetIndian1962:
    case Qgis::DistanceUnit::FeetIndian1975:
    case Qgis::DistanceUnit::FeetUSSurvey:
      return QObject::tr( "ft", "distance" );

    case Qgis::DistanceUnit::Yards:
    case Qgis::DistanceUnit::YardsBritishBenoit1895A:
    case Qgis::DistanceUnit::YardsBritishBenoit1895B:
    case Qgis::DistanceUnit::YardsBritishSears1922Truncated:
    case Qgis::DistanceUnit::YardsBritishSears1922:
    case Qgis::DistanceUnit::YardsClarkes:
    case Qgis::DistanceUnit::YardsIndian:
    case Qgis::DistanceUnit::YardsIndian1937:
    case Qgis::DistanceUnit::YardsIndian1962:
    case Qgis::DistanceUnit::YardsIndian1975:
      return QObject::tr( "yd", "distance" );

    case Qgis::DistanceUnit::Miles:
    case Qgis::DistanceUnit::MilesUSSurvey:
      return QObject::tr( "mi", "distance" );

    case Qgis::DistanceUnit::Degrees:
      return QObject::tr( "deg", "distance" );

    case Qgis::DistanceUnit::Centimeters:
      return QObject::tr( "cm", "distance" );

    case Qgis::DistanceUnit::Millimeters:
      return QObject::tr( "mm", "distance" );

    case Qgis::DistanceUnit::Inches:
      return QObject::tr( "in", "distance" );

    case Qgis::DistanceUnit::Unknown:
      return QString();

    case Qgis::DistanceUnit::NauticalMiles:
      return QObject::tr( "NM", "distance" );

    case Qgis::DistanceUnit::ChainsInternational:
    case Qgis::DistanceUnit::ChainsBritishBenoit1895A:
    case Qgis::DistanceUnit::ChainsBritishBenoit1895B:
    case Qgis::DistanceUnit::ChainsBritishSears1922Truncated:
    case Qgis::DistanceUnit::ChainsBritishSears1922:
    case Qgis::DistanceUnit::ChainsClarkes:
    case Qgis::DistanceUnit::ChainsUSSurvey:
      return QObject::tr( "ch", "distance" );

    case Qgis::DistanceUnit::LinksInternational:
    case Qgis::DistanceUnit::LinksBritishBenoit1895A:
    case Qgis::DistanceUnit::LinksBritishBenoit1895B:
    case Qgis::DistanceUnit::LinksBritishSears1922Truncated:
    case Qgis::DistanceUnit::LinksBritishSears1922:
    case Qgis::DistanceUnit::LinksClarkes:
    case Qgis::DistanceUnit::LinksUSSurvey:
      return QObject::tr( "lk", "distance" );

    case Qgis::DistanceUnit::Fathoms:
      return QObject::tr( "f", "distance" );

    case Qgis::DistanceUnit::MetersGermanLegal:
      return QObject::tr( "glm", "distance" );

  }
  return QString();
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in test_qgsunittypes.py.
 * See details in QEP #17
 ****************************************************************************/

Qgis::DistanceUnit QgsUnitTypes::stringToDistanceUnit( const QString &string, bool *ok )
{
  const QString normalized = string.trimmed().toLower();

  if ( ok )
    *ok = true;

  for ( const Qgis::DistanceUnit unit :
        {
          Qgis::DistanceUnit::Meters,
          Qgis::DistanceUnit::Feet,
          Qgis::DistanceUnit::Degrees,
          Qgis::DistanceUnit::NauticalMiles,
          Qgis::DistanceUnit::Kilometers,
          Qgis::DistanceUnit::Yards,
          Qgis::DistanceUnit::Miles,
          Qgis::DistanceUnit::Centimeters,
          Qgis::DistanceUnit::Millimeters,
          Qgis::DistanceUnit::Inches,
          Qgis::DistanceUnit::Unknown,
          Qgis::DistanceUnit::ChainsInternational,
          Qgis::DistanceUnit::ChainsBritishBenoit1895A,
          Qgis::DistanceUnit::ChainsBritishBenoit1895B,
          Qgis::DistanceUnit::ChainsBritishSears1922Truncated,
          Qgis::DistanceUnit::ChainsBritishSears1922,
          Qgis::DistanceUnit::ChainsClarkes,
          Qgis::DistanceUnit::ChainsUSSurvey,
          Qgis::DistanceUnit::FeetBritish1865,
          Qgis::DistanceUnit::FeetBritish1936,
          Qgis::DistanceUnit::FeetBritishBenoit1895A,
          Qgis::DistanceUnit::FeetBritishBenoit1895B,
          Qgis::DistanceUnit::FeetBritishSears1922Truncated,
          Qgis::DistanceUnit::FeetBritishSears1922,
          Qgis::DistanceUnit::FeetClarkes,
          Qgis::DistanceUnit::FeetGoldCoast,
          Qgis::DistanceUnit::FeetIndian,
          Qgis::DistanceUnit::FeetIndian1937,
          Qgis::DistanceUnit::FeetIndian1962,
          Qgis::DistanceUnit::FeetIndian1975,
          Qgis::DistanceUnit::FeetUSSurvey,
          Qgis::DistanceUnit::LinksInternational,
          Qgis::DistanceUnit::LinksBritishBenoit1895A,
          Qgis::DistanceUnit::LinksBritishBenoit1895B,
          Qgis::DistanceUnit::LinksBritishSears1922Truncated,
          Qgis::DistanceUnit::LinksBritishSears1922,
          Qgis::DistanceUnit::LinksClarkes,
          Qgis::DistanceUnit::LinksUSSurvey,
          Qgis::DistanceUnit::YardsBritishBenoit1895A,
          Qgis::DistanceUnit::YardsBritishBenoit1895B,
          Qgis::DistanceUnit::YardsBritishSears1922Truncated,
          Qgis::DistanceUnit::YardsBritishSears1922,
          Qgis::DistanceUnit::YardsClarkes,
          Qgis::DistanceUnit::YardsIndian,
          Qgis::DistanceUnit::YardsIndian1937,
          Qgis::DistanceUnit::YardsIndian1962,
          Qgis::DistanceUnit::YardsIndian1975,
          Qgis::DistanceUnit::MilesUSSurvey,
          Qgis::DistanceUnit::Fathoms,
          Qgis::DistanceUnit::MetersGermanLegal,
        } )
  {
    if ( normalized.compare( toString( unit ), Qt::CaseInsensitive ) == 0 )
      return unit;
  }

  if ( ok )
    *ok = false;

  return Qgis::DistanceUnit::Unknown;
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in test_qgsunittypes.py.
 * See details in QEP #17
 ****************************************************************************/

constexpr double distanceUnitToMeter( Qgis::DistanceUnit unit )
{
  // values are from the EPSG units database:
  switch ( unit )
  {
    case Qgis::DistanceUnit::Meters:
      return 1.0;
    case Qgis::DistanceUnit::Kilometers:
      return 1000.0;
    case Qgis::DistanceUnit::Feet:
      return 0.3048;
    case Qgis::DistanceUnit::NauticalMiles:
      return 1852.0;
    case Qgis::DistanceUnit::Yards:
      return 0.9144;
    case Qgis::DistanceUnit::Miles:
      return 1609.344;
    case Qgis::DistanceUnit::Degrees:
      return 111319.49079327358;
    case Qgis::DistanceUnit::Centimeters:
      return 0.01;
    case Qgis::DistanceUnit::Millimeters:
      return 0.001;
    case Qgis::DistanceUnit::Inches:
      return 0.0254;
    case Qgis::DistanceUnit::ChainsInternational:
      return 20.1168;
    case Qgis::DistanceUnit::ChainsBritishBenoit1895A:
      return 20.1167824;
    case Qgis::DistanceUnit::ChainsBritishBenoit1895B:
      return 20.116782494376;
    case Qgis::DistanceUnit::ChainsBritishSears1922Truncated:
      return 20.116756;
    case Qgis::DistanceUnit::ChainsBritishSears1922:
      return 20.116765121553;
    case Qgis::DistanceUnit::ChainsClarkes:
      return 20.1166195164;
    case Qgis::DistanceUnit::ChainsUSSurvey:
      return 20.11684023368;
    case Qgis::DistanceUnit::FeetBritish1865:
      return 0.30480083333333;
    case Qgis::DistanceUnit::FeetBritish1936:
      return 0.3048007491;
    case Qgis::DistanceUnit::FeetBritishBenoit1895A:
      return 0.30479973333333;
    case Qgis::DistanceUnit::FeetBritishBenoit1895B:
      return 0.30479973476327;
    case Qgis::DistanceUnit::FeetBritishSears1922Truncated:
      return 0.30479933333333;
    case Qgis::DistanceUnit::FeetBritishSears1922:
      return 0.30479947153868;
    case Qgis::DistanceUnit::FeetClarkes:
      return 0.3047972654;
    case Qgis::DistanceUnit::FeetGoldCoast:
      return 0.30479971018151;
    case Qgis::DistanceUnit::FeetIndian:
      return 0.30479951024815;
    case Qgis::DistanceUnit::FeetIndian1937:
      return 0.30479841;
    case Qgis::DistanceUnit::FeetIndian1962:
      return 0.3047996;
    case Qgis::DistanceUnit::FeetIndian1975:
      return 0.3047995;
    case Qgis::DistanceUnit::FeetUSSurvey:
      return 0.30480060960122;
    case Qgis::DistanceUnit::LinksInternational:
      return 0.201168;
    case Qgis::DistanceUnit::LinksBritishBenoit1895A:
      return 0.201167824;
    case Qgis::DistanceUnit::LinksBritishBenoit1895B:
      return 0.20116782494376;
    case Qgis::DistanceUnit::LinksBritishSears1922Truncated:
      return 0.20116756;
    case Qgis::DistanceUnit::LinksBritishSears1922:
      return 0.20116765121553;
    case Qgis::DistanceUnit::LinksClarkes:
      return 0.201166195164;
    case Qgis::DistanceUnit::LinksUSSurvey:
      return 0.2011684023368;
    case Qgis::DistanceUnit::YardsBritishBenoit1895A:
      return 0.9143992;
    case Qgis::DistanceUnit::YardsBritishBenoit1895B:
      return 0.91439920428981;
    case Qgis::DistanceUnit::YardsBritishSears1922Truncated:
      return 0.914398;
    case Qgis::DistanceUnit::YardsBritishSears1922:
      return 0.91439841461603;
    case Qgis::DistanceUnit::YardsClarkes:
      return 0.9143917962;
    case Qgis::DistanceUnit::YardsIndian:
      return 0.91439853074444;
    case Qgis::DistanceUnit::YardsIndian1937:
      return 0.91439523;
    case Qgis::DistanceUnit::YardsIndian1962:
      return 0.9143988;
    case Qgis::DistanceUnit::YardsIndian1975:
      return 0.9143985;
    case Qgis::DistanceUnit::MilesUSSurvey:
      return 1609.3472186944;
    case Qgis::DistanceUnit::Fathoms:
      return 1.8288;
    case Qgis::DistanceUnit::Unknown:
      return 1;
    case Qgis::DistanceUnit::MetersGermanLegal:
      return 1.0000135965;
  }
  return 1;
}

double QgsUnitTypes::fromUnitToUnitFactor( Qgis::DistanceUnit fromUnit, Qgis::DistanceUnit toUnit )
{
  if ( fromUnit == toUnit || fromUnit == Qgis::DistanceUnit::Unknown || toUnit == Qgis::DistanceUnit::Unknown )
    return 1.0;

  constexpr double FEET_TO_INCHES = 12;
  constexpr double YARDS_TO_FEET = 3.0;

  // Calculate the conversion factor between the specified units

  // special cases, where we don't go through meters as an intermediate unit:
  switch ( fromUnit )
  {
    case Qgis::DistanceUnit::Feet:
    {
      switch ( toUnit )
      {
        case Qgis::DistanceUnit::Inches:
          return FEET_TO_INCHES;
        case Qgis::DistanceUnit::Yards:
          return 1.0 / YARDS_TO_FEET;
        default:
          break;
      }

      break;
    }
    case Qgis::DistanceUnit::Yards:
    {
      switch ( toUnit )
      {
        case Qgis::DistanceUnit::Feet:
          return YARDS_TO_FEET;
        case Qgis::DistanceUnit::Inches:
          return YARDS_TO_FEET * FEET_TO_INCHES;
        default:
          break;
      }

      break;
    }

    case Qgis::DistanceUnit::Inches:
    {
      switch ( toUnit )
      {
        case Qgis::DistanceUnit::Feet:
          return 1.0 / FEET_TO_INCHES;
        case Qgis::DistanceUnit::Yards:
          return 1.0 / ( YARDS_TO_FEET * FEET_TO_INCHES );
        default:
          break;
      }

      break;
    }

    default:
      break;
  }

  return distanceUnitToMeter( fromUnit ) / distanceUnitToMeter( toUnit );
}

QString QgsUnitTypes::encodeUnit( Qgis::AreaUnit unit )
{
  switch ( unit )
  {
    case Qgis::AreaUnit::SquareMeters:
      return QStringLiteral( "m2" );
    case Qgis::AreaUnit::SquareKilometers:
      return QStringLiteral( "km2" );
    case Qgis::AreaUnit::SquareFeet:
      return QStringLiteral( "ft2" );
    case Qgis::AreaUnit::SquareYards:
      return QStringLiteral( "y2" );
    case Qgis::AreaUnit::SquareMiles:
      return QStringLiteral( "mi2" );
    case Qgis::AreaUnit::Hectares:
      return QStringLiteral( "ha" );
    case Qgis::AreaUnit::Acres:
      return QStringLiteral( "ac" );
    case Qgis::AreaUnit::SquareNauticalMiles:
      return QStringLiteral( "nm2" );
    case Qgis::AreaUnit::SquareDegrees:
      return QStringLiteral( "deg2" );
    case Qgis::AreaUnit::SquareCentimeters:
      return QStringLiteral( "cm2" );
    case Qgis::AreaUnit::SquareMillimeters:
      return QStringLiteral( "mm2" );
    case Qgis::AreaUnit::SquareInches:
      return QStringLiteral( "in2" );
    case Qgis::AreaUnit::Unknown:
      return QStringLiteral( "<unknown>" );
  }
  return QString();
}

Qgis::AreaUnit QgsUnitTypes::decodeAreaUnit( const QString &string, bool *ok )
{
  const QString normalized = string.trimmed().toLower();

  if ( ok )
    *ok = true;

  if ( normalized == encodeUnit( Qgis::AreaUnit::SquareMeters ) )
    return Qgis::AreaUnit::SquareMeters;
  if ( normalized == encodeUnit( Qgis::AreaUnit::SquareKilometers ) )
    return Qgis::AreaUnit::SquareKilometers;
  if ( normalized == encodeUnit( Qgis::AreaUnit::SquareFeet ) )
    return Qgis::AreaUnit::SquareFeet;
  if ( normalized == encodeUnit( Qgis::AreaUnit::SquareYards ) )
    return Qgis::AreaUnit::SquareYards;
  if ( normalized == encodeUnit( Qgis::AreaUnit::SquareMiles ) )
    return Qgis::AreaUnit::SquareMiles;
  if ( normalized == encodeUnit( Qgis::AreaUnit::Hectares ) )
    return Qgis::AreaUnit::Hectares;
  if ( normalized == encodeUnit( Qgis::AreaUnit::Acres ) )
    return Qgis::AreaUnit::Acres;
  if ( normalized == encodeUnit( Qgis::AreaUnit::SquareNauticalMiles ) )
    return Qgis::AreaUnit::SquareNauticalMiles;
  if ( normalized == encodeUnit( Qgis::AreaUnit::SquareDegrees ) )
    return Qgis::AreaUnit::SquareDegrees;
  if ( normalized == encodeUnit( Qgis::AreaUnit::SquareCentimeters ) )
    return Qgis::AreaUnit::SquareCentimeters;
  if ( normalized == encodeUnit( Qgis::AreaUnit::SquareMillimeters ) )
    return Qgis::AreaUnit::SquareMillimeters;
  if ( normalized == encodeUnit( Qgis::AreaUnit::SquareInches ) )
    return Qgis::AreaUnit::SquareInches;
  if ( normalized == encodeUnit( Qgis::AreaUnit::Unknown ) )
    return Qgis::AreaUnit::Unknown;

  if ( ok )
    *ok = false;

  return Qgis::AreaUnit::Unknown;
}

QString QgsUnitTypes::toString( Qgis::AreaUnit unit )
{
  switch ( unit )
  {
    case Qgis::AreaUnit::SquareMeters:
      return QObject::tr( "square meters", "area" );
    case Qgis::AreaUnit::SquareKilometers:
      return QObject::tr( "square kilometers", "area" );
    case Qgis::AreaUnit::SquareFeet:
      return QObject::tr( "square feet", "area" );
    case Qgis::AreaUnit::SquareYards:
      return QObject::tr( "square yards", "area" );
    case Qgis::AreaUnit::SquareMiles:
      return QObject::tr( "square miles", "area" );
    case Qgis::AreaUnit::Hectares:
      return QObject::tr( "hectares", "area" );
    case Qgis::AreaUnit::Acres:
      return QObject::tr( "acres", "area" );
    case Qgis::AreaUnit::SquareNauticalMiles:
      return QObject::tr( "square nautical miles", "area" );
    case Qgis::AreaUnit::SquareDegrees:
      return QObject::tr( "square degrees", "area" );
    case Qgis::AreaUnit::SquareMillimeters:
      return QObject::tr( "square millimeters", "area" );
    case Qgis::AreaUnit::SquareCentimeters:
      return QObject::tr( "square centimeters", "area" );
    case Qgis::AreaUnit::SquareInches:
      return QObject::tr( "square inches", "area" );
    case Qgis::AreaUnit::Unknown:
      return QObject::tr( "<unknown>", "area" );
  }
  return QString();
}

QString QgsUnitTypes::toAbbreviatedString( Qgis::AreaUnit unit )
{
  switch ( unit )
  {
    case Qgis::AreaUnit::SquareMeters:
      return QObject::tr( "m²", "area" );
    case Qgis::AreaUnit::SquareKilometers:
      return QObject::tr( "km²", "area" );
    case Qgis::AreaUnit::SquareFeet:
      return QObject::tr( "ft²", "area" );
    case Qgis::AreaUnit::SquareYards:
      return QObject::tr( "yd²", "area" );
    case Qgis::AreaUnit::SquareMiles:
      return QObject::tr( "mi²", "area" );
    case Qgis::AreaUnit::Hectares:
      return QObject::tr( "ha", "area" );
    case Qgis::AreaUnit::Acres:
      return QObject::tr( "ac", "area" );
    case Qgis::AreaUnit::SquareNauticalMiles:
      return QObject::tr( "NM²", "area" );
    case Qgis::AreaUnit::SquareDegrees:
      return QObject::tr( "deg²", "area" );
    case Qgis::AreaUnit::SquareCentimeters:
      return QObject::tr( "cm²", "area" );
    case Qgis::AreaUnit::SquareMillimeters:
      return QObject::tr( "mm²", "area" );
    case Qgis::AreaUnit::SquareInches:
      return QObject::tr( "in²", "area" );
    case Qgis::AreaUnit::Unknown:
      return QString();
  }
  return QString();
}

Qgis::AreaUnit QgsUnitTypes::stringToAreaUnit( const QString &string, bool *ok )
{
  const QString normalized = string.trimmed().toLower();

  if ( ok )
    *ok = true;

  if ( normalized == toString( Qgis::AreaUnit::SquareMeters ) )
    return Qgis::AreaUnit::SquareMeters;
  if ( normalized == toString( Qgis::AreaUnit::SquareKilometers ) )
    return Qgis::AreaUnit::SquareKilometers;
  if ( normalized == toString( Qgis::AreaUnit::SquareFeet ) )
    return Qgis::AreaUnit::SquareFeet;
  if ( normalized == toString( Qgis::AreaUnit::SquareYards ) )
    return Qgis::AreaUnit::SquareYards;
  if ( normalized == toString( Qgis::AreaUnit::SquareMiles ) )
    return Qgis::AreaUnit::SquareMiles;
  if ( normalized == toString( Qgis::AreaUnit::Hectares ) )
    return Qgis::AreaUnit::Hectares;
  if ( normalized == toString( Qgis::AreaUnit::Acres ) )
    return Qgis::AreaUnit::Acres;
  if ( normalized == toString( Qgis::AreaUnit::SquareNauticalMiles ) )
    return Qgis::AreaUnit::SquareNauticalMiles;
  if ( normalized == toString( Qgis::AreaUnit::SquareDegrees ) )
    return Qgis::AreaUnit::SquareDegrees;
  if ( normalized == toString( Qgis::AreaUnit::SquareMillimeters ) )
    return Qgis::AreaUnit::SquareMillimeters;
  if ( normalized == toString( Qgis::AreaUnit::SquareCentimeters ) )
    return Qgis::AreaUnit::SquareCentimeters;
  if ( normalized == toString( Qgis::AreaUnit::SquareInches ) )
    return Qgis::AreaUnit::SquareInches;
  if ( normalized == toString( Qgis::AreaUnit::Unknown ) )
    return Qgis::AreaUnit::Unknown;
  if ( ok )
    *ok = false;

  return Qgis::AreaUnit::Unknown;
}

double QgsUnitTypes::fromUnitToUnitFactor( Qgis::AreaUnit fromUnit, Qgis::AreaUnit toUnit )
{
#define KM2_TO_M2 1000000.0
#define CM2_TO_M2 0.0001
#define MM2_TO_M2 0.000001
#define FT2_TO_M2 0.09290304
#define IN2_TO_M2 0.00064516
#define YD2_TO_M2 0.83612736
#define MI2_TO_M2 2589988.110336
#define HA_TO_M2 10000.0
#define AC_TO_FT2 43560.0
#define DEG2_TO_M2 12392029030.5
#define NM2_TO_M2 3429904.0

  // Calculate the conversion factor between the specified units
  switch ( fromUnit )
  {
    case Qgis::AreaUnit::SquareMeters:
    {
      switch ( toUnit )
      {
        case Qgis::AreaUnit::SquareMeters:
          return 1.0;
        case Qgis::AreaUnit::SquareKilometers:
          return 1.0 / KM2_TO_M2;
        case Qgis::AreaUnit::SquareFeet:
          return 1.0 / FT2_TO_M2;
        case Qgis::AreaUnit::SquareYards:
          return 1.0 / YD2_TO_M2;
        case Qgis::AreaUnit::SquareMiles:
          return 1.0 / MI2_TO_M2;
        case Qgis::AreaUnit::Hectares:
          return 1.0 / HA_TO_M2;
        case Qgis::AreaUnit::Acres:
          return 1.0 / AC_TO_FT2 / FT2_TO_M2;
        case Qgis::AreaUnit::SquareNauticalMiles:
          return 1.0 / NM2_TO_M2;
        case Qgis::AreaUnit::SquareDegrees:
          return 1.0 / DEG2_TO_M2;
        case Qgis::AreaUnit::SquareCentimeters:
          return 1.0 / CM2_TO_M2;
        case Qgis::AreaUnit::SquareMillimeters:
          return 1.0 / MM2_TO_M2;
        case Qgis::AreaUnit::SquareInches:
          return 1.0 / IN2_TO_M2;
        case Qgis::AreaUnit::Unknown:
          break;
      }

      break;
    }
    case Qgis::AreaUnit::SquareKilometers:
    {
      switch ( toUnit )
      {
        case Qgis::AreaUnit::SquareMeters:
          return KM2_TO_M2;
        case Qgis::AreaUnit::SquareKilometers:
          return 1.0;
        case Qgis::AreaUnit::SquareFeet:
          return KM2_TO_M2 / FT2_TO_M2;
        case Qgis::AreaUnit::SquareYards:
          return KM2_TO_M2 / YD2_TO_M2;
        case Qgis::AreaUnit::SquareMiles:
          return KM2_TO_M2 / MI2_TO_M2;
        case Qgis::AreaUnit::Hectares:
          return KM2_TO_M2 / HA_TO_M2;
        case Qgis::AreaUnit::Acres:
          return KM2_TO_M2 / AC_TO_FT2 / FT2_TO_M2;
        case Qgis::AreaUnit::SquareNauticalMiles:
          return KM2_TO_M2 / NM2_TO_M2;
        case Qgis::AreaUnit::SquareDegrees:
          return KM2_TO_M2 / DEG2_TO_M2;
        case Qgis::AreaUnit::SquareCentimeters:
          return KM2_TO_M2 / CM2_TO_M2;
        case Qgis::AreaUnit::SquareMillimeters:
          return KM2_TO_M2 / MM2_TO_M2;
        case Qgis::AreaUnit::SquareInches:
          return KM2_TO_M2 / IN2_TO_M2;
        case Qgis::AreaUnit::Unknown:
          break;
      }

      break;
    }
    case Qgis::AreaUnit::SquareFeet:
    {
      switch ( toUnit )
      {
        case Qgis::AreaUnit::SquareMeters:
          return FT2_TO_M2;
        case Qgis::AreaUnit::SquareKilometers:
          return FT2_TO_M2 / KM2_TO_M2;
        case Qgis::AreaUnit::SquareFeet:
          return 1.0;
        case Qgis::AreaUnit::SquareYards:
          return FT2_TO_M2 / YD2_TO_M2;
        case Qgis::AreaUnit::SquareMiles:
          return FT2_TO_M2 / MI2_TO_M2;
        case Qgis::AreaUnit::Hectares:
          return FT2_TO_M2 / HA_TO_M2;
        case Qgis::AreaUnit::Acres:
          return 1.0 / AC_TO_FT2;
        case Qgis::AreaUnit::SquareNauticalMiles:
          return FT2_TO_M2 / NM2_TO_M2;
        case Qgis::AreaUnit::SquareDegrees:
          return FT2_TO_M2 / DEG2_TO_M2;
        case Qgis::AreaUnit::SquareCentimeters:
          return FT2_TO_M2 / CM2_TO_M2;
        case Qgis::AreaUnit::SquareMillimeters:
          return FT2_TO_M2 / MM2_TO_M2;
        case Qgis::AreaUnit::SquareInches:
          return FT2_TO_M2 / IN2_TO_M2;
        case Qgis::AreaUnit::Unknown:
          break;
      }

      break;
    }

    case Qgis::AreaUnit::SquareYards:
    {
      switch ( toUnit )
      {
        case Qgis::AreaUnit::SquareMeters:
          return YD2_TO_M2;
        case Qgis::AreaUnit::SquareKilometers:
          return YD2_TO_M2 / KM2_TO_M2;
        case Qgis::AreaUnit::SquareFeet:
          return YD2_TO_M2 / FT2_TO_M2;
        case Qgis::AreaUnit::SquareYards:
          return 1.0;
        case Qgis::AreaUnit::SquareMiles:
          return YD2_TO_M2 / MI2_TO_M2;
        case Qgis::AreaUnit::Hectares:
          return YD2_TO_M2 / HA_TO_M2;
        case Qgis::AreaUnit::Acres:
          return YD2_TO_M2 / FT2_TO_M2 / AC_TO_FT2;
        case Qgis::AreaUnit::SquareNauticalMiles:
          return YD2_TO_M2 / NM2_TO_M2;
        case Qgis::AreaUnit::SquareDegrees:
          return YD2_TO_M2 / DEG2_TO_M2;
        case Qgis::AreaUnit::SquareCentimeters:
          return YD2_TO_M2 / CM2_TO_M2;
        case Qgis::AreaUnit::SquareMillimeters:
          return YD2_TO_M2 / MM2_TO_M2;
        case Qgis::AreaUnit::SquareInches:
          return YD2_TO_M2 / IN2_TO_M2;
        case Qgis::AreaUnit::Unknown:
          break;
      }
      break;
    }

    case Qgis::AreaUnit::SquareMiles:
    {
      switch ( toUnit )
      {
        case Qgis::AreaUnit::SquareMeters:
          return MI2_TO_M2;
        case Qgis::AreaUnit::SquareKilometers:
          return MI2_TO_M2 / KM2_TO_M2;
        case Qgis::AreaUnit::SquareFeet:
          return MI2_TO_M2 / FT2_TO_M2;
        case Qgis::AreaUnit::SquareYards:
          return MI2_TO_M2 / YD2_TO_M2;
        case Qgis::AreaUnit::SquareMiles:
          return 1.0;
        case Qgis::AreaUnit::Hectares:
          return MI2_TO_M2 / HA_TO_M2;
        case Qgis::AreaUnit::Acres:
          return MI2_TO_M2 / FT2_TO_M2 / AC_TO_FT2;
        case Qgis::AreaUnit::SquareNauticalMiles:
          return MI2_TO_M2 / NM2_TO_M2;
        case Qgis::AreaUnit::SquareDegrees:
          return MI2_TO_M2 / DEG2_TO_M2;
        case Qgis::AreaUnit::SquareCentimeters:
          return MI2_TO_M2 / CM2_TO_M2;
        case Qgis::AreaUnit::SquareMillimeters:
          return MI2_TO_M2 / MM2_TO_M2;
        case Qgis::AreaUnit::SquareInches:
          return MI2_TO_M2 / IN2_TO_M2;
        case Qgis::AreaUnit::Unknown:
          break;
      }

      break;
    }

    case Qgis::AreaUnit::Hectares:
    {
      switch ( toUnit )
      {
        case Qgis::AreaUnit::SquareMeters:
          return HA_TO_M2;
        case Qgis::AreaUnit::SquareKilometers:
          return HA_TO_M2 / KM2_TO_M2;
        case Qgis::AreaUnit::SquareFeet:
          return HA_TO_M2 / FT2_TO_M2;
        case Qgis::AreaUnit::SquareYards:
          return HA_TO_M2 / YD2_TO_M2;
        case Qgis::AreaUnit::SquareMiles:
          return HA_TO_M2 / MI2_TO_M2;
        case Qgis::AreaUnit::Hectares:
          return 1.0;
        case Qgis::AreaUnit::Acres:
          return HA_TO_M2 / FT2_TO_M2 / AC_TO_FT2;
        case Qgis::AreaUnit::SquareNauticalMiles:
          return HA_TO_M2 / NM2_TO_M2;
        case Qgis::AreaUnit::SquareDegrees:
          return HA_TO_M2 / DEG2_TO_M2;
        case Qgis::AreaUnit::SquareCentimeters:
          return HA_TO_M2 / CM2_TO_M2;
        case Qgis::AreaUnit::SquareMillimeters:
          return HA_TO_M2 / MM2_TO_M2;
        case Qgis::AreaUnit::SquareInches:
          return HA_TO_M2 / IN2_TO_M2;
        case Qgis::AreaUnit::Unknown:
          break;
      }

      break;
    }

    case Qgis::AreaUnit::Acres:
    {
      switch ( toUnit )
      {
        case Qgis::AreaUnit::SquareMeters:
          return AC_TO_FT2 * FT2_TO_M2;
        case Qgis::AreaUnit::SquareKilometers:
          return AC_TO_FT2 * FT2_TO_M2 / KM2_TO_M2;
        case Qgis::AreaUnit::SquareFeet:
          return AC_TO_FT2;
        case Qgis::AreaUnit::SquareYards:
          return AC_TO_FT2 * FT2_TO_M2 / YD2_TO_M2;
        case Qgis::AreaUnit::SquareMiles:
          return AC_TO_FT2 * FT2_TO_M2 / MI2_TO_M2;
        case Qgis::AreaUnit::Hectares:
          return AC_TO_FT2 * FT2_TO_M2 / HA_TO_M2;
        case Qgis::AreaUnit::Acres:
          return 1.0;
        case Qgis::AreaUnit::SquareNauticalMiles:
          return AC_TO_FT2 * FT2_TO_M2 / NM2_TO_M2;
        case Qgis::AreaUnit::SquareDegrees:
          return AC_TO_FT2 * FT2_TO_M2 / DEG2_TO_M2;
        case Qgis::AreaUnit::SquareCentimeters:
          return AC_TO_FT2 * FT2_TO_M2 / CM2_TO_M2;
        case Qgis::AreaUnit::SquareMillimeters:
          return AC_TO_FT2 * FT2_TO_M2 / MM2_TO_M2;
        case Qgis::AreaUnit::SquareInches:
          return AC_TO_FT2 * FT2_TO_M2 / IN2_TO_M2;
        case Qgis::AreaUnit::Unknown:
          break;
      }

      break;
    }

    case Qgis::AreaUnit::SquareNauticalMiles:
    {
      switch ( toUnit )
      {
        case Qgis::AreaUnit::SquareMeters:
          return NM2_TO_M2;
        case Qgis::AreaUnit::SquareKilometers:
          return NM2_TO_M2 / KM2_TO_M2;
        case Qgis::AreaUnit::SquareFeet:
          return NM2_TO_M2 / FT2_TO_M2;
        case Qgis::AreaUnit::SquareYards:
          return NM2_TO_M2 / YD2_TO_M2;
        case Qgis::AreaUnit::SquareMiles:
          return NM2_TO_M2 / MI2_TO_M2;
        case Qgis::AreaUnit::Hectares:
          return NM2_TO_M2 / HA_TO_M2;
        case Qgis::AreaUnit::Acres:
          return NM2_TO_M2 / FT2_TO_M2 / AC_TO_FT2;
        case Qgis::AreaUnit::SquareNauticalMiles:
          return 1.0;
        case Qgis::AreaUnit::SquareDegrees:
          return NM2_TO_M2 / DEG2_TO_M2;
        case Qgis::AreaUnit::SquareCentimeters:
          return NM2_TO_M2 / CM2_TO_M2;
        case Qgis::AreaUnit::SquareMillimeters:
          return NM2_TO_M2 / MM2_TO_M2;
        case Qgis::AreaUnit::SquareInches:
          return NM2_TO_M2 / IN2_TO_M2;
        case Qgis::AreaUnit::Unknown:
          break;
      }

      break;
    }

    case Qgis::AreaUnit::SquareDegrees:
    {
      switch ( toUnit )
      {
        case Qgis::AreaUnit::SquareMeters:
          return DEG2_TO_M2;
        case Qgis::AreaUnit::SquareKilometers:
          return DEG2_TO_M2 / KM2_TO_M2;
        case Qgis::AreaUnit::SquareFeet:
          return DEG2_TO_M2 / FT2_TO_M2;
        case Qgis::AreaUnit::SquareYards:
          return DEG2_TO_M2 / YD2_TO_M2;
        case Qgis::AreaUnit::SquareMiles:
          return DEG2_TO_M2 / MI2_TO_M2;
        case Qgis::AreaUnit::Hectares:
          return DEG2_TO_M2 / HA_TO_M2;
        case Qgis::AreaUnit::Acres:
          return DEG2_TO_M2 / FT2_TO_M2 / AC_TO_FT2;
        case Qgis::AreaUnit::SquareNauticalMiles:
          return DEG2_TO_M2 / NM2_TO_M2;
        case Qgis::AreaUnit::SquareDegrees:
          return 1.0;
        case Qgis::AreaUnit::SquareCentimeters:
          return DEG2_TO_M2 / CM2_TO_M2;
        case Qgis::AreaUnit::SquareMillimeters:
          return DEG2_TO_M2 / MM2_TO_M2;
        case Qgis::AreaUnit::SquareInches:
          return DEG2_TO_M2 / IN2_TO_M2;
        case Qgis::AreaUnit::Unknown:
          break;
      }

      break;
    }

    case Qgis::AreaUnit::SquareMillimeters:
    {
      switch ( toUnit )
      {
        case Qgis::AreaUnit::SquareMeters:
          return MM2_TO_M2;
        case Qgis::AreaUnit::SquareKilometers:
          return MM2_TO_M2 / KM2_TO_M2;
        case Qgis::AreaUnit::SquareFeet:
          return MM2_TO_M2 / FT2_TO_M2;
        case Qgis::AreaUnit::SquareYards:
          return MM2_TO_M2 / YD2_TO_M2;
        case Qgis::AreaUnit::SquareMiles:
          return MM2_TO_M2 / MI2_TO_M2;
        case Qgis::AreaUnit::Hectares:
          return MM2_TO_M2 / HA_TO_M2;
        case Qgis::AreaUnit::Acres:
          return MM2_TO_M2 / AC_TO_FT2 / FT2_TO_M2;
        case Qgis::AreaUnit::SquareNauticalMiles:
          return MM2_TO_M2 / NM2_TO_M2;
        case Qgis::AreaUnit::SquareDegrees:
          return MM2_TO_M2 / DEG2_TO_M2;
        case Qgis::AreaUnit::SquareCentimeters:
          return MM2_TO_M2 / CM2_TO_M2;
        case Qgis::AreaUnit::SquareMillimeters:
          return 1.0;
        case Qgis::AreaUnit::SquareInches:
          return MM2_TO_M2 / IN2_TO_M2;
        case Qgis::AreaUnit::Unknown:
          break;
      }

      break;
    }
    case Qgis::AreaUnit::SquareCentimeters:
    {
      switch ( toUnit )
      {
        case Qgis::AreaUnit::SquareMeters:
          return CM2_TO_M2;
        case Qgis::AreaUnit::SquareKilometers:
          return CM2_TO_M2 / KM2_TO_M2;
        case Qgis::AreaUnit::SquareFeet:
          return CM2_TO_M2 / FT2_TO_M2;
        case Qgis::AreaUnit::SquareYards:
          return CM2_TO_M2 / YD2_TO_M2;
        case Qgis::AreaUnit::SquareMiles:
          return CM2_TO_M2 / MI2_TO_M2;
        case Qgis::AreaUnit::Hectares:
          return CM2_TO_M2 / HA_TO_M2;
        case Qgis::AreaUnit::Acres:
          return CM2_TO_M2 / AC_TO_FT2 / FT2_TO_M2;
        case Qgis::AreaUnit::SquareNauticalMiles:
          return CM2_TO_M2 / NM2_TO_M2;
        case Qgis::AreaUnit::SquareDegrees:
          return CM2_TO_M2 / DEG2_TO_M2;
        case Qgis::AreaUnit::SquareCentimeters:
          return 1.0;
        case Qgis::AreaUnit::SquareMillimeters:
          return CM2_TO_M2 / MM2_TO_M2;
        case Qgis::AreaUnit::SquareInches:
          return CM2_TO_M2 / IN2_TO_M2;
        case Qgis::AreaUnit::Unknown:
          break;
      }

      break;
    }
    case Qgis::AreaUnit::SquareInches:
    {
      switch ( toUnit )
      {
        case Qgis::AreaUnit::SquareMeters:
          return IN2_TO_M2;
        case Qgis::AreaUnit::SquareKilometers:
          return IN2_TO_M2 / KM2_TO_M2;
        case Qgis::AreaUnit::SquareFeet:
          return IN2_TO_M2 / FT2_TO_M2;
        case Qgis::AreaUnit::SquareYards:
          return IN2_TO_M2 / YD2_TO_M2;
        case Qgis::AreaUnit::SquareMiles:
          return IN2_TO_M2 / MI2_TO_M2;
        case Qgis::AreaUnit::Hectares:
          return IN2_TO_M2 / HA_TO_M2;
        case Qgis::AreaUnit::Acres:
          return IN2_TO_M2 / AC_TO_FT2 / FT2_TO_M2;
        case Qgis::AreaUnit::SquareNauticalMiles:
          return IN2_TO_M2 / NM2_TO_M2;
        case Qgis::AreaUnit::SquareDegrees:
          return IN2_TO_M2 / DEG2_TO_M2;
        case Qgis::AreaUnit::SquareCentimeters:
          return IN2_TO_M2 / CM2_TO_M2;
        case Qgis::AreaUnit::SquareMillimeters:
          return IN2_TO_M2 / MM2_TO_M2;
        case Qgis::AreaUnit::SquareInches:
          return 1;
        case Qgis::AreaUnit::Unknown:
          break;
      }

      break;
    }
    case Qgis::AreaUnit::Unknown:
      break;
  }
  return 1.0;
}

Qgis::AreaUnit QgsUnitTypes::distanceToAreaUnit( Qgis::DistanceUnit distanceUnit )
{
  switch ( distanceUnit )
  {
    case Qgis::DistanceUnit::Meters:
    case Qgis::DistanceUnit::MetersGermanLegal:
      return Qgis::AreaUnit::SquareMeters;

    case Qgis::DistanceUnit::Kilometers:
      return Qgis::AreaUnit::SquareKilometers;

    case Qgis::DistanceUnit::Centimeters:
      return Qgis::AreaUnit::SquareCentimeters;

    case Qgis::DistanceUnit::Millimeters:
      return Qgis::AreaUnit::SquareMillimeters;

    case Qgis::DistanceUnit::Feet:
    case Qgis::DistanceUnit::ChainsInternational:
    case Qgis::DistanceUnit::ChainsBritishBenoit1895A:
    case Qgis::DistanceUnit::ChainsBritishBenoit1895B:
    case Qgis::DistanceUnit::ChainsBritishSears1922Truncated:
    case Qgis::DistanceUnit::ChainsBritishSears1922:
    case Qgis::DistanceUnit::ChainsClarkes:
    case Qgis::DistanceUnit::ChainsUSSurvey:
    case Qgis::DistanceUnit::FeetBritish1865:
    case Qgis::DistanceUnit::FeetBritish1936:
    case Qgis::DistanceUnit::FeetBritishBenoit1895A:
    case Qgis::DistanceUnit::FeetBritishBenoit1895B:
    case Qgis::DistanceUnit::FeetBritishSears1922Truncated:
    case Qgis::DistanceUnit::FeetBritishSears1922:
    case Qgis::DistanceUnit::FeetClarkes:
    case Qgis::DistanceUnit::FeetGoldCoast:
    case Qgis::DistanceUnit::FeetIndian:
    case Qgis::DistanceUnit::FeetIndian1937:
    case Qgis::DistanceUnit::FeetIndian1962:
    case Qgis::DistanceUnit::FeetIndian1975:
    case Qgis::DistanceUnit::FeetUSSurvey:
    case Qgis::DistanceUnit::LinksInternational:
    case Qgis::DistanceUnit::LinksBritishBenoit1895A:
    case Qgis::DistanceUnit::LinksBritishBenoit1895B:
    case Qgis::DistanceUnit::LinksBritishSears1922Truncated:
    case Qgis::DistanceUnit::LinksBritishSears1922:
    case Qgis::DistanceUnit::LinksClarkes:
    case Qgis::DistanceUnit::LinksUSSurvey:
    case Qgis::DistanceUnit::YardsBritishBenoit1895A:
    case Qgis::DistanceUnit::YardsBritishBenoit1895B:
    case Qgis::DistanceUnit::YardsBritishSears1922Truncated:
    case Qgis::DistanceUnit::YardsBritishSears1922:
    case Qgis::DistanceUnit::YardsClarkes:
    case Qgis::DistanceUnit::YardsIndian:
    case Qgis::DistanceUnit::YardsIndian1937:
    case Qgis::DistanceUnit::YardsIndian1962:
    case Qgis::DistanceUnit::YardsIndian1975:
    case Qgis::DistanceUnit::Fathoms:
      return Qgis::AreaUnit::SquareFeet;

    case Qgis::DistanceUnit::Yards:
      return Qgis::AreaUnit::SquareYards;

    case Qgis::DistanceUnit::Miles:
    case Qgis::DistanceUnit::MilesUSSurvey:
      return Qgis::AreaUnit::SquareMiles;

    case Qgis::DistanceUnit::Degrees:
      return Qgis::AreaUnit::SquareDegrees;

    case Qgis::DistanceUnit::Unknown:
      return Qgis::AreaUnit::Unknown;

    case Qgis::DistanceUnit::NauticalMiles:
      return Qgis::AreaUnit::SquareNauticalMiles;

    case Qgis::DistanceUnit::Inches:
      return Qgis::AreaUnit::SquareInches;
  }

  return Qgis::AreaUnit::Unknown;
}

Qgis::DistanceUnit QgsUnitTypes::areaToDistanceUnit( Qgis::AreaUnit areaUnit )
{
  switch ( areaUnit )
  {
    case Qgis::AreaUnit::SquareMeters:
    case Qgis::AreaUnit::Hectares:
      return Qgis::DistanceUnit::Meters;

    case Qgis::AreaUnit::SquareKilometers:
      return Qgis::DistanceUnit::Kilometers;

    case Qgis::AreaUnit::SquareCentimeters:
      return Qgis::DistanceUnit::Centimeters;

    case Qgis::AreaUnit::SquareMillimeters:
      return Qgis::DistanceUnit::Millimeters;

    case Qgis::AreaUnit::SquareFeet:
      return Qgis::DistanceUnit::Feet;

    case Qgis::AreaUnit::SquareYards:
    case Qgis::AreaUnit::Acres:
      return Qgis::DistanceUnit::Yards;

    case Qgis::AreaUnit::SquareMiles:
      return Qgis::DistanceUnit::Miles;

    case Qgis::AreaUnit::SquareDegrees:
      return Qgis::DistanceUnit::Degrees;

    case Qgis::AreaUnit::Unknown:
      return Qgis::DistanceUnit::Unknown;

    case Qgis::AreaUnit::SquareNauticalMiles:
      return Qgis::DistanceUnit::NauticalMiles;

    case Qgis::AreaUnit::SquareInches:
      return Qgis::DistanceUnit::Inches;
  }

  return Qgis::DistanceUnit::Unknown;
}

QString QgsUnitTypes::encodeUnit( Qgis::TemporalUnit unit )
{
  switch ( unit )
  {
    case Qgis::TemporalUnit::Seconds:
      return QStringLiteral( "s" );
    case Qgis::TemporalUnit::Milliseconds:
      return QStringLiteral( "ms" );
    case Qgis::TemporalUnit::Minutes:
      return QStringLiteral( "min" );
    case Qgis::TemporalUnit::Hours:
      return QStringLiteral( "h" );
    case Qgis::TemporalUnit::Days:
      return QStringLiteral( "d" );
    case Qgis::TemporalUnit::Weeks:
      return QStringLiteral( "wk" );
    case Qgis::TemporalUnit::Months:
      return QStringLiteral( "mon" );
    case Qgis::TemporalUnit::Years:
      return QStringLiteral( "y" );
    case Qgis::TemporalUnit::Decades:
      return QStringLiteral( "dec" );
    case Qgis::TemporalUnit::Centuries:
      return QStringLiteral( "c" );
    case Qgis::TemporalUnit::IrregularStep:
      return QStringLiteral( "xxx" );
    case Qgis::TemporalUnit::Unknown:
      return QStringLiteral( "<unknown>" );
  }
  return QString();
}

Qgis::TemporalUnit QgsUnitTypes::decodeTemporalUnit( const QString &string, bool *ok )
{
  const QString normalized = string.trimmed().toLower();

  if ( ok )
    *ok = true;

  if ( normalized == encodeUnit( Qgis::TemporalUnit::Seconds ) )
    return Qgis::TemporalUnit::Seconds;
  if ( normalized == encodeUnit( Qgis::TemporalUnit::Milliseconds ) )
    return Qgis::TemporalUnit::Milliseconds;
  if ( normalized == encodeUnit( Qgis::TemporalUnit::Minutes ) )
    return Qgis::TemporalUnit::Minutes;
  if ( normalized == encodeUnit( Qgis::TemporalUnit::Hours ) )
    return Qgis::TemporalUnit::Hours;
  if ( normalized == encodeUnit( Qgis::TemporalUnit::Days ) )
    return Qgis::TemporalUnit::Days;
  if ( normalized == encodeUnit( Qgis::TemporalUnit::Weeks ) )
    return Qgis::TemporalUnit::Weeks;
  if ( normalized == encodeUnit( Qgis::TemporalUnit::Months ) )
    return Qgis::TemporalUnit::Months;
  if ( normalized == encodeUnit( Qgis::TemporalUnit::Years ) )
    return Qgis::TemporalUnit::Years;
  if ( normalized == encodeUnit( Qgis::TemporalUnit::Decades ) )
    return Qgis::TemporalUnit::Decades;
  if ( normalized == encodeUnit( Qgis::TemporalUnit::Centuries ) )
    return Qgis::TemporalUnit::Centuries;
  if ( normalized == encodeUnit( Qgis::TemporalUnit::IrregularStep ) )
    return Qgis::TemporalUnit::IrregularStep;
  if ( normalized == encodeUnit( Qgis::TemporalUnit::Unknown ) )
    return Qgis::TemporalUnit::Unknown;

  if ( ok )
    *ok = false;

  return Qgis::TemporalUnit::Unknown;
}

QString QgsUnitTypes::toString( Qgis::TemporalUnit unit )
{
  switch ( unit )
  {
    case Qgis::TemporalUnit::Seconds:
      return QObject::tr( "seconds", "temporal" );
    case Qgis::TemporalUnit::Milliseconds:
      return QObject::tr( "milliseconds", "temporal" );
    case Qgis::TemporalUnit::Minutes:
      return QObject::tr( "minutes", "temporal" );
    case Qgis::TemporalUnit::Hours:
      return QObject::tr( "hours", "temporal" );
    case Qgis::TemporalUnit::Days:
      return QObject::tr( "days", "temporal" );
    case Qgis::TemporalUnit::Weeks:
      return QObject::tr( "weeks", "temporal" );
    case Qgis::TemporalUnit::Months:
      return QObject::tr( "months", "temporal" );
    case Qgis::TemporalUnit::Years:
      return QObject::tr( "years", "temporal" );
    case Qgis::TemporalUnit::Decades:
      return QObject::tr( "decades", "temporal" );
    case Qgis::TemporalUnit::Centuries:
      return QObject::tr( "centuries", "temporal" );
    case Qgis::TemporalUnit::IrregularStep:
      return QObject::tr( "steps", "temporal" );
    case Qgis::TemporalUnit::Unknown:
      return QObject::tr( "<unknown>", "temporal" );
  }
  return QString();
}

QString QgsUnitTypes::toAbbreviatedString( Qgis::TemporalUnit unit )
{
  switch ( unit )
  {
    case Qgis::TemporalUnit::Seconds:
      return QObject::tr( "s", "temporal" );
    case Qgis::TemporalUnit::Milliseconds:
      return QObject::tr( "ms", "temporal" );
    case Qgis::TemporalUnit::Minutes:
      return QObject::tr( "min", "temporal" );
    case Qgis::TemporalUnit::Hours:
      return QObject::tr( "h", "temporal" );
    case Qgis::TemporalUnit::Days:
      return QObject::tr( "d", "temporal" );
    case Qgis::TemporalUnit::Weeks:
      return QObject::tr( "wk", "temporal" );
    case Qgis::TemporalUnit::Months:
      return QObject::tr( "mon", "temporal" );
    case Qgis::TemporalUnit::Years:
      return QObject::tr( "y", "temporal" );
    case Qgis::TemporalUnit::Decades:
      return QObject::tr( "dec", "temporal" );
    case Qgis::TemporalUnit::Centuries:
      return QObject::tr( "cen", "temporal" );
    case Qgis::TemporalUnit::IrregularStep:
      return QObject::tr( "steps", "temporal" );
    case Qgis::TemporalUnit::Unknown:
      return QObject::tr( "<unknown>", "temporal" );
  }
  return QString();
}

Qgis::TemporalUnit QgsUnitTypes::stringToTemporalUnit( const QString &string, bool *ok )
{
  const QString normalized = string.trimmed().toLower();

  if ( ok )
    *ok = true;

  if ( normalized == toString( Qgis::TemporalUnit::Seconds ) )
    return Qgis::TemporalUnit::Seconds;
  if ( normalized == toString( Qgis::TemporalUnit::Milliseconds ) )
    return Qgis::TemporalUnit::Milliseconds;
  if ( normalized == toString( Qgis::TemporalUnit::Minutes ) )
    return Qgis::TemporalUnit::Minutes;
  if ( normalized == toString( Qgis::TemporalUnit::Hours ) )
    return Qgis::TemporalUnit::Hours;
  if ( normalized == toString( Qgis::TemporalUnit::Days ) )
    return Qgis::TemporalUnit::Days;
  if ( normalized == toString( Qgis::TemporalUnit::Weeks ) )
    return Qgis::TemporalUnit::Weeks;
  if ( normalized == toString( Qgis::TemporalUnit::Months ) )
    return Qgis::TemporalUnit::Months;
  if ( normalized == toString( Qgis::TemporalUnit::Years ) )
    return Qgis::TemporalUnit::Years;
  if ( normalized == toString( Qgis::TemporalUnit::Decades ) )
    return Qgis::TemporalUnit::Decades;
  if ( normalized == toString( Qgis::TemporalUnit::Centuries ) )
    return Qgis::TemporalUnit::Centuries;
  if ( normalized == toString( Qgis::TemporalUnit::IrregularStep ) )
    return Qgis::TemporalUnit::IrregularStep;
  if ( normalized == toString( Qgis::TemporalUnit::Unknown ) )
    return Qgis::TemporalUnit::Unknown;

  if ( ok )
    *ok = false;

  return Qgis::TemporalUnit::Unknown;
}

double QgsUnitTypes::fromUnitToUnitFactor( Qgis::TemporalUnit fromUnit, Qgis::TemporalUnit toUnit )
{
  switch ( fromUnit )
  {
    case Qgis::TemporalUnit::Seconds:
    {
      switch ( toUnit )
      {
        case Qgis::TemporalUnit::Seconds:
          return 1.0;
        case Qgis::TemporalUnit::Milliseconds:
          return 1000.0;
        case Qgis::TemporalUnit::Minutes:
          return 1 / 60.0;
        case Qgis::TemporalUnit::Hours:
          return 1 / 3600.0;
        case Qgis::TemporalUnit::Days:
          return 1 / 86400.0;
        case Qgis::TemporalUnit::Weeks:
          return 1 / 604800.0;
        case Qgis::TemporalUnit::Months:
          return 1 / 2592000.0;
        case Qgis::TemporalUnit::Years:
          return 1 / 31557600.0;
        case Qgis::TemporalUnit::Decades:
          return 1 / 315576000.0;
        case Qgis::TemporalUnit::Centuries:
          return 1 / 3155760000.0;
        case Qgis::TemporalUnit::Unknown:
        case Qgis::TemporalUnit::IrregularStep:
          return 1.0;
      }
      break;
    }
    case Qgis::TemporalUnit::Milliseconds:
    {
      switch ( toUnit )
      {
        case Qgis::TemporalUnit::Seconds:
          return 1 / 1000.0;
        case Qgis::TemporalUnit::Milliseconds:
          return 1.0;
        case Qgis::TemporalUnit::Minutes:
          return 1 / 60000.0;
        case Qgis::TemporalUnit::Hours:
          return 1 / 3600000.0;
        case Qgis::TemporalUnit::Days:
          return 1 / 86400000.0;
        case Qgis::TemporalUnit::Weeks:
          return 1 / 604800000.0;
        case Qgis::TemporalUnit::Months:
          return 1 / 2592000000.0;
        case Qgis::TemporalUnit::Years:
          return 1 / 31557600000.0;
        case Qgis::TemporalUnit::Decades:
          return 1 / 315576000000.0;
        case Qgis::TemporalUnit::Centuries:
          return 1 / 3155760000000.0;
        case Qgis::TemporalUnit::Unknown:
        case Qgis::TemporalUnit::IrregularStep:
          return 1.0;
      }
      break;
    }
    case Qgis::TemporalUnit::Minutes:
    {
      switch ( toUnit )
      {
        case Qgis::TemporalUnit::Seconds:
          return 60.0;
        case Qgis::TemporalUnit::Milliseconds:
          return 60000.0;
        case Qgis::TemporalUnit::Minutes:
          return 1;
        case Qgis::TemporalUnit::Hours:
          return 1 / 60.0;
        case Qgis::TemporalUnit::Days:
          return 1 / 1440.0;
        case Qgis::TemporalUnit::Weeks:
          return 1 / 10080.0;
        case Qgis::TemporalUnit::Months:
          return 1 / 43200.0;
        case Qgis::TemporalUnit::Years:
          return 1 / 525960.0;
        case Qgis::TemporalUnit::Decades:
          return 1 / 5259600.0;
        case Qgis::TemporalUnit::Centuries:
          return 1 / 52596000.0;
        case Qgis::TemporalUnit::Unknown:
        case Qgis::TemporalUnit::IrregularStep:
          return 1.0;
      }
      break;
    }
    case Qgis::TemporalUnit::Hours:
    {
      switch ( toUnit )
      {
        case Qgis::TemporalUnit::Seconds:
          return 3600.0;
        case Qgis::TemporalUnit::Milliseconds:
          return 3600000.0;
        case Qgis::TemporalUnit::Minutes:
          return 60;
        case Qgis::TemporalUnit::Hours:
          return 1;
        case Qgis::TemporalUnit::Days:
          return 1 / 24.0;
        case Qgis::TemporalUnit::Weeks:
          return 1 / 168.0;
        case Qgis::TemporalUnit::Months:
          return 1 / 720.0;
        case Qgis::TemporalUnit::Years:
          return 1 / 8766.0;
        case Qgis::TemporalUnit::Decades:
          return 1 / 87660.0;
        case Qgis::TemporalUnit::Centuries:
          return 1 / 876600.0;
        case Qgis::TemporalUnit::Unknown:
        case Qgis::TemporalUnit::IrregularStep:
          return 1.0;
      }
      break;
    }
    case Qgis::TemporalUnit::Days:
    {
      switch ( toUnit )
      {
        case Qgis::TemporalUnit::Seconds:
          return 86400.0;
        case Qgis::TemporalUnit::Milliseconds:
          return 86400000.0;
        case Qgis::TemporalUnit::Minutes:
          return 1440;
        case Qgis::TemporalUnit::Hours:
          return 24;
        case Qgis::TemporalUnit::Days:
          return 1;
        case Qgis::TemporalUnit::Weeks:
          return 1 / 7.0;
        case Qgis::TemporalUnit::Months:
          return 1 / 30.0;
        case Qgis::TemporalUnit::Years:
          return 1 / 365.25;
        case Qgis::TemporalUnit::Decades:
          return 1 / 3652.5;
        case Qgis::TemporalUnit::Centuries:
          return 1 / 36525.0;
        case Qgis::TemporalUnit::Unknown:
        case Qgis::TemporalUnit::IrregularStep:
          return 1.0;
      }
      break;
    }
    case Qgis::TemporalUnit::Weeks:
    {
      switch ( toUnit )
      {
        case Qgis::TemporalUnit::Seconds:
          return 604800.0;
        case Qgis::TemporalUnit::Milliseconds:
          return 604800000.0;
        case Qgis::TemporalUnit::Minutes:
          return 10080;
        case Qgis::TemporalUnit::Hours:
          return 168;
        case Qgis::TemporalUnit::Days:
          return 7;
        case Qgis::TemporalUnit::Weeks:
          return 1;
        case Qgis::TemporalUnit::Months:
          return 7 / 30.0;
        case Qgis::TemporalUnit::Years:
          return 7 / 365.25;
        case Qgis::TemporalUnit::Decades:
          return 7 / 3652.5;
        case Qgis::TemporalUnit::Centuries:
          return 7 / 36525.0;
        case Qgis::TemporalUnit::Unknown:
        case Qgis::TemporalUnit::IrregularStep:
          return 1.0;
      }
      break;
    }
    case Qgis::TemporalUnit::Months:
    {
      switch ( toUnit )
      {
        case Qgis::TemporalUnit::Seconds:
          return 2592000.0;
        case Qgis::TemporalUnit::Milliseconds:
          return 2592000000.0;
        case Qgis::TemporalUnit::Minutes:
          return 43200;
        case Qgis::TemporalUnit::Hours:
          return 720;
        case Qgis::TemporalUnit::Days:
          return 30;
        case Qgis::TemporalUnit::Weeks:
          return 30 / 7.0;
        case Qgis::TemporalUnit::Months:
          return 1;
        case Qgis::TemporalUnit::Years:
          return 30 / 365.25;
        case Qgis::TemporalUnit::Decades:
          return 30 / 3652.5;
        case Qgis::TemporalUnit::Centuries:
          return 30 / 36525.0;
        case Qgis::TemporalUnit::Unknown:
        case Qgis::TemporalUnit::IrregularStep:
          return 1.0;
      }
      break;
    }
    case Qgis::TemporalUnit::Years:
    {
      switch ( toUnit )
      {
        case Qgis::TemporalUnit::Seconds:
          return 31557600.0;
        case Qgis::TemporalUnit::Milliseconds:
          return 31557600000.0;
        case Qgis::TemporalUnit::Minutes:
          return 525960.0;
        case Qgis::TemporalUnit::Hours:
          return 8766.0;
        case Qgis::TemporalUnit::Days:
          return 365.25;
        case Qgis::TemporalUnit::Weeks:
          return 365.25 / 7.0;
        case Qgis::TemporalUnit::Months:
          return 365.25 / 30.0;
        case Qgis::TemporalUnit::Years:
          return 1;
        case Qgis::TemporalUnit::Decades:
          return 0.1;
        case Qgis::TemporalUnit::Centuries:
          return 0.01;
        case Qgis::TemporalUnit::Unknown:
        case Qgis::TemporalUnit::IrregularStep:
          return 1.0;
      }
      break;
    }
    case Qgis::TemporalUnit::Decades:
    {
      switch ( toUnit )
      {
        case Qgis::TemporalUnit::Seconds:
          return 315576000.0;
        case Qgis::TemporalUnit::Milliseconds:
          return 315576000000.0;
        case Qgis::TemporalUnit::Minutes:
          return 5259600.0;
        case Qgis::TemporalUnit::Hours:
          return 87660.0;
        case Qgis::TemporalUnit::Days:
          return 3652.5;
        case Qgis::TemporalUnit::Weeks:
          return 3652.5 / 7.0;
        case Qgis::TemporalUnit::Months:
          return 3652.5 / 30.0;
        case Qgis::TemporalUnit::Years:
          return 10;
        case Qgis::TemporalUnit::Decades:
          return 1;
        case Qgis::TemporalUnit::Centuries:
          return 0.1;
        case Qgis::TemporalUnit::Unknown:
        case Qgis::TemporalUnit::IrregularStep:
          return 1.0;
      }
      break;
    }

    case Qgis::TemporalUnit::Centuries:
    {
      switch ( toUnit )
      {
        case Qgis::TemporalUnit::Seconds:
          return 3155760000.0;
        case Qgis::TemporalUnit::Milliseconds:
          return 3155760000000.0;
        case Qgis::TemporalUnit::Minutes:
          return 52596000.0;
        case Qgis::TemporalUnit::Hours:
          return 876600.0;
        case Qgis::TemporalUnit::Days:
          return 36525;
        case Qgis::TemporalUnit::Weeks:
          return 36525 / 7.0;
        case Qgis::TemporalUnit::Months:
          return 36525 / 30.0;
        case Qgis::TemporalUnit::Years:
          return 100;
        case Qgis::TemporalUnit::Decades:
          return 10;
        case Qgis::TemporalUnit::Centuries:
          return 1;
        case Qgis::TemporalUnit::Unknown:
        case Qgis::TemporalUnit::IrregularStep:
          return 1.0;
      }
      break;
    }

    case Qgis::TemporalUnit::Unknown:
    case Qgis::TemporalUnit::IrregularStep:
    {
      return 1.0;
    }
  }
  return 1.0;
}

Qgis::VolumeUnit QgsUnitTypes::decodeVolumeUnit( const QString &string, bool *ok )
{
  const QString normalized = string.trimmed().toLower();

  if ( ok )
    *ok = true;

  if ( normalized == encodeUnit( Qgis::VolumeUnit::CubicMeters ) )
    return Qgis::VolumeUnit::CubicMeters;
  if ( normalized == encodeUnit( Qgis::VolumeUnit::CubicFeet ) )
    return Qgis::VolumeUnit::CubicFeet;
  if ( normalized == encodeUnit( Qgis::VolumeUnit::CubicYards ) )
    return Qgis::VolumeUnit::CubicYards;
  if ( normalized == encodeUnit( Qgis::VolumeUnit::Barrel ) )
    return Qgis::VolumeUnit::Barrel;
  if ( normalized == encodeUnit( Qgis::VolumeUnit::CubicDecimeter ) )
    return Qgis::VolumeUnit::CubicDecimeter;
  if ( normalized == encodeUnit( Qgis::VolumeUnit::Liters ) )
    return Qgis::VolumeUnit::Liters;
  if ( normalized == encodeUnit( Qgis::VolumeUnit::GallonUS ) )
    return Qgis::VolumeUnit::GallonUS;
  if ( normalized == encodeUnit( Qgis::VolumeUnit::CubicInch ) )
    return Qgis::VolumeUnit::CubicInch;
  if ( normalized == encodeUnit( Qgis::VolumeUnit::CubicCentimeter ) )
    return Qgis::VolumeUnit::CubicCentimeter;
  if ( normalized == encodeUnit( Qgis::VolumeUnit::CubicDegrees ) )
    return Qgis::VolumeUnit::CubicDegrees;
  if ( normalized == encodeUnit( Qgis::VolumeUnit::Unknown ) )
    return Qgis::VolumeUnit::Unknown;

  if ( ok )
    *ok = false;

  return Qgis::VolumeUnit::Unknown;
}

QString QgsUnitTypes::toString( Qgis::VolumeUnit unit )
{
  switch ( unit )
  {
    case Qgis::VolumeUnit::CubicMeters:
      return QObject::tr( "cubic meters", "volume" );
    case Qgis::VolumeUnit::CubicFeet:
      return QObject::tr( "cubic feet", "volume" );
    case Qgis::VolumeUnit::CubicYards:
      return QObject::tr( "cubic yards", "volume" );
    case Qgis::VolumeUnit::Barrel:
      return QObject::tr( "barrels", "volume" );
    case Qgis::VolumeUnit::CubicDecimeter:
      return QObject::tr( "cubic decimeters", "volume" );
    case Qgis::VolumeUnit::Liters:
      return QObject::tr( "liters", "volume" );
    case Qgis::VolumeUnit::GallonUS:
      return QObject::tr( "gallons", "volume" );
    case Qgis::VolumeUnit::CubicInch:
      return QObject::tr( "cubic inches", "volume" );
    case Qgis::VolumeUnit::CubicCentimeter:
      return QObject::tr( "cubic centimeters", "volume" );
    case Qgis::VolumeUnit::CubicDegrees:
      return QObject::tr( "cubic degrees", "volume" );
    case Qgis::VolumeUnit::Unknown:
      return QObject::tr( "<unknown>", "volume" );
  }
  return QString();
}

QString QgsUnitTypes::toAbbreviatedString( Qgis::VolumeUnit unit )
{
  switch ( unit )
  {
    case Qgis::VolumeUnit::CubicMeters:
      return QObject::tr( "m³", "volume" );
    case Qgis::VolumeUnit::CubicFeet:
      return QObject::tr( "ft³", "volume" );
    case Qgis::VolumeUnit::CubicYards:
      return QObject::tr( "yds³", "volume" );
    case Qgis::VolumeUnit::Barrel:
      return QObject::tr( "bbl", "volume" );
    case Qgis::VolumeUnit::CubicDecimeter:
      return QObject::tr( "dm³", "volume" );
    case Qgis::VolumeUnit::Liters:
      return QObject::tr( "l", "volume" );
    case Qgis::VolumeUnit::GallonUS:
      return QObject::tr( "gal", "volume" );
    case Qgis::VolumeUnit::CubicInch:
      return QObject::tr( "in³", "volume" );
    case Qgis::VolumeUnit::CubicCentimeter:
      return QObject::tr( "cm³", "volume" );
    case Qgis::VolumeUnit::CubicDegrees:
      return QObject::tr( "deg³", "volume" );
    case Qgis::VolumeUnit::Unknown:
      return QObject::tr( "<unknown>", "volume" );
  }
  return QString();

}

Qgis::VolumeUnit QgsUnitTypes::stringToVolumeUnit( const QString &string, bool *ok )
{
  const QString normalized = string.trimmed().toLower();

  if ( ok )
    *ok = true;

  if ( normalized == toString( Qgis::VolumeUnit::CubicMeters ) )
    return Qgis::VolumeUnit::CubicMeters;
  if ( normalized == toString( Qgis::VolumeUnit::CubicFeet ) )
    return Qgis::VolumeUnit::CubicFeet;
  if ( normalized == toString( Qgis::VolumeUnit::CubicYards ) )
    return Qgis::VolumeUnit::CubicYards;
  if ( normalized == toString( Qgis::VolumeUnit::Barrel ) )
    return Qgis::VolumeUnit::Barrel;
  if ( normalized == toString( Qgis::VolumeUnit::CubicDecimeter ) )
    return Qgis::VolumeUnit::CubicDecimeter;
  if ( normalized == toString( Qgis::VolumeUnit::Liters ) )
    return Qgis::VolumeUnit::Liters;
  if ( normalized == toString( Qgis::VolumeUnit::GallonUS ) )
    return Qgis::VolumeUnit::GallonUS;
  if ( normalized == toString( Qgis::VolumeUnit::CubicInch ) )
    return Qgis::VolumeUnit::CubicInch;
  if ( normalized == toString( Qgis::VolumeUnit::CubicCentimeter ) )
    return Qgis::VolumeUnit::CubicCentimeter;
  if ( normalized == toString( Qgis::VolumeUnit::CubicDegrees ) )
    return Qgis::VolumeUnit::CubicDegrees;
  if ( normalized == toString( Qgis::VolumeUnit::Unknown ) )
    return Qgis::VolumeUnit::Unknown;

  if ( ok )
    *ok = false;

  return Qgis::VolumeUnit::Unknown;
}

#define DEG2_TO_M3 1379474361572186.2
double QgsUnitTypes::fromUnitToUnitFactor( Qgis::VolumeUnit fromUnit, Qgis::VolumeUnit toUnit )
{
  // cloned branches are intentional here for improved readability
  // NOLINTBEGIN(bugprone-branch-clone)
  switch ( fromUnit )
  {
    case Qgis::VolumeUnit::CubicMeters:
    {
      switch ( toUnit )
      {
        case Qgis::VolumeUnit::CubicMeters:
          return 1.0;
        case Qgis::VolumeUnit::CubicFeet:
          return 35.314666572222;
        case Qgis::VolumeUnit::CubicYards:
          return  1.307950613786;
        case Qgis::VolumeUnit::Barrel:
          return 6.2898107438466;
        case Qgis::VolumeUnit::CubicDecimeter:
          return 1000;
        case Qgis::VolumeUnit::Liters:
          return 1000;
        case Qgis::VolumeUnit::GallonUS:
          return 264.17205124156;
        case Qgis::VolumeUnit::CubicInch:
          return 61023.7438368;
        case Qgis::VolumeUnit::CubicCentimeter:
          return 1000000;
        case Qgis::VolumeUnit::CubicDegrees:
          return 1 / DEG2_TO_M3; // basically meaningless!
        case Qgis::VolumeUnit::Unknown:
          return 1.0;
      }
      break;
    }
    case Qgis::VolumeUnit::CubicFeet:
    {
      switch ( toUnit )
      {
        case Qgis::VolumeUnit::CubicMeters:
          return 0.028316846592;
        case Qgis::VolumeUnit::CubicFeet:
          return 1.0;
        case Qgis::VolumeUnit::CubicYards:
          return 0.037037037;
        case Qgis::VolumeUnit::Barrel:
          return 0.178107622;
        case Qgis::VolumeUnit::CubicDecimeter:
          return 28.31685;
        case Qgis::VolumeUnit::Liters:
          return 28.31685;
        case Qgis::VolumeUnit::GallonUS:
          return 7.480519954;
        case Qgis::VolumeUnit::CubicInch:
          return 1728.000629765;
        case Qgis::VolumeUnit::CubicCentimeter:
          return 28316.85;
        case Qgis::VolumeUnit::CubicDegrees:
          return 0.028316846592 / DEG2_TO_M3; // basically meaningless!
        case Qgis::VolumeUnit::Unknown:
          return 1.0;
      }
      break;
    }
    case Qgis::VolumeUnit::CubicYards:
    {
      switch ( toUnit )
      {
        case Qgis::VolumeUnit::CubicMeters:
          return 0.764554900;
        case Qgis::VolumeUnit::CubicFeet:
          return 26.999998234;
        case Qgis::VolumeUnit::CubicYards:
          return 1.0;
        case Qgis::VolumeUnit::Barrel:
          return 4.808905491;
        case Qgis::VolumeUnit::CubicDecimeter:
          return 764.5549;
        case Qgis::VolumeUnit::Liters:
          return 764.5549;
        case Qgis::VolumeUnit::GallonUS:
          return 201.974025549;
        case Qgis::VolumeUnit::CubicInch:
          return 46656.013952472;
        case Qgis::VolumeUnit::CubicCentimeter:
          return 764554.9;
        case Qgis::VolumeUnit::CubicDegrees:
          return 0.764554900 / DEG2_TO_M3; // basically meaningless!
        case Qgis::VolumeUnit::Unknown:
          return 1.0;
      }
      break;
    }
    case Qgis::VolumeUnit::Barrel:
    {
      switch ( toUnit )
      {
        case Qgis::VolumeUnit::CubicMeters:
          return 0.158987300;
        case Qgis::VolumeUnit::CubicFeet:
          return 5.614582837;
        case Qgis::VolumeUnit::CubicYards:
          return 0.207947526;
        case Qgis::VolumeUnit::Barrel:
          return 1.0;
        case Qgis::VolumeUnit::CubicDecimeter:
          return 158.9873;
        case Qgis::VolumeUnit::Liters:
          return 158.9873;
        case Qgis::VolumeUnit::GallonUS:
          return 41.999998943;
        case Qgis::VolumeUnit::CubicInch:
          return 9702.002677722;
        case Qgis::VolumeUnit::CubicCentimeter:
          return 158987.3;
        case Qgis::VolumeUnit::CubicDegrees:
          return 0.158987300 / DEG2_TO_M3; // basically meaningless!
        case Qgis::VolumeUnit::Unknown:
          return 1.0;
      }
      break;
    }
    case Qgis::VolumeUnit::CubicDecimeter:
    case Qgis::VolumeUnit::Liters:
    {
      switch ( toUnit )
      {
        case Qgis::VolumeUnit::CubicMeters:
          return 0.001;
        case Qgis::VolumeUnit::CubicFeet:
          return 0.035314662;
        case Qgis::VolumeUnit::CubicYards:
          return 0.001307951;
        case Qgis::VolumeUnit::Barrel:
          return 0.006289811;
        case Qgis::VolumeUnit::CubicDecimeter:
        case Qgis::VolumeUnit::Liters:
          return 1.0;
        case Qgis::VolumeUnit::GallonUS:
          return 0.264172037;
        case Qgis::VolumeUnit::CubicInch:
          return 61.023758990;
        case Qgis::VolumeUnit::CubicCentimeter:
          return 1000;
        case Qgis::VolumeUnit::CubicDegrees:
          return 0.001 / DEG2_TO_M3; // basically meaningless!
        case Qgis::VolumeUnit::Unknown:
          return 1.0;
      }
      break;
    }
    case Qgis::VolumeUnit::GallonUS:
    {
      switch ( toUnit )
      {
        case Qgis::VolumeUnit::CubicMeters:
          return 0.003785412;
        case Qgis::VolumeUnit::CubicFeet:
          return 0.133680547;
        case Qgis::VolumeUnit::CubicYards:
          return 0.004951132;
        case Qgis::VolumeUnit::Barrel:
          return 0.023809524;
        case Qgis::VolumeUnit::CubicDecimeter:
        case Qgis::VolumeUnit::Liters:
          return 3.785412000;
        case Qgis::VolumeUnit::GallonUS:
          return 1.0;
        case Qgis::VolumeUnit::CubicInch:
          return 231.000069567;
        case Qgis::VolumeUnit::CubicCentimeter:
          return 3785.412;
        case Qgis::VolumeUnit::CubicDegrees:
          return 0.003785412 / DEG2_TO_M3; // basically meaningless!
        case Qgis::VolumeUnit::Unknown:
          return 1.0;
      }
      break;
    }
    case Qgis::VolumeUnit::CubicInch:
    {
      switch ( toUnit )
      {
        case Qgis::VolumeUnit::CubicMeters:
          return 0.000016387;
        case Qgis::VolumeUnit::CubicFeet:
          return 0.000578703;
        case Qgis::VolumeUnit::CubicYards:
          return 0.000021433;
        case Qgis::VolumeUnit::Barrel:
          return 0.000103072;
        case Qgis::VolumeUnit::CubicDecimeter:
        case Qgis::VolumeUnit::Liters:
          return 0.016387060;
        case Qgis::VolumeUnit::GallonUS:
          return 0.004329003;
        case Qgis::VolumeUnit::CubicInch:
          return 1.0;
        case Qgis::VolumeUnit::CubicCentimeter:
          return 16.387060000;
        case Qgis::VolumeUnit::CubicDegrees:
          return 0.000016387 / DEG2_TO_M3; // basically meaningless!
        case Qgis::VolumeUnit::Unknown:
          return 1.0;
      }
      break;
    }
    case Qgis::VolumeUnit::CubicCentimeter:
    {
      switch ( toUnit )
      {
        case Qgis::VolumeUnit::CubicMeters:
          return 0.000001;
        case Qgis::VolumeUnit::CubicFeet:
          return 0.000035315;
        case Qgis::VolumeUnit::CubicYards:
          return 0.000001308;
        case Qgis::VolumeUnit::Barrel:
          return 0.000006290;
        case Qgis::VolumeUnit::CubicDecimeter:
        case Qgis::VolumeUnit::Liters:
          return 0.001;
        case Qgis::VolumeUnit::GallonUS:
          return 0.000264172 ;
        case Qgis::VolumeUnit::CubicInch:
          return 0.061023759;
        case Qgis::VolumeUnit::CubicCentimeter:
          return 1.0;
        case Qgis::VolumeUnit::CubicDegrees:
          return 0.000001 / DEG2_TO_M3; // basically meaningless!
        case Qgis::VolumeUnit::Unknown:
          return 1.0;
      }
      break;
    }
    case Qgis::VolumeUnit::CubicDegrees:
      if ( toUnit == Qgis::VolumeUnit::Unknown || toUnit == Qgis::VolumeUnit::CubicDegrees )
        return 1.0;
      else
        return fromUnitToUnitFactor( toUnit, Qgis::VolumeUnit::CubicMeters ) * DEG2_TO_M3;

    case Qgis::VolumeUnit::Unknown:
    {
      return 1.0;
    }
  }
  // NOLINTEND(bugprone-branch-clone)
  return 1.0;
}

Qgis::VolumeUnit QgsUnitTypes::distanceToVolumeUnit( Qgis::DistanceUnit distanceUnit )
{
  // cloned branches are intentional here for improved readability
  // NOLINTBEGIN(bugprone-branch-clone)
  switch ( distanceUnit )
  {
    case Qgis::DistanceUnit::Meters:
    case Qgis::DistanceUnit::MetersGermanLegal:
      return Qgis::VolumeUnit::CubicMeters;

    case Qgis::DistanceUnit::Kilometers:
      return Qgis::VolumeUnit::CubicMeters;

    case Qgis::DistanceUnit::Centimeters:
      return Qgis::VolumeUnit::CubicCentimeter;

    case Qgis::DistanceUnit::Millimeters:
      return Qgis::VolumeUnit::CubicCentimeter;

    case Qgis::DistanceUnit::Feet:
    case Qgis::DistanceUnit::FeetBritish1865:
    case Qgis::DistanceUnit::FeetBritish1936:
    case Qgis::DistanceUnit::FeetBritishBenoit1895A:
    case Qgis::DistanceUnit::FeetBritishBenoit1895B:
    case Qgis::DistanceUnit::FeetBritishSears1922Truncated:
    case Qgis::DistanceUnit::FeetBritishSears1922:
    case Qgis::DistanceUnit::FeetClarkes:
    case Qgis::DistanceUnit::FeetGoldCoast:
    case Qgis::DistanceUnit::FeetIndian:
    case Qgis::DistanceUnit::FeetIndian1937:
    case Qgis::DistanceUnit::FeetIndian1962:
    case Qgis::DistanceUnit::FeetIndian1975:
    case Qgis::DistanceUnit::FeetUSSurvey:
      return Qgis::VolumeUnit::CubicFeet;

    case Qgis::DistanceUnit::Yards:
    case Qgis::DistanceUnit::YardsBritishBenoit1895A:
    case Qgis::DistanceUnit::YardsBritishBenoit1895B:
    case Qgis::DistanceUnit::YardsBritishSears1922Truncated:
    case Qgis::DistanceUnit::YardsBritishSears1922:
    case Qgis::DistanceUnit::YardsClarkes:
    case Qgis::DistanceUnit::YardsIndian:
    case Qgis::DistanceUnit::YardsIndian1937:
    case Qgis::DistanceUnit::YardsIndian1962:
    case Qgis::DistanceUnit::YardsIndian1975:
      return Qgis::VolumeUnit::CubicYards;

    case Qgis::DistanceUnit::Miles:
    case Qgis::DistanceUnit::MilesUSSurvey:
    case Qgis::DistanceUnit::ChainsInternational:
    case Qgis::DistanceUnit::ChainsBritishBenoit1895A:
    case Qgis::DistanceUnit::ChainsBritishBenoit1895B:
    case Qgis::DistanceUnit::ChainsBritishSears1922Truncated:
    case Qgis::DistanceUnit::ChainsBritishSears1922:
    case Qgis::DistanceUnit::ChainsClarkes:
    case Qgis::DistanceUnit::ChainsUSSurvey:
    case Qgis::DistanceUnit::LinksInternational:
    case Qgis::DistanceUnit::LinksBritishBenoit1895A:
    case Qgis::DistanceUnit::LinksBritishBenoit1895B:
    case Qgis::DistanceUnit::LinksBritishSears1922Truncated:
    case Qgis::DistanceUnit::LinksBritishSears1922:
    case Qgis::DistanceUnit::LinksClarkes:
    case Qgis::DistanceUnit::LinksUSSurvey:
    case Qgis::DistanceUnit::Fathoms:
      return Qgis::VolumeUnit::CubicFeet;

    case Qgis::DistanceUnit::Degrees:
      return Qgis::VolumeUnit::CubicDegrees;

    case Qgis::DistanceUnit::Unknown:
      return Qgis::VolumeUnit::Unknown;

    case Qgis::DistanceUnit::NauticalMiles:
      return Qgis::VolumeUnit::CubicFeet;

    case Qgis::DistanceUnit::Inches:
      return Qgis::VolumeUnit::CubicInch;
  }
  // NOLINTEND(bugprone-branch-clone)

  return Qgis::VolumeUnit::Unknown;
}

Qgis::DistanceUnit QgsUnitTypes::volumeToDistanceUnit( Qgis::VolumeUnit volumeUnit )
{
  switch ( volumeUnit )
  {
    case Qgis::VolumeUnit::CubicMeters:
      return Qgis::DistanceUnit::Meters;
    case Qgis::VolumeUnit::CubicFeet:
      return Qgis::DistanceUnit::Feet;
    case Qgis::VolumeUnit::CubicYards:
      return Qgis::DistanceUnit::Yards;
    case Qgis::VolumeUnit::Barrel:
      return Qgis::DistanceUnit::Feet;
    case Qgis::VolumeUnit::CubicDecimeter:
      return Qgis::DistanceUnit::Centimeters;
    case Qgis::VolumeUnit::Liters:
      return Qgis::DistanceUnit::Meters;
    case Qgis::VolumeUnit::GallonUS:
      return Qgis::DistanceUnit::Feet;
    case Qgis::VolumeUnit::CubicInch:
      return Qgis::DistanceUnit::Inches;
    case Qgis::VolumeUnit::CubicCentimeter:
      return Qgis::DistanceUnit::Centimeters;
    case Qgis::VolumeUnit::CubicDegrees:
      return Qgis::DistanceUnit::Degrees;
    case Qgis::VolumeUnit::Unknown:
      return Qgis::DistanceUnit::Unknown;
  }
  return Qgis::DistanceUnit::Unknown;
}

Qgis::DistanceUnitType QgsUnitTypes::unitType( Qgis::VolumeUnit unit )
{
  switch ( unit )
  {
    case Qgis::VolumeUnit::CubicMeters:
    case Qgis::VolumeUnit::CubicFeet:
    case Qgis::VolumeUnit::CubicYards:
    case Qgis::VolumeUnit::Barrel:
    case Qgis::VolumeUnit::CubicDecimeter:
    case Qgis::VolumeUnit::Liters:
    case Qgis::VolumeUnit::GallonUS:
    case Qgis::VolumeUnit::CubicInch:
    case Qgis::VolumeUnit::CubicCentimeter:
      return Qgis::DistanceUnitType::Standard;
    case Qgis::VolumeUnit::CubicDegrees:
      return Qgis::DistanceUnitType::Geographic;
    case Qgis::VolumeUnit::Unknown:
      return Qgis::DistanceUnitType::Unknown;
  }
  return Qgis::DistanceUnitType::Unknown;
}

QString QgsUnitTypes::encodeUnit( Qgis::VolumeUnit unit )
{
  switch ( unit )
  {
    case Qgis::VolumeUnit::CubicMeters:
      return QStringLiteral( "m3" );
    case Qgis::VolumeUnit::CubicFeet:
      return QStringLiteral( "ft3" );
    case Qgis::VolumeUnit::CubicYards:
      return QStringLiteral( "yd3" );
    case Qgis::VolumeUnit::Barrel:
      return QStringLiteral( "bbl" );
    case Qgis::VolumeUnit::CubicDecimeter:
      return QStringLiteral( "dm3" );
    case Qgis::VolumeUnit::Liters:
      return QStringLiteral( "l" );
    case Qgis::VolumeUnit::GallonUS:
      return QStringLiteral( "gal" );
    case Qgis::VolumeUnit::CubicInch:
      return QStringLiteral( "in3" );
    case Qgis::VolumeUnit::CubicCentimeter:
      return QStringLiteral( "cm3" );
    case Qgis::VolumeUnit::CubicDegrees:
      return QStringLiteral( "deg3" );
    case Qgis::VolumeUnit::Unknown:
      return QStringLiteral( "<unknown>" );
  }
  return QString();
}

QString QgsUnitTypes::encodeUnit( Qgis::AngleUnit unit )
{
  switch ( unit )
  {
    case Qgis::AngleUnit::Degrees:
      return QStringLiteral( "degrees" );
    case Qgis::AngleUnit::Radians:
      return QStringLiteral( "radians" );
    case Qgis::AngleUnit::Gon:
      return QStringLiteral( "gon" );
    case Qgis::AngleUnit::MinutesOfArc:
      return QStringLiteral( "moa" );
    case Qgis::AngleUnit::SecondsOfArc:
      return QStringLiteral( "soa" );
    case Qgis::AngleUnit::Turn:
      return QStringLiteral( "tr" );
    case Qgis::AngleUnit::MilliradiansSI:
      return QStringLiteral( "milliradians" );
    case Qgis::AngleUnit::MilNATO:
      return QStringLiteral( "mil" );
    case Qgis::AngleUnit::Unknown:
      return QStringLiteral( "<unknown>" );
  }
  return QString();
}

Qgis::AngleUnit QgsUnitTypes::decodeAngleUnit( const QString &string, bool *ok )
{
  const QString normalized = string.trimmed().toLower();

  if ( ok )
    *ok = true;

  if ( normalized == encodeUnit( Qgis::AngleUnit::Degrees ) )
    return Qgis::AngleUnit::Degrees;
  if ( normalized == encodeUnit( Qgis::AngleUnit::Radians ) )
    return Qgis::AngleUnit::Radians;
  if ( normalized == encodeUnit( Qgis::AngleUnit::Gon ) )
    return Qgis::AngleUnit::Gon;
  if ( normalized == encodeUnit( Qgis::AngleUnit::MinutesOfArc ) )
    return Qgis::AngleUnit::MinutesOfArc;
  if ( normalized == encodeUnit( Qgis::AngleUnit::SecondsOfArc ) )
    return Qgis::AngleUnit::SecondsOfArc;
  if ( normalized == encodeUnit( Qgis::AngleUnit::Turn ) )
    return Qgis::AngleUnit::Turn;
  if ( normalized == encodeUnit( Qgis::AngleUnit::MilliradiansSI ) )
    return Qgis::AngleUnit::MilliradiansSI;
  if ( normalized == encodeUnit( Qgis::AngleUnit::MilNATO ) )
    return Qgis::AngleUnit::MilNATO;
  if ( normalized == encodeUnit( Qgis::AngleUnit::Unknown ) )
    return Qgis::AngleUnit::Unknown;
  if ( ok )
    *ok = false;

  return Qgis::AngleUnit::Unknown;
}

QString QgsUnitTypes::toString( Qgis::AngleUnit unit )
{
  switch ( unit )
  {
    case Qgis::AngleUnit::Degrees:
      return QObject::tr( "degrees", "angle" );
    case Qgis::AngleUnit::Radians:
      return QObject::tr( "radians", "angle" );
    case Qgis::AngleUnit::Gon:
      return QObject::tr( "gon", "angle" );
    case Qgis::AngleUnit::MinutesOfArc:
      return QObject::tr( "minutes of arc", "angle" );
    case Qgis::AngleUnit::SecondsOfArc:
      return QObject::tr( "seconds of arc", "angle" );
    case Qgis::AngleUnit::Turn:
      return QObject::tr( "turns", "angle" );
    case Qgis::AngleUnit::MilliradiansSI:
      return QObject::tr( "milliradians", "angle" );
    case Qgis::AngleUnit::MilNATO:
      return QObject::tr( "mil", "angle" );
    case Qgis::AngleUnit::Unknown:
      return QObject::tr( "<unknown>", "angle" );
  }
  return QString();
}

double QgsUnitTypes::fromUnitToUnitFactor( Qgis::AngleUnit fromUnit, Qgis::AngleUnit toUnit )
{
  // Calculate the conversion factor between the specified units
  switch ( fromUnit )
  {
    case Qgis::AngleUnit::Degrees:
    {
      switch ( toUnit )
      {
        case Qgis::AngleUnit::Degrees:
          return 1.0;
        case Qgis::AngleUnit::Radians:
          return M_PI / 180.0;
        case Qgis::AngleUnit::Gon:
          return 400.0 / 360.0;
        case Qgis::AngleUnit::MinutesOfArc:
          return 60;
        case Qgis::AngleUnit::SecondsOfArc:
          return 3600;
        case Qgis::AngleUnit::Turn:
          return 1.0 / 360.0;
        case Qgis::AngleUnit::MilliradiansSI:
          return M_PI / 180.0 * 1000;
        case Qgis::AngleUnit::MilNATO:
          return 3200.0 / 180;
        case Qgis::AngleUnit::Unknown:
          break;
      }
      break;
    }
    case Qgis::AngleUnit::Radians:
    {
      switch ( toUnit )
      {
        case Qgis::AngleUnit::Degrees:
          return 180.0 / M_PI;
        case Qgis::AngleUnit::Radians:
          return 1.0;
        case Qgis::AngleUnit::Gon:
          return 200.0 / M_PI;
        case Qgis::AngleUnit::MinutesOfArc:
          return 60 * 180.0 / M_PI;
        case Qgis::AngleUnit::SecondsOfArc:
          return 3600 * 180.0 / M_PI;
        case Qgis::AngleUnit::Turn:
          return 0.5 / M_PI;
        case Qgis::AngleUnit::MilliradiansSI:
          return 1000;
        case Qgis::AngleUnit::MilNATO:
          return 3200.0 / M_PI;
        case Qgis::AngleUnit::Unknown:
          break;
      }
      break;
    }
    case Qgis::AngleUnit::Gon:
    {
      switch ( toUnit )
      {
        case Qgis::AngleUnit::Degrees:
          return 360.0 / 400.0;
        case Qgis::AngleUnit::Radians:
          return M_PI / 200.0;
        case Qgis::AngleUnit::Gon:
          return 1.0;
        case Qgis::AngleUnit::MinutesOfArc:
          return 60 * 360.0 / 400.0;
        case Qgis::AngleUnit::SecondsOfArc:
          return 3600 * 360.0 / 400.0;
        case Qgis::AngleUnit::Turn:
          return 1.0 / 400.0;
        case Qgis::AngleUnit::MilliradiansSI:
          return M_PI / 200.0 * 1000;
        case Qgis::AngleUnit::MilNATO:
          return 3200.0 / 200.0;
        case Qgis::AngleUnit::Unknown:
          break;
      }
      break;
    }
    case Qgis::AngleUnit::MinutesOfArc:
    {
      switch ( toUnit )
      {
        case Qgis::AngleUnit::Degrees:
          return 1 / 60.0;
        case Qgis::AngleUnit::Radians:
          return M_PI / 180.0 / 60.0;
        case Qgis::AngleUnit::Gon:
          return 400.0 / 360.0 / 60.0;
        case Qgis::AngleUnit::MinutesOfArc:
          return 1.0;
        case Qgis::AngleUnit::SecondsOfArc:
          return 60.0;
        case Qgis::AngleUnit::Turn:
          return 1.0 / 360.0 / 60.0;
        case Qgis::AngleUnit::MilliradiansSI:
          return M_PI / 180.0 / 60.0 * 1000;
        case Qgis::AngleUnit::MilNATO:
          return 3200.0 / 180.0 / 60.0;
        case Qgis::AngleUnit::Unknown:
          break;
      }
      break;
    }
    case Qgis::AngleUnit::SecondsOfArc:
    {
      switch ( toUnit )
      {
        case Qgis::AngleUnit::Degrees:
          return 1 / 3600.0;
        case Qgis::AngleUnit::Radians:
          return M_PI / 180.0 / 3600.0;
        case Qgis::AngleUnit::Gon:
          return 400.0 / 360.0 / 3600.0;
        case Qgis::AngleUnit::MinutesOfArc:
          return 1.0 / 60.0;
        case Qgis::AngleUnit::SecondsOfArc:
          return 1.0;
        case Qgis::AngleUnit::Turn:
          return 1.0 / 360.0 / 3600.0;
        case Qgis::AngleUnit::MilliradiansSI:
          return M_PI / 180.0 / 3600.0 * 1000;
        case Qgis::AngleUnit::MilNATO:
          return 3200.0 / 180.0 / 3600.0;
        case Qgis::AngleUnit::Unknown:
          break;
      }
      break;
    }
    case Qgis::AngleUnit::Turn:
    {
      switch ( toUnit )
      {
        case Qgis::AngleUnit::Degrees:
          return 360.0;
        case Qgis::AngleUnit::Radians:
          return 2 * M_PI;
        case Qgis::AngleUnit::Gon:
          return 400.0;
        case Qgis::AngleUnit::MinutesOfArc:
          return 360.0 * 60.0;
        case Qgis::AngleUnit::SecondsOfArc:
          return 360.0 * 3600.0;
        case Qgis::AngleUnit::Turn:
          return 1.0;
        case Qgis::AngleUnit::MilliradiansSI:
          return 2 * M_PI * 1000;
        case Qgis::AngleUnit::MilNATO:
          return 2 * 3200;
        case Qgis::AngleUnit::Unknown:
          break;
      }
      break;
    }
    case Qgis::AngleUnit::MilliradiansSI:
    {
      switch ( toUnit )
      {
        case Qgis::AngleUnit::Degrees:
          return 180.0 / M_PI / 1000;
        case Qgis::AngleUnit::Radians:
          return 0.001;
        case Qgis::AngleUnit::Gon:
          return 200.0 / M_PI / 1000;
        case Qgis::AngleUnit::MinutesOfArc:
          return 180.0 * 60.0 / M_PI / 1000;
        case Qgis::AngleUnit::SecondsOfArc:
          return 180.0 * 3600.0 / M_PI / 1000;
        case Qgis::AngleUnit::Turn:
          return M_PI / 2 / 1000;
        case Qgis::AngleUnit::MilliradiansSI:
          return 1.0;
        case Qgis::AngleUnit::MilNATO:
          return 3200.0 / 1000.0 / M_PI;
        case Qgis::AngleUnit::Unknown:
          break;
      }
      break;
    }

    case Qgis::AngleUnit::MilNATO:
    {
      switch ( toUnit )
      {
        case Qgis::AngleUnit::Degrees:
          return 180.0 / 3200;
        case Qgis::AngleUnit::Radians:
          return M_PI / 3200;
        case Qgis::AngleUnit::Gon:
          return 200.0 / 3200;
        case Qgis::AngleUnit::MinutesOfArc:
          return 60 * 180.0 / 3200;
        case Qgis::AngleUnit::SecondsOfArc:
          return 3600.0 * 180 / 3200;
        case Qgis::AngleUnit::Turn:
          return 1.0 / ( 2 * 32000 );
        case Qgis::AngleUnit::MilliradiansSI:
          return 1000.0 * M_PI / 3200.0;
        case Qgis::AngleUnit::MilNATO:
          return 1.0;
        case Qgis::AngleUnit::Unknown:
          break;
      }
      break;
    }

    case Qgis::AngleUnit::Unknown:
      break;
  }
  return 1.0;
}

QString QgsUnitTypes::formatAngle( double angle, int decimals, Qgis::AngleUnit unit )
{
  QString unitLabel;
  int decimalPlaces = 2;

  switch ( unit )
  {
    case Qgis::AngleUnit::Degrees:
      unitLabel = QObject::tr( "°", "angle" );
      decimalPlaces = 0;
      break;
    case Qgis::AngleUnit::Radians:
      unitLabel = QObject::tr( " rad", "angle" );
      decimalPlaces = 2;
      break;
    case Qgis::AngleUnit::Gon:
      unitLabel = QObject::tr( " gon", "angle" );
      decimalPlaces = 0;
      break;
    case Qgis::AngleUnit::MinutesOfArc:
      unitLabel = QObject::tr( "′", "angle minutes" );
      decimalPlaces = 0;
      break;
    case Qgis::AngleUnit::SecondsOfArc:
      unitLabel = QObject::tr( "″", "angle seconds" );
      decimalPlaces = 0;
      break;
    case Qgis::AngleUnit::Turn:
      unitLabel = QObject::tr( " tr", "angle turn" );
      decimalPlaces = 3;
      break;
    case Qgis::AngleUnit::MilliradiansSI:
      unitLabel = QObject::tr( " millirad", "angular mil SI" );
      decimalPlaces = 0;
      break;
    case Qgis::AngleUnit::MilNATO:
      unitLabel = QObject::tr( " mil", "angular mil NATO" );
      decimalPlaces = 0;
      break;
    case Qgis::AngleUnit::Unknown:
      break;
  }

  if ( decimals >= 0 )
    decimalPlaces = decimals;

  return QStringLiteral( "%L1%2" ).arg( angle, 0, 'f', decimalPlaces ).arg( unitLabel );
}

QgsUnitTypes::DistanceValue QgsUnitTypes::scaledDistance( double distance,  Qgis::DistanceUnit unit, int decimals, bool keepBaseUnit )
{
  DistanceValue result;

  // cloned branches are intentional here for improved readability
  // NOLINTBEGIN(bugprone-branch-clone)
  switch ( unit )
  {
    case Qgis::DistanceUnit::Meters:
      if ( keepBaseUnit )
      {
        result.value = qgsRound( distance, decimals );
        result.unit = Qgis::DistanceUnit::Meters;
      }
      else if ( std::fabs( distance ) > 1000.0 )
      {
        result.value = qgsRound( distance / 1000, decimals );
        result.unit = Qgis::DistanceUnit::Kilometers;
      }
      else if ( std::fabs( distance ) < 0.01 )
      {
        result.value = qgsRound( distance * 1000, decimals );
        result.unit = Qgis::DistanceUnit::Millimeters;
      }
      else if ( std::fabs( distance ) < 0.1 )
      {

        result.value = qgsRound( distance * 100, decimals );
        result.unit = Qgis::DistanceUnit::Centimeters;
      }
      else
      {
        result.value = qgsRound( distance, decimals );
        result.unit = Qgis::DistanceUnit::Meters;
      }
      break;

    case Qgis::DistanceUnit::Kilometers:
      if ( keepBaseUnit || std::fabs( distance ) >= 1.0 )
      {
        result.value = qgsRound( distance, decimals );
        result.unit = Qgis::DistanceUnit::Kilometers;
      }
      else
      {
        result.value = qgsRound( distance * 1000, decimals );
        result.unit = Qgis::DistanceUnit::Meters;
      }
      break;

    case Qgis::DistanceUnit::Feet:
      if ( std::fabs( distance ) <= 5280.0 || keepBaseUnit )
      {
        result.value = qgsRound( distance, decimals );
        result.unit = Qgis::DistanceUnit::Feet;
      }
      else
      {
        result.value = qgsRound( distance / 5280.0, decimals );
        result.unit = Qgis::DistanceUnit::Miles;
      }
      break;

    case Qgis::DistanceUnit::Yards:
      if ( std::fabs( distance ) <= 1760.0 || keepBaseUnit )
      {
        result.value = qgsRound( distance, decimals );
        result.unit = Qgis::DistanceUnit::Yards;
      }
      else
      {
        result.value = qgsRound( distance / 1760.0, decimals );
        result.unit = Qgis::DistanceUnit::Miles;
      }
      break;

    case Qgis::DistanceUnit::Miles:
      if ( std::fabs( distance ) >= 1.0 || keepBaseUnit )
      {
        result.value = qgsRound( distance, decimals );
        result.unit = Qgis::DistanceUnit::Miles;
      }
      else
      {
        result.value = qgsRound( distance * 5280.0, decimals );
        result.unit = Qgis::DistanceUnit::Feet;
      }
      break;

    case Qgis::DistanceUnit::NauticalMiles:
      result.value = qgsRound( distance, decimals );
      result.unit = Qgis::DistanceUnit::NauticalMiles;
      break;

    case Qgis::DistanceUnit::Degrees:
      result.value = qgsRound( distance, decimals );
      result.unit = Qgis::DistanceUnit::Degrees;
      break;

    case Qgis::DistanceUnit::Unknown:
      result.value = qgsRound( distance, decimals );
      result.unit = Qgis::DistanceUnit::Unknown;
      break;

    default:
      result.value = qgsRound( distance, decimals );
      result.unit = unit;
      break;
  }
  // NOLINTEND(bugprone-branch-clone)

  return result;
}

QgsUnitTypes::AreaValue QgsUnitTypes::scaledArea( double area, Qgis::AreaUnit unit, int decimals, bool keepBaseUnit )
{
  QgsUnitTypes::AreaValue result;
  result.value = -1.0;
  result.unit = Qgis::AreaUnit::Unknown;

  // If we are not forced to keep the base units, switch to meter calculation
  if ( unit == Qgis::AreaUnit::SquareMillimeters )
  {
    if ( keepBaseUnit )
    {
      result.value = qgsRound( area, decimals );
      result.unit = Qgis::AreaUnit::SquareMillimeters;
    }
    else
    {
      area /= 1000000.0;
      unit = Qgis::AreaUnit::SquareMeters;
    }
  }
  else if ( unit == Qgis::AreaUnit::SquareCentimeters )
  {
    if ( keepBaseUnit )
    {
      result.value = qgsRound( area, decimals );
      result.unit = Qgis::AreaUnit::SquareCentimeters;
    }
    else
    {
      area /= 10000.0;
      unit = Qgis::AreaUnit::SquareMeters;
    }
  }

  // cloned branches are intentional here for improved readability
  // NOLINTBEGIN(bugprone-branch-clone)
  switch ( unit )
  {
    case Qgis::AreaUnit::SquareCentimeters:
      // handled in the if above
      break;
    case Qgis::AreaUnit::SquareMillimeters:
      // handled in the if above
      break;
    case Qgis::AreaUnit::SquareMeters:
    {
      if ( keepBaseUnit )
      {
        result.value = qgsRound( area, decimals );
        result.unit = Qgis::AreaUnit::SquareMeters;
      }
      else if ( std::fabs( area ) > QgsUnitTypes::fromUnitToUnitFactor( Qgis::AreaUnit::SquareKilometers, Qgis::AreaUnit::SquareMeters ) )
      {
        result.value = qgsRound( area * QgsUnitTypes::fromUnitToUnitFactor( Qgis::AreaUnit::SquareMeters, Qgis::AreaUnit::SquareKilometers ), decimals );
        result.unit = Qgis::AreaUnit::SquareKilometers;
      }
      else if ( std::fabs( area ) > QgsUnitTypes::fromUnitToUnitFactor( Qgis::AreaUnit::Hectares, Qgis::AreaUnit::SquareMeters ) )
      {
        result.value = qgsRound( area * QgsUnitTypes::fromUnitToUnitFactor( Qgis::AreaUnit::SquareMeters, Qgis::AreaUnit::Hectares ), decimals );
        result.unit = Qgis::AreaUnit::Hectares;
      }
      else
      {
        result.value = qgsRound( area, decimals );
        result.unit = Qgis::AreaUnit::SquareMeters;
      }
      break;
    }

    case Qgis::AreaUnit::SquareKilometers:
    {
      result.value = qgsRound( area, decimals );
      result.unit = Qgis::AreaUnit::SquareKilometers;
      break;
    }

    case Qgis::AreaUnit::SquareInches:
    {
      result.value = qgsRound( area, decimals );
      result.unit = Qgis::AreaUnit::SquareInches;
      break;
    }

    case Qgis::AreaUnit::SquareFeet:
    {
      if ( keepBaseUnit )
      {
        result.value = qgsRound( area, decimals );
        result.unit = Qgis::AreaUnit::SquareFeet;
      }
      else if ( std::fabs( area ) > QgsUnitTypes::fromUnitToUnitFactor( Qgis::AreaUnit::SquareMiles, Qgis::AreaUnit::SquareFeet ) )
      {
        result.value = qgsRound( area * QgsUnitTypes::fromUnitToUnitFactor( Qgis::AreaUnit::SquareFeet, Qgis::AreaUnit::SquareMiles ), decimals );
        result.unit = Qgis::AreaUnit::SquareMiles;
      }
      else
      {
        result.value = qgsRound( area, decimals );
        result.unit = Qgis::AreaUnit::SquareFeet;
      }
      break;
    }

    case Qgis::AreaUnit::SquareYards:
    {
      if ( keepBaseUnit )
      {
        result.value = qgsRound( area, decimals );
        result.unit = Qgis::AreaUnit::SquareYards;
      }
      else if ( std::fabs( area ) > QgsUnitTypes::fromUnitToUnitFactor( Qgis::AreaUnit::SquareMiles, Qgis::AreaUnit::SquareYards ) )
      {
        result.value = qgsRound( area * QgsUnitTypes::fromUnitToUnitFactor( Qgis::AreaUnit::SquareYards, Qgis::AreaUnit::SquareMiles ), decimals );
        result.unit = Qgis::AreaUnit::SquareMiles;
      }
      else
      {
        result.value = qgsRound( area, decimals );
        result.unit = Qgis::AreaUnit::SquareYards;
      }
      break;
    }

    case Qgis::AreaUnit::SquareMiles:
    {
      result.value = qgsRound( area, decimals );
      result.unit = Qgis::AreaUnit::SquareMiles;
      break;
    }

    case Qgis::AreaUnit::Hectares:
    {
      if ( keepBaseUnit )
      {
        result.value = qgsRound( area, decimals );
        result.unit = Qgis::AreaUnit::Hectares;
      }
      else if ( std::fabs( area ) > QgsUnitTypes::fromUnitToUnitFactor( Qgis::AreaUnit::SquareKilometers, Qgis::AreaUnit::Hectares ) )
      {
        result.value = qgsRound( area * QgsUnitTypes::fromUnitToUnitFactor( Qgis::AreaUnit::Hectares, Qgis::AreaUnit::SquareKilometers ), decimals );
        result.unit = Qgis::AreaUnit::SquareKilometers;
      }
      else
      {
        result.value = qgsRound( area, decimals );
        result.unit = Qgis::AreaUnit::Hectares;
      }
      break;
    }

    case Qgis::AreaUnit::Acres:
    {
      if ( keepBaseUnit )
      {
        result.value = qgsRound( area, decimals );
        result.unit = Qgis::AreaUnit::Acres;
      }
      else if ( std::fabs( area ) > QgsUnitTypes::fromUnitToUnitFactor( Qgis::AreaUnit::SquareMiles, Qgis::AreaUnit::Acres ) )
      {
        result.value = qgsRound( area * QgsUnitTypes::fromUnitToUnitFactor( Qgis::AreaUnit::Acres, Qgis::AreaUnit::SquareMiles ), decimals );
        result.unit = Qgis::AreaUnit::SquareMiles;
      }
      else
      {
        result.value = qgsRound( area, decimals );
        result.unit = Qgis::AreaUnit::Acres;
      }
      break;
    }

    case Qgis::AreaUnit::SquareNauticalMiles:
    {
      result.value = qgsRound( area, decimals );
      result.unit = Qgis::AreaUnit::SquareNauticalMiles;
      break;
    }

    case Qgis::AreaUnit::SquareDegrees:
    {
      result.value = qgsRound( area, decimals );
      result.unit = Qgis::AreaUnit::SquareDegrees;
      break;
    }

    case Qgis::AreaUnit::Unknown:
    {
      result.value = qgsRound( area, decimals );
      result.unit = Qgis::AreaUnit::Unknown;
      break;
    }
  }
  // NOLINTEND(bugprone-branch-clone)
  return result;
}


QString QgsUnitTypes::formatDistance( double distance, int decimals,  Qgis::DistanceUnit unit, bool keepBaseUnit )
{
  const DistanceValue dist = scaledDistance( distance, unit, decimals, keepBaseUnit );

  QString unitText;

  if ( dist.unit != Qgis::DistanceUnit::Unknown )
    unitText = QChar( ' ' ) + QgsUnitTypes::toAbbreviatedString( dist.unit );

  if ( qgsDoubleNear( dist.value, 0 ) )
  {
    unitText = QChar( ' ' ) + QgsUnitTypes::toAbbreviatedString( unit );
    return QStringLiteral( "%L1%2" ).arg( distance, 0, 'e', decimals ).arg( unitText );
  }
  else
  {
    return QStringLiteral( "%L1%2" ).arg( dist.value, 0, 'f', decimals ).arg( unitText );
  }
}

QString QgsUnitTypes::formatArea( double area, int decimals, Qgis::AreaUnit unit, bool keepBaseUnit )
{
  const QgsUnitTypes::AreaValue areaValue = scaledArea( area, unit, decimals, keepBaseUnit );

  QString unitText;

  if ( areaValue.unit != Qgis::AreaUnit::Unknown )
    unitText = QChar( ' ' ) + QgsUnitTypes::toAbbreviatedString( areaValue.unit );

  if ( qgsDoubleNear( areaValue.value, 0 ) )
  {
    unitText = QChar( ' ' ) + QgsUnitTypes::toAbbreviatedString( unit );
    return QStringLiteral( "%L1%2" ).arg( area, 0, 'e', decimals ).arg( unitText );
  }
  else
  {
    return QStringLiteral( "%L1%2" ).arg( areaValue.value, 0, 'f', decimals ).arg( unitText );
  }
}

QString QgsUnitTypes::encodeUnit( Qgis::RenderUnit unit )
{
  switch ( unit )
  {
    case Qgis::RenderUnit::Millimeters:
      return QStringLiteral( "MM" );
    case Qgis::RenderUnit::MetersInMapUnits:
      return QStringLiteral( "RenderMetersInMapUnits" );
    case Qgis::RenderUnit::MapUnits:
      return QStringLiteral( "MapUnit" );
    case Qgis::RenderUnit::Pixels:
      return QStringLiteral( "Pixel" );
    case Qgis::RenderUnit::Percentage:
      return QStringLiteral( "Percentage" );
    case Qgis::RenderUnit::Points:
      return QStringLiteral( "Point" );
    case Qgis::RenderUnit::Inches:
      return QStringLiteral( "Inch" );
    case Qgis::RenderUnit::Unknown:
      return QString();
  }
  return QString();
}

Qgis::RenderUnit QgsUnitTypes::decodeRenderUnit( const QString &string, bool *ok )
{
  const QString normalized = string.trimmed().toLower();

  if ( ok )
    *ok = true;

  if ( normalized == encodeUnit( Qgis::RenderUnit::Millimeters ).toLower() )
    return Qgis::RenderUnit::Millimeters;
  if ( normalized == encodeUnit( Qgis::RenderUnit::MetersInMapUnits ).toLower() )
    return Qgis::RenderUnit::MetersInMapUnits;
  if ( normalized == QLatin1String( "meters" ) )
    return Qgis::RenderUnit::MetersInMapUnits;
  if ( normalized == encodeUnit( Qgis::RenderUnit::MapUnits ).toLower() )
    return Qgis::RenderUnit::MapUnits;
  if ( normalized == QLatin1String( "mapunits" ) )
    return Qgis::RenderUnit::MapUnits;
  if ( normalized == encodeUnit( Qgis::RenderUnit::Pixels ).toLower() )
    return Qgis::RenderUnit::Pixels;
  if ( normalized == encodeUnit( Qgis::RenderUnit::Percentage ).toLower() )
    return Qgis::RenderUnit::Percentage;
  if ( normalized == QLatin1String( "percent" ) )
    return Qgis::RenderUnit::Percentage;
  if ( normalized == encodeUnit( Qgis::RenderUnit::Points ).toLower() )
    return Qgis::RenderUnit::Points;
  if ( normalized == QLatin1String( "points" ) )
    return Qgis::RenderUnit::Points;
  if ( normalized == encodeUnit( Qgis::RenderUnit::Inches ).toLower() )
    return Qgis::RenderUnit::Inches;

  if ( ok )
    *ok = false;

  // millimeters are default
  return Qgis::RenderUnit::Millimeters;
}

QString QgsUnitTypes::toString( Qgis::RenderUnit unit )
{
  switch ( unit )
  {
    case Qgis::RenderUnit::Millimeters:
      return QObject::tr( "millimeters", "render" );

    case Qgis::RenderUnit::MetersInMapUnits:
      return QObject::tr( "meters at scale", "render" );

    case Qgis::RenderUnit::MapUnits:
      return QObject::tr( "map units", "render" );

    case Qgis::RenderUnit::Pixels:
      return QObject::tr( "pixels", "render" );

    case Qgis::RenderUnit::Percentage:
      return QObject::tr( "percent", "render" );

    case Qgis::RenderUnit::Points:
      return QObject::tr( "points", "render" );

    case Qgis::RenderUnit::Inches:
      return QObject::tr( "inches", "render" );

    case Qgis::RenderUnit::Unknown:
      return QObject::tr( "<unknown>", "render" );

  }
  return QString();
}



QString QgsUnitTypes::encodeUnit( Qgis::LayoutUnit unit )
{
  switch ( unit )
  {
    case Qgis::LayoutUnit::Centimeters:
      return QStringLiteral( "cm" );
    case Qgis::LayoutUnit::Meters:
      return QStringLiteral( "m" );
    case Qgis::LayoutUnit::Inches:
      return QStringLiteral( "in" );
    case Qgis::LayoutUnit::Feet:
      return QStringLiteral( "ft" );
    case Qgis::LayoutUnit::Points:
      return QStringLiteral( "pt" );
    case Qgis::LayoutUnit::Picas:
      return QStringLiteral( "pi" );
    case Qgis::LayoutUnit::Pixels:
      return QStringLiteral( "px" );
    case  Qgis::LayoutUnit::Millimeters:
      return QStringLiteral( "mm" );
  }
  return QString();
}

Qgis::LayoutUnit QgsUnitTypes::decodeLayoutUnit( const QString &string, bool *ok )
{
  const QString normalized = string.trimmed().toLower();

  if ( ok )
    *ok = true;

  if ( normalized == encodeUnit( Qgis::LayoutUnit::Millimeters ).toLower() )
    return Qgis::LayoutUnit::Millimeters;
  if ( normalized == encodeUnit( Qgis::LayoutUnit::Centimeters ).toLower() )
    return Qgis::LayoutUnit::Centimeters;
  if ( normalized == encodeUnit( Qgis::LayoutUnit::Meters ).toLower() )
    return Qgis::LayoutUnit::Meters;
  if ( normalized == encodeUnit( Qgis::LayoutUnit::Inches ).toLower() )
    return Qgis::LayoutUnit::Inches;
  if ( normalized == encodeUnit( Qgis::LayoutUnit::Feet ).toLower() )
    return Qgis::LayoutUnit::Feet;
  if ( normalized == encodeUnit( Qgis::LayoutUnit::Points ).toLower() )
    return Qgis::LayoutUnit::Points;
  if ( normalized == encodeUnit( Qgis::LayoutUnit::Picas ).toLower() )
    return Qgis::LayoutUnit::Picas;
  if ( normalized == encodeUnit( Qgis::LayoutUnit::Pixels ).toLower() )
    return Qgis::LayoutUnit::Pixels;

  if ( ok )
    *ok = false;

  // millimeters are default
  return Qgis::LayoutUnit::Millimeters;
}

Qgis::LayoutUnitType QgsUnitTypes::unitType( const Qgis::LayoutUnit units )
{
  switch ( units )
  {
    case Qgis::LayoutUnit::Pixels:
      return Qgis::LayoutUnitType::ScreenUnits;
    case  Qgis::LayoutUnit::Millimeters:
    case Qgis::LayoutUnit::Centimeters:
    case Qgis::LayoutUnit::Meters:
    case Qgis::LayoutUnit::Inches:
    case Qgis::LayoutUnit::Feet:
    case Qgis::LayoutUnit::Points:
    case Qgis::LayoutUnit::Picas:
      return Qgis::LayoutUnitType::PaperUnits;
  }

  // avoid warnings
  return Qgis::LayoutUnitType::PaperUnits;
}

QString QgsUnitTypes::toAbbreviatedString( Qgis::LayoutUnit unit )
{
  switch ( unit )
  {
    case Qgis::LayoutUnit::Pixels:
      return QObject::tr( "px" );
    case  Qgis::LayoutUnit::Millimeters:
      return QObject::tr( "mm" );
    case Qgis::LayoutUnit::Centimeters:
      return QObject::tr( "cm" );
    case Qgis::LayoutUnit::Meters:
      return QObject::tr( "m" );
    case Qgis::LayoutUnit::Inches:
      return QObject::tr( "in", "unit inch" );
    case Qgis::LayoutUnit::Feet:
      return QObject::tr( "ft" );
    case Qgis::LayoutUnit::Points:
      return QObject::tr( "pt" );
    case Qgis::LayoutUnit::Picas:
      return QObject::tr( "pica" );
  }
  return QString(); // no warnings
}

QString QgsUnitTypes::toString( Qgis::LayoutUnit unit )
{
  switch ( unit )
  {
    case Qgis::LayoutUnit::Pixels:
      return QObject::tr( "pixels" );
    case  Qgis::LayoutUnit::Millimeters:
      return QObject::tr( "millimeters" );
    case Qgis::LayoutUnit::Centimeters:
      return QObject::tr( "centimeters" );
    case Qgis::LayoutUnit::Meters:
      return QObject::tr( "meters" );
    case Qgis::LayoutUnit::Inches:
      return QObject::tr( "inches" );
    case Qgis::LayoutUnit::Feet:
      return QObject::tr( "feet" );
    case Qgis::LayoutUnit::Points:
      return QObject::tr( "points" );
    case Qgis::LayoutUnit::Picas:
      return QObject::tr( "picas" );
  }
  return QString(); // no warnings
}
