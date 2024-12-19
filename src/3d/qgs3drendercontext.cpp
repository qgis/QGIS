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
#include "qgsabstractterrainsettings.h"

Qgs3DRenderContext::Qgs3DRenderContext( const Qgs3DRenderContext &other )
  : mCrs( other.mCrs )
  , mTransformContext( other.mTransformContext )
  , mOrigin( other.mOrigin )
  , mExtent( other.mExtent )
  , mTemporalRange( other.mTemporalRange )
  , mSelectionColor( other.mSelectionColor )
  , mDpi( other.mDpi )
  , mFieldOfView( other.mFieldOfView )
  , mTerrainRenderingEnabled( other.mTerrainRenderingEnabled )
  , mTerrainSettings( std::unique_ptr<QgsAbstractTerrainSettings>( other.mTerrainSettings->clone() ) )
  , mExpressionContext( other.mExpressionContext )
  , mTerrainGenerator( other.mTerrainGenerator )
{
}

Qgs3DRenderContext::~Qgs3DRenderContext() = default;

Qgs3DRenderContext &Qgs3DRenderContext::operator=( const Qgs3DRenderContext &other )
{
  if ( &other == this )
    return *this;

  mCrs = other.mCrs;
  mTransformContext = other.mTransformContext;
  mOrigin = other.mOrigin;
  mExtent = other.mExtent;
  mTemporalRange = other.mTemporalRange;
  mSelectionColor = other.mSelectionColor;
  mDpi = other.mDpi;
  mFieldOfView = other.mFieldOfView;
  mTerrainRenderingEnabled = other.mTerrainRenderingEnabled;
  mTerrainSettings.reset( other.mTerrainSettings->clone() );
  mExpressionContext = other.mExpressionContext;
  mTerrainGenerator = other.mTerrainGenerator;
  return *this;
}

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
  res.mTerrainSettings = std::unique_ptr<QgsAbstractTerrainSettings>( mapSettings->terrainSettings()->clone() );
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

const QgsAbstractTerrainSettings *Qgs3DRenderContext::terrainSettings() const
{
  return mTerrainSettings.get();
}
