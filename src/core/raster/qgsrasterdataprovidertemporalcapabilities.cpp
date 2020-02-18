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

//QgsRasterDataProviderTemporalProperties &QgsRasterDataProviderTemporalProperties::operator=( const QgsRasterDataProviderTemporalProperties &other )
//{
//  mRange = other.mRange;
//  mFixedRange = other.mFixedRange;
//  mEnableTime = other.mEnableTime;

//  return *this;
//}

void QgsRasterDataProviderTemporalCapabilities::setTemporalRange( const QgsDateTimeRange &dateTimeRange )
{
  if ( !isActive() )
    setIsActive( true );

  mRange = dateTimeRange;

}

const QgsDateTimeRange &QgsRasterDataProviderTemporalCapabilities::temporalRange() const
{
  return mRange;
}

void QgsRasterDataProviderTemporalCapabilities::setFixedTemporalRange( const QgsDateTimeRange &dateTimeRange )
{
  if ( !isActive() )
    setIsActive( true );

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

void QgsRasterDataProviderTemporalCapabilities::setReferenceTemporalRange( const QgsDateTimeRange &dateTimeRange )
{
  if ( !isActive() )
    setIsActive( true );

  mReferenceRange = dateTimeRange;

}

const QgsDateTimeRange &QgsRasterDataProviderTemporalCapabilities::referenceTemporalRange() const
{
  return mReferenceRange;
}

void QgsRasterDataProviderTemporalCapabilities::setFixedReferenceTemporalRange( const QgsDateTimeRange &dateTimeRange )
{
  if ( !isActive() )
    setIsActive( true );

  mFixedReferenceRange = dateTimeRange;

}

const QgsDateTimeRange &QgsRasterDataProviderTemporalCapabilities::fixedReferenceTemporalRange() const
{
  return mFixedReferenceRange;
}


void QgsRasterDataProviderTemporalCapabilities::setHasReference( bool enabled )
{
  mHasReferenceRange = enabled;
}

bool QgsRasterDataProviderTemporalCapabilities::hasReference() const
{
  return mHasReferenceRange;
}

void QgsRasterDataProviderTemporalCapabilities::setReferenceEnable( bool enabled )
{
  mReferenceEnable = enabled;
}

bool QgsRasterDataProviderTemporalCapabilities::isReferenceEnable() const
{
  return mReferenceEnable;
}

