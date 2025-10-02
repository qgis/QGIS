/***************************************************************************
    qgsraycasthit.cpp
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

#include "qgsraycasthit.h"


double QgsRayCastHit::distance() const
{
  return mDistance;
}

QgsVector3D QgsRayCastHit::mapCoordinates() const
{
  return mPos;
}

QVariantMap QgsRayCastHit::properties() const
{
  return mAttributes;
}

void QgsRayCastHit::setDistance( double distance )
{
  mDistance = distance;
}

void QgsRayCastHit::setMapCoordinates( const QgsVector3D &point )
{
  mPos = point;
}

void QgsRayCastHit::setProperties( const QVariantMap &attributes )
{
  mAttributes = attributes;
}
