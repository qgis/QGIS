/***************************************************************************
                             qgsmeasureutils.cpp
                             ----------------------
    begin                : February 2026
    copyright            : (C) 2026 by Nyall Dawson
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

#include "qgsmeasureutils.h"

#include "qgis.h"
#include "qgsproject.h"
#include "qgssettings.h"
#include "qgsunittypes.h"

#include <QString>

#include "moc_qgsmeasureutils.cpp"

using namespace Qt::StringLiterals;

QString QgsMeasureUtils::formatAreaForProject( QgsProject *project, double area, Qgis::AreaUnit unit )
{
  QgsSettings settings;
  const bool keepBaseUnit = settings.value( u"qgis/measure/keepbaseunit"_s, true ).toBool();

  const Qgis::AreaUnit targetUnit = project->areaUnits();
  const double areaInTargetUnits = QgsUnitTypes::fromUnitToUnitFactor( unit, targetUnit ) * area;

  int decimals = 3;
  switch ( targetUnit )
  {
    case Qgis::AreaUnit::SquareDegrees:
      decimals = 6;
      break;

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
    case Qgis::AreaUnit::Unknown:
      break;
  }

  return QgsUnitTypes::formatArea( areaInTargetUnits, decimals, targetUnit, keepBaseUnit );
}

QString QgsMeasureUtils::formatDistanceForProject( QgsProject *project, double distance, Qgis::DistanceUnit unit )
{
  QgsSettings settings;
  const bool keepBaseUnit = settings.value( u"qgis/measure/keepbaseunit"_s, true ).toBool();

  const Qgis::DistanceUnit targetUnit = project->distanceUnits();
  const double distanceInTargetUnits = QgsUnitTypes::fromUnitToUnitFactor( unit, targetUnit ) * distance;

  int decimals = 3;
  switch ( targetUnit )
  {
    case Qgis::DistanceUnit::Degrees:
      decimals = 6;
      break;

    case Qgis::DistanceUnit::Meters:
    case Qgis::DistanceUnit::Kilometers:
    case Qgis::DistanceUnit::Feet:
    case Qgis::DistanceUnit::NauticalMiles:
    case Qgis::DistanceUnit::Yards:
    case Qgis::DistanceUnit::Miles:
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
    case Qgis::DistanceUnit::Unknown:
      break;
  }

  return QgsUnitTypes::formatDistance( distanceInTargetUnits, decimals, targetUnit, keepBaseUnit );
}
