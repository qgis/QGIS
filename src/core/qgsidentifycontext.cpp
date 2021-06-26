/***************************************************************************
     qgsidentifycontext.cpp
     ----------------------
    Date                 : November 2020
    Copyright            : (C) 2020 by Mathieu Pellerin
    Email                : nirvn dot asia at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsidentifycontext.h"


void QgsIdentifyContext::setTemporalRange( const QgsDateTimeRange &range )
{
  mTemporalRange = range;
}

const QgsDateTimeRange &QgsIdentifyContext::temporalRange() const
{
  return mTemporalRange;
}

bool QgsIdentifyContext::isTemporal() const
{
  return mTemporalRange.begin().isValid() || mTemporalRange.end().isValid();
}
