/***************************************************************************
                         qgstemporalrangeobject.cpp
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

#include "qgstemporalrangeobject.h"

QgsTemporalRangeObject::QgsTemporalRangeObject( bool enabled )
  : mTemporal( enabled )
{
}

void QgsTemporalRangeObject::setIsTemporal( bool enabled )
{
  mTemporal = enabled;
}

bool QgsTemporalRangeObject::isTemporal() const
{
  return mTemporal;
}

void QgsTemporalRangeObject::setTemporalRange( const QgsDateTimeRange &dateTimeRange )
{
  if ( !isTemporal() )
    setIsTemporal( true );

  mDateTimeRange = dateTimeRange;
}

const QgsDateTimeRange &QgsTemporalRangeObject::temporalRange() const
{
  return mDateTimeRange;
}

