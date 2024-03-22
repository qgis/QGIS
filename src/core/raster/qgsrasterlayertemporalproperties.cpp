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
    case Qgis::RasterTemporalMode::FixedTemporalRange:
      return range.isInfinite() || mFixedRange.isInfinite() || mFixedRange.overlaps( range );

    case Qgis::RasterTemporalMode::FixedRangePerBand:
    {
      for ( auto it = mRangePerBand.constBegin(); it != mRangePerBand.constEnd(); ++it )
      {
        if ( it.value().overlaps( range ) )
          return true;
      }
      return false;
    }

    case Qgis::RasterTemporalMode::TemporalRangeFromDataProvider:
    case Qgis::RasterTemporalMode::RedrawLayerOnly:
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
    case Qgis::RasterTemporalMode::FixedTemporalRange:
      return mFixedRange;

    case Qgis::RasterTemporalMode::TemporalRangeFromDataProvider:
      return rasterLayer->dataProvider()->temporalCapabilities()->availableTemporalRange();

    case Qgis::RasterTemporalMode::FixedRangePerBand:
    {
      QDateTime begin;
      QDateTime end;
      bool includeBeginning = true;
      bool includeEnd = true;
      for ( auto it = mRangePerBand.constBegin(); it != mRangePerBand.constEnd(); ++it )
      {
        if ( it.value().begin() < begin || !begin.isValid() )
        {
          begin = it.value().begin();
          includeBeginning = it.value().includeBeginning();
        }
        else if ( !includeBeginning && it.value().begin() == begin && it.value().includeBeginning() )
        {
          includeBeginning = true;
        }
        if ( it.value().end() > end || !end.isValid() )
        {
          end = it.value().end();
          includeEnd = it.value().includeEnd();
        }
        else if ( !includeEnd && it.value().end() == end && it.value().includeEnd() )
        {
          includeEnd = true;
        }
      }
      return QgsDateTimeRange( begin, end, includeBeginning, includeEnd );
    }

    case Qgis::RasterTemporalMode::RedrawLayerOnly:
      break;
  }

  return QgsDateTimeRange();
}

QList<QgsDateTimeRange> QgsRasterLayerTemporalProperties::allTemporalRanges( QgsMapLayer *layer ) const
{
  QgsRasterLayer *rasterLayer = qobject_cast< QgsRasterLayer *>( layer );

  switch ( mMode )
  {
    case Qgis::RasterTemporalMode::FixedTemporalRange:
      return { mFixedRange };

    case Qgis::RasterTemporalMode::FixedRangePerBand:
    {
      QList<QgsDateTimeRange> results;
      results.reserve( mRangePerBand.size() );
      for ( auto it = mRangePerBand.constBegin(); it != mRangePerBand.constEnd(); ++it )
      {
        results.append( it.value() );
      }
      return results;
    }

    case Qgis::RasterTemporalMode::TemporalRangeFromDataProvider:
    {
      if ( !rasterLayer || !rasterLayer->dataProvider() )
        return {};

      const QList< QgsDateTimeRange > ranges = rasterLayer->dataProvider()->temporalCapabilities()->allAvailableTemporalRanges();
      return ranges.empty() ? QList< QgsDateTimeRange > { rasterLayer->dataProvider()->temporalCapabilities()->availableTemporalRange() } : ranges;
    }

    case Qgis::RasterTemporalMode::RedrawLayerOnly:
      break;
  }

  return {};
}

Qgis::RasterTemporalMode QgsRasterLayerTemporalProperties::mode() const
{
  return mMode;
}

void QgsRasterLayerTemporalProperties::setMode( Qgis::RasterTemporalMode mode )
{
  if ( mMode == mode )
    return;
  mMode = mode;
}

QgsTemporalProperty::Flags QgsRasterLayerTemporalProperties::flags() const
{
  switch ( mMode )
  {
    case Qgis::RasterTemporalMode::FixedTemporalRange:
      return QgsTemporalProperty::FlagDontInvalidateCachedRendersWhenRangeChanges;

    case Qgis::RasterTemporalMode::TemporalRangeFromDataProvider:
    case Qgis::RasterTemporalMode::RedrawLayerOnly:
    case Qgis::RasterTemporalMode::FixedRangePerBand:
      return QgsTemporalProperty::Flags();
  }
  BUILTIN_UNREACHABLE
}

