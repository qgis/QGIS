/***************************************************************************
                         qgsmeshdataprovidertemporalcapabilities.cpp
                         -----------------------
    begin                : March 2020
    copyright            : (C) 2020 by Vincent Cloarec
    email                : vcloarec at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmeshdataprovidertemporalcapabilities.h"


QgsMeshDataProviderTemporalCapabilities::QgsMeshDataProviderTemporalCapabilities(): QgsDataProviderTemporalCapabilities()
{}

QgsMeshDatasetIndex QgsMeshDataProviderTemporalCapabilities::datasetIndexClosestFromRelativeTime( int group, qint64 timeSinceGlobalReference ) const
{
  // No time --> non temporal dataset, so return the dataset that has to be the only one
  const QList<qint64> &datasetTimes = mDatasetTimeSinceGroupReference[group];
  if ( datasetTimes.isEmpty() )
    return QgsMeshDatasetIndex( group, 0 );
  const QDateTime groupReference = mGroupsReferenceDateTime[group];
  const qint64 timeSinceGroupReference =
    timeSinceGlobalReference - mGlobalReferenceDateTime.msecsTo( groupReference );

  if ( timeSinceGroupReference > datasetTimes.last() // after last time
       || timeSinceGroupReference < datasetTimes.first() ) // before first time
    return QgsMeshDatasetIndex();

  for ( int i = 1 ; i < datasetTimes.count(); ++i )
  {
    const qint64 time1 = datasetTimes.at( i - 1 );
    const qint64 time2 = datasetTimes.at( i );
    if ( time1 <= timeSinceGroupReference && timeSinceGroupReference <= time2 )
    {
      if ( abs( timeSinceGroupReference - time2 ) < abs( timeSinceGroupReference - time1 ) )
        return QgsMeshDatasetIndex( group, i );
      else
        return QgsMeshDatasetIndex( group, i - 1 );
    }
  }

  return QgsMeshDatasetIndex( QgsMeshDatasetIndex( group, datasetTimes.count() - 1 ) );
}

QgsMeshDatasetIndex QgsMeshDataProviderTemporalCapabilities::datasetIndexClosestBeforeRelativeTime( int group, qint64 timeSinceGlobalReference ) const
{
  // No time --> non temporal dataset, so return the dataset that has to be the only one
  const QList<qint64> &datasetTimes = mDatasetTimeSinceGroupReference[group];
  if ( datasetTimes.isEmpty() )
    return QgsMeshDatasetIndex( group, 0 );
  const QDateTime groupReference = mGroupsReferenceDateTime[group];
  const qint64 timeSinceGroupReference =
    timeSinceGlobalReference - mGlobalReferenceDateTime.msecsTo( groupReference );

  if ( timeSinceGroupReference > datasetTimes.last() // after last time
       || timeSinceGroupReference < datasetTimes.first() ) // before first time
    return QgsMeshDatasetIndex();

  for ( int i = 1; i < datasetTimes.count(); ++i )
  {
    const qint64 time = datasetTimes.at( i );
    if ( timeSinceGroupReference < time )
      return QgsMeshDatasetIndex( group, i - 1 );
  }

  return QgsMeshDatasetIndex( QgsMeshDatasetIndex( group, datasetTimes.count() - 1 ) );
}

void QgsMeshDataProviderTemporalCapabilities::addGroupReferenceDateTime( int group, const QDateTime &reference )
{
  if ( ( !mGlobalReferenceDateTime.isValid() && reference.isValid() ) ||
       ( reference.isValid()  && mGlobalReferenceDateTime.isValid() && reference < mGlobalReferenceDateTime ) )
    mGlobalReferenceDateTime = reference;

  mGroupsReferenceDateTime[group] = reference;
}

void QgsMeshDataProviderTemporalCapabilities::addDatasetTimeInMilliseconds( int group, qint64 time )
{
  QList<qint64> &datasetTimes = mDatasetTimeSinceGroupReference[group];
  datasetTimes.append( time );
}

void QgsMeshDataProviderTemporalCapabilities::addDatasetTime( int group, double time )
{
  const qint64 unitTimeFactor = QgsUnitTypes::fromUnitToUnitFactor( mTemporalUnit, QgsUnitTypes::TemporalMilliseconds );
  addDatasetTimeInMilliseconds( group, time * unitTimeFactor );
}

bool QgsMeshDataProviderTemporalCapabilities::hasReferenceTime() const
{
  return mGlobalReferenceDateTime.isValid();
}

QDateTime QgsMeshDataProviderTemporalCapabilities::referenceTime() const
{
  return mGlobalReferenceDateTime;
}

QgsDateTimeRange QgsMeshDataProviderTemporalCapabilities::timeExtent() const
{

  return timeExtent( mGlobalReferenceDateTime );
}

QgsDateTimeRange QgsMeshDataProviderTemporalCapabilities::timeExtent( const QDateTime &reference ) const
{
  QDateTime end;
  QDateTime begin;
  for ( QHash<int, QDateTime>::const_iterator it = mGroupsReferenceDateTime.constBegin() ;
        it != mGroupsReferenceDateTime.constEnd(); ++it )
  {
    QDateTime groupReference = it.value();
    if ( !groupReference.isValid() ) //the dataset group has not a valid reference time -->take global reference
      groupReference = mGlobalReferenceDateTime;

    if ( !groupReference.isValid() )
      groupReference = reference;

    const QList<qint64> times = mDatasetTimeSinceGroupReference[it.key()];
    qint64 durationSinceFirst = groupReference.msecsTo( reference );
    qint64 durationSinceLast = groupReference.msecsTo( reference );
    if ( !times.isEmpty() )
    {
      durationSinceFirst += times.first();
      durationSinceLast += times.last();
    }

    if ( !end.isValid() || groupReference.addMSecs( durationSinceLast ) > end )
      end = groupReference.addMSecs( durationSinceLast );

    if ( !begin.isValid() || groupReference.addMSecs( durationSinceFirst ) > begin )
      begin = groupReference.addMSecs( durationSinceFirst );
  }

  return  QgsDateTimeRange( begin, end );
}

void QgsMeshDataProviderTemporalCapabilities::setTemporalUnit( QgsUnitTypes::TemporalUnit timeUnit )
{
  mTemporalUnit = timeUnit;
}

QgsUnitTypes::TemporalUnit QgsMeshDataProviderTemporalCapabilities::temporalUnit() const
{
  return mTemporalUnit;
}

qint64 QgsMeshDataProviderTemporalCapabilities::datasetTime( const QgsMeshDatasetIndex &index ) const
{
  if ( !index.isValid() )
    return INVALID_MESHLAYER_TIME;

  const QList<qint64> &timesList = mDatasetTimeSinceGroupReference[index.group()];
  if ( index.dataset() < timesList.count() )
    return timesList.at( index.dataset() );
  else
    return INVALID_MESHLAYER_TIME;
}

void QgsMeshDataProviderTemporalCapabilities::clear()
{
  mGlobalReferenceDateTime = QDateTime();
  mGroupsReferenceDateTime.clear();
  mDatasetTimeSinceGroupReference.clear();
}

qint64 QgsMeshDataProviderTemporalCapabilities::firstTimeStepDuration( int group ) const
{
  qint64 ret = -1;
  if ( mDatasetTimeSinceGroupReference.contains( group ) )
  {
    const QList<qint64> times = mDatasetTimeSinceGroupReference[group];
    if ( times.count() > 1 )
      ret = times.at( 1 ) - times.at( 0 );
  }
  return ret;
}
