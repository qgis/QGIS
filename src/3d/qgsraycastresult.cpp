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
#include "qgsmaplayer.h"


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

QList<QgsMapLayer *> QgsRayCastResult::layers() const
{
  QList<QgsMapLayer *> layerList { mLayerResults.keyBegin(), mLayerResults.keyEnd() };

  // Remove any deleted layers, or python won't be happy
  layerList.erase(
    std::remove_if(
      layerList.begin(),
      layerList.end(),
      [this]( QgsMapLayer *l ) { return mLayerPointers[l].isNull(); }
    ),
    layerList.end()
  );

  return layerList;
}

QList<QgsRayCastHit> QgsRayCastResult::layerHits( QgsMapLayer *layer ) const
{
  return mLayerResults.value( layer );
}

bool QgsRayCastResult::hasTerrainHits() const
{
  return !mTerrainResults.isEmpty();
}

QList<QgsRayCastHit> QgsRayCastResult::terrainHits() const
{
  return mTerrainResults;
}

QList<QgsRayCastHit> QgsRayCastResult::allHits() const
{
  QList<QgsRayCastHit> result { mTerrainResults.constBegin(), mTerrainResults.constEnd() };

  for ( auto it = mLayerResults.constBegin(); it != mLayerResults.constEnd(); ++it )
  {
    result.append( { it->constBegin(), it->constEnd() } );
  }

  return result;
}

void QgsRayCastResult::addLayerHits( QgsMapLayer *layer, const QList<QgsRayCastHit> &hits )
{
  mLayerResults[layer].append( hits );
  if ( !mLayerPointers.contains( layer ) )
  {
    mLayerPointers[layer] = layer;
  }
}

void QgsRayCastResult::addTerrainHits( const QList<QgsRayCastHit> &hits )
{
  mTerrainResults.append( hits );
}
