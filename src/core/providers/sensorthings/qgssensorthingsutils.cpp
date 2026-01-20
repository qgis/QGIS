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

#include <nlohmann/json.hpp>

#include "qgsblockingnetworkrequest.h"
#include "qgsfield.h"
#include "qgsfields.h"
#include "qgslogger.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsrectangle.h"
#include "qgssetrequestinitiator_p.h"
#include "qgswkbtypes.h"

#include <QNetworkRequest>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QUrl>

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
               u"phenomenonTime"_s, Qt::SortOrder::DescendingOrder
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
    parts.append( u"orderby=%1,%2"_s.arg( mOrderBy, mSortOrder == Qt::SortOrder::AscendingOrder ? u"asc"_s : u"desc"_s ) );
  if ( mLimit >= 0 )
    parts.append( u"limit=%1"_s.arg( mLimit ) );
  if ( !mFilter.trimmed().isEmpty() )
  {
    QString escapedFilter = mFilter;
    escapedFilter.replace( ':', "\\colon"_L1 );
    parts.append( u"filter=%1"_s.arg( escapedFilter ) );
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
    const thread_local QRegularExpression orderByRegEx( u"^orderby=(.*),(.*?)$"_s );
    const thread_local QRegularExpression orderLimitRegEx( u"^limit=(\\d+)$"_s );
    const thread_local QRegularExpression filterRegEx( u"^filter=(.*)$"_s );

    const QRegularExpressionMatch orderByMatch = orderByRegEx.match( part );
    if ( orderByMatch.hasMatch() )
    {
      definition.setOrderBy( orderByMatch.captured( 1 ) );
      definition.setSortOrder( orderByMatch.captured( 2 ) == "asc"_L1 ? Qt::SortOrder::AscendingOrder : Qt::SortOrder::DescendingOrder );
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
      filter.replace( "\\colon"_L1, ":"_L1 );
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

  QString res = u"$expand=%1"_s.arg( childEntityString );

  QStringList queryOptions;
  if ( !mOrderBy.isEmpty() )
    queryOptions.append( u"$orderby=%1%2"_s.arg( mOrderBy, mSortOrder == Qt::SortOrder::AscendingOrder ? QString() : u" desc"_s ) );

  if ( mLimit > -1 )
    queryOptions.append( u"$top=%1"_s.arg( mLimit ) );

  if ( !mFilter.isEmpty() )
    queryOptions.append( u"$filter=%1"_s.arg( mFilter ) );

  queryOptions.append( additionalOptions );

  if ( !queryOptions.isEmpty() )
    res.append( u"(%1)"_s.arg( queryOptions.join( ';' ) ) );

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
  if ( trimmed.compare( "Thing"_L1, Qt::CaseInsensitive ) == 0 )
    return Qgis::SensorThingsEntity::Thing;
  if ( trimmed.compare( "Location"_L1, Qt::CaseInsensitive ) == 0 )
    return Qgis::SensorThingsEntity::Location;
  if ( trimmed.compare( "HistoricalLocation"_L1, Qt::CaseInsensitive ) == 0 )
    return Qgis::SensorThingsEntity::HistoricalLocation;
  if ( trimmed.compare( "Datastream"_L1, Qt::CaseInsensitive ) == 0 )
    return Qgis::SensorThingsEntity::Datastream;
  if ( trimmed.compare( "Sensor"_L1, Qt::CaseInsensitive ) == 0 )
    return Qgis::SensorThingsEntity::Sensor;
  if ( trimmed.compare( "ObservedProperty"_L1, Qt::CaseInsensitive ) == 0 )
    return Qgis::SensorThingsEntity::ObservedProperty;
  if ( trimmed.compare( "Observation"_L1, Qt::CaseInsensitive ) == 0 )
    return Qgis::SensorThingsEntity::Observation;
  if ( trimmed.compare( "FeatureOfInterest"_L1, Qt::CaseInsensitive ) == 0 )
    return Qgis::SensorThingsEntity::FeatureOfInterest;
  if ( trimmed.compare( "MultiDatastream"_L1, Qt::CaseInsensitive ) == 0 )
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
  if ( trimmed.compare( "Things"_L1, Qt::CaseInsensitive ) == 0 )
    return Qgis::SensorThingsEntity::Thing;
  if ( trimmed.compare( "Locations"_L1, Qt::CaseInsensitive ) == 0 )
    return Qgis::SensorThingsEntity::Location;
  if ( trimmed.compare( "HistoricalLocations"_L1, Qt::CaseInsensitive ) == 0 )
    return Qgis::SensorThingsEntity::HistoricalLocation;
  if ( trimmed.compare( "Datastreams"_L1, Qt::CaseInsensitive ) == 0 )
    return Qgis::SensorThingsEntity::Datastream;
  if ( trimmed.compare( "Sensors"_L1, Qt::CaseInsensitive ) == 0 )
    return Qgis::SensorThingsEntity::Sensor;
  if ( trimmed.compare( "ObservedProperties"_L1, Qt::CaseInsensitive ) == 0 )
    return Qgis::SensorThingsEntity::ObservedProperty;
  if ( trimmed.compare( "Observations"_L1, Qt::CaseInsensitive ) == 0 )
    return Qgis::SensorThingsEntity::Observation;
  if ( trimmed.compare( "FeaturesOfInterest"_L1, Qt::CaseInsensitive ) == 0 )
    return Qgis::SensorThingsEntity::FeatureOfInterest;
  if ( trimmed.compare( "MultiDatastreams"_L1, Qt::CaseInsensitive ) == 0 )
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
      return u"Things"_s;
    case Qgis::SensorThingsEntity::Location:
      return u"Locations"_s;
    case Qgis::SensorThingsEntity::HistoricalLocation:
      return u"HistoricalLocations"_s;
    case Qgis::SensorThingsEntity::Datastream:
      return u"Datastreams"_s;
    case Qgis::SensorThingsEntity::Sensor:
      return u"Sensors"_s;
    case Qgis::SensorThingsEntity::ObservedProperty:
      return u"ObservedProperties"_s;
    case Qgis::SensorThingsEntity::Observation:
      return u"Observations"_s;
    case Qgis::SensorThingsEntity::FeatureOfInterest:
      return u"FeaturesOfInterest"_s;
    case Qgis::SensorThingsEntity::MultiDatastream:
      return u"MultiDatastreams"_s;
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
      return { u"id"_s,
               u"selfLink"_s,
               u"name"_s,
               u"description"_s,
               u"properties"_s,
           };

    case Qgis::SensorThingsEntity::Location:
      // https://docs.ogc.org/is/18-088/18-088.html#location
      return { u"id"_s,
               u"selfLink"_s,
               u"name"_s,
               u"description"_s,
               u"properties"_s,
           };

    case Qgis::SensorThingsEntity::HistoricalLocation:
      // https://docs.ogc.org/is/18-088/18-088.html#historicallocation
      return { u"id"_s,
               u"selfLink"_s,
               u"time"_s,
           };

    case Qgis::SensorThingsEntity::Datastream:
      // https://docs.ogc.org/is/18-088/18-088.html#datastream
      return { u"id"_s,
               u"selfLink"_s,
               u"name"_s,
               u"description"_s,
               u"unitOfMeasurement"_s,
               u"observationType"_s,
               u"properties"_s,
               u"phenomenonTime"_s,
               u"resultTime"_s,
           };

    case Qgis::SensorThingsEntity::Sensor:
      // https://docs.ogc.org/is/18-088/18-088.html#sensor
      return { u"id"_s,
               u"selfLink"_s,
               u"name"_s,
               u"description"_s,
               u"metadata"_s,
               u"properties"_s,
           };

    case Qgis::SensorThingsEntity::ObservedProperty:
      // https://docs.ogc.org/is/18-088/18-088.html#observedproperty
      return { u"id"_s,
               u"selfLink"_s,
               u"name"_s,
               u"definition"_s,
               u"description"_s,
               u"properties"_s,
           };

    case Qgis::SensorThingsEntity::Observation:
      // https://docs.ogc.org/is/18-088/18-088.html#observation
      return { u"id"_s,
               u"selfLink"_s,
               u"phenomenonTime"_s,
               u"result"_s,
               u"resultTime"_s,
               u"resultQuality"_s,
               u"validTime"_s,
               u"parameters"_s,
           };

    case Qgis::SensorThingsEntity::FeatureOfInterest:
      // https://docs.ogc.org/is/18-088/18-088.html#featureofinterest
      return { u"id"_s,
               u"selfLink"_s,
               u"name"_s,
               u"description"_s,
               u"properties"_s,
           };

    case Qgis::SensorThingsEntity::MultiDatastream:
      // https://docs.ogc.org/is/18-088/18-088.html#multidatastream-extension
      return { u"id"_s,
               u"selfLink"_s,
               u"name"_s,
               u"description"_s,
               u"unitOfMeasurements"_s,
               u"observationType"_s,
               u"multiObservationDataTypes"_s,
               u"properties"_s,
               u"phenomenonTime"_s,
               u"resultTime"_s,
           };
  }

  return {};
}

