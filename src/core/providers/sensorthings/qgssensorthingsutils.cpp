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
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <nlohmann/json.hpp>


//
// QgsSensorThingsExpansionDefinition
//
QgsSensorThingsExpansionDefinition::QgsSensorThingsExpansionDefinition( Qgis::SensorThingsEntity childEntity, const QString &orderBy, Qt::SortOrder sortOrder, int limit, const QString &filter )
  : mChildEntity( childEntity )
  , mOrderBy( orderBy )
  , mSortOrder( sortOrder )
  , mLimit( limit )
  , mFilter( filter )
{

}

QgsSensorThingsExpansionDefinition QgsSensorThingsExpansionDefinition::defaultDefinitionForEntity( Qgis::SensorThingsEntity entity )
{
  switch ( entity )
  {
    case Qgis::SensorThingsEntity::Invalid:
      return QgsSensorThingsExpansionDefinition();

    case Qgis::SensorThingsEntity::Thing:
    case Qgis::SensorThingsEntity::Location:
    case Qgis::SensorThingsEntity::HistoricalLocation:
    case Qgis::SensorThingsEntity::Sensor:
    case Qgis::SensorThingsEntity::FeatureOfInterest:
      // no special defaults for these entities
      return QgsSensorThingsExpansionDefinition(
               entity
             );

    case Qgis::SensorThingsEntity::Observation:
      // default to descending sort by phenomenonTime
      return QgsSensorThingsExpansionDefinition(
               Qgis::SensorThingsEntity::Observation,
               QStringLiteral( "phenomenonTime" ), Qt::SortOrder::DescendingOrder
             );

    case Qgis::SensorThingsEntity::Datastream:
    case Qgis::SensorThingsEntity::MultiDatastream:
    case Qgis::SensorThingsEntity::ObservedProperty:
      // use smaller limit by default
      return QgsSensorThingsExpansionDefinition(
               entity,
               QString(), Qt::SortOrder::AscendingOrder, 10
             );
  }
  BUILTIN_UNREACHABLE
}

bool QgsSensorThingsExpansionDefinition::isValid() const
{
  return mChildEntity != Qgis::SensorThingsEntity::Invalid;
}

Qgis::SensorThingsEntity QgsSensorThingsExpansionDefinition::childEntity() const
{
  return mChildEntity;
}

void QgsSensorThingsExpansionDefinition::setChildEntity( Qgis::SensorThingsEntity entity )
{
  mChildEntity = entity;
}

Qt::SortOrder QgsSensorThingsExpansionDefinition::sortOrder() const
{
  return mSortOrder;
}

void QgsSensorThingsExpansionDefinition::setSortOrder( Qt::SortOrder order )
{
  mSortOrder = order;
}

int QgsSensorThingsExpansionDefinition::limit() const
{
  return mLimit;
}

void QgsSensorThingsExpansionDefinition::setLimit( int limit )
{
  mLimit = limit;
}

QString QgsSensorThingsExpansionDefinition::filter() const
{
  return mFilter;
}

void QgsSensorThingsExpansionDefinition::setFilter( const QString &filter )
{
  mFilter = filter;
}

QString QgsSensorThingsExpansionDefinition::toString() const
{
  if ( !isValid() )
    return QString();

  QStringList parts;
  parts.append( qgsEnumValueToKey( mChildEntity ) );
  if ( !mOrderBy.isEmpty() )
    parts.append( QStringLiteral( "orderby=%1,%2" ).arg( mOrderBy, mSortOrder == Qt::SortOrder::AscendingOrder ? QStringLiteral( "asc" ) : QStringLiteral( "desc" ) ) );
  if ( mLimit >= 0 )
    parts.append( QStringLiteral( "limit=%1" ).arg( mLimit ) );
  if ( !mFilter.trimmed().isEmpty() )
  {
    QString escapedFilter = mFilter;
    escapedFilter.replace( ':', QLatin1String( "\\colon" ) );
    parts.append( QStringLiteral( "filter=%1" ).arg( escapedFilter ) );
  }
  return parts.join( ':' );
}

