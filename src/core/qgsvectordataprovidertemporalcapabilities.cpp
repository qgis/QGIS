/***************************************************************************
                         qgsvectordataprovidertemporalcapabilities.cpp
                         ---------------
    begin                : March 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectordataprovidertemporalcapabilities.h"

QgsVectorDataProviderTemporalCapabilities::QgsVectorDataProviderTemporalCapabilities( bool enabled )
  :  QgsDataProviderTemporalCapabilities( enabled )
{
}

QgsVectorDataProviderTemporalCapabilities::TemporalMode QgsVectorDataProviderTemporalCapabilities::mode() const
{
  return mMode;
}

void QgsVectorDataProviderTemporalCapabilities::setMode( QgsVectorDataProviderTemporalCapabilities::TemporalMode mode )
{
  mMode = mode;
}

void QgsVectorDataProviderTemporalCapabilities::setAvailableTemporalRange( const QgsDateTimeRange &dateTimeRange )
{
  if ( !hasTemporalCapabilities() )
    setHasTemporalCapabilities( true );

  mAvailableTemporalRange = dateTimeRange;
}

const QgsDateTimeRange &QgsVectorDataProviderTemporalCapabilities::availableTemporalRange() const
{
  return mAvailableTemporalRange;
}

QString QgsVectorDataProviderTemporalCapabilities::startField() const
{
  return mStartField;
}

void QgsVectorDataProviderTemporalCapabilities::setStartField( const QString &field )
{
  mStartField = field;
}

QString QgsVectorDataProviderTemporalCapabilities::endField() const
{
  return mEndField;
}

void QgsVectorDataProviderTemporalCapabilities::setEndField( const QString &field )
{
  mEndField = field;
}