QgsFields QgsSensorThingsUtils::fieldsForEntityType( Qgis::SensorThingsEntity type, bool includeRangeFieldProxies )
{
  QgsFields fields;

  // common fields: https://docs.ogc.org/is/18-088/18-088.html#common-control-information
  fields.append( QgsField( u"id"_s, QMetaType::Type::QString ) );
  fields.append( QgsField( u"selfLink"_s, QMetaType::Type::QString ) );

  switch ( type )
  {
    case Qgis::SensorThingsEntity::Invalid:
      break;

    case Qgis::SensorThingsEntity::Thing:
      // https://docs.ogc.org/is/18-088/18-088.html#thing
      fields.append( QgsField( u"name"_s, QMetaType::Type::QString ) );
      fields.append( QgsField( u"description"_s, QMetaType::Type::QString ) );
      fields.append( QgsField( u"properties"_s, QMetaType::Type::QVariantMap, u"json"_s, 0, 0, QString(), QMetaType::Type::QString ) );
      break;

    case Qgis::SensorThingsEntity::Location:
      // https://docs.ogc.org/is/18-088/18-088.html#location
      fields.append( QgsField( u"name"_s, QMetaType::Type::QString ) );
      fields.append( QgsField( u"description"_s, QMetaType::Type::QString ) );
      fields.append( QgsField( u"properties"_s, QMetaType::Type::QVariantMap, u"json"_s, 0, 0, QString(), QMetaType::Type::QString ) );
      break;

    case Qgis::SensorThingsEntity::HistoricalLocation:
      // https://docs.ogc.org/is/18-088/18-088.html#historicallocation
      fields.append( QgsField( u"time"_s, QMetaType::Type::QDateTime ) );
      break;

    case Qgis::SensorThingsEntity::Datastream:
      // https://docs.ogc.org/is/18-088/18-088.html#datastream
      fields.append( QgsField( u"name"_s, QMetaType::Type::QString ) );
      fields.append( QgsField( u"description"_s, QMetaType::Type::QString ) );
      fields.append( QgsField( u"unitOfMeasurement"_s, QMetaType::Type::QVariantMap, u"json"_s, 0, 0, QString(), QMetaType::Type::QString ) );
      fields.append( QgsField( u"observationType"_s, QMetaType::Type::QString ) );
      fields.append( QgsField( u"properties"_s, QMetaType::Type::QVariantMap, u"json"_s, 0, 0, QString(), QMetaType::Type::QString ) );
      if ( includeRangeFieldProxies )
      {
        fields.append( QgsField( u"phenomenonTimeStart"_s, QMetaType::Type::QDateTime ) );
        fields.append( QgsField( u"phenomenonTimeEnd"_s, QMetaType::Type::QDateTime ) );
        fields.append( QgsField( u"resultTimeStart"_s, QMetaType::Type::QDateTime ) );
        fields.append( QgsField( u"resultTimeEnd"_s, QMetaType::Type::QDateTime ) );
      }
      break;

    case Qgis::SensorThingsEntity::Sensor:
      // https://docs.ogc.org/is/18-088/18-088.html#sensor
      fields.append( QgsField( u"name"_s, QMetaType::Type::QString ) );
      fields.append( QgsField( u"description"_s, QMetaType::Type::QString ) );
      fields.append( QgsField( u"metadata"_s, QMetaType::Type::QString ) );
      fields.append( QgsField( u"properties"_s, QMetaType::Type::QVariantMap, u"json"_s, 0, 0, QString(), QMetaType::Type::QString ) );
      break;

    case Qgis::SensorThingsEntity::ObservedProperty:
      // https://docs.ogc.org/is/18-088/18-088.html#observedproperty
      fields.append( QgsField( u"name"_s, QMetaType::Type::QString ) );
      fields.append( QgsField( u"definition"_s, QMetaType::Type::QString ) );
      fields.append( QgsField( u"description"_s, QMetaType::Type::QString ) );
      fields.append( QgsField( u"properties"_s, QMetaType::Type::QVariantMap, u"json"_s, 0, 0, QString(), QMetaType::Type::QString ) );
      break;

    case Qgis::SensorThingsEntity::Observation:
      // https://docs.ogc.org/is/18-088/18-088.html#observation
      if ( includeRangeFieldProxies )
      {
        fields.append( QgsField( u"phenomenonTimeStart"_s, QMetaType::Type::QDateTime ) );
        fields.append( QgsField( u"phenomenonTimeEnd"_s, QMetaType::Type::QDateTime ) );
      }

      // TODO -- handle type correctly
      fields.append( QgsField( u"result"_s, QMetaType::Type::QString ) );

      fields.append( QgsField( u"resultTime"_s, QMetaType::Type::QDateTime ) );
      fields.append( QgsField( u"resultQuality"_s, QMetaType::Type::QStringList, QString(), 0, 0, QString(), QMetaType::Type::QString ) );
      if ( includeRangeFieldProxies )
      {
        fields.append( QgsField( u"validTimeStart"_s, QMetaType::Type::QDateTime ) );
        fields.append( QgsField( u"validTimeEnd"_s, QMetaType::Type::QDateTime ) );
      }
      fields.append( QgsField( u"parameters"_s, QMetaType::Type::QVariantMap, u"json"_s, 0, 0, QString(), QMetaType::Type::QString ) );
      break;

    case Qgis::SensorThingsEntity::FeatureOfInterest:
      // https://docs.ogc.org/is/18-088/18-088.html#featureofinterest
      fields.append( QgsField( u"name"_s, QMetaType::Type::QString ) );
      fields.append( QgsField( u"description"_s, QMetaType::Type::QString ) );
      fields.append( QgsField( u"properties"_s, QMetaType::Type::QVariantMap, u"json"_s, 0, 0, QString(), QMetaType::Type::QString ) );
      break;

    case Qgis::SensorThingsEntity::MultiDatastream:
      // https://docs.ogc.org/is/18-088/18-088.html#multidatastream-extension
      fields.append( QgsField( u"name"_s, QMetaType::Type::QString ) );
      fields.append( QgsField( u"description"_s, QMetaType::Type::QString ) );
      fields.append( QgsField( u"unitOfMeasurements"_s, QMetaType::Type::QVariantMap, u"json"_s, 0, 0, QString(), QMetaType::Type::QString ) );
      fields.append( QgsField( u"observationType"_s, QMetaType::Type::QString ) );
      fields.append( QgsField( u"multiObservationDataTypes"_s, QMetaType::Type::QStringList, QString(), 0, 0, QString(), QMetaType::Type::QString ) );
      fields.append( QgsField( u"properties"_s, QMetaType::Type::QVariantMap, u"json"_s, 0, 0, QString(), QMetaType::Type::QString ) );
      if ( includeRangeFieldProxies )
      {
        fields.append( QgsField( u"phenomenonTimeStart"_s, QMetaType::Type::QDateTime ) );
        fields.append( QgsField( u"phenomenonTimeEnd"_s, QMetaType::Type::QDateTime ) );
        fields.append( QgsField( u"resultTimeStart"_s, QMetaType::Type::QDateTime ) );
        fields.append( QgsField( u"resultTimeEnd"_s, QMetaType::Type::QDateTime ) );
      }
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
      renamedExpandedField.setName( u"%1_%2"_s.arg( path, expandedField.name() ) );
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
    case Qgis::SensorThingsEntity::Sensor:
    case Qgis::SensorThingsEntity::Observation:
    case Qgis::SensorThingsEntity::ObservedProperty:
      return QString();

    case Qgis::SensorThingsEntity::Location:
      return u"location"_s;

    case Qgis::SensorThingsEntity::FeatureOfInterest:
      return u"feature"_s;

    case Qgis::SensorThingsEntity::Datastream:
    case Qgis::SensorThingsEntity::MultiDatastream:
      return u"observedArea"_s;
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
    case Qgis::SensorThingsEntity::Sensor:
    case Qgis::SensorThingsEntity::Observation:
    case Qgis::SensorThingsEntity::ObservedProperty:
      return false;

    case Qgis::SensorThingsEntity::Datastream:
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
    case Qgis::SensorThingsEntity::Sensor:
    case Qgis::SensorThingsEntity::Observation:
    case Qgis::SensorThingsEntity::ObservedProperty:
      return Qgis::GeometryType::Null;

    case Qgis::SensorThingsEntity::Datastream:
    case Qgis::SensorThingsEntity::Location:
    case Qgis::SensorThingsEntity::FeatureOfInterest:
    case Qgis::SensorThingsEntity::MultiDatastream:
      return Qgis::GeometryType::Unknown;
  }
  BUILTIN_UNREACHABLE
}

