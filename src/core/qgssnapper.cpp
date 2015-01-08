/***************************************************************************
                              qgssnapper.cpp
                              --------------
  begin                : June 7, 2007
  copyright            : (C) 2007 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssnapper.h"
#include "qgsmapsettings.h"
#include "qgsmaprenderer.h"
#include "qgsmaptopixel.h"
#include "qgsvectorlayer.h"
#include <QMultiMap>
#include <QPoint>
#include <cmath>


QgsSnapper::QgsSnapper( QgsMapRenderer* mapRenderer )
    : mMapSettings( mapRenderer->mapSettings() )
{

}

QgsSnapper::QgsSnapper( const QgsMapSettings& mapSettings )
    : mMapSettings( mapSettings )
{
}

QgsSnapper::~QgsSnapper()
{

}

int QgsSnapper::snapPoint( const QPoint& startPoint, QList<QgsSnappingResult>& snappingResult, const QList<QgsPoint>& excludePoints )
{
  QgsPoint mapCoordPoint = mMapSettings.mapToPixel().toMapCoordinates( startPoint.x(), startPoint.y() );
  return snapPoint( mapCoordPoint, snappingResult, excludePoints );
}

int QgsSnapper::snapPoint( const QgsPoint& mapCoordPoint, QList<QgsSnappingResult>& snappingResult, const QList<QgsPoint>& excludePoints )
{
  snappingResult.clear();

  QMultiMap<double, QgsSnappingResult> snappingResultList;//all snapping results
  QMultiMap<double, QgsSnappingResult> currentResultList; //snapping results of examined layer

  //start point in (output) map coordinates

  QgsPoint layerCoordPoint; //start point in layer coordinates
  QgsSnappingResult newResult;

  QList<QgsSnapper::SnapLayer>::iterator snapLayerIt;
  for ( snapLayerIt = mSnapLayers.begin(); snapLayerIt != mSnapLayers.end(); ++snapLayerIt )
  {
    if ( !snapLayerIt->mLayer->hasGeometryType() )
      continue;

    currentResultList.clear();
    //transform point from map coordinates to layer coordinates
    layerCoordPoint = mMapSettings.mapToLayerCoordinates( snapLayerIt->mLayer, mapCoordPoint );

    double tolerance = QgsTolerance::toleranceInMapUnits( snapLayerIt->mTolerance, snapLayerIt->mLayer, mMapSettings, snapLayerIt->mUnitType );
    if ( snapLayerIt->mLayer->snapWithContext( layerCoordPoint, tolerance,
         currentResultList, snapLayerIt->mSnapTo ) != 0 )
    {
      //error
    }

    //transform each result from layer crs to map crs (including distance)
    QMultiMap<double, QgsSnappingResult>::iterator currentResultIt;
    for ( currentResultIt = currentResultList.begin(); currentResultIt != currentResultList.end(); ++currentResultIt )
    {
      //for each snapping result: transform start point, snap point and other points into map coordinates to find out distance
      //store results in snapping result list
      newResult = currentResultIt.value();
      newResult.snappedVertex = mMapSettings.layerToMapCoordinates( snapLayerIt->mLayer, currentResultIt.value().snappedVertex );
      newResult.beforeVertex = mMapSettings.layerToMapCoordinates( snapLayerIt->mLayer, currentResultIt.value().beforeVertex );
      newResult.afterVertex = mMapSettings.layerToMapCoordinates( snapLayerIt->mLayer, currentResultIt.value().afterVertex );
      snappingResultList.insert( sqrt( newResult.snappedVertex.sqrDist( mapCoordPoint ) ), newResult );
    }
  }

  //excluded specific points from result
  cleanResultList( snappingResultList, excludePoints );

  //evaluate results according to snap mode
  QMultiMap<double, QgsSnappingResult>::iterator evalIt =  snappingResultList.begin();
  if ( evalIt == snappingResultList.end() )
  {
    return 0;
  }


  //Gives a priority to vertex snapping over segment snapping
  QgsSnappingResult returnResult = evalIt.value();
  for ( evalIt = snappingResultList.begin(); evalIt != snappingResultList.end(); ++evalIt )
  {
    if ( evalIt.value().snappedVertexNr != -1 )
    {
      returnResult = evalIt.value();
      snappingResultList.erase( evalIt );
      break;
    }
  }

  //We return the preferred result
  snappingResult.push_back( returnResult );

  if ( mSnapMode == QgsSnapper::SnapWithOneResult )
  {
    //return only a single  result, nothing more to do
  }
  else if ( mSnapMode == QgsSnapper::SnapWithResultsForSamePosition )
  {
    //take all snapping results within a certain tolerance because rounding differences may occur
    double tolerance = 0.000001;

    for ( evalIt = snappingResultList.begin(); evalIt != snappingResultList.end(); ++evalIt )
    {
      if ( returnResult.snappedVertex.sqrDist( evalIt.value().snappedVertex ) < tolerance*tolerance )
      {
        snappingResult.push_back( evalIt.value() );
      }
    }

  }

  else //take all results
  {
    for ( evalIt = snappingResultList.begin(); evalIt != snappingResultList.end(); ++evalIt )
    {
      snappingResult.push_back( evalIt.value() );
    }
  }

  return 0;
}

void QgsSnapper::setSnapLayers( const QList<QgsSnapper::SnapLayer>& snapLayers )
{
  mSnapLayers = snapLayers;
}


void QgsSnapper::setSnapMode( QgsSnapper::SnappingMode snapMode )
{
  mSnapMode = snapMode;
}

void QgsSnapper::cleanResultList( QMultiMap<double, QgsSnappingResult>& list, const QList<QgsPoint>& excludeList ) const
{
  QgsPoint currentResultPoint;
  QgsSnappingResult currentSnappingResult;
  QList<double> keysToRemove;

  QMultiMap<double, QgsSnappingResult>::iterator result_it = list.begin();
  for ( ; result_it != list.end(); ++result_it )
  {
    currentSnappingResult = result_it.value();
    if ( currentSnappingResult.snappedVertexNr != -1 )
    {
      currentResultPoint = currentSnappingResult.snappedVertex;
      if ( excludeList.contains( currentResultPoint ) )
      {
        keysToRemove.push_back( result_it.key() );
      }
    }
  }

  QList<double>::const_iterator remove_it = keysToRemove.constBegin();
  for ( ; remove_it != keysToRemove.constEnd(); ++remove_it )
  {
    list.remove( *remove_it );
  }
}
