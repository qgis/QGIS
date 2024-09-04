/***************************************************************************
  qgs3drendercontext.cpp
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

#include "qgs3drendercontext.h"
#include "qgs3dutils.h"
#include "qgsthreadingutils.h"

Qgs3DRenderContext Qgs3DRenderContext::fromMapSettings( const Qgs3DMapSettings *mapSettings )
{
  QGIS_CHECK_OTHER_QOBJECT_THREAD_ACCESS( mapSettings );

  Qgs3DRenderContext res;
  res.mCrs = mapSettings->crs();
  res.mTransformContext = mapSettings->transformContext();
  res.mOrigin = mapSettings->origin();
  res.mExtent = mapSettings->extent();
  res.mTemporalRange = mapSettings->temporalRange();
  res.mSelectionColor = mapSettings->selectionColor();
  res.mDpi = mapSettings->outputDpi();
  res.mFieldOfView = mapSettings->fieldOfView();
  res.mTerrainRenderingEnabled = mapSettings->terrainRenderingEnabled();
  res.mTerrainVerticalScale = mapSettings->terrainVerticalScale();
  res.mTerrainGenerator = mapSettings->terrainGenerator();
  return res;
}

QgsVector3D Qgs3DRenderContext::mapToWorldCoordinates( const QgsVector3D &mapCoords ) const
{
  return Qgs3DUtils::mapToWorldCoordinates( mapCoords, mOrigin );
}

QgsVector3D Qgs3DRenderContext::worldToMapCoordinates( const QgsVector3D &worldCoords ) const
{
  return Qgs3DUtils::worldToMapCoordinates( worldCoords, mOrigin );
}