QString QgsSensorThingsUtils::filterForWkbType( Qgis::SensorThingsEntity entityType, Qgis::WkbType wkbType )
{
  QString geometryTypeString;
  switch ( QgsWkbTypes::geometryType( wkbType ) )
  {
    case Qgis::GeometryType::Point:
      geometryTypeString = u"Point"_s;
      break;
    case Qgis::GeometryType::Polygon:
      geometryTypeString = u"Polygon"_s;
      break;
    case Qgis::GeometryType::Line:
      geometryTypeString = u"LineString"_s;
      break;

    case Qgis::GeometryType::Unknown:
    case Qgis::GeometryType::Null:
      return QString();
  }

  const QString filterTarget = geometryFieldForEntityType( entityType );
  if ( filterTarget.isEmpty() )
    return QString();

  return u"%1/type eq '%2' or %1/geometry/type eq '%2'"_s.arg( filterTarget, geometryTypeString );
}

QString QgsSensorThingsUtils::filterForExtent( const QString &geometryField, const QgsRectangle &extent )
{
  // TODO -- confirm using 'geography' is always correct here
  return ( extent.isNull() || geometryField.isEmpty() )
         ? QString()
         : u"geo.intersects(%1, geography'%2')"_s.arg( geometryField, extent.asWktPolygon() );
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

  return u"("_s + nonEmptyFilters.join( ") and ("_L1 ) + u")"_s;
}

