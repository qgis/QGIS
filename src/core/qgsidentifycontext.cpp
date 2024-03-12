/***************************************************************************
     qgsidentifycontext.cpp
     ----------------------
    Date                 : November 2020
    Copyright            : (C) 2020 by Mathieu Pellerin
    Email                : nirvn dot asia at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
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

QgsDoubleRange QgsIdentifyContext::zRange() const
{
  return mZRange;
}

void QgsIdentifyContext::setZRange( const QgsDoubleRange &range )
{
  mZRange = range;
}