Qgis::TemporalIntervalMatchMethod QgsRasterLayerTemporalProperties::intervalHandlingMethod() const
{
  return mIntervalHandlingMethod;
}

void QgsRasterLayerTemporalProperties::setIntervalHandlingMethod( Qgis::TemporalIntervalMatchMethod method )
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

QMap<int, QgsDateTimeRange> QgsRasterLayerTemporalProperties::fixedRangePerBand() const
{
  return mRangePerBand;
}

void QgsRasterLayerTemporalProperties::setFixedRangePerBand( const QMap<int, QgsDateTimeRange> &ranges )
{
  if ( mRangePerBand == ranges )
    return;

  mRangePerBand = ranges;
  emit changed();
}

int QgsRasterLayerTemporalProperties::bandForTemporalRange( QgsRasterLayer *, const QgsDateTimeRange &range ) const
{
  switch ( mMode )
  {
    case Qgis::RasterTemporalMode::FixedTemporalRange:
    case Qgis::RasterTemporalMode::TemporalRangeFromDataProvider:
    case Qgis::RasterTemporalMode::RedrawLayerOnly:
      return -1;

    case Qgis::RasterTemporalMode::FixedRangePerBand:
    {
      // find the latest-most band which matches the map range
      int currentMatchingBand = -1;
      QgsDateTimeRange currentMatchingRange;
      for ( auto it = mRangePerBand.constBegin(); it != mRangePerBand.constEnd(); ++it )
      {
        if ( it.value().overlaps( range ) )
        {
          if ( currentMatchingRange.isInfinite()
               || ( it.value().includeEnd() && it.value().end() >= currentMatchingRange.end() ) // cppcheck-suppress mismatchingContainerExpression
               || ( !currentMatchingRange.includeEnd() && it.value().end() >= currentMatchingRange.end() ) ) // cppcheck-suppress mismatchingContainerExpression
          {
            currentMatchingBand = it.key();
            currentMatchingRange = it.value();
          }
        }
      }
      return currentMatchingBand;
    }
  }
  BUILTIN_UNREACHABLE
}

QList<int> QgsRasterLayerTemporalProperties::filteredBandsForTemporalRange( QgsRasterLayer *layer, const QgsDateTimeRange &range ) const
{
  switch ( mMode )
  {
    case Qgis::RasterTemporalMode::FixedTemporalRange:
    case Qgis::RasterTemporalMode::TemporalRangeFromDataProvider:
    case Qgis::RasterTemporalMode::RedrawLayerOnly:
    {
      const int bandCount = layer->bandCount();
      QList< int > res;
      res.reserve( bandCount );
      for ( int i = 1; i <= bandCount; ++i )
        res.append( i );
      return res;
    }

    case Qgis::RasterTemporalMode::FixedRangePerBand:
    {
      QList<int> res;
      res.reserve( mRangePerBand.size() );
      // find the latest-most band which matches the map range
      QgsDateTimeRange currentMatchingRange;
      for ( auto it = mRangePerBand.constBegin(); it != mRangePerBand.constEnd(); ++it )
      {
        if ( it.value().overlaps( range ) )
        {
          res.append( it.key() );
        }
      }
      return res;
    }
  }
  BUILTIN_UNREACHABLE
}

