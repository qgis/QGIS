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
#include "qgsfield.h"
#include "qgsfields.h"

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

QgsFields QgsSensorThingsUtils::fieldsForEntityType( Qgis::SensorThingsEntity type )
{
  QgsFields fields;

  // common fields: https://docs.ogc.org/is/18-088/18-088.html#common-control-information
  fields.append( QgsField( QStringLiteral( "id" ), QVariant::String ) );
  fields.append( QgsField( QStringLiteral( "selfLink" ), QVariant::String ) );
  fields.append( QgsField( QStringLiteral( "navigationLink" ), QVariant::String ) );

  switch ( type )
  {
    case Qgis::SensorThingsEntity::Invalid:
      break;

    case Qgis::SensorThingsEntity::Thing:
      // https://docs.ogc.org/is/18-088/18-088.html#thing
      fields.append( QgsField( QStringLiteral( "name" ), QVariant::String ) );
      fields.append( QgsField( QStringLiteral( "description" ), QVariant::String ) );
      fields.append( QgsField( QStringLiteral( "properties" ), QVariant::Map, QStringLiteral( "json" ), 0, 0, QString(), QVariant::String ) );
      break;

    case Qgis::SensorThingsEntity::Location:
      // https://docs.ogc.org/is/18-088/18-088.html#location
      fields.append( QgsField( QStringLiteral( "name" ), QVariant::String ) );
      fields.append( QgsField( QStringLiteral( "description" ), QVariant::String ) );
      fields.append( QgsField( QStringLiteral( "properties" ), QVariant::Map, QStringLiteral( "json" ), 0, 0, QString(), QVariant::String ) );
      break;

    case Qgis::SensorThingsEntity::HistoricalLocation:
      // https://docs.ogc.org/is/18-088/18-088.html#historicallocation
      fields.append( QgsField( QStringLiteral( "time" ), QVariant::DateTime ) );
      break;

    case Qgis::SensorThingsEntity::Datastream:
      // https://docs.ogc.org/is/18-088/18-088.html#datastream
      fields.append( QgsField( QStringLiteral( "name" ), QVariant::String ) );
      fields.append( QgsField( QStringLiteral( "description" ), QVariant::String ) );
      fields.append( QgsField( QStringLiteral( "unitOfMeasurement" ), QVariant::Map, QStringLiteral( "json" ), 0, 0, QString(), QVariant::String ) );
      fields.append( QgsField( QStringLiteral( "observationType" ), QVariant::String ) );
      fields.append( QgsField( QStringLiteral( "properties" ), QVariant::Map, QStringLiteral( "json" ), 0, 0, QString(), QVariant::String ) );
      fields.append( QgsField( QStringLiteral( "phenomenonTimeStart" ), QVariant::DateTime ) );
      fields.append( QgsField( QStringLiteral( "phenomenonTimeEnd" ), QVariant::DateTime ) );
      fields.append( QgsField( QStringLiteral( "resultTimeStart" ), QVariant::DateTime ) );
      fields.append( QgsField( QStringLiteral( "resultTimeEnd" ), QVariant::DateTime ) );
      break;

    case Qgis::SensorThingsEntity::Sensor:
      // https://docs.ogc.org/is/18-088/18-088.html#sensor
      fields.append( QgsField( QStringLiteral( "name" ), QVariant::String ) );
      fields.append( QgsField( QStringLiteral( "description" ), QVariant::String ) );
      fields.append( QgsField( QStringLiteral( "metadata" ), QVariant::String ) );
      fields.append( QgsField( QStringLiteral( "properties" ), QVariant::Map, QStringLiteral( "json" ), 0, 0, QString(), QVariant::String ) );
      break;

    case Qgis::SensorThingsEntity::ObservedProperty:
      // https://docs.ogc.org/is/18-088/18-088.html#observedproperty
      fields.append( QgsField( QStringLiteral( "name" ), QVariant::String ) );
      fields.append( QgsField( QStringLiteral( "definition" ), QVariant::String ) );
      fields.append( QgsField( QStringLiteral( "description" ), QVariant::String ) );
      fields.append( QgsField( QStringLiteral( "properties" ), QVariant::Map, QStringLiteral( "json" ), 0, 0, QString(), QVariant::String ) );
      break;

    case Qgis::SensorThingsEntity::Observation:
      // https://docs.ogc.org/is/18-088/18-088.html#observation
      fields.append( QgsField( QStringLiteral( "phenomenonTimeStart" ), QVariant::DateTime ) );
      fields.append( QgsField( QStringLiteral( "phenomenonTimeEnd" ), QVariant::DateTime ) );

      // TODO -- handle type correctly
      fields.append( QgsField( QStringLiteral( "result" ), QVariant::String ) );

      fields.append( QgsField( QStringLiteral( "resultTime" ), QVariant::DateTime ) );
      fields.append( QgsField( QStringLiteral( "resultQuality" ), QVariant::StringList, QString(), 0, 0, QString(), QVariant::String ) );
      fields.append( QgsField( QStringLiteral( "validTimeStart" ), QVariant::DateTime ) );
      fields.append( QgsField( QStringLiteral( "validTimeEnd" ), QVariant::DateTime ) );
      fields.append( QgsField( QStringLiteral( "parameters" ), QVariant::Map, QStringLiteral( "json" ), 0, 0, QString(), QVariant::String ) );
      break;

    case Qgis::SensorThingsEntity::FeatureOfInterest:
      // https://docs.ogc.org/is/18-088/18-088.html#featureofinterest
      fields.append( QgsField( QStringLiteral( "name" ), QVariant::String ) );
      fields.append( QgsField( QStringLiteral( "description" ), QVariant::String ) );
      fields.append( QgsField( QStringLiteral( "properties" ), QVariant::Map, QStringLiteral( "json" ), 0, 0, QString(), QVariant::String ) );
      break;
  }

  return fields;
}

bool QgsSensorThingsUtils::entityTypeHasGeometry( Qgis::SensorThingsEntity type )
{
  switch ( type )
  {
    case Qgis::SensorThingsEntity::Invalid:
    case Qgis::SensorThingsEntity::Thing:
    case Qgis::SensorThingsEntity::HistoricalLocation:
    case Qgis::SensorThingsEntity::Datastream:
    case Qgis::SensorThingsEntity::Sensor:
    case Qgis::SensorThingsEntity::Observation:
    case Qgis::SensorThingsEntity::ObservedProperty:
      return false;

    case Qgis::SensorThingsEntity::Location:
    case Qgis::SensorThingsEntity::FeatureOfInterest:
      return true;
  }
  BUILTIN_UNREACHABLE
}
