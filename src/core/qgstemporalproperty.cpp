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

QgsTemporalProperty::QgsTemporalProperty()
{
}

QgsTemporalProperty::QgsTemporalProperty( const QgsDateTimeRange dateTimeRange )
  : mDateTimeRange( dateTimeRange )
{
}

QgsTemporalProperty::QgsTemporalProperty( const QDateTime dateTime )
  : mDateTime( dateTime )
{
}

QgsTemporalProperty::QgsTemporalProperty( const QDate date )
  : mDate( date )
{
}

QgsTemporalProperty::QgsTemporalProperty( const QgsTemporalProperty &temporalProperty )
{
  mDateTimeRange = temporalProperty.mDateTimeRange;
  mDateTime = temporalProperty.mDateTime;
  mDate = temporalProperty.mDate;
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