QgsSensorThingsExpansionDefinition QgsSensorThingsExpansionDefinition::fromString( const QString &string )
{
  const QStringList parts = string.split( ':', Qt::SkipEmptyParts );
  if ( parts.empty() )
    return QgsSensorThingsExpansionDefinition();

  QgsSensorThingsExpansionDefinition definition( qgsEnumKeyToValue( parts.at( 0 ), Qgis::SensorThingsEntity::Invalid ) );
  definition.setLimit( -1 );
  for ( int i = 1; i < parts.count(); ++i )
  {
    const QString &part = parts.at( i );
    const thread_local QRegularExpression orderByRegEx( QStringLiteral( "^orderby=(.*),(.*?)$" ) );
    const thread_local QRegularExpression orderLimitRegEx( QStringLiteral( "^limit=(\\d+)$" ) );
    const thread_local QRegularExpression filterRegEx( QStringLiteral( "^filter=(.*)$" ) );

    const QRegularExpressionMatch orderByMatch = orderByRegEx.match( part );
    if ( orderByMatch.hasMatch() )
    {
      definition.setOrderBy( orderByMatch.captured( 1 ) );
      definition.setSortOrder( orderByMatch.captured( 2 ) == QLatin1String( "asc" ) ? Qt::SortOrder::AscendingOrder : Qt::SortOrder::DescendingOrder );
      continue;
    }

    const QRegularExpressionMatch limitMatch = orderLimitRegEx.match( part );
    if ( limitMatch.hasMatch() )
    {
      definition.setLimit( limitMatch.captured( 1 ).toInt() );
      continue;
    }

    const QRegularExpressionMatch filterMatch = filterRegEx.match( part );
    if ( filterMatch.hasMatch() )
    {
      QString filter = filterMatch.captured( 1 );
      filter.replace( QLatin1String( "\\colon" ), QLatin1String( ":" ) );
      definition.setFilter( filter );
      continue;
    }
  }
  return definition;
}

QString QgsSensorThingsExpansionDefinition::asQueryString( Qgis::SensorThingsEntity parentEntityType, const QStringList &additionalOptions ) const
{
  if ( !isValid() )
    return QString();

  bool ok = false;
  // From the specifications, it SOMETIMES the plural form is used for the expansion query, and sometimes singular.
  // The choice depends on the cardinality of the relationship between the involved entities.
  const Qgis::RelationshipCardinality cardinality = QgsSensorThingsUtils::relationshipCardinality( parentEntityType, mChildEntity, ok );
  QString childEntityString;
  if ( !ok )
  {
    childEntityString = QgsSensorThingsUtils::entityToSetString( mChildEntity );
  }
  else
  {
    switch ( cardinality )
    {
      case Qgis::RelationshipCardinality::OneToOne:
      case Qgis::RelationshipCardinality::ManyToOne:
        // use singular strings, eg "Thing"
        childEntityString = qgsEnumValueToKey( mChildEntity );
        break;

      case Qgis::RelationshipCardinality::OneToMany:
      case Qgis::RelationshipCardinality::ManyToMany:
        // use plural strings, eg "Things"
        childEntityString = QgsSensorThingsUtils::entityToSetString( mChildEntity );
        break;
    }
  }

  QString res = QStringLiteral( "$expand=%1" ).arg( childEntityString );

  QStringList queryOptions;
  if ( !mOrderBy.isEmpty() )
    queryOptions.append( QStringLiteral( "$orderby=%1%2" ).arg( mOrderBy, mSortOrder == Qt::SortOrder::AscendingOrder ? QString() : QStringLiteral( " desc" ) ) );

  if ( mLimit > -1 )
    queryOptions.append( QStringLiteral( "$top=%1" ).arg( mLimit ) );

  if ( !mFilter.isEmpty() )
    queryOptions.append( QStringLiteral( "$filter=%1" ).arg( mFilter ) );

  queryOptions.append( additionalOptions );

  if ( !queryOptions.isEmpty() )
    res.append( QStringLiteral( "(%1)" ).arg( queryOptions.join( ';' ) ) );

  return res;
}

bool QgsSensorThingsExpansionDefinition::operator==( const QgsSensorThingsExpansionDefinition &other ) const
{
  if ( mChildEntity == Qgis::SensorThingsEntity::Invalid )
    return other.mChildEntity == Qgis::SensorThingsEntity::Invalid;

  return mChildEntity == other.mChildEntity
         && mSortOrder == other.mSortOrder
         && mLimit == other.mLimit
         && mOrderBy == other.mOrderBy
         && mFilter == other.mFilter;
}

