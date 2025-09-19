/***************************************************************************
    qgsraycastresult.cpp
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

#include "qgsraycastresult.h"


QgsRayCastResult::QgsRayCastResult()
{
}

bool QgsRayCastResult::isEmpty() const
{
  return mLayerResults.isEmpty() && mTerrainResults.isEmpty();
}

bool QgsRayCastResult::hasLayerHits() const
{
  return !mLayerResults.isEmpty();
}

QVector<QgsMapLayer *> QgsRayCastResult::layers() const
{
  return { mLayerResults.keyBegin(), mLayerResults.keyEnd() };
}

QVector<QgsRayCastHit> QgsRayCastResult::layerHits( QgsMapLayer *layer ) const
{
  return mLayerResults.value( layer );
}

bool QgsRayCastResult::hasTerrainHits() const
{
  return !mTerrainResults.isEmpty();
}

QVector<QgsRayCastHit> QgsRayCastResult::terrainHits() const
{
  return mTerrainResults;
}

QVector<QgsRayCastHit> QgsRayCastResult::allHits() const
{
  QVector<QgsRayCastHit> result { mTerrainResults.constBegin(), mTerrainResults.constEnd() };

  for ( auto it = mLayerResults.constBegin(); it != mLayerResults.constEnd(); ++it )
  {
    result.append( { it->constBegin(), it->constEnd() } );
  }

  return result;
}

void QgsRayCastResult::addLayerHits( QgsMapLayer *layer, const QVector<QgsRayCastHit> &hits )
{
  if ( mLayerResults.contains( layer ) )
    mLayerResults[layer].append( hits );
  else
    mLayerResults[layer] = hits;
}

void QgsRayCastResult::addTerrainHits( const QVector<QgsRayCastHit> &hits )
{
  mTerrainResults.append( hits );
}
