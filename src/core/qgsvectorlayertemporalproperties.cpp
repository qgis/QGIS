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
#include "qgsvectordataprovidertemporalcapabilities.h"
#include "qgsexpression.h"
#include "qgsvectorlayer.h"

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
    case ModeFixedTemporalRange:
      return range.isInfinite() || mFixedRange.isInfinite() || mFixedRange.overlaps( range );

    case ModeFeatureDateTimeInstantFromField:
    case ModeFeatureDateTimeStartAndEndFromFields:
    case ModeRedrawLayerOnly:
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
    case QgsVectorLayerTemporalProperties::ModeFixedTemporalRange:
      return mFixedRange;

    case QgsVectorLayerTemporalProperties::ModeFeatureDateTimeInstantFromField:
    {
      const int fieldIndex = vectorLayer->fields().lookupField( mStartFieldName );
      if ( fieldIndex >= 0 )
      {
        return QgsDateTimeRange( vectorLayer->minimumValue( fieldIndex ).toDateTime(),
                                 vectorLayer->maximumValue( fieldIndex ).toDateTime() );
      }
      break;
    }

    case QgsVectorLayerTemporalProperties::ModeFeatureDateTimeStartAndEndFromFields:
    {
      const int startFieldIndex = vectorLayer->fields().lookupField( mStartFieldName );
      const int endFieldIndex = vectorLayer->fields().lookupField( mEndFieldName );
      if ( startFieldIndex >= 0 && endFieldIndex >= 0 )
      {
        return QgsDateTimeRange( std::min( vectorLayer->minimumValue( startFieldIndex ).toDateTime(),
                                           vectorLayer->minimumValue( endFieldIndex ).toDateTime() ),
                                 std::max( vectorLayer->maximumValue( startFieldIndex ).toDateTime(),
                                           vectorLayer->maximumValue( endFieldIndex ).toDateTime() ) );
      }
      else if ( startFieldIndex >= 0 )
      {
        return QgsDateTimeRange( vectorLayer->minimumValue( startFieldIndex ).toDateTime(),
                                 vectorLayer->maximumValue( startFieldIndex ).toDateTime() );
      }
      else if ( endFieldIndex >= 0 )
      {
        return QgsDateTimeRange( vectorLayer->minimumValue( endFieldIndex ).toDateTime(),
                                 vectorLayer->maximumValue( endFieldIndex ).toDateTime() );
      }
      break;
    }

    case QgsVectorLayerTemporalProperties::ModeRedrawLayerOnly:
      break;
  }

  return QgsDateTimeRange();
}

QgsVectorLayerTemporalProperties::TemporalMode QgsVectorLayerTemporalProperties::mode() const
{
  return mMode;
}

void QgsVectorLayerTemporalProperties::setMode( QgsVectorLayerTemporalProperties::TemporalMode mode )
{
  if ( mMode == mode )
    return;
  mMode = mode;
}

QgsTemporalProperty::Flags QgsVectorLayerTemporalProperties::flags() const
{
  return mode() == ModeFixedTemporalRange ? QgsTemporalProperty::FlagDontInvalidateCachedRendersWhenRangeChanges : QgsTemporalProperty::Flags( nullptr );
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

  QDomElement temporalNode = element.firstChildElement( QStringLiteral( "temporal" ) );

  setIsActive( temporalNode.attribute( QStringLiteral( "enabled" ), QStringLiteral( "0" ) ).toInt() );

  mMode = static_cast< TemporalMode >( temporalNode.attribute( QStringLiteral( "mode" ), QStringLiteral( "0" ) ). toInt() );

  mStartFieldName = temporalNode.attribute( QStringLiteral( "startField" ) );
  mEndFieldName = temporalNode.attribute( QStringLiteral( "endField" ) );

  QDomNode rangeElement = temporalNode.namedItem( QStringLiteral( "fixedRange" ) );

  QDomNode begin = rangeElement.namedItem( QStringLiteral( "start" ) );
  QDomNode end = rangeElement.namedItem( QStringLiteral( "end" ) );

  QDateTime beginDate = QDateTime::fromString( begin.toElement().text(), Qt::ISODate );
  QDateTime endDate = QDateTime::fromString( end.toElement().text(), Qt::ISODate );

  QgsDateTimeRange range = QgsDateTimeRange( beginDate, endDate );
  setFixedTemporalRange( range );

  return true;
}