bool QgsSensorThingsExpansionDefinition::operator!=( const QgsSensorThingsExpansionDefinition &other ) const
{
  return !( *this == other );
}

QString QgsSensorThingsExpansionDefinition::orderBy() const
{
  return mOrderBy;
}

void QgsSensorThingsExpansionDefinition::setOrderBy( const QString &field )
{
  mOrderBy = field;
}

//
// QgsSensorThingsUtils
//

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

QString QgsSensorThingsUtils::entityToSetString( Qgis::SensorThingsEntity type )
{
  switch ( type )
  {
    case Qgis::SensorThingsEntity::Invalid:
      return QString();
    case Qgis::SensorThingsEntity::Thing:
      return QStringLiteral( "Things" );
    case Qgis::SensorThingsEntity::Location:
      return QStringLiteral( "Locations" );
    case Qgis::SensorThingsEntity::HistoricalLocation:
      return QStringLiteral( "HistoricalLocations" );
    case Qgis::SensorThingsEntity::Datastream:
      return QStringLiteral( "Datastreams" );
    case Qgis::SensorThingsEntity::Sensor:
      return QStringLiteral( "Sensors" );
    case Qgis::SensorThingsEntity::ObservedProperty:
      return QStringLiteral( "ObservedProperties" );
    case Qgis::SensorThingsEntity::Observation:
      return QStringLiteral( "Observations" );
    case Qgis::SensorThingsEntity::FeatureOfInterest:
      return QStringLiteral( "FeaturesOfInterest" );
    case Qgis::SensorThingsEntity::MultiDatastream:
      return QStringLiteral( "MultiDatastreams" );
  }
  BUILTIN_UNREACHABLE
}

QStringList QgsSensorThingsUtils::propertiesForEntityType( Qgis::SensorThingsEntity type )
{
  switch ( type )
  {
    case Qgis::SensorThingsEntity::Invalid:
      return {};

    case Qgis::SensorThingsEntity::Thing:
      // https://docs.ogc.org/is/18-088/18-088.html#thing
      return { QStringLiteral( "id" ),
               QStringLiteral( "selfLink" ),
               QStringLiteral( "name" ),
               QStringLiteral( "description" ),
               QStringLiteral( "properties" ),
           };

    case Qgis::SensorThingsEntity::Location:
      // https://docs.ogc.org/is/18-088/18-088.html#location
      return { QStringLiteral( "id" ),
               QStringLiteral( "selfLink" ),
               QStringLiteral( "name" ),
               QStringLiteral( "description" ),
               QStringLiteral( "properties" ),
           };

    case Qgis::SensorThingsEntity::HistoricalLocation:
      // https://docs.ogc.org/is/18-088/18-088.html#historicallocation
      return { QStringLiteral( "id" ),
               QStringLiteral( "selfLink" ),
               QStringLiteral( "time" ),
           };

    case Qgis::SensorThingsEntity::Datastream:
      // https://docs.ogc.org/is/18-088/18-088.html#datastream
      return { QStringLiteral( "id" ),
               QStringLiteral( "selfLink" ),
               QStringLiteral( "name" ),
               QStringLiteral( "description" ),
               QStringLiteral( "unitOfMeasurement" ),
               QStringLiteral( "observationType" ),
               QStringLiteral( "properties" ),
               QStringLiteral( "phenomenonTime" ),
               QStringLiteral( "resultTime" ),
           };

    case Qgis::SensorThingsEntity::Sensor:
      // https://docs.ogc.org/is/18-088/18-088.html#sensor
      return { QStringLiteral( "id" ),
               QStringLiteral( "selfLink" ),
               QStringLiteral( "name" ),
               QStringLiteral( "description" ),
               QStringLiteral( "metadata" ),
               QStringLiteral( "properties" ),
           };

    case Qgis::SensorThingsEntity::ObservedProperty:
      // https://docs.ogc.org/is/18-088/18-088.html#observedproperty
      return { QStringLiteral( "id" ),
               QStringLiteral( "selfLink" ),
               QStringLiteral( "name" ),
               QStringLiteral( "definition" ),
               QStringLiteral( "description" ),
               QStringLiteral( "properties" ),
           };

    case Qgis::SensorThingsEntity::Observation:
      // https://docs.ogc.org/is/18-088/18-088.html#observation
      return { QStringLiteral( "id" ),
               QStringLiteral( "selfLink" ),
               QStringLiteral( "phenomenonTime" ),
               QStringLiteral( "result" ),
               QStringLiteral( "resultTime" ),
               QStringLiteral( "resultQuality" ),
               QStringLiteral( "validTime" ),
               QStringLiteral( "parameters" ),
           };

    case Qgis::SensorThingsEntity::FeatureOfInterest:
      // https://docs.ogc.org/is/18-088/18-088.html#featureofinterest
      return { QStringLiteral( "id" ),
               QStringLiteral( "selfLink" ),
               QStringLiteral( "name" ),
               QStringLiteral( "description" ),
               QStringLiteral( "properties" ),
           };

    case Qgis::SensorThingsEntity::MultiDatastream:
      // https://docs.ogc.org/is/18-088/18-088.html#multidatastream-extension
      return { QStringLiteral( "id" ),
               QStringLiteral( "selfLink" ),
               QStringLiteral( "name" ),
               QStringLiteral( "description" ),
               QStringLiteral( "unitOfMeasurements" ),
               QStringLiteral( "observationType" ),
               QStringLiteral( "multiObservationDataTypes" ),
               QStringLiteral( "properties" ),
               QStringLiteral( "phenomenonTime" ),
               QStringLiteral( "resultTime" ),
           };
  }

  return {};
}

