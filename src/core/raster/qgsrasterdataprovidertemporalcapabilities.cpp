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

void QgsRasterDataProviderTemporalCapabilities::setFixedTemporalRange( const QgsDateTimeRange &dateTimeRange )
{
  if ( !hasTemporalCapabilities() )
    setHasTemporalCapabilities( true );

  mFixedRange = dateTimeRange;
}

const QgsDateTimeRange &QgsRasterDataProviderTemporalCapabilities::fixedTemporalRange() const
{
  return mFixedRange;
}

void QgsRasterDataProviderTemporalCapabilities::setEnableTime( bool enabled )
{
  mEnableTime = enabled;
}

bool QgsRasterDataProviderTemporalCapabilities::isTimeEnabled() const
{
  return mEnableTime;
}

void QgsRasterDataProviderTemporalCapabilities::setFixedReferenceTemporalRange( const QgsDateTimeRange &dateTimeRange )
{
  if ( !hasTemporalCapabilities() )
    setHasTemporalCapabilities( true );

  mFixedReferenceRange = dateTimeRange;
}

const QgsDateTimeRange &QgsRasterDataProviderTemporalCapabilities::fixedReferenceTemporalRange() const
{
  return mFixedReferenceRange;
}

void QgsRasterDataProviderTemporalCapabilities::setRequestedTemporalRange( const QgsDateTimeRange &dateTimeRange )
{
  if ( mFixedRange.contains( dateTimeRange ) )
    mRequestedRange = dateTimeRange;
}

const QgsDateTimeRange &QgsRasterDataProviderTemporalCapabilities::requestedTemporalRange() const
{
  return mRequestedRange;
}

void QgsRasterDataProviderTemporalCapabilities::setRequestedReferenceTemporalRange( const QgsDateTimeRange &dateTimeRange )
{
  if ( mFixedReferenceRange.contains( dateTimeRange ) )
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

QgsRasterDataProviderTemporalCapabilities::TemporalMode QgsRasterDataProviderTemporalCapabilities::mode() const
{
  return mMode;
}

void QgsRasterDataProviderTemporalCapabilities::setMode( TemporalMode mode )
{
  if ( mMode == mode )
    return;
  mMode = mode;
}

QgsRasterDataProviderTemporalCapabilities::FetchMode QgsRasterDataProviderTemporalCapabilities::fetchMode() const
{
  return mFetchMode;
}

void QgsRasterDataProviderTemporalCapabilities::setFetchMode( FetchMode mode )
{
  if ( mFetchMode == mode )
    return;
  mFetchMode = mode;
}

QgsRasterDataProviderTemporalCapabilities::TimeInterval QgsRasterDataProviderTemporalCapabilities::timeInterval() const
{
  return mTimeInteval;
}

void QgsRasterDataProviderTemporalCapabilities::setTimeInterval( TimeInterval interval )
{
  if ( mTimeInteval == interval )
    return;
  mTimeInteval = interval;
}
