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
#include "qgswkbtypes.h"
#include "qgssetrequestinitiator_p.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsblockingnetworkrequest.h"
#include "qgslogger.h"
#include "qgsrectangle.h"
#include <QUrl>
#include <QNetworkRequest>
#include <nlohmann/json.hpp>

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
  if ( trimmed.compare( QLatin1String( "MultiDatastream" ), Qt::CaseInsensitive ) == 0 )
    return Qgis::SensorThingsEntity::MultiDatastream;

  return Qgis::SensorThingsEntity::Invalid;
}

QString QgsSensorThingsUtils::displayString( Qgis::SensorThingsEntity type, bool plural )
{
  switch ( type )
  {
    case Qgis::SensorThingsEntity::Invalid:
      return QString();
    case Qgis::SensorThingsEntity::Thing:
      return plural ? QObject::tr( "Things" ) : QObject::tr( "Thing" );
    case Qgis::SensorThingsEntity::Location:
      return plural ? QObject::tr( "Locations" ) : QObject::tr( "Location" );
    case Qgis::SensorThingsEntity::HistoricalLocation:
      return plural ? QObject::tr( "Historical Locations" ) : QObject::tr( "Historical Location" );
    case Qgis::SensorThingsEntity::Datastream:
      return plural ? QObject::tr( "Datastreams" ) : QObject::tr( "Datastream" );
    case Qgis::SensorThingsEntity::Sensor:
      return plural ? QObject::tr( "Sensors" ) : QObject::tr( "Sensor" );
    case Qgis::SensorThingsEntity::ObservedProperty:
      return plural ? QObject::tr( "Observed Properties" ) : QObject::tr( "Observed Property" );
    case Qgis::SensorThingsEntity::Observation:
      return plural ? QObject::tr( "Observations" ) : QObject::tr( "Observation" );
    case Qgis::SensorThingsEntity::FeatureOfInterest:
      return plural ? QObject::tr( "Features of Interest" ) : QObject::tr( "Feature of Interest" );
    case Qgis::SensorThingsEntity::MultiDatastream:
      return plural ? QObject::tr( "MultiDatastreams" ) : QObject::tr( "MultiDatastream" );
  }
  BUILTIN_UNREACHABLE
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
  if ( trimmed.compare( QLatin1String( "MultiDatastreams" ), Qt::CaseInsensitive ) == 0 )
    return Qgis::SensorThingsEntity::MultiDatastream;

  return Qgis::SensorThingsEntity::Invalid;
}

QgsFields QgsSensorThingsUtils::fieldsForEntityType( Qgis::SensorThingsEntity type )
{
  QgsFields fields;

  // common fields: https://docs.ogc.org/is/18-088/18-088.html#common-control-information
  fields.append( QgsField( QStringLiteral( "id" ), QVariant::String ) );
  fields.append( QgsField( QStringLiteral( "selfLink" ), QVariant::String ) );

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

    case Qgis::SensorThingsEntity::MultiDatastream:
      // https://docs.ogc.org/is/18-088/18-088.html#multidatastream-extension
      fields.append( QgsField( QStringLiteral( "name" ), QVariant::String ) );
      fields.append( QgsField( QStringLiteral( "description" ), QVariant::String ) );
      fields.append( QgsField( QStringLiteral( "unitOfMeasurements" ), QVariant::Map, QStringLiteral( "json" ), 0, 0, QString(), QVariant::String ) );
      fields.append( QgsField( QStringLiteral( "observationType" ), QVariant::String ) );
      fields.append( QgsField( QStringLiteral( "multiObservationDataTypes" ), QVariant::StringList, QString(), 0, 0, QString(), QVariant::String ) );
      fields.append( QgsField( QStringLiteral( "properties" ), QVariant::Map, QStringLiteral( "json" ), 0, 0, QString(), QVariant::String ) );
      fields.append( QgsField( QStringLiteral( "phenomenonTimeStart" ), QVariant::DateTime ) );
      fields.append( QgsField( QStringLiteral( "phenomenonTimeEnd" ), QVariant::DateTime ) );
      fields.append( QgsField( QStringLiteral( "resultTimeStart" ), QVariant::DateTime ) );
      fields.append( QgsField( QStringLiteral( "resultTimeEnd" ), QVariant::DateTime ) );
      break;
  }

  return fields;
}