QDomElement QgsVectorLayerTemporalProperties::writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )
  if ( element.isNull() )
    return QDomElement();

  QDomElement temporalElement = document.createElement( QStringLiteral( "temporal" ) );
  temporalElement.setAttribute( QStringLiteral( "enabled" ), isActive() ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  temporalElement.setAttribute( QStringLiteral( "mode" ), QString::number( mMode ) );

  temporalElement.setAttribute( QStringLiteral( "startField" ), mStartFieldName );
  temporalElement.setAttribute( QStringLiteral( "endField" ), mEndFieldName );

  QDomElement rangeElement = document.createElement( QStringLiteral( "fixedRange" ) );

  QDomElement startElement = document.createElement( QStringLiteral( "start" ) );
  QDomElement endElement = document.createElement( QStringLiteral( "end" ) );

  QDomText startText = document.createTextNode( mFixedRange.begin().toTimeSpec( Qt::OffsetFromUTC ).toString( Qt::ISODate ) );
  QDomText endText = document.createTextNode( mFixedRange.end().toTimeSpec( Qt::OffsetFromUTC ).toString( Qt::ISODate ) );
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
      case QgsVectorDataProviderTemporalCapabilities::ProviderHasFixedTemporalRange:
        setMode( ModeFixedTemporalRange );
        break;
      case QgsVectorDataProviderTemporalCapabilities::ProviderStoresFeatureDateTimeInstantInField:
        setMode( ModeFeatureDateTimeInstantFromField );
        break;
      case QgsVectorDataProviderTemporalCapabilities::ProviderStoresFeatureDateTimeStartAndEndInSeparateFields:
        setMode( ModeFeatureDateTimeStartAndEndFromFields );
        break;
    }
  }
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

QString dateTimeExpressionLiteral( const QDateTime &datetime )
{
  return QStringLiteral( "make_datetime(%1,%2,%3,%4,%5,%6)" ).arg( datetime.date().year() )
         .arg( datetime.date().month() )
         .arg( datetime.date().day() )
         .arg( datetime.time().hour() )
         .arg( datetime.time().minute() )
         .arg( datetime.time().second() + datetime.time().msec() / 1000.0 );
}

QString QgsVectorLayerTemporalProperties::createFilterString( QgsVectorLayer *, const QgsDateTimeRange &range ) const
{
  if ( !isActive() )
    return QString();

  switch ( mMode )
  {
    case ModeFixedTemporalRange:
    case ModeRedrawLayerOnly:
      return QString();

    case ModeFeatureDateTimeInstantFromField:
      return QStringLiteral( "(%1 %2 %3 AND %1 %4 %5) OR %1 IS NULL" ).arg( QgsExpression::quotedColumnRef( mStartFieldName ),
             range.includeBeginning() ? QStringLiteral( ">=" ) : QStringLiteral( ">" ),
             dateTimeExpressionLiteral( range.begin() ),
             range.includeEnd() ? QStringLiteral( "<=" ) : QStringLiteral( "<" ),
             dateTimeExpressionLiteral( range.end() ) );

    case ModeFeatureDateTimeStartAndEndFromFields:
    {
      if ( !mStartFieldName.isEmpty() && !mEndFieldName.isEmpty() )
      {
        return QStringLiteral( "(%1 %2 %3 OR %1 IS NULL) AND (%4 %5 %6 OR %4 IS NULL)" ).arg( QgsExpression::quotedColumnRef( mStartFieldName ),
               range.includeEnd() ? QStringLiteral( "<=" ) : QStringLiteral( "<" ),
               dateTimeExpressionLiteral( range.end() ),
               QgsExpression::quotedColumnRef( mEndFieldName ),
               range.includeBeginning() ? QStringLiteral( ">=" ) : QStringLiteral( ">" ),
               dateTimeExpressionLiteral( range.begin() ) );
      }
      else if ( !mStartFieldName.isEmpty() )
      {
        return QStringLiteral( "%1 %2 %3 OR %1 IS NULL" ).arg( QgsExpression::quotedColumnRef( mStartFieldName ),
               range.includeBeginning() ? QStringLiteral( "<=" ) : QStringLiteral( "<" ),
               dateTimeExpressionLiteral( range.end() ) );
      }
      else if ( !mEndFieldName.isEmpty() )
      {
        return QStringLiteral( "%1 %2 %3 OR %1 IS NULL" ).arg( QgsExpression::quotedColumnRef( mEndFieldName ),
               range.includeBeginning() ? QStringLiteral( ">=" ) : QStringLiteral( ">" ),
               dateTimeExpressionLiteral( range.begin() ) );
      }
      break;
    }
  }

  return QString();
}
