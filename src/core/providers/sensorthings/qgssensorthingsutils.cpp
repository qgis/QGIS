/***************************************************************************
    qgssensorthingsutils.cpp
    --------------------
    begin                : November 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssensorthingsutils.h"

Qgis::SensorThingsEntity QgsSensorThingsUtils::stringToEntity( const QString &type )
{
  const QString trimmed = type.trimmed();
  if ( trimmed.compare( QLatin1String( "Thing" ), Qt::CaseInsensitive ) == 0 )
    return Qgis::SensorThingsEntity::Thing;
  if ( trimmed.compare( QLatin1String( "Location" ), Qt::CaseInsensitive ) == 0 )
    return Qgis::SensorThingsEntity::Location;
  if ( trimmed.compare( QLatin1String( "HistoricalLocation" ), Qt::CaseInsensitive ) == 0 )
    return Qgis::SensorThingsEntity::HistoricalLocation;
  if ( trimmed.compare( QLatin1String( "Datastream" ), Qt::CaseInsensitive ) == 0 )
    return Qgis::SensorThingsEntity::Datastream;
  if ( trimmed.compare( QLatin1String( "Sensor" ), Qt::CaseInsensitive ) == 0 )
    return Qgis::SensorThingsEntity::Sensor;
  if ( trimmed.compare( QLatin1String( "ObservedProperty" ), Qt::CaseInsensitive ) == 0 )
    return Qgis::SensorThingsEntity::ObservedProperty;
  if ( trimmed.compare( QLatin1String( "Observation" ), Qt::CaseInsensitive ) == 0 )
    return Qgis::SensorThingsEntity::Observation;
  if ( trimmed.compare( QLatin1String( "FeatureOfInterest" ), Qt::CaseInsensitive ) == 0 )
    return Qgis::SensorThingsEntity::FeatureOfInterest;

  return Qgis::SensorThingsEntity::Invalid;
}

Qgis::SensorThingsEntity QgsSensorThingsUtils::entitySetStringToEntity( const QString &type )
{
  const QString trimmed = type.trimmed();
  if ( trimmed.compare( QLatin1String( "Things" ), Qt::CaseInsensitive ) == 0 )
    return Qgis::SensorThingsEntity::Thing;
  if ( trimmed.compare( QLatin1String( "Locations" ), Qt::CaseInsensitive ) == 0 )
    return Qgis::SensorThingsEntity::Location;
  if ( trimmed.compare( QLatin1String( "HistoricalLocations" ), Qt::CaseInsensitive ) == 0 )
    return Qgis::SensorThingsEntity::HistoricalLocation;
  if ( trimmed.compare( QLatin1String( "Datastreams" ), Qt::CaseInsensitive ) == 0 )
    return Qgis::SensorThingsEntity::Datastream;
  if ( trimmed.compare( QLatin1String( "Sensors" ), Qt::CaseInsensitive ) == 0 )
    return Qgis::SensorThingsEntity::Sensor;
  if ( trimmed.compare( QLatin1String( "ObservedProperties" ), Qt::CaseInsensitive ) == 0 )
    return Qgis::SensorThingsEntity::ObservedProperty;
  if ( trimmed.compare( QLatin1String( "Observations" ), Qt::CaseInsensitive ) == 0 )
    return Qgis::SensorThingsEntity::Observation;
  if ( trimmed.compare( QLatin1String( "FeaturesOfInterest" ), Qt::CaseInsensitive ) == 0 )
    return Qgis::SensorThingsEntity::FeatureOfInterest;

  return Qgis::SensorThingsEntity::Invalid;
}
