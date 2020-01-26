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

QgsRasterLayerTemporalProperties::QgsRasterLayerTemporalProperties( bool enabled )
  :  QgsMapLayerTemporalProperties( enabled )
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

void  QgsRasterLayerTemporalProperties::setFixedTemporalRange( const QgsDateTimeRange &range )
{
  if ( range == mRange )
    return;

  mRange = range;
}

const QgsDateTimeRange &QgsRasterLayerTemporalProperties::fixedTemporalRange() const
{
  return mRange;
}

//void  QgsRasterLayerTemporalProperties::setProviderTemporalRange( const QgsDateTimeRange &range )
//{
//  if ( range == mRange )
//    return;

//  QgsWmsProvider *wmsProvider = qobject_cast<QgsWmsProvider*> (mDataProvider);

//  if ( wmsProvider)
//      wmsProvider->setTemporalRange( range );

//  mRange = range;

//}

//void QgsRasterLayerTemporalProperties::setDataProvider( QgsRasterDataProvider *provider)
//{
//    mDataProvider = provider;
//}

void  QgsRasterLayerTemporalProperties::setWmstRelatedSettings( const QString &dimension )
{
  Q_UNUSED( dimension )

  // TODO add WMS-T handling here,
  // For WMS-T instant time values, WMS-T List times and for WMS-T intervals time values.
}

bool QgsRasterLayerTemporalProperties::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )
  // TODO add support for raster layers with multi-temporal properties.

  QDomNode temporalNode = element.elementsByTagName( QStringLiteral( "temporal" ) ).at( 0 );

  TemporalMode mode = indexToMode( temporalNode.toElement().attribute( QStringLiteral( "mode" ), QStringLiteral( "0" ) ). toInt() );
  setMode( mode );

  QDomNode rangeElement = temporalNode.namedItem( QStringLiteral( "range" ) );

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
  Q_UNUSED( context );
  if ( element.isNull() )
    return QDomElement();

  QDomElement temporalElement = document.createElement( QStringLiteral( "temporal" ) );
  temporalElement.setAttribute( QStringLiteral( "mode" ), QString::number( mMode ) );

  QDomElement rangeElement = document.createElement( QStringLiteral( "range" ) );

  QDomElement startElement = document.createElement( QStringLiteral( "start" ) );
  QDomElement endElement = document.createElement( QStringLiteral( "end" ) );

  QDomText startText = document.createTextNode( mRange.begin().toTimeSpec( Qt::OffsetFromUTC ).toString( Qt::ISODate ) );
  QDomText endText = document.createTextNode( mRange.end().toTimeSpec( Qt::OffsetFromUTC ).toString( Qt::ISODate ) );

  startElement.appendChild( startText );
  endElement.appendChild( endText );

  rangeElement.appendChild( startElement );
  rangeElement.appendChild( endElement );

  temporalElement.appendChild( rangeElement );

  element.appendChild( temporalElement );

  return element;
}

QgsRasterLayerTemporalProperties::TemporalMode QgsRasterLayerTemporalProperties::indexToMode( int number )
{
  switch ( number )
  {
    case 0:
      return TemporalMode::ModeFixedTemporalRange;
    case 1:
      return TemporalMode::ModeTemporalRangeFromDataProvider;
    case 2:
      return TemporalMode::ModeTemporalRangesList;
    default:
      return TemporalMode::ModeFixedTemporalRange;
  }
}