QgsFields QgsSensorThingsUtils::fieldsForEntityType( Qgis::SensorThingsEntity type )
{
  QgsFields fields;

  // common fields: https://docs.ogc.org/is/18-088/18-088.html#common-control-information
  fields.append( QgsField( QStringLiteral( "id" ), QMetaType::Type::QString ) );
  fields.append( QgsField( QStringLiteral( "selfLink" ), QMetaType::Type::QString ) );

  switch ( type )
  {
    case Qgis::SensorThingsEntity::Invalid:
      break;

    case Qgis::SensorThingsEntity::Thing:
      // https://docs.ogc.org/is/18-088/18-088.html#thing
      fields.append( QgsField( QStringLiteral( "name" ), QMetaType::Type::QString ) );
      fields.append( QgsField( QStringLiteral( "description" ), QMetaType::Type::QString ) );
      fields.append( QgsField( QStringLiteral( "properties" ), QMetaType::Type::QVariantMap, QStringLiteral( "json" ), 0, 0, QString(), QMetaType::Type::QString ) );
      break;

    case Qgis::SensorThingsEntity::Location:
      // https://docs.ogc.org/is/18-088/18-088.html#location
      fields.append( QgsField( QStringLiteral( "name" ), QMetaType::Type::QString ) );
      fields.append( QgsField( QStringLiteral( "description" ), QMetaType::Type::QString ) );
      fields.append( QgsField( QStringLiteral( "properties" ), QMetaType::Type::QVariantMap, QStringLiteral( "json" ), 0, 0, QString(), QMetaType::Type::QString ) );
      break;

    case Qgis::SensorThingsEntity::HistoricalLocation:
      // https://docs.ogc.org/is/18-088/18-088.html#historicallocation
      fields.append( QgsField( QStringLiteral( "time" ), QMetaType::Type::QDateTime ) );
      break;

    case Qgis::SensorThingsEntity::Datastream:
      // https://docs.ogc.org/is/18-088/18-088.html#datastream
      fields.append( QgsField( QStringLiteral( "name" ), QMetaType::Type::QString ) );
      fields.append( QgsField( QStringLiteral( "description" ), QMetaType::Type::QString ) );
      fields.append( QgsField( QStringLiteral( "unitOfMeasurement" ), QMetaType::Type::QVariantMap, QStringLiteral( "json" ), 0, 0, QString(), QMetaType::Type::QString ) );
      fields.append( QgsField( QStringLiteral( "observationType" ), QMetaType::Type::QString ) );
      fields.append( QgsField( QStringLiteral( "properties" ), QMetaType::Type::QVariantMap, QStringLiteral( "json" ), 0, 0, QString(), QMetaType::Type::QString ) );
      fields.append( QgsField( QStringLiteral( "phenomenonTimeStart" ), QMetaType::Type::QDateTime ) );
      fields.append( QgsField( QStringLiteral( "phenomenonTimeEnd" ), QMetaType::Type::QDateTime ) );
      fields.append( QgsField( QStringLiteral( "resultTimeStart" ), QMetaType::Type::QDateTime ) );
      fields.append( QgsField( QStringLiteral( "resultTimeEnd" ), QMetaType::Type::QDateTime ) );
      break;

    case Qgis::SensorThingsEntity::Sensor:
      // https://docs.ogc.org/is/18-088/18-088.html#sensor
      fields.append( QgsField( QStringLiteral( "name" ), QMetaType::Type::QString ) );
      fields.append( QgsField( QStringLiteral( "description" ), QMetaType::Type::QString ) );
      fields.append( QgsField( QStringLiteral( "metadata" ), QMetaType::Type::QString ) );
      fields.append( QgsField( QStringLiteral( "properties" ), QMetaType::Type::QVariantMap, QStringLiteral( "json" ), 0, 0, QString(), QMetaType::Type::QString ) );
      break;

    case Qgis::SensorThingsEntity::ObservedProperty:
      // https://docs.ogc.org/is/18-088/18-088.html#observedproperty
      fields.append( QgsField( QStringLiteral( "name" ), QMetaType::Type::QString ) );
      fields.append( QgsField( QStringLiteral( "definition" ), QMetaType::Type::QString ) );
      fields.append( QgsField( QStringLiteral( "description" ), QMetaType::Type::QString ) );
      fields.append( QgsField( QStringLiteral( "properties" ), QMetaType::Type::QVariantMap, QStringLiteral( "json" ), 0, 0, QString(), QMetaType::Type::QString ) );
      break;

    case Qgis::SensorThingsEntity::Observation:
      // https://docs.ogc.org/is/18-088/18-088.html#observation
      fields.append( QgsField( QStringLiteral( "phenomenonTimeStart" ), QMetaType::Type::QDateTime ) );
      fields.append( QgsField( QStringLiteral( "phenomenonTimeEnd" ), QMetaType::Type::QDateTime ) );

      // TODO -- handle type correctly
      fields.append( QgsField( QStringLiteral( "result" ), QMetaType::Type::QString ) );

      fields.append( QgsField( QStringLiteral( "resultTime" ), QMetaType::Type::QDateTime ) );
      fields.append( QgsField( QStringLiteral( "resultQuality" ), QMetaType::Type::QStringList, QString(), 0, 0, QString(), QMetaType::Type::QString ) );
      fields.append( QgsField( QStringLiteral( "validTimeStart" ), QMetaType::Type::QDateTime ) );
      fields.append( QgsField( QStringLiteral( "validTimeEnd" ), QMetaType::Type::QDateTime ) );
      fields.append( QgsField( QStringLiteral( "parameters" ), QMetaType::Type::QVariantMap, QStringLiteral( "json" ), 0, 0, QString(), QMetaType::Type::QString ) );
      break;

    case Qgis::SensorThingsEntity::FeatureOfInterest:
      // https://docs.ogc.org/is/18-088/18-088.html#featureofinterest
      fields.append( QgsField( QStringLiteral( "name" ), QMetaType::Type::QString ) );
      fields.append( QgsField( QStringLiteral( "description" ), QMetaType::Type::QString ) );
      fields.append( QgsField( QStringLiteral( "properties" ), QMetaType::Type::QVariantMap, QStringLiteral( "json" ), 0, 0, QString(), QMetaType::Type::QString ) );
      break;

    case Qgis::SensorThingsEntity::MultiDatastream:
      // https://docs.ogc.org/is/18-088/18-088.html#multidatastream-extension
      fields.append( QgsField( QStringLiteral( "name" ), QMetaType::Type::QString ) );
      fields.append( QgsField( QStringLiteral( "description" ), QMetaType::Type::QString ) );
      fields.append( QgsField( QStringLiteral( "unitOfMeasurements" ), QMetaType::Type::QVariantMap, QStringLiteral( "json" ), 0, 0, QString(), QMetaType::Type::QString ) );
      fields.append( QgsField( QStringLiteral( "observationType" ), QMetaType::Type::QString ) );
      fields.append( QgsField( QStringLiteral( "multiObservationDataTypes" ), QMetaType::Type::QStringList, QString(), 0, 0, QString(), QMetaType::Type::QString ) );
      fields.append( QgsField( QStringLiteral( "properties" ), QMetaType::Type::QVariantMap, QStringLiteral( "json" ), 0, 0, QString(), QMetaType::Type::QString ) );
      fields.append( QgsField( QStringLiteral( "phenomenonTimeStart" ), QMetaType::Type::QDateTime ) );
      fields.append( QgsField( QStringLiteral( "phenomenonTimeEnd" ), QMetaType::Type::QDateTime ) );
      fields.append( QgsField( QStringLiteral( "resultTimeStart" ), QMetaType::Type::QDateTime ) );
      fields.append( QgsField( QStringLiteral( "resultTimeEnd" ), QMetaType::Type::QDateTime ) );
      break;
  }

  return fields;
}