QList<Qgis::GeometryType> QgsSensorThingsUtils::availableGeometryTypes( const QString &uri, Qgis::SensorThingsEntity type, QgsFeedback *feedback, const QString &authCfg )
{
  QNetworkRequest request = QNetworkRequest( QUrl( uri ) );
  QgsSetRequestInitiatorClass( request, u"QgsSensorThingsUtils"_s )

  QgsBlockingNetworkRequest networkRequest;
  networkRequest.setAuthCfg( authCfg );

  switch ( networkRequest.get( request ) )
  {
    case QgsBlockingNetworkRequest::NoError:
      break;

    case QgsBlockingNetworkRequest::NetworkError:
    case QgsBlockingNetworkRequest::TimeoutError:
    case QgsBlockingNetworkRequest::ServerExceptionError:
      QgsDebugError( u"Connection failed: %1"_s.arg( networkRequest.errorMessage() ) );
      return {};
  }

  QString entityBaseUri;
  const QgsNetworkReplyContent content = networkRequest.reply();
  try
  {
    auto rootContent = nlohmann::json::parse( content.content().toStdString() );
    if ( !rootContent.contains( "value" ) )
    {
      QgsDebugError( u"No 'value' array in response"_s );
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
      QgsDebugError( u"Could not find url for %1"_s.arg( qgsEnumValueToKey( type ) ) );
      return {};
    }
  }
  catch ( const nlohmann::json::parse_error &ex )
  {
    QgsDebugError( u"Error parsing response: %1"_s.arg( ex.what() ) );
    return {};
  }

  auto getCountForType = [entityBaseUri, type, authCfg, feedback]( Qgis::GeometryType geometryType ) -> long long
  {
    // return no features, just the total count
    QString countUri = u"%1?$top=0&$count=true"_s.arg( entityBaseUri );
    Qgis::WkbType wkbType = geometryType == Qgis::GeometryType::Polygon ? Qgis::WkbType::Polygon : ( geometryType == Qgis::GeometryType::Line ? Qgis::WkbType::LineString : Qgis::WkbType::Point );
    const QString typeFilter = QgsSensorThingsUtils::filterForWkbType( type, wkbType );
    if ( !typeFilter.isEmpty() )
      countUri += u"&$filter="_s + typeFilter;

    const QUrl url( countUri );

    QNetworkRequest request( url );
    QgsSetRequestInitiatorClass( request, u"QgsSensorThingsSharedData"_s );

    QgsBlockingNetworkRequest networkRequest;
    networkRequest.setAuthCfg( authCfg );
    const QgsBlockingNetworkRequest::ErrorCode error = networkRequest.get( request, false, feedback );

    if ( feedback && feedback->isCanceled() )
      return -1;

    // Handle network errors
    if ( error != QgsBlockingNetworkRequest::NoError )
    {
      QgsDebugError( u"Network error: %1"_s.arg( networkRequest.errorMessage() ) );
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
          QgsDebugError( u"No '@iot.count' value in response"_s );
          return -1;
        }

        return rootContent["@iot.count"].get<long long>();
      }
      catch ( const nlohmann::json::parse_error &ex )
      {
        QgsDebugError( u"Error parsing response: %1"_s.arg( ex.what() ) );
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
        Qgis::SensorThingsEntity::Datastream,
        Qgis::SensorThingsEntity::MultiDatastream,
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
        Qgis::SensorThingsEntity::Datastream,
        Qgis::SensorThingsEntity::MultiDatastream,
      };

    case Qgis::SensorThingsEntity::ObservedProperty:
      return
      {
        Qgis::SensorThingsEntity::Datastream,
        Qgis::SensorThingsEntity::MultiDatastream
      };

    case Qgis::SensorThingsEntity::Observation:
      return
      {
        Qgis::SensorThingsEntity::Datastream,
        Qgis::SensorThingsEntity::MultiDatastream
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