bool QgsRasterLayerTemporalProperties::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )
  // TODO add support for raster layers with multi-temporal properties.

  const QDomElement temporalNode = element.firstChildElement( QStringLiteral( "temporal" ) );

  setIsActive( temporalNode.attribute( QStringLiteral( "enabled" ), QStringLiteral( "0" ) ).toInt() );

  mMode = static_cast< Qgis::RasterTemporalMode >( temporalNode.attribute( QStringLiteral( "mode" ), QStringLiteral( "0" ) ). toInt() );
  mIntervalHandlingMethod = static_cast< Qgis::TemporalIntervalMatchMethod >( temporalNode.attribute( QStringLiteral( "fetchMode" ), QStringLiteral( "0" ) ). toInt() );

  switch ( mMode )
  {
    case Qgis::RasterTemporalMode::FixedTemporalRange:
    {
      const QDomNode rangeElement = temporalNode.namedItem( QStringLiteral( "fixedRange" ) );

      const QDomNode begin = rangeElement.namedItem( QStringLiteral( "start" ) );
      const QDomNode end = rangeElement.namedItem( QStringLiteral( "end" ) );

      const QDateTime beginDate = QDateTime::fromString( begin.toElement().text(), Qt::ISODate );
      const QDateTime endDate = QDateTime::fromString( end.toElement().text(), Qt::ISODate );

      const QgsDateTimeRange range = QgsDateTimeRange( beginDate, endDate );
      setFixedTemporalRange( range );
      break;
    }

    case Qgis::RasterTemporalMode::FixedRangePerBand:
    {
      mRangePerBand.clear();

      const QDomNodeList ranges = temporalNode.firstChildElement( QStringLiteral( "ranges" ) ).childNodes();
      for ( int i = 0; i < ranges.size(); ++i )
      {
        const QDomElement rangeElement = ranges.at( i ).toElement();
        const int band = rangeElement.attribute( QStringLiteral( "band" ) ).toInt();
        const QDateTime begin = QDateTime::fromString( rangeElement.attribute( QStringLiteral( "begin" ) ), Qt::ISODate );
        const QDateTime end = QDateTime::fromString( rangeElement.attribute( QStringLiteral( "end" ) ), Qt::ISODate );
        const bool includeBeginning = rangeElement.attribute( QStringLiteral( "includeBeginning" ) ).toInt();
        const bool includeEnd = rangeElement.attribute( QStringLiteral( "includeEnd" ) ).toInt();
        mRangePerBand.insert( band, QgsDateTimeRange( begin, end, includeBeginning, includeEnd ) );
      }
      break;
    }

    case Qgis::RasterTemporalMode::TemporalRangeFromDataProvider:
    case Qgis::RasterTemporalMode::RedrawLayerOnly:
      break;
  }

  return true;
}

QDomElement QgsRasterLayerTemporalProperties::writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )
  if ( element.isNull() )
    return QDomElement();

  QDomElement temporalElement = document.createElement( QStringLiteral( "temporal" ) );
  temporalElement.setAttribute( QStringLiteral( "enabled" ), isActive() ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  temporalElement.setAttribute( QStringLiteral( "mode" ), QString::number( static_cast< int >( mMode ) ) );
  temporalElement.setAttribute( QStringLiteral( "fetchMode" ), QString::number( static_cast< int >( mIntervalHandlingMethod ) ) );

  switch ( mMode )
  {
    case Qgis::RasterTemporalMode::FixedTemporalRange:
    {

      QDomElement rangeElement = document.createElement( QStringLiteral( "fixedRange" ) );

      QDomElement startElement = document.createElement( QStringLiteral( "start" ) );
      QDomElement endElement = document.createElement( QStringLiteral( "end" ) );

      const QDomText startText = document.createTextNode( mFixedRange.begin().toTimeSpec( Qt::OffsetFromUTC ).toString( Qt::ISODate ) );
      const QDomText endText = document.createTextNode( mFixedRange.end().toTimeSpec( Qt::OffsetFromUTC ).toString( Qt::ISODate ) );
      startElement.appendChild( startText );
      endElement.appendChild( endText );
      rangeElement.appendChild( startElement );
      rangeElement.appendChild( endElement );

      temporalElement.appendChild( rangeElement );
      break;
    }

    case Qgis::RasterTemporalMode::FixedRangePerBand:
    {
      QDomElement ranges = document.createElement( QStringLiteral( "ranges" ) );
      for ( auto it = mRangePerBand.constBegin(); it != mRangePerBand.constEnd(); ++it )
      {
        QDomElement range = document.createElement( QStringLiteral( "range" ) );
        range.setAttribute( QStringLiteral( "band" ), it.key() );
        range.setAttribute( QStringLiteral( "begin" ), it.value().begin().toString( Qt::ISODate ) );
        range.setAttribute( QStringLiteral( "end" ), it.value().end().toString( Qt::ISODate ) );
        range.setAttribute( QStringLiteral( "includeBeginning" ), it.value().includeBeginning() ? "1" : "0" );
        range.setAttribute( QStringLiteral( "includeEnd" ), it.value().includeEnd() ? "1" : "0" );
        ranges.appendChild( range );
      }
      temporalElement.appendChild( ranges );
      break;
    }

    case Qgis::RasterTemporalMode::RedrawLayerOnly:
    case Qgis::RasterTemporalMode::TemporalRangeFromDataProvider:
      break;
  }

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
      setMode( Qgis::RasterTemporalMode::TemporalRangeFromDataProvider );
    }

    mIntervalHandlingMethod = rasterCaps->intervalHandlingMethod();
  }
}
