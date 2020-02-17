/***************************************************************************
                         qgsrasterdataprovidertemporalproperties.cpp
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

#include "qgsrasterdataprovidertemporalproperties.h"

QgsRasterDataProviderTemporalProperties::QgsRasterDataProviderTemporalProperties( bool enabled )
  :  QgsDataProviderTemporalProperties( enabled )
{
}

//QgsRasterDataProviderTemporalProperties &QgsRasterDataProviderTemporalProperties::operator=( const QgsRasterDataProviderTemporalProperties &other )
//{
//  mRange = other.mRange;
//  mFixedRange = other.mFixedRange;
//  mEnableTime = other.mEnableTime;

//  return *this;
//}

void QgsRasterDataProviderTemporalProperties::setTemporalRange( const QgsDateTimeRange &dateTimeRange )
{
  if ( !isActive() )
    setIsActive( true );

  mRange = dateTimeRange;

}

const QgsDateTimeRange &QgsRasterDataProviderTemporalProperties::temporalRange() const
{
  return mRange;
}

void QgsRasterDataProviderTemporalProperties::setFixedTemporalRange( const QgsDateTimeRange &dateTimeRange )
{
  if ( !isActive() )
    setIsActive( true );

  mFixedRange = dateTimeRange;

}

const QgsDateTimeRange &QgsRasterDataProviderTemporalProperties::fixedTemporalRange() const
{
  return mFixedRange;
}

void QgsRasterDataProviderTemporalProperties::setEnableTime( bool enabled )
{
  mEnableTime = enabled;
}

bool QgsRasterDataProviderTemporalProperties::isTimeEnabled() const
{
  return mEnableTime;
}

void QgsRasterDataProviderTemporalProperties::setReferenceTemporalRange( const QgsDateTimeRange &dateTimeRange )
{
  if ( !isActive() )
    setIsActive( true );

  mReferenceRange = dateTimeRange;

}

const QgsDateTimeRange &QgsRasterDataProviderTemporalProperties::referenceTemporalRange() const
{
  return mReferenceRange;
}

void QgsRasterDataProviderTemporalProperties::setFixedReferenceTemporalRange( const QgsDateTimeRange &dateTimeRange )
{
  if ( !isActive() )
    setIsActive( true );

  mFixedReferenceRange = dateTimeRange;

}

const QgsDateTimeRange &QgsRasterDataProviderTemporalProperties::fixedReferenceTemporalRange() const
{
  return mFixedReferenceRange;
}


void QgsRasterDataProviderTemporalProperties::setHasReference( bool enabled )
{
  mHasReferenceRange = enabled;
}

bool QgsRasterDataProviderTemporalProperties::hasReference() const
{
  return mHasReferenceRange;
}

void QgsRasterDataProviderTemporalProperties::setReferenceEnable( bool enabled )
{
  mReferenceEnable = enabled;
}

bool QgsRasterDataProviderTemporalProperties::isReferenceEnable() const
{
  return mReferenceEnable;
}

