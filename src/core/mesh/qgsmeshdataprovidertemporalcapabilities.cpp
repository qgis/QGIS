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

QgsMeshDatasetIndex QgsMeshDataProviderTemporalCapabilities::datasetIndexFromRelativeTimeRange( int group, qint64 startTimeSinceGlobalReference, qint64 endTimeSinceGlobalReference ) const
{
  // No time --> non temporal dataset, so return the dataset that has to be the only one
  const QList<qint64> &datasetTimes = mDatasetTimeSinceGroupReference[group];
  if ( datasetTimes.isEmpty() )
    return QgsMeshDatasetIndex( group, 0 );
  const QDateTime groupReference = mGroupsReferenceDateTime[group];
  const qint64 startTimeSinceGroupReference =
    startTimeSinceGlobalReference - mGlobalReferenceDateTime.msecsTo( groupReference );
  const qint64 endTimeSinceGroupReference =
    endTimeSinceGlobalReference - mGlobalReferenceDateTime.msecsTo( groupReference );

  if ( startTimeSinceGroupReference >= datasetTimes.last() )
    return QgsMeshDatasetIndex();

  for ( int i = 0; i < datasetTimes.count(); ++i )
  {
    qint64 time = datasetTimes.at( i );
    if ( startTimeSinceGroupReference <= time )
    {
      if ( endTimeSinceGroupReference <= time )
        return QgsMeshDatasetIndex( group, i - 1 ); // invalid if i=0
      else
        return QgsMeshDatasetIndex( group, i );
    }
  }

  // if we are here (normally, this could no happen), return invalid dataset index
  return QgsMeshDatasetIndex();
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
  qint64 unitTimeFactor = QgsUnitTypes::fromUnitToUnitFactor( mTemporalUnit, QgsUnitTypes::TemporalMilliseconds );
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
