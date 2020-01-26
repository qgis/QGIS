/***************************************************************************
                         qgsabstracttemporal.h
                         ---------------
    begin                : January 2020
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


#include "qgsabstracttemporal.h"

QgsAbstractTemporal::QgsAbstractTemporal()
{
}

QgsAbstractTemporal::QgsAbstractTemporal( const bool enabled )
  : mTemporal( enabled )
{
}

void QgsAbstractTemporal::setIsTemporal( bool enabled )
{
  mTemporal = enabled;
}

bool QgsAbstractTemporal::isTemporal() const
{
  return mTemporal;
}

void QgsAbstractTemporal::setTemporalRange( const QgsDateTimeRange& dateTimeRange )
{
  if ( !isTemporal() )
    return;

  if ( dateTimeRange == mDateTimeRange )
    return;

  mDateTimeRange = dateTimeRange;

}

const QgsDateTimeRange& QgsAbstractTemporal::temporalRange() const
{
  return mDateTimeRange;
}

void QgsAbstractTemporal::setCurrentDateTime( QDateTime *dateTime )
{
  if ( !isTemporal() )
    return;

  if ( dateTime == mDateTime )
    return;

  delete mDateTime;
  mDateTime = dateTime;

}

QDateTime *QgsAbstractTemporal::currentDateTime() const
{
  return mDateTime;
}
