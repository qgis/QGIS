/***************************************************************************
    qgsraycastcontext.cpp
    ---------------------
    begin                : September 2025
    copyright            : (C) 2025 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsraycastcontext.h"


void QgsRayCastContext::setSingleResult( bool enable )
{
  mSingleResult = enable;
}

bool QgsRayCastContext::singleResult() const
{
  return mSingleResult;
}

float QgsRayCastContext::maximumDistance() const
{
  return mMaxDistance;
}

void QgsRayCastContext::setMaximumDistance( float distance )
{
  mMaxDistance = distance;
}

void QgsRayCastContext::setAngleThreshold( float angle )
{
  mAngleThreshold = std::clamp( angle, 0.f, 90.f );
}

float QgsRayCastContext::angleThreshold() const
{
  return mAngleThreshold;
}