QString QgsSensorThingsUtils::geometryFieldForEntityType( Qgis::SensorThingsEntity type )
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
      return QString();

    case Qgis::SensorThingsEntity::Location:
      return QStringLiteral( "location" );

    case Qgis::SensorThingsEntity::FeatureOfInterest:
      return QStringLiteral( "feature" );

    case Qgis::SensorThingsEntity::MultiDatastream:
      return QStringLiteral( "observedArea" );
  }
  BUILTIN_UNREACHABLE
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
    case Qgis::SensorThingsEntity::MultiDatastream:
      return true;
  }
  BUILTIN_UNREACHABLE
}

Qgis::GeometryType QgsSensorThingsUtils::geometryTypeForEntity( Qgis::SensorThingsEntity type )
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
      return Qgis::GeometryType::Null;

    case Qgis::SensorThingsEntity::Location:
    case Qgis::SensorThingsEntity::FeatureOfInterest:
      return Qgis::GeometryType::Unknown;

    case Qgis::SensorThingsEntity::MultiDatastream:
      return Qgis::GeometryType::Polygon;
  }
  BUILTIN_UNREACHABLE
}

QString QgsSensorThingsUtils::filterForWkbType( Qgis::SensorThingsEntity entityType, Qgis::WkbType wkbType )
{
  QString geometryTypeString;
  switch ( QgsWkbTypes::geometryType( wkbType ) )
  {
    case Qgis::GeometryType::Point:
      geometryTypeString = QStringLiteral( "Point" );
      break;
    case Qgis::GeometryType::Polygon:
      geometryTypeString = QStringLiteral( "Polygon" );
      break;
    case Qgis::GeometryType::Line:
      geometryTypeString = QStringLiteral( "LineString" );
      break;

    case Qgis::GeometryType::Unknown:
    case Qgis::GeometryType::Null:
      return QString();
  }

  const QString filterTarget = geometryFieldForEntityType( entityType );
  if ( filterTarget.isEmpty() )
    return QString();

  return QStringLiteral( "%1/type eq '%2' or %1/geometry/type eq '%2'" ).arg( filterTarget, geometryTypeString );
}

QString QgsSensorThingsUtils::filterForExtent( const QString &geometryField, const QgsRectangle &extent )
{
  // TODO -- confirm using 'geography' is always correct here
  return ( extent.isNull() || geometryField.isEmpty() )
         ? QString()
         : QStringLiteral( "geo.intersects(%1, geography'%2')" ).arg( geometryField, extent.asWktPolygon() );
}

QString QgsSensorThingsUtils::combineFilters( const QStringList &filters )
{
  QStringList nonEmptyFilters;
  for ( const QString &filter : filters )
  {
    if ( !filter.isEmpty() )
      nonEmptyFilters.append( filter );
  }
  if ( nonEmptyFilters.empty() )
    return QString();
  if ( nonEmptyFilters.size() == 1 )
    return nonEmptyFilters.at( 0 );

  return QStringLiteral( "(" ) + nonEmptyFilters.join( QLatin1String( ") and (" ) ) + QStringLiteral( ")" );
}

