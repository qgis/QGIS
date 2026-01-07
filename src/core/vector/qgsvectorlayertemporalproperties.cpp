/***************************************************************************
                         qgsvectorlayertemporalproperties.cpp
                         ---------------
    begin                : May 2020
    copyright            : (C) 2020 by Nyall Dawson
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

#include "qgsvectorlayertemporalproperties.h"

#include "qgsexpression.h"
#include "qgsexpressioncontextutils.h"
#include "qgsfields.h"
#include "qgsunittypes.h"
#include "qgsvectordataprovidertemporalcapabilities.h"
#include "qgsvectorlayer.h"

#include "moc_qgsvectorlayertemporalproperties.cpp"

QgsVectorLayerTemporalProperties::QgsVectorLayerTemporalProperties( QObject *parent, bool enabled )
  :  QgsMapLayerTemporalProperties( parent, enabled )
{
}

bool QgsVectorLayerTemporalProperties::isVisibleInTemporalRange( const QgsDateTimeRange &range ) const
{
  if ( !isActive() )
    return true;

  switch ( mMode )
  {
    case Qgis::VectorTemporalMode::FixedTemporalRange:
      return range.isInfinite() || mFixedRange.isInfinite() || mFixedRange.overlaps( range );

    case Qgis::VectorTemporalMode::FeatureDateTimeInstantFromField:
    case Qgis::VectorTemporalMode::FeatureDateTimeStartAndEndFromFields:
    case Qgis::VectorTemporalMode::RedrawLayerOnly:
    case Qgis::VectorTemporalMode::FeatureDateTimeStartAndDurationFromFields:
    case Qgis::VectorTemporalMode::FeatureDateTimeStartAndEndFromExpressions:
      return true;
  }
  return true;
}

QgsDateTimeRange QgsVectorLayerTemporalProperties::calculateTemporalExtent( QgsMapLayer *layer ) const
{
  QgsVectorLayer *vectorLayer = qobject_cast<QgsVectorLayer *>( layer );
  if ( !layer )
    return QgsDateTimeRange();

  switch ( mMode )
  {
    case Qgis::VectorTemporalMode::FixedTemporalRange:
      return mFixedRange;

    case Qgis::VectorTemporalMode::FeatureDateTimeInstantFromField:
    {
      const int fieldIndex = vectorLayer->fields().lookupField( mStartFieldName );
      if ( fieldIndex >= 0 )
      {
        QVariant minVal;
        QVariant maxVal;
        vectorLayer->minimumAndMaximumValue( fieldIndex, minVal, maxVal );

        const QDateTime min = minVal.toDateTime();
        const QDateTime maxStartTime = maxVal.toDateTime();
        const QgsInterval eventDuration = QgsInterval( mFixedDuration, mDurationUnit );
        return QgsDateTimeRange( min, maxStartTime + eventDuration );
      }
      break;
    }

    case Qgis::VectorTemporalMode::FeatureDateTimeStartAndDurationFromFields:
    {
      const int fieldIndex = vectorLayer->fields().lookupField( mStartFieldName );
      const int durationFieldIndex = vectorLayer->fields().lookupField( mDurationFieldName );
      if ( fieldIndex >= 0 && durationFieldIndex >= 0 )
      {
        const QDateTime minTime = vectorLayer->minimumValue( fieldIndex ).toDateTime();
        // no choice here but to loop through all features to calculate max time :(

        QgsFeature f;
        QgsFeatureIterator it = vectorLayer->getFeatures( QgsFeatureRequest().setFlags( Qgis::FeatureRequestFlag::NoGeometry ).setSubsetOfAttributes( QgsAttributeList() << durationFieldIndex << fieldIndex ) );
        QDateTime maxTime;
        while ( it.nextFeature( f ) )
        {
          const QDateTime start = f.attribute( fieldIndex ).toDateTime();
          if ( start.isValid() )
          {
            const QVariant durationValue = f.attribute( durationFieldIndex );
            if ( durationValue.isValid() )
            {
              const double duration = durationValue.toDouble();
              const QDateTime end = start.addMSecs( QgsInterval( duration, mDurationUnit ).seconds() * 1000.0 );
              if ( end.isValid() )
                maxTime = maxTime.isValid() ? std::max( maxTime, end ) : end;
            }
          }
        }
        return QgsDateTimeRange( minTime, maxTime );
      }
      break;
    }

    case Qgis::VectorTemporalMode::FeatureDateTimeStartAndEndFromFields:
    {
      const int startFieldIndex = vectorLayer->fields().lookupField( mStartFieldName );
      const int endFieldIndex = vectorLayer->fields().lookupField( mEndFieldName );
      if ( startFieldIndex >= 0 && endFieldIndex >= 0 )
      {
        QVariant startMinVal;
        QVariant startMaxVal;
        vectorLayer->minimumAndMaximumValue( startFieldIndex, startMinVal, startMaxVal );
        QVariant endMinVal;
        QVariant endMaxVal;
        vectorLayer->minimumAndMaximumValue( endFieldIndex, endMinVal, endMaxVal );

        return QgsDateTimeRange( std::min( startMinVal.toDateTime(),
                                           endMinVal.toDateTime() ),
                                 std::max( startMaxVal.toDateTime(),
                                           endMaxVal.toDateTime() ) );
      }
      else if ( startFieldIndex >= 0 )
      {
        QVariant startMinVal;
        QVariant startMaxVal;
        vectorLayer->minimumAndMaximumValue( startFieldIndex, startMinVal, startMaxVal );
        return QgsDateTimeRange( startMinVal.toDateTime(),
                                 startMaxVal.toDateTime() );
      }
      else if ( endFieldIndex >= 0 )
      {
        QVariant endMinVal;
        QVariant endMaxVal;
        vectorLayer->minimumAndMaximumValue( endFieldIndex, endMinVal, endMaxVal );
        return QgsDateTimeRange( endMinVal.toDateTime(),
                                 endMaxVal.toDateTime() );
      }
      break;
    }

    case Qgis::VectorTemporalMode::FeatureDateTimeStartAndEndFromExpressions:
    {
      const bool hasStartExpression = !mStartExpression.isEmpty();
      const bool hasEndExpression = !mEndExpression.isEmpty();
      if ( !hasStartExpression && !hasEndExpression )
        return QgsDateTimeRange();

      QDateTime minTime;
      QDateTime maxTime;

      // no choice here but to loop through all features
      QgsExpressionContext context;
      context.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( vectorLayer ) );

      QgsExpression startExpression;
      if ( hasStartExpression )
      {
        startExpression.setExpression( mStartExpression );
        startExpression.prepare( &context );
      }

      QgsExpression endExpression;
      if ( hasEndExpression )
      {
        endExpression.setExpression( mEndExpression );
        endExpression.prepare( &context );
      }

      QSet< QString > fields;
      if ( hasStartExpression )
        fields.unite( startExpression.referencedColumns() );
      if ( hasEndExpression )
        fields.unite( endExpression.referencedColumns() );

      const bool needsGeom = startExpression.needsGeometry() || endExpression.needsGeometry();

      QgsFeatureRequest req;
      if ( !needsGeom )
        req.setFlags( Qgis::FeatureRequestFlag::NoGeometry );

      req.setSubsetOfAttributes( fields, vectorLayer->fields() );

      QgsFeature f;
      QgsFeatureIterator it = vectorLayer->getFeatures( req );
      while ( it.nextFeature( f ) )
      {
        context.setFeature( f );
        const QDateTime start = hasStartExpression ? startExpression.evaluate( &context ).toDateTime() : QDateTime();
        const QDateTime end = hasEndExpression ? endExpression.evaluate( &context ).toDateTime() : QDateTime();

        if ( start.isValid() )
        {
          minTime = minTime.isValid() ? std::min( minTime, start ) : start;
          if ( !hasEndExpression )
            maxTime = maxTime.isValid() ? std::max( maxTime, start ) : start;
        }
        if ( end.isValid() )
        {
          maxTime = maxTime.isValid() ? std::max( maxTime, end ) : end;
          if ( !hasStartExpression )
            minTime = minTime.isValid() ? std::min( minTime, end ) : end;
        }
      }
      return QgsDateTimeRange( minTime, maxTime );
    }

    case Qgis::VectorTemporalMode::RedrawLayerOnly:
      break;
  }

  return QgsDateTimeRange();
}

Qgis::VectorTemporalMode QgsVectorLayerTemporalProperties::mode() const
{
  return mMode;
}

void QgsVectorLayerTemporalProperties::setMode( Qgis::VectorTemporalMode mode )
{
  if ( mMode == mode )
    return;
  mMode = mode;
}

Qgis::VectorTemporalLimitMode QgsVectorLayerTemporalProperties::limitMode() const
{
  return mLimitMode;
}

void QgsVectorLayerTemporalProperties::setLimitMode( Qgis::VectorTemporalLimitMode limitMode )
{
  if ( mLimitMode == limitMode )
    return;
  mLimitMode = limitMode;
}

QgsTemporalProperty::Flags QgsVectorLayerTemporalProperties::flags() const
{
  return mode() == Qgis::VectorTemporalMode::FixedTemporalRange ? QgsTemporalProperty::FlagDontInvalidateCachedRendersWhenRangeChanges : QgsTemporalProperty::Flags();
}

void  QgsVectorLayerTemporalProperties::setFixedTemporalRange( const QgsDateTimeRange &range )
{
  mFixedRange = range;
}

const QgsDateTimeRange &QgsVectorLayerTemporalProperties::fixedTemporalRange() const
{
  return mFixedRange;
}

bool QgsVectorLayerTemporalProperties::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )

  const QDomElement temporalNode = element.firstChildElement( u"temporal"_s );

  setIsActive( temporalNode.attribute( u"enabled"_s, u"0"_s ).toInt() );

  mMode = static_cast< Qgis::VectorTemporalMode >( temporalNode.attribute( u"mode"_s, u"0"_s ). toInt() );

  mLimitMode = static_cast< Qgis::VectorTemporalLimitMode >( temporalNode.attribute( u"limitMode"_s, u"0"_s ). toInt() );
  mStartFieldName = temporalNode.attribute( u"startField"_s );
  mEndFieldName = temporalNode.attribute( u"endField"_s );
  mStartExpression = temporalNode.attribute( u"startExpression"_s );
  mEndExpression = temporalNode.attribute( u"endExpression"_s );
  mDurationFieldName = temporalNode.attribute( u"durationField"_s );
  mDurationUnit = QgsUnitTypes::decodeTemporalUnit( temporalNode.attribute( u"durationUnit"_s, QgsUnitTypes::encodeUnit( Qgis::TemporalUnit::Minutes ) ) );
  mFixedDuration = temporalNode.attribute( u"fixedDuration"_s ).toDouble();
  mAccumulateFeatures = temporalNode.attribute( u"accumulate"_s, u"0"_s ).toInt();

  const QDomNode rangeElement = temporalNode.namedItem( u"fixedRange"_s );

  const QDomNode begin = rangeElement.namedItem( u"start"_s );
  const QDomNode end = rangeElement.namedItem( u"end"_s );

  const QDateTime beginDate = QDateTime::fromString( begin.toElement().text(), Qt::ISODate );
  const QDateTime endDate = QDateTime::fromString( end.toElement().text(), Qt::ISODate );

  const QgsDateTimeRange range = QgsDateTimeRange( beginDate, endDate, true, mLimitMode == Qgis::VectorTemporalLimitMode::IncludeBeginIncludeEnd );
  setFixedTemporalRange( range );

  return true;
}

QDomElement QgsVectorLayerTemporalProperties::writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )
  if ( element.isNull() )
    return QDomElement();

  QDomElement temporalElement = document.createElement( u"temporal"_s );
  temporalElement.setAttribute( u"enabled"_s, isActive() ? u"1"_s : u"0"_s );
  temporalElement.setAttribute( u"mode"_s, QString::number( static_cast< int >( mMode ) ) );

  temporalElement.setAttribute( u"limitMode"_s, QString::number( static_cast< int >( mLimitMode ) ) );
  temporalElement.setAttribute( u"startField"_s, mStartFieldName );
  temporalElement.setAttribute( u"endField"_s, mEndFieldName );
  temporalElement.setAttribute( u"startExpression"_s, mStartExpression );
  temporalElement.setAttribute( u"endExpression"_s, mEndExpression );
  temporalElement.setAttribute( u"durationField"_s, mDurationFieldName );
  temporalElement.setAttribute( u"durationUnit"_s, QgsUnitTypes::encodeUnit( mDurationUnit ) );
  temporalElement.setAttribute( u"fixedDuration"_s, qgsDoubleToString( mFixedDuration ) );
  temporalElement.setAttribute( u"accumulate"_s, mAccumulateFeatures ? u"1"_s : u"0"_s );

  QDomElement rangeElement = document.createElement( u"fixedRange"_s );

  QDomElement startElement = document.createElement( u"start"_s );
  QDomElement endElement = document.createElement( u"end"_s );

  const QDomText startText = document.createTextNode( mFixedRange.begin().toTimeSpec( Qt::OffsetFromUTC ).toString( Qt::ISODate ) );
  const QDomText endText = document.createTextNode( mFixedRange.end().toTimeSpec( Qt::OffsetFromUTC ).toString( Qt::ISODate ) );
  startElement.appendChild( startText );
  endElement.appendChild( endText );
  rangeElement.appendChild( startElement );
  rangeElement.appendChild( endElement );

  temporalElement.appendChild( rangeElement );

  element.appendChild( temporalElement );

  return element;
}

void QgsVectorLayerTemporalProperties::setDefaultsFromDataProviderTemporalCapabilities( const QgsDataProviderTemporalCapabilities *capabilities )
{
  if ( const QgsVectorDataProviderTemporalCapabilities *vectorCaps = dynamic_cast< const QgsVectorDataProviderTemporalCapabilities *>( capabilities ) )
  {
    setIsActive( vectorCaps->hasTemporalCapabilities() );
    setFixedTemporalRange( vectorCaps->availableTemporalRange() );
    setStartField( vectorCaps->startField() );
    setEndField( vectorCaps->endField() );
    switch ( vectorCaps->mode() )
    {
      case Qgis::VectorDataProviderTemporalMode::HasFixedTemporalRange:
        setMode( Qgis::VectorTemporalMode::FixedTemporalRange );
        break;
      case Qgis::VectorDataProviderTemporalMode::StoresFeatureDateTimeInstantInField:
        setMode( Qgis::VectorTemporalMode::FeatureDateTimeInstantFromField );
        break;
      case Qgis::VectorDataProviderTemporalMode::StoresFeatureDateTimeStartAndEndInSeparateFields:
        setMode( Qgis::VectorTemporalMode::FeatureDateTimeStartAndEndFromFields );
        break;
    }
  }
}

QString QgsVectorLayerTemporalProperties::startExpression() const
{
  return mStartExpression;
}

void QgsVectorLayerTemporalProperties::setStartExpression( const QString &startExpression )
{
  mStartExpression = startExpression;
}

QString QgsVectorLayerTemporalProperties::endExpression() const
{
  return mEndExpression;
}

void QgsVectorLayerTemporalProperties::setEndExpression( const QString &endExpression )
{
  mEndExpression = endExpression;
}

bool QgsVectorLayerTemporalProperties::accumulateFeatures() const
{
  return mAccumulateFeatures;
}

void QgsVectorLayerTemporalProperties::setAccumulateFeatures( bool accumulateFeatures )
{
  mAccumulateFeatures = accumulateFeatures;
}

double QgsVectorLayerTemporalProperties::fixedDuration() const
{
  return mFixedDuration;
}

void QgsVectorLayerTemporalProperties::setFixedDuration( double fixedDuration )
{
  mFixedDuration = fixedDuration;
}

QString QgsVectorLayerTemporalProperties::startField() const
{
  return mStartFieldName;
}

void QgsVectorLayerTemporalProperties::setStartField( const QString &startFieldName )
{
  mStartFieldName = startFieldName;
}

QString QgsVectorLayerTemporalProperties::endField() const
{
  return mEndFieldName;
}

void QgsVectorLayerTemporalProperties::setEndField( const QString &field )
{
  mEndFieldName = field;
}

QString QgsVectorLayerTemporalProperties::durationField() const
{
  return mDurationFieldName;
}

void QgsVectorLayerTemporalProperties::setDurationField( const QString &field )
{
  mDurationFieldName = field;
}

Qgis::TemporalUnit QgsVectorLayerTemporalProperties::durationUnits() const
{
  return mDurationUnit;
}

void QgsVectorLayerTemporalProperties::setDurationUnits( Qgis::TemporalUnit units )
{
  mDurationUnit = units;
}

QString dateTimeExpressionLiteral( const QDateTime &datetime )
{
  return u"make_datetime(%1,%2,%3,%4,%5,%6)"_s.arg( datetime.date().year() )
         .arg( datetime.date().month() )
         .arg( datetime.date().day() )
         .arg( datetime.time().hour() )
         .arg( datetime.time().minute() )
         .arg( datetime.time().second() + datetime.time().msec() / 1000.0 );
}

QString QgsVectorLayerTemporalProperties::createFilterString( const QgsVectorLayerTemporalContext &context, const QgsDateTimeRange &filterRange ) const
{
  if ( !isActive() )
    return QString();

  auto dateTimefieldCast = [ &context ]( const QString & fieldName ) -> QString
  {
    if ( context.layer()
         && context.layer()->fields().lookupField( fieldName ) >= 0
         && context.layer()->fields().at( context.layer()->fields().lookupField( fieldName ) ).type() != QMetaType::Type::QDateTime )
    {
      return u"to_datetime( %1 )"_s .arg( QgsExpression::quotedColumnRef( fieldName ) );
    }
    return QgsExpression::quotedColumnRef( fieldName );
  };

  switch ( mMode )
  {
    case Qgis::VectorTemporalMode::FixedTemporalRange:
    case Qgis::VectorTemporalMode::RedrawLayerOnly:
      return QString();

    case Qgis::VectorTemporalMode::FeatureDateTimeInstantFromField:
    {
      if ( mAccumulateFeatures )
      {
        return u"(%1 %2 %3) OR %1 IS NULL"_s.arg( dateTimefieldCast( mStartFieldName ),
               filterRange.includeEnd() ? u"<="_s : u"<"_s,
               dateTimeExpressionLiteral( filterRange.end() ) );
      }
      else if ( qgsDoubleNear( mFixedDuration, 0.0 ) )
      {
        return u"(%1 %2 %3 AND %1 %4 %5) OR %1 IS NULL"_s.arg( dateTimefieldCast( mStartFieldName ),
               filterRange.includeBeginning() ? u">="_s : u">"_s,
               dateTimeExpressionLiteral( filterRange.begin() ),
               filterRange.includeEnd() ? u"<="_s : u"<"_s,
               dateTimeExpressionLiteral( filterRange.end() ) );
      }
      else
      {
        // Working with features with events with a duration, so taking this duration into account (+ QgsInterval( -mFixedDuration, mDurationUnit ) ))
        return u"(%1 %2 %3 AND %1 %4 %5) OR %1 IS NULL"_s.arg( dateTimefieldCast( mStartFieldName ),
               limitMode() == Qgis::VectorTemporalLimitMode::IncludeBeginIncludeEnd ? u">="_s : u">"_s,
               dateTimeExpressionLiteral( filterRange.begin() + QgsInterval( -mFixedDuration, mDurationUnit ) ),
               filterRange.includeEnd() ? u"<="_s : u"<"_s,
               dateTimeExpressionLiteral( filterRange.end() ) );
      }
    }

    case Qgis::VectorTemporalMode::FeatureDateTimeStartAndDurationFromFields:
    {
      QString intervalExpression;
      switch ( mDurationUnit )
      {
        case Qgis::TemporalUnit::Milliseconds:
          intervalExpression = u"make_interval(0,0,0,0,0,0,%1/1000)"_s.arg( QgsExpression::quotedColumnRef( mDurationFieldName ) );
          break;

        case Qgis::TemporalUnit::Seconds:
          intervalExpression = u"make_interval(0,0,0,0,0,0,%1)"_s.arg( QgsExpression::quotedColumnRef( mDurationFieldName ) );
          break;

        case Qgis::TemporalUnit::Minutes:
          intervalExpression = u"make_interval(0,0,0,0,0,%1,0)"_s.arg( QgsExpression::quotedColumnRef( mDurationFieldName ) );
          break;

        case Qgis::TemporalUnit::Hours:
          intervalExpression = u"make_interval(0,0,0,0,%1,0,0)"_s.arg( QgsExpression::quotedColumnRef( mDurationFieldName ) );
          break;

        case Qgis::TemporalUnit::Days:
          intervalExpression = u"make_interval(0,0,0,%1,0,0,0)"_s.arg( QgsExpression::quotedColumnRef( mDurationFieldName ) );
          break;

        case Qgis::TemporalUnit::Weeks:
          intervalExpression = u"make_interval(0,0,%1,0,0,0,0)"_s.arg( QgsExpression::quotedColumnRef( mDurationFieldName ) );
          break;

        case Qgis::TemporalUnit::Months:
          intervalExpression = u"make_interval(0,%1,0,0,0,0,0)"_s.arg( QgsExpression::quotedColumnRef( mDurationFieldName ) );
          break;

        case Qgis::TemporalUnit::Years:
          intervalExpression = u"make_interval(%1,0,0,0,0,0,0)"_s.arg( QgsExpression::quotedColumnRef( mDurationFieldName ) );
          break;

        case Qgis::TemporalUnit::Decades:
          intervalExpression = u"make_interval(10 * %1,0,0,0,0,0,0)"_s.arg( QgsExpression::quotedColumnRef( mDurationFieldName ) );
          break;

        case Qgis::TemporalUnit::Centuries:
          intervalExpression = u"make_interval(100 * %1,0,0,0,0,0,0)"_s.arg( QgsExpression::quotedColumnRef( mDurationFieldName ) );
          break;

        case Qgis::TemporalUnit::Unknown:
        case Qgis::TemporalUnit::IrregularStep:
          return QString();
      }
      return u"(%1 %2 %3 OR %1 IS NULL) AND ((%1 + %4 %5 %6) OR %7 IS NULL)"_s.arg( dateTimefieldCast( mStartFieldName ),
             filterRange.includeEnd() ? u"<="_s : u"<"_s,
             dateTimeExpressionLiteral( filterRange.end() ),
             intervalExpression,
             limitMode() == Qgis::VectorTemporalLimitMode::IncludeBeginIncludeEnd ? u">="_s : u">"_s,
             dateTimeExpressionLiteral( filterRange.begin() ),
             QgsExpression::quotedColumnRef( mDurationFieldName ) );
    }

    case Qgis::VectorTemporalMode::FeatureDateTimeStartAndEndFromFields:
    {
      if ( !mStartFieldName.isEmpty() && !mEndFieldName.isEmpty() )
      {
        return u"(%1 %2 %3 OR %1 IS NULL) AND (%4 %5 %6 OR %4 IS NULL)"_s.arg( dateTimefieldCast( mStartFieldName ),
               filterRange.includeEnd() ? u"<="_s : u"<"_s,
               dateTimeExpressionLiteral( filterRange.end() ),
               dateTimefieldCast( mEndFieldName ),
               limitMode() == Qgis::VectorTemporalLimitMode::IncludeBeginIncludeEnd ? u">="_s : u">"_s,
               dateTimeExpressionLiteral( filterRange.begin() ) );

      }
      else if ( !mStartFieldName.isEmpty() )
      {
        return u"%1 %2 %3 OR %1 IS NULL"_s.arg( dateTimefieldCast( mStartFieldName ),
                                                filterRange.includeEnd() ? u"<="_s : u"<"_s,
                                                dateTimeExpressionLiteral( filterRange.end() ) );
      }
      else if ( !mEndFieldName.isEmpty() )
      {
        return u"%1 %2 %3 OR %1 IS NULL"_s.arg( dateTimefieldCast( mEndFieldName ),
                                                limitMode() == Qgis::VectorTemporalLimitMode::IncludeBeginIncludeEnd ? u">="_s : u">"_s,
                                                dateTimeExpressionLiteral( filterRange.begin() ) );
      }
      break;
    }

    case Qgis::VectorTemporalMode::FeatureDateTimeStartAndEndFromExpressions:
    {
      if ( !mStartExpression.isEmpty() && !mEndExpression.isEmpty() )
      {
        return u"((%1) %2 %3) AND ((%4) %5 %6)"_s.arg( mStartExpression,
               filterRange.includeEnd() ? u"<="_s : u"<"_s,
               dateTimeExpressionLiteral( filterRange.end() ),
               mEndExpression,
               limitMode() == Qgis::VectorTemporalLimitMode::IncludeBeginIncludeEnd ? u">="_s : u">"_s,
               dateTimeExpressionLiteral( filterRange.begin() ) );
      }
      else if ( !mStartExpression.isEmpty() )
      {
        return u"(%1) %2 %3"_s.arg( mStartExpression,
                                    filterRange.includeEnd() ? u"<="_s : u"<"_s,
                                    dateTimeExpressionLiteral( filterRange.end() ) );
      }
      else if ( !mEndExpression.isEmpty() )
      {
        return u"(%1) %2 %3"_s.arg( mEndExpression,
                                    limitMode() == Qgis::VectorTemporalLimitMode::IncludeBeginIncludeEnd ? u">="_s : u">"_s,
                                    dateTimeExpressionLiteral( filterRange.begin() ) );
      }
      break;
    }
  }

  return QString();
}

void QgsVectorLayerTemporalProperties::guessDefaultsFromFields( const QgsFields &fields )
{

  // Check the fields and keep the first one that matches.
  // We assume that the user has organized the data with the
  // more "interesting" field names first.
  // This candidates list is a prioritized list of candidates ranked by "interestingness"!
  // See discussion at https://github.com/qgis/QGIS/pull/30245 - this list must NOT be translated,
  // but adding hardcoded localized variants of the strings is encouraged.
  static const QStringList sStartCandidates{ u"start"_s,
      u"begin"_s,
      u"from"_s,
      u"since"_s,
      // German candidates
      u"anfang"_s,
      u"von"_s,
      u"ab"_s,
      u"seit"_s };

  static const QStringList sEndCandidates{ u"end"_s,
      u"last"_s,
      u"to"_s,
      u"stop"_s,
      // German candidates
      u"ende"_s,
      u"bis"_s };


  static const QStringList sSingleFieldCandidates{ u"event"_s };


  bool foundStart = false;
  bool foundEnd = false;

  for ( const QgsField &field : fields )
  {
    if ( field.type() != QMetaType::Type::QDate && field.type() != QMetaType::Type::QDateTime )
      continue;

    if ( !foundStart )
    {
      for ( const QString &candidate : sStartCandidates )
      {
        const QString fldName = field.name();
        if ( fldName.indexOf( candidate, 0, Qt::CaseInsensitive ) > -1 )
        {
          mStartFieldName = fldName;
          foundStart = true;
        }
      }
    }

    if ( !foundEnd )
    {
      for ( const QString &candidate : sEndCandidates )
      {
        const QString fldName = field.name();
        if ( fldName.indexOf( candidate, 0, Qt::CaseInsensitive ) > -1 )
        {
          mEndFieldName = fldName;
          foundEnd = true;
        }
      }
    }

    if ( foundStart && foundEnd )
      break;
  }

  if ( !foundStart )
  {
    // loop again, looking for likely "single field" candidates
    for ( const QgsField &field : fields )
    {
      if ( field.type() != QMetaType::Type::QDate && field.type() != QMetaType::Type::QDateTime )
        continue;

      for ( const QString &candidate : sSingleFieldCandidates )
      {
        const QString fldName = field.name();
        if ( fldName.indexOf( candidate, 0, Qt::CaseInsensitive ) > -1 )
        {
          mStartFieldName = fldName;
          foundStart = true;
        }
      }

      if ( foundStart )
        break;
    }
  }

  if ( foundStart && foundEnd )
    mMode = Qgis::VectorTemporalMode::FeatureDateTimeStartAndEndFromFields;
  else if ( foundStart )
    mMode = Qgis::VectorTemporalMode::FeatureDateTimeInstantFromField;

  // note -- NEVER auto enable temporal properties here! It's just a helper designed
  // to shortcut the initial field selection
}

QgsVectorLayer *QgsVectorLayerTemporalContext::layer() const
{
  return mLayer;
}

void QgsVectorLayerTemporalContext::setLayer( QgsVectorLayer *layer )
{
  mLayer = layer;
}
