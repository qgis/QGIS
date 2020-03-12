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

QgsRasterLayerTemporalProperties::QgsRasterLayerTemporalProperties( QObject *parent, bool enabled )
  :  QgsMapLayerTemporalProperties( parent, enabled )
{
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

void  QgsRasterLayerTemporalProperties::setFixedReferenceTemporalRange( const QgsDateTimeRange &range )
{
  mFixedReferenceRange = range;
}

const QgsDateTimeRange &QgsRasterLayerTemporalProperties::fixedReferenceTemporalRange() const
{
  return mFixedReferenceRange;
}

void QgsRasterLayerTemporalProperties::setTemporalRange( const QgsDateTimeRange &dateTimeRange )
{
  // Don't set temporal range outside fixed temporal range limits,
  // instead set equal to the fixed temporal range

  if ( !isActive() )
    setIsActive( true );

  if ( mFixedRange.contains( dateTimeRange ) )
    mRange = dateTimeRange;
  else
    mRange = mFixedRange;
}

const QgsDateTimeRange &QgsRasterLayerTemporalProperties::temporalRange() const
{
  return mRange;
}

void QgsRasterLayerTemporalProperties::setReferenceTemporalRange( const QgsDateTimeRange &dateTimeRange )
{
  if ( mFixedReferenceRange.contains( dateTimeRange ) )
    mReferenceRange = dateTimeRange;
}

const QgsDateTimeRange &QgsRasterLayerTemporalProperties::referenceTemporalRange() const
{
  return mReferenceRange;
}

bool QgsRasterLayerTemporalProperties::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )
  // TODO add support for raster layers with multi-temporal properties.

  QDomElement temporalNode = element.firstChildElement( QStringLiteral( "temporal" ) );

  mMode = static_cast< TemporalMode >( temporalNode.attribute( QStringLiteral( "mode" ), QStringLiteral( "0" ) ). toInt() );
  mIntervalHandlingMethod = static_cast< QgsRasterDataProviderTemporalCapabilities::IntervalHandlingMethod >( temporalNode.attribute( QStringLiteral( "fetchMode" ), QStringLiteral( "0" ) ). toInt() );

  int sourceIndex = temporalNode.attribute( QStringLiteral( "source" ), QStringLiteral( "0" ) ).toInt();

  if ( sourceIndex == 0 )
    setTemporalSource( TemporalSource::Layer );
  else
    setTemporalSource( TemporalSource::Project );

  for ( QString rangeString : { "fixedRange", "fixedReferenceRange", "normalRange", "referenceRange" } )
  {
    QDomNode rangeElement = temporalNode.namedItem( rangeString );

    QDomNode begin = rangeElement.namedItem( QStringLiteral( "start" ) );
    QDomNode end = rangeElement.namedItem( QStringLiteral( "end" ) );

    QDateTime beginDate = QDateTime::fromString( begin.toElement().text(), Qt::ISODate );
    QDateTime endDate = QDateTime::fromString( end.toElement().text(), Qt::ISODate );

    QgsDateTimeRange range = QgsDateTimeRange( beginDate, endDate );

    if ( rangeString == QLatin1String( "fixedRange" ) )
      setFixedTemporalRange( range );
    if ( rangeString == QLatin1String( "normalRange" ) )
      setTemporalRange( range );
    if ( rangeString == QLatin1String( "fixedReferenceRange" ) )
      setFixedReferenceTemporalRange( range );
    if ( rangeString == QLatin1String( "referenceRange" ) )
      setReferenceTemporalRange( range );
  }
  return true;
}

QDomElement QgsRasterLayerTemporalProperties::writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )
  if ( element.isNull() )
    return QDomElement();

  QDomElement temporalElement = document.createElement( QStringLiteral( "temporal" ) );
  temporalElement.setAttribute( QStringLiteral( "mode" ), QString::number( mMode ) );
  temporalElement.setAttribute( QStringLiteral( "source" ), QString::number( temporalSource() ) );
  temporalElement.setAttribute( QStringLiteral( "fetchMode" ), QString::number( mIntervalHandlingMethod ) );

  for ( QString rangeString : { "fixedRange", "fixedReferenceRange", "normalRange", "referenceRange" } )
  {
    QgsDateTimeRange range;

    if ( rangeString == QLatin1String( "fixedRange" ) )
      range = mFixedRange;
    if ( rangeString == QLatin1String( "fixedReferenceRange" ) )
      range = mFixedReferenceRange;
    if ( rangeString == QLatin1String( "normalRange" ) )
      range = mRange;
    if ( rangeString == QLatin1String( "referenceRange" ) )
      range = mReferenceRange;

    QDomElement rangeElement = document.createElement( rangeString );

    QDomElement startElement = document.createElement( QStringLiteral( "start" ) );
    QDomElement endElement = document.createElement( QStringLiteral( "end" ) );

    QDomText startText = document.createTextNode( range.begin().toTimeSpec( Qt::OffsetFromUTC ).toString( Qt::ISODate ) );
    QDomText endText = document.createTextNode( range.end().toTimeSpec( Qt::OffsetFromUTC ).toString( Qt::ISODate ) );

    startElement.appendChild( startText );
    endElement.appendChild( endText );

    rangeElement.appendChild( startElement );
    rangeElement.appendChild( endElement );

    temporalElement.appendChild( rangeElement );
  }
  element.appendChild( temporalElement );

  return element;
}

void QgsRasterLayerTemporalProperties::setDefaultsFromDataProviderTemporalCapabilities( QgsRasterDataProviderTemporalCapabilities *capabilities )
{
  setFixedTemporalRange( capabilities->fixedTemporalRange() );
  setFixedReferenceTemporalRange( capabilities->fixedReferenceTemporalRange() );

  if ( capabilities->isTimeEnabled() )
  {
    setMode( ModeTemporalRangeFromDataProvider );
  }

  mIntervalHandlingMethod = capabilities->intervalHandlingMethod();
}