QList<Qgis::GeometryType> QgsSensorThingsUtils::availableGeometryTypes( const QString &uri, Qgis::SensorThingsEntity type, QgsFeedback *feedback, const QString &authCfg )
{
  QNetworkRequest request = QNetworkRequest( QUrl( uri ) );
  QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsSensorThingsUtils" ) )

  QgsBlockingNetworkRequest networkRequest;
  networkRequest.setAuthCfg( authCfg );

  switch ( networkRequest.get( request ) )
  {
    case QgsBlockingNetworkRequest::NoError:
      break;

    case QgsBlockingNetworkRequest::NetworkError:
    case QgsBlockingNetworkRequest::TimeoutError:
    case QgsBlockingNetworkRequest::ServerExceptionError:
      QgsDebugError( QStringLiteral( "Connection failed: %1" ).arg( networkRequest.errorMessage() ) );
      return {};
  }

  QString entityBaseUri;
  const QgsNetworkReplyContent content = networkRequest.reply();
  try
  {
    auto rootContent = nlohmann::json::parse( content.content().toStdString() );
    if ( !rootContent.contains( "value" ) )
    {
      QgsDebugError( QStringLiteral( "No 'value' array in response" ) );
      return {};
    }

    bool foundMatchingEntity = false;
    for ( const auto &valueJson : rootContent["value"] )
    {
      if ( valueJson.contains( "name" ) && valueJson.contains( "url" ) )
      {
        const QString name = QString::fromStdString( valueJson["name"].get<std::string>() );
        Qgis::SensorThingsEntity entityType = QgsSensorThingsUtils::entitySetStringToEntity( name );
        if ( entityType == type )
        {
          const QString url = QString::fromStdString( valueJson["url"].get<std::string>() );
          if ( !url.isEmpty() )
          {
            foundMatchingEntity = true;
            entityBaseUri = url;
            break;
          }
        }
      }
    }

    if ( !foundMatchingEntity )
    {
      QgsDebugError( QStringLiteral( "Could not find url for %1" ).arg( qgsEnumValueToKey( type ) ) );
      return {};
    }
  }
  catch ( const nlohmann::json::parse_error &ex )
  {
    QgsDebugError( QStringLiteral( "Error parsing response: %1" ).arg( ex.what() ) );
    return {};
  }

  auto getCountForType = [entityBaseUri, type, authCfg, feedback]( Qgis::GeometryType geometryType ) -> long long
  {
    // return no features, just the total count
    QString countUri = QStringLiteral( "%1?$top=0&$count=true" ).arg( entityBaseUri );
    Qgis::WkbType wkbType = geometryType == Qgis::GeometryType::Polygon ? Qgis::WkbType::Polygon : ( geometryType == Qgis::GeometryType::Line ? Qgis::WkbType::LineString : Qgis::WkbType::Point );
    const QString typeFilter = QgsSensorThingsUtils::filterForWkbType( type, wkbType );
    if ( !typeFilter.isEmpty() )
      countUri += QStringLiteral( "&$filter=" ) + typeFilter;

    const QUrl url( countUri );

    QNetworkRequest request( url );
    QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsSensorThingsSharedData" ) );

    QgsBlockingNetworkRequest networkRequest;
    networkRequest.setAuthCfg( authCfg );
    const QgsBlockingNetworkRequest::ErrorCode error = networkRequest.get( request, false, feedback );

    if ( feedback && feedback->isCanceled() )
      return -1;

    // Handle network errors
    if ( error != QgsBlockingNetworkRequest::NoError )
    {
      QgsDebugError( QStringLiteral( "Network error: %1" ).arg( networkRequest.errorMessage() ) );
      return -1;
    }
    else
    {
      const QgsNetworkReplyContent content = networkRequest.reply();
      try
      {
        auto rootContent = nlohmann::json::parse( content.content().toStdString() );
        if ( !rootContent.contains( "@iot.count" ) )
        {
          QgsDebugError( QStringLiteral( "No '@iot.count' value in response" ) );
          return -1;
        }

        return rootContent["@iot.count"].get<long long>();
      }
      catch ( const nlohmann::json::parse_error &ex )
      {
        QgsDebugError( QStringLiteral( "Error parsing response: %1" ).arg( ex.what() ) );
        return -1;
      }
    }
  };

  QList<Qgis::GeometryType> types;
  for ( Qgis::GeometryType geometryType :
        {
          Qgis::GeometryType::Point,
          Qgis::GeometryType::Line,
          Qgis::GeometryType::Polygon
        } )
  {
    const long long matchCount = getCountForType( geometryType );
    if ( matchCount < 0 )
      return {};
    else if ( matchCount > 0 )
      types.append( geometryType );
  }
  return types;
}
