/***************************************************************************
                         qgsmeshdataprovidertemporalcapabilities.cpp
                         -----------------------
    begin                : March 2020
    copyright            : (C) 2020 by Vincent
    email                : zilolv at gmail dot com
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

QgsMeshDatasetIndex QgsMeshDataProviderTemporalCapabilities::datasetIndexTimeInMilliseconds( int group, qint64 timeSinceGlobalReference ) const
{
  const QDateTime &groupReference = mGroupsReferenceDateTime[group];

  const qint64 timeSinceGroupeReference =
    timeSinceGlobalReference - mGlobalReferenceDateTime.msecsTo( groupReference );
  const QList<qint64> &datasetTimes = mDatasetTimeSinceGroupReference[group];
  // No dataset
  if ( datasetTimes.isEmpty() )
    return QgsMeshDatasetIndex();

  // If requested time is before the timestamp of the first dataset, return the first dataset
  // Allow to display "static" dataset (as terrain elevation)
  if ( timeSinceGroupeReference < datasetTimes.first() )
    return QgsMeshDatasetIndex( group, 0 );
  for ( int i = 1; i < datasetTimes.count(); ++i )
  {
    if ( timeSinceGroupeReference < datasetTimes.at( i ) )
      return QgsMeshDatasetIndex( group, i - 1 );
  }

  // The requested time is after last timestamp, return the last dataset
  return QgsMeshDatasetIndex( group, datasetTimes.count() - 1 );
}

QgsMeshDatasetIndex QgsMeshDataProviderTemporalCapabilities::datasetIndex( int group, double timeSinceGlobalReferenceInHours ) const
{
  return datasetIndexTimeInMilliseconds( group, qint64( 3600 * 1000 * timeSinceGlobalReferenceInHours ) );
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
  qint64 unitTimeFactor = 1;
  switch ( mTemporalUnit )
  {
    case QgsUnitTypes::TemporalMilliseconds:
      unitTimeFactor = 1;
      break;
    case QgsUnitTypes::TemporalSeconds:
      unitTimeFactor = 1000;
      break;
    case QgsUnitTypes::TemporalMinutes:
      unitTimeFactor = 60 * 1000;
      break;
    case QgsUnitTypes::TemporalHours:
      unitTimeFactor = 60 * 60 * 1000;
      break;
    case QgsUnitTypes::TemporalDays:
      unitTimeFactor = 24 * 60 * 60 * 1000;
      break;
    case QgsUnitTypes::TemporalWeeks:
      unitTimeFactor = 7 * 24 * 60 * 60 * 1000;
      break;
    default:
      break;
  }
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

void QgsMeshDataProviderTemporalCapabilities::setTemporalUnit( const QgsUnitTypes::TemporalUnit &timeUnit )
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
    return -999999;

  const QList<qint64> &timesList = mDatasetTimeSinceGroupReference[index.group()];
  if ( index.dataset() < timesList.count() )
    return timesList.at( index.dataset() );
  else
    return -999999;
}

void QgsMeshDataProviderTemporalCapabilities::clear()
{
  mGlobalReferenceDateTime = QDateTime();
  mGroupsReferenceDateTime.clear();
  mDatasetTimeSinceGroupReference.clear();
}
