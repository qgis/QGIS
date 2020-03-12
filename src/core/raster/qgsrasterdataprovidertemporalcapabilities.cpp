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

void QgsRasterDataProviderTemporalCapabilities::setEnableTime( bool enabled )
{
  mEnableTime = enabled;
}

bool QgsRasterDataProviderTemporalCapabilities::isTimeEnabled() const
{
  return mEnableTime;
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
  if ( mAvailableTemporalRange.contains( dateTimeRange ) )
    mRequestedRange = dateTimeRange;
}

const QgsDateTimeRange &QgsRasterDataProviderTemporalCapabilities::requestedTemporalRange() const
{
  return mRequestedRange;
}

void QgsRasterDataProviderTemporalCapabilities::setRequestedReferenceTemporalRange( const QgsDateTimeRange &dateTimeRange )
{
  if ( mAvailableReferenceRange.contains( dateTimeRange ) )
    mRequestedReferenceRange = dateTimeRange;
}

const QgsDateTimeRange &QgsRasterDataProviderTemporalCapabilities::requestedReferenceTemporalRange() const
{
  return mRequestedReferenceRange;
}

void QgsRasterDataProviderTemporalCapabilities::setReferenceEnable( bool enabled )
{
  mReferenceEnable = enabled;
}

bool QgsRasterDataProviderTemporalCapabilities::isReferenceEnable() const
{
  return mReferenceEnable;
}

QgsRasterDataProviderTemporalCapabilities::IntervalHandlingMethod QgsRasterDataProviderTemporalCapabilities::intervalHandlingMethod() const
{
  return mIntervalMatchMethod;
}

void QgsRasterDataProviderTemporalCapabilities::setIntervalHandlingMethod( IntervalHandlingMethod mode )
{
  if ( mIntervalMatchMethod == mode )
    return;
  mIntervalMatchMethod = mode;
}
