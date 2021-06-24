/***************************************************************************
                         qgsrasterlayertemporalproperties.cpp
                         ---------------
    begin                : February 2020
    copyright            : (C) 2020 by Samweli Mwakisambwe
    email                : samweli at kartoza dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrasterlayertemporalproperties.h"
#include "qgsrasterdataprovidertemporalcapabilities.h"
#include "qgsrasterlayer.h"

QgsRasterLayerTemporalProperties::QgsRasterLayerTemporalProperties( QObject *parent, bool enabled )
  :  QgsMapLayerTemporalProperties( parent, enabled )
{
}

bool QgsRasterLayerTemporalProperties::isVisibleInTemporalRange( const QgsDateTimeRange &range ) const
{
  if ( !isActive() )
    return true;

  switch ( mMode )
  {
    case ModeFixedTemporalRange:
      return range.isInfinite() || mFixedRange.isInfinite() || mFixedRange.overlaps( range );

    case ModeTemporalRangeFromDataProvider:
    case ModeRedrawLayerOnly:
      return true;
  }
  return true;
}

QgsDateTimeRange QgsRasterLayerTemporalProperties::calculateTemporalExtent( QgsMapLayer *layer ) const
{
  QgsRasterLayer *rasterLayer = qobject_cast< QgsRasterLayer *>( layer );
  if ( !rasterLayer )
    return QgsDateTimeRange();

  switch ( mMode )
  {
    case QgsRasterLayerTemporalProperties::ModeFixedTemporalRange:
      return mFixedRange;

    case QgsRasterLayerTemporalProperties::ModeTemporalRangeFromDataProvider:
      return rasterLayer->dataProvider()->temporalCapabilities()->availableTemporalRange();

    case QgsRasterLayerTemporalProperties::ModeRedrawLayerOnly:
      break;
  }

  return QgsDateTimeRange();
}

QList<QgsDateTimeRange> QgsRasterLayerTemporalProperties::allTemporalRanges( QgsMapLayer *layer ) const
{
  QgsRasterLayer *rasterLayer = qobject_cast< QgsRasterLayer *>( layer );
  if ( !rasterLayer )
    return {};

  switch ( mMode )
  {
    case QgsRasterLayerTemporalProperties::ModeFixedTemporalRange:
      return { mFixedRange };

    case QgsRasterLayerTemporalProperties::ModeTemporalRangeFromDataProvider:
    {
      QList< QgsDateTimeRange > ranges = rasterLayer->dataProvider()->temporalCapabilities()->allAvailableTemporalRanges();
      return ranges.empty() ? QList< QgsDateTimeRange > { rasterLayer->dataProvider()->temporalCapabilities()->availableTemporalRange() } : ranges;
    }

    case QgsRasterLayerTemporalProperties::ModeRedrawLayerOnly:
      break;
  }

  return {};
}

QgsRasterLayerTemporalProperties::TemporalMode QgsRasterLayerTemporalProperties::mode() const
{
  return mMode;
}

void QgsRasterLayerTemporalProperties::setMode( QgsRasterLayerTemporalProperties::TemporalMode mode )
{
  if ( mMode == mode )
    return;
  mMode = mode;
}

QgsTemporalProperty::Flags QgsRasterLayerTemporalProperties::flags() const
{
  return mode() == ModeFixedTemporalRange ? QgsTemporalProperty::FlagDontInvalidateCachedRendersWhenRangeChanges : QgsTemporalProperty::Flags();
}

QgsRasterDataProviderTemporalCapabilities::IntervalHandlingMethod QgsRasterLayerTemporalProperties::intervalHandlingMethod() const
{
  return mIntervalHandlingMethod;
}

void QgsRasterLayerTemporalProperties::setIntervalHandlingMethod( QgsRasterDataProviderTemporalCapabilities::IntervalHandlingMethod method )
{
  if ( mIntervalHandlingMethod == method )
    return;
  mIntervalHandlingMethod = method;
}

void  QgsRasterLayerTemporalProperties::setFixedTemporalRange( const QgsDateTimeRange &range )
{
  mFixedRange = range;
}

const QgsDateTimeRange &QgsRasterLayerTemporalProperties::fixedTemporalRange() const
{
  return mFixedRange;
}

bool QgsRasterLayerTemporalProperties::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )
  // TODO add support for raster layers with multi-temporal properties.

  QDomElement temporalNode = element.firstChildElement( QStringLiteral( "temporal" ) );

  setIsActive( temporalNode.attribute( QStringLiteral( "enabled" ), QStringLiteral( "0" ) ).toInt() );

  mMode = static_cast< TemporalMode >( temporalNode.attribute( QStringLiteral( "mode" ), QStringLiteral( "0" ) ). toInt() );
  mIntervalHandlingMethod = static_cast< QgsRasterDataProviderTemporalCapabilities::IntervalHandlingMethod >( temporalNode.attribute( QStringLiteral( "fetchMode" ), QStringLiteral( "0" ) ). toInt() );

  QDomNode rangeElement = temporalNode.namedItem( QStringLiteral( "fixedRange" ) );

  QDomNode begin = rangeElement.namedItem( QStringLiteral( "start" ) );
  QDomNode end = rangeElement.namedItem( QStringLiteral( "end" ) );

  QDateTime beginDate = QDateTime::fromString( begin.toElement().text(), Qt::ISODate );
  QDateTime endDate = QDateTime::fromString( end.toElement().text(), Qt::ISODate );

  QgsDateTimeRange range = QgsDateTimeRange( beginDate, endDate );
  setFixedTemporalRange( range );

  return true;
}

QDomElement QgsRasterLayerTemporalProperties::writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )
  if ( element.isNull() )
    return QDomElement();

  QDomElement temporalElement = document.createElement( QStringLiteral( "temporal" ) );
  temporalElement.setAttribute( QStringLiteral( "enabled" ), isActive() ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  temporalElement.setAttribute( QStringLiteral( "mode" ), QString::number( mMode ) );
  temporalElement.setAttribute( QStringLiteral( "fetchMode" ), QString::number( mIntervalHandlingMethod ) );

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

void QgsRasterLayerTemporalProperties::setDefaultsFromDataProviderTemporalCapabilities( const QgsDataProviderTemporalCapabilities *capabilities )
{
  if ( const QgsRasterDataProviderTemporalCapabilities *rasterCaps = dynamic_cast< const QgsRasterDataProviderTemporalCapabilities *>( capabilities ) )
  {
    setIsActive( rasterCaps->hasTemporalCapabilities() );
    setFixedTemporalRange( rasterCaps->availableTemporalRange() );

    if ( rasterCaps->hasTemporalCapabilities() )
    {
      setMode( ModeTemporalRangeFromDataProvider );
    }

    mIntervalHandlingMethod = rasterCaps->intervalHandlingMethod();
  }
}