QgsFields QgsSensorThingsUtils::fieldsForExpandedEntityType( Qgis::SensorThingsEntity baseType, const QList<Qgis::SensorThingsEntity> &expandedTypes )
{
  if ( expandedTypes.empty() )
    return fieldsForEntityType( baseType );

  QgsFields fields = fieldsForEntityType( baseType );
  QString path;
  for ( const Qgis::SensorThingsEntity expandedType : expandedTypes )
  {
    path = ( path.isEmpty() ? QString() : ( path + '_' ) ) + qgsEnumValueToKey( expandedType );
    const QgsFields expandedFields = fieldsForEntityType( expandedType );
    for ( const QgsField &expandedField : expandedFields )
    {
      QgsField renamedExpandedField = expandedField;
      renamedExpandedField.setName( QStringLiteral( "%1_%2" ).arg( path, expandedField.name() ) );
      fields.append( renamedExpandedField );
    }
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

QList<Qgis::SensorThingsEntity> QgsSensorThingsUtils::expandableTargets( Qgis::SensorThingsEntity type )
{
  // note that we are restricting these choices so that the geometry enabled entity type MUST be the base type

  // NOLINTBEGIN(bugprone-branch-clone)
  switch ( type )
  {
    case Qgis::SensorThingsEntity::Invalid:
      return {};

    case Qgis::SensorThingsEntity::Thing:
      return
      {
        Qgis::SensorThingsEntity::HistoricalLocation,
        Qgis::SensorThingsEntity::Datastream
      };

    case Qgis::SensorThingsEntity::Location:
      return
      {
        Qgis::SensorThingsEntity::Thing,
        Qgis::SensorThingsEntity::HistoricalLocation,
      };

    case Qgis::SensorThingsEntity::HistoricalLocation:
      return
      {
        Qgis::SensorThingsEntity::Thing
      };

    case Qgis::SensorThingsEntity::Datastream:
      return
      {
        Qgis::SensorThingsEntity::Thing,
        Qgis::SensorThingsEntity::Sensor,
        Qgis::SensorThingsEntity::ObservedProperty,
        Qgis::SensorThingsEntity::Observation
      };

    case Qgis::SensorThingsEntity::Sensor:
      return
      {
        Qgis::SensorThingsEntity::Datastream
      };

    case Qgis::SensorThingsEntity::ObservedProperty:
      return
      {
        Qgis::SensorThingsEntity::Datastream
      };

    case Qgis::SensorThingsEntity::Observation:
      return
      {
        Qgis::SensorThingsEntity::Datastream
      };

    case Qgis::SensorThingsEntity::FeatureOfInterest:
      return
      {
        Qgis::SensorThingsEntity::Observation
      };

    case Qgis::SensorThingsEntity::MultiDatastream:
      return
      {
        Qgis::SensorThingsEntity::Thing,
        Qgis::SensorThingsEntity::Sensor,
        Qgis::SensorThingsEntity::ObservedProperty,
        Qgis::SensorThingsEntity::Observation
      };
  }
  // NOLINTEND(bugprone-branch-clone)

  BUILTIN_UNREACHABLE
}

Qgis::RelationshipCardinality QgsSensorThingsUtils::relationshipCardinality( Qgis::SensorThingsEntity baseType, Qgis::SensorThingsEntity relatedType, bool &valid )
{
  valid = true;
  switch ( baseType )
  {
    case Qgis::SensorThingsEntity::Invalid:
      break;

    case Qgis::SensorThingsEntity::Thing:
    {
      switch ( relatedType )
      {
        case Qgis::SensorThingsEntity::Location:
          return Qgis::RelationshipCardinality::ManyToMany;

        case Qgis::SensorThingsEntity::HistoricalLocation:
        case Qgis::SensorThingsEntity::Datastream:
        case Qgis::SensorThingsEntity::MultiDatastream:
          return Qgis::RelationshipCardinality::OneToMany;

        case Qgis::SensorThingsEntity::Invalid:
        case Qgis::SensorThingsEntity::Thing:
        case Qgis::SensorThingsEntity::Sensor:
        case Qgis::SensorThingsEntity::ObservedProperty:
        case Qgis::SensorThingsEntity::Observation:
        case Qgis::SensorThingsEntity::FeatureOfInterest:
          break;
      }
      break;
    }
    case Qgis::SensorThingsEntity::Location:
    {
      switch ( relatedType )
      {
        case Qgis::SensorThingsEntity::Thing:
        case Qgis::SensorThingsEntity::HistoricalLocation:
          return Qgis::RelationshipCardinality::ManyToMany;

        case Qgis::SensorThingsEntity::Invalid:
        case Qgis::SensorThingsEntity::Location:
        case Qgis::SensorThingsEntity::Datastream:
        case Qgis::SensorThingsEntity::Sensor:
        case Qgis::SensorThingsEntity::ObservedProperty:
        case Qgis::SensorThingsEntity::Observation:
        case Qgis::SensorThingsEntity::FeatureOfInterest:
        case Qgis::SensorThingsEntity::MultiDatastream:
          break;
      }
      break;
    }
    case Qgis::SensorThingsEntity::HistoricalLocation:
    {
      switch ( relatedType )
      {
        case Qgis::SensorThingsEntity::Location:
          // The SensorThings specification MAY be wrong here. There's an inconsistency between
          // the inheritance graph which shows HistoricalLocation linking to "location", when
          // the text description describes the relationship as linking to "locationS".
          // We assume the text description is correct and the graph is wrong, as the reverse
          // relationship between Location and HistoricalLocation is many-to-many.
          return Qgis::RelationshipCardinality::ManyToMany;

        case Qgis::SensorThingsEntity::Thing:
          return Qgis::RelationshipCardinality::OneToOne;

        case Qgis::SensorThingsEntity::Invalid:
        case Qgis::SensorThingsEntity::HistoricalLocation:
        case Qgis::SensorThingsEntity::Datastream:
        case Qgis::SensorThingsEntity::Sensor:
        case Qgis::SensorThingsEntity::ObservedProperty:
        case Qgis::SensorThingsEntity::Observation:
        case Qgis::SensorThingsEntity::FeatureOfInterest:
        case Qgis::SensorThingsEntity::MultiDatastream:
          break;
      }

      break;
    }

    case Qgis::SensorThingsEntity::Datastream:
    {
      switch ( relatedType )
      {
        case Qgis::SensorThingsEntity::Thing:
        case Qgis::SensorThingsEntity::Sensor:
        case Qgis::SensorThingsEntity::ObservedProperty:
          return Qgis::RelationshipCardinality::ManyToOne;

        case Qgis::SensorThingsEntity::Observation:
          return Qgis::RelationshipCardinality::OneToMany;

        case Qgis::SensorThingsEntity::Invalid:
        case Qgis::SensorThingsEntity::Location:
        case Qgis::SensorThingsEntity::HistoricalLocation:
        case Qgis::SensorThingsEntity::Datastream:
        case Qgis::SensorThingsEntity::FeatureOfInterest:
        case Qgis::SensorThingsEntity::MultiDatastream:
          break;
      }

      break;
    }

    case Qgis::SensorThingsEntity::Sensor:
    {
      switch ( relatedType )
      {
        case Qgis::SensorThingsEntity::Datastream:
        case Qgis::SensorThingsEntity::MultiDatastream:
          return Qgis::RelationshipCardinality::OneToMany;

        case Qgis::SensorThingsEntity::Invalid:
        case Qgis::SensorThingsEntity::Thing:
        case Qgis::SensorThingsEntity::Location:
        case Qgis::SensorThingsEntity::HistoricalLocation:
        case Qgis::SensorThingsEntity::Observation:
        case Qgis::SensorThingsEntity::Sensor:
        case Qgis::SensorThingsEntity::ObservedProperty:
        case Qgis::SensorThingsEntity::FeatureOfInterest:
          break;
      }

      break;
    }

    case Qgis::SensorThingsEntity::ObservedProperty:
    {
      switch ( relatedType )
      {
        case Qgis::SensorThingsEntity::Datastream:
          return Qgis::RelationshipCardinality::OneToMany;

        case Qgis::SensorThingsEntity::MultiDatastream:
          return Qgis::RelationshipCardinality::ManyToMany;

        case Qgis::SensorThingsEntity::Invalid:
        case Qgis::SensorThingsEntity::Thing:
        case Qgis::SensorThingsEntity::Location:
        case Qgis::SensorThingsEntity::HistoricalLocation:
        case Qgis::SensorThingsEntity::Sensor:
        case Qgis::SensorThingsEntity::ObservedProperty:
        case Qgis::SensorThingsEntity::Observation:
        case Qgis::SensorThingsEntity::FeatureOfInterest:
          break;
      }
      break;
    }

    case Qgis::SensorThingsEntity::Observation:
    {
      switch ( relatedType )
      {
        case Qgis::SensorThingsEntity::Datastream:
        case Qgis::SensorThingsEntity::FeatureOfInterest:
        case Qgis::SensorThingsEntity::MultiDatastream:
          return Qgis::RelationshipCardinality::ManyToOne;

        case Qgis::SensorThingsEntity::Invalid:
        case Qgis::SensorThingsEntity::Thing:
        case Qgis::SensorThingsEntity::Location:
        case Qgis::SensorThingsEntity::HistoricalLocation:
        case Qgis::SensorThingsEntity::Sensor:
        case Qgis::SensorThingsEntity::ObservedProperty:
        case Qgis::SensorThingsEntity::Observation:
          break;
      }
      break;
    }

    case Qgis::SensorThingsEntity::FeatureOfInterest:
    {
      switch ( relatedType )
      {
        case Qgis::SensorThingsEntity::Observation:
          return Qgis::RelationshipCardinality::OneToMany;

        case Qgis::SensorThingsEntity::Invalid:
        case Qgis::SensorThingsEntity::Thing:
        case Qgis::SensorThingsEntity::Location:
        case Qgis::SensorThingsEntity::HistoricalLocation:
        case Qgis::SensorThingsEntity::Datastream:
        case Qgis::SensorThingsEntity::Sensor:
        case Qgis::SensorThingsEntity::ObservedProperty:
        case Qgis::SensorThingsEntity::FeatureOfInterest:
        case Qgis::SensorThingsEntity::MultiDatastream:
          break;
      }

      break;
    }

    case Qgis::SensorThingsEntity::MultiDatastream:
    {
      switch ( relatedType )
      {
        case Qgis::SensorThingsEntity::Thing:
        case Qgis::SensorThingsEntity::Sensor:
          return Qgis::RelationshipCardinality::ManyToOne;

        case Qgis::SensorThingsEntity::ObservedProperty:
          return Qgis::RelationshipCardinality::ManyToMany;

        case Qgis::SensorThingsEntity::Observation:
          return Qgis::RelationshipCardinality::OneToMany;

        case Qgis::SensorThingsEntity::Invalid:
        case Qgis::SensorThingsEntity::Location:
        case Qgis::SensorThingsEntity::HistoricalLocation:
        case Qgis::SensorThingsEntity::Datastream:
        case Qgis::SensorThingsEntity::FeatureOfInterest:
        case Qgis::SensorThingsEntity::MultiDatastream:
          break;
      }
      break;
    }
  }

  valid = false;
  return Qgis::RelationshipCardinality::ManyToMany;
}

QString QgsSensorThingsUtils::asQueryString( Qgis::SensorThingsEntity baseType, const QList<QgsSensorThingsExpansionDefinition> &expansions )
{
  QString res;
  for ( int i = expansions.size() - 1; i >= 0 ; i-- )
  {
    const QgsSensorThingsExpansionDefinition &expansion = expansions.at( i );
    if ( !expansion.isValid() )
      continue;

    const Qgis::SensorThingsEntity parentType = i > 0 ? expansions.at( i - 1 ).childEntity() : baseType;

    res = expansion.asQueryString( parentType, res.isEmpty() ? QStringList() : QStringList{ res } );
  }

  return res;
}
