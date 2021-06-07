/***************************************************************************
                         qgsrasterdataprovidertemporalcapabilities.cpp
                         ---------------
    begin                : February 2020
    copyright            : (C) 2020 by Samweli Mwakisambwe
    email                : samweli at kartoza dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
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

const QgsDateTimeRange &QgsRasterDataProviderTemporalCapabilities::requestedTemporalRange() const
{
  return mRequestedRange;
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
