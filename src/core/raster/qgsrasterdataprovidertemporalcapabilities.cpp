/***************************************************************************
                         qgsrasterdataprovidertemporalcapabilities.cpp
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

#include "qgsrasterdataprovidertemporalcapabilities.h"

QgsRasterDataProviderTemporalCapabilities::QgsRasterDataProviderTemporalCapabilities( bool enabled )
  :  QgsDataProviderTemporalCapabilities( enabled )
{
}

void QgsRasterDataProviderTemporalCapabilities::setAvailableTemporalRange( const QgsDateTimeRange &dateTimeRange )
{
  if ( !hasTemporalCapabilities() )
    setHasTemporalCapabilities( true );

  mAvailableTemporalRange = dateTimeRange;
}

const QgsDateTimeRange &QgsRasterDataProviderTemporalCapabilities::availableTemporalRange() const
{
  return mAvailableTemporalRange;
}

void QgsRasterDataProviderTemporalCapabilities::setAllAvailableTemporalRanges( const QList<QgsDateTimeRange> &ranges )
{
  mAllAvailableTemporalRanges = ranges;
}

QList<QgsDateTimeRange> QgsRasterDataProviderTemporalCapabilities::allAvailableTemporalRanges() const
{
  return mAllAvailableTemporalRanges;
}

void QgsRasterDataProviderTemporalCapabilities::setAvailableReferenceTemporalRange( const QgsDateTimeRange &dateTimeRange )
{
  if ( !hasTemporalCapabilities() )
    setHasTemporalCapabilities( true );

  mAvailableReferenceRange = dateTimeRange;
}

const QgsDateTimeRange &QgsRasterDataProviderTemporalCapabilities::availableReferenceTemporalRange() const
{
  return mAvailableReferenceRange;
}

void QgsRasterDataProviderTemporalCapabilities::setRequestedTemporalRange( const QgsDateTimeRange &dateTimeRange )
{
  mRequestedRange = dateTimeRange;
}

QgsInterval QgsRasterDataProviderTemporalCapabilities::defaultInterval() const
{
  return mDefaultInterval;
}

void QgsRasterDataProviderTemporalCapabilities::setDefaultInterval( const QgsInterval &defaultInterval )
{
  mDefaultInterval = defaultInterval;
}

const QgsDateTimeRange &QgsRasterDataProviderTemporalCapabilities::requestedTemporalRange() const
{
  return mRequestedRange;
}

Qgis::TemporalIntervalMatchMethod QgsRasterDataProviderTemporalCapabilities::intervalHandlingMethod() const
{
  return mIntervalMatchMethod;
}

void QgsRasterDataProviderTemporalCapabilities::setIntervalHandlingMethod( Qgis::TemporalIntervalMatchMethod mode )
{
  if ( mIntervalMatchMethod == mode )
    return;
  mIntervalMatchMethod = mode;
}
