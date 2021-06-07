/***************************************************************************
                         qgstemporalrangeobject.cpp
                         ---------------
    begin                : January 2020
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

