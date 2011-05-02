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
#include "qgsmaprenderer.h"
#include "qgsmaptopixel.h"
#include "qgsvectorlayer.h"
#include <QMultiMap>
#include <QPoint>
#include <cmath>


QgsSnapper::QgsSnapper( QgsMapRenderer* mapRenderer ): mMapRenderer( mapRenderer )
{

}

QgsSnapper::QgsSnapper()
{

}

QgsSnapper::~QgsSnapper()
{

}

int QgsSnapper::snapPoint( const QPoint& startPoint, QList<QgsSnappingResult>& snappingResult, const QList<QgsPoint>& excludePoints )
{
  snappingResult.clear();

  QMultiMap<double, QgsSnappingResult> snappingResultList;//all snapping results
  QMultiMap<double, QgsSnappingResult> currentResultList; //snapping results of examined layer

  //start point in (output) map coordinates
  QgsPoint mapCoordPoint = mMapRenderer->coordinateTransform()->toMapCoordinates( startPoint.x(), startPoint.y() );
  QgsPoint layerCoordPoint; //start point in layer coordinates
  QgsSnappingResult newResult;

  QList<QgsSnapper::SnapLayer>::iterator snapLayerIt;
  for ( snapLayerIt = mSnapLayers.begin(); snapLayerIt != mSnapLayers.end(); ++snapLayerIt )
  {
    //transform point from map coordinates to layer coordinates
    layerCoordPoint = mMapRenderer->mapToLayerCoordinates( snapLayerIt->mLayer, mapCoordPoint );

    double tolerance = QgsTolerance::toleranceInMapUnits( snapLayerIt->mTolerance, snapLayerIt->mLayer, mMapRenderer, snapLayerIt->mUnitType );
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
      newResult.snappedVertex = mMapRenderer->layerToMapCoordinates( snapLayerIt->mLayer, currentResultIt.value().snappedVertex );
      newResult.beforeVertex = mMapRenderer->layerToMapCoordinates( snapLayerIt->mLayer, currentResultIt.value().beforeVertex );
      newResult.afterVertex = mMapRenderer->layerToMapCoordinates( snapLayerIt->mLayer, currentResultIt.value().afterVertex );
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

  if ( mSnapMode == QgsSnapper::SnapWithOneResult )
  {
    //return only closest result
    snappingResult.push_back( evalIt.value() );
  }
  else if ( mSnapMode == QgsSnapper::SnapWithResultsForSamePosition )
  {
    //take all snapping Results within a certain tolerance because rounding differences may occur
    double tolerance = 0.000001;
    double minDistance = evalIt.key();

    for ( evalIt = snappingResultList.begin(); evalIt != snappingResultList.end(); ++evalIt )
    {
      if ( evalIt.key() > ( minDistance + tolerance ) )
      {
        break;
      }
      snappingResult.push_back( evalIt.value() );
    }

  }

  else //take all results
  {
    for ( ; evalIt != snappingResultList.end(); ++evalIt )
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
