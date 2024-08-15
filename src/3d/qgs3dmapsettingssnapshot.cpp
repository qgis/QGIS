/***************************************************************************
  qgs3dmapsettingssnapshot.cpp
  --------------------------------------
  Date                 : August 2024
  Copyright            : (C) 2024 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3dmapsettingssnapshot.h"
#include "qgs3dutils.h"

QgsVector3D Qgs3DMapSettingsSnapshot::mapToWorldCoordinates( const QgsVector3D &mapCoords ) const
{
  return Qgs3DUtils::mapToWorldCoordinates( mapCoords, mOrigin );
}

QgsVector3D Qgs3DMapSettingsSnapshot::worldToMapCoordinates( const QgsVector3D &worldCoords ) const
{
  return Qgs3DUtils::worldToMapCoordinates( worldCoords, mOrigin );
}
