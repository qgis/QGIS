/***************************************************************************
                         qgstemporalproperty.cpp
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


#include "qgstemporalproperty.h"

QgsTemporalProperty::QgsTemporalProperty( bool enabled )
    : mActive( enabled )
{
}

QgsTemporalProperty::QgsTemporalProperty( const QgsDateTimeRange dateTimeRange, bool enabled )
  : mDateTimeRange( dateTimeRange )
  , mActive( enabled )
{
}

QgsTemporalProperty::QgsTemporalProperty( const QDateTime dateTime, bool enabled )
  : mDateTime( dateTime )
  , mActive( enabled )
{
}

QgsTemporalProperty::QgsTemporalProperty( const QDate date, bool enabled )
  : mDate( date )
  , mActive( enabled )
{
}

QgsTemporalProperty::QgsTemporalProperty( const QString date, bool enabled )
    :mActive ( enabled )
{
    // parseDate( date );
}

QgsTemporalProperty::QgsTemporalProperty( const QgsTemporalProperty &temporalProperty )
{
  mDateTimeRange = temporalProperty.mDateTimeRange;
  mDateTime = temporalProperty.mDateTime;
  mDate = temporalProperty.mDate;
}

bool QgsTemporalProperty::equal(const QgsTemporalProperty &otherTemporalProperty ) const
{
    return mDateTimeRange == otherTemporalProperty.mDateTimeRange
             && mDateTime == otherTemporalProperty.mDateTime
             && mDate == otherTemporalProperty.mDate;
}

void QgsTemporalProperty::setTemporalRange( const QgsDateTimeRange &dateTimeRange )
{
  if ( dateTimeRange == mDateTimeRange )
    return;

  mDateTimeRange = dateTimeRange;
}

const QgsDateTimeRange &QgsTemporalProperty::temporalRange() const
{
  return mDateTimeRange;
}

void QgsTemporalProperty::setTemporalDateTimeInstant( const QDateTime &dateTime )
{
  if ( dateTime == mDateTime )
    return;

  mDateTime = dateTime;
}

const QDateTime &QgsTemporalProperty::temporalDateTimeInstant() const
{
  return mDateTime;
}

void QgsTemporalProperty::setTemporalDateInstant( const QDate &date )
{
  if ( date == mDate )
    return;

  mDate = date;
}

const QDate &QgsTemporalProperty::temporalDateInstant() const
{
  return mDate;
}

void QgsTemporalProperty::setIsActive( bool enabled )
{
  mActive = enabled;
}

bool QgsTemporalProperty::isActive() const
{
  return mActive;
}

void parseDate()
{
    // For single value

    // QDateTime dateTime = QDateTime::fromString( date, "YYYY-MM-DDTHH:MM:SS");

    // For list values

    // For interval values
}
