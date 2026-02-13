/***************************************************************************
    qgsmaptoolannotation.cpp
    ----------------
    copyright            : (C) 2025 by Mathieu Pellerin
    email                : mathieu at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsannotationmaptool.h"

#include "qgsannotationitem.h"
#include "qgsannotationlayer.h"
#include "qgsmapcanvas.h"
#include "qgsmaptoolselectannotationmousehandles.h"
#include "qgsproject.h"
#include "qgsrenderedannotationitemdetails.h"
#include "qgsrendereditemdetails.h"
#include "qgsrendereditemresults.h"

#include "moc_qgsannotationmaptool.cpp"

QgsAnnotationMapTool::QgsAnnotationMapTool( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget )
  : QgsMapToolAdvancedDigitizing( canvas, cadDockWidget )
{
}

const QgsRenderedAnnotationItemDetails *QgsAnnotationMapTool::findClosestItemToPoint( const QgsPointXY &mapPoint, const QList<const QgsRenderedAnnotationItemDetails *> &items, QgsRectangle &bounds )
{
  const QgsRenderedAnnotationItemDetails *closestItem = nullptr;
  double closestItemDistance = std::numeric_limits<double>::max();
  double closestItemArea = std::numeric_limits<double>::max();

  for ( const QgsRenderedAnnotationItemDetails *item : items )
  {
    const QgsAnnotationItem *annotationItem = annotationItemFromId( item->layerId(), item->itemId() );
    if ( !annotationItem )
      continue;

    const QgsRectangle itemBounds = item->boundingBox();
    const double itemDistance = itemBounds.contains( mapPoint ) ? 0 : itemBounds.distance( mapPoint );
    if ( !closestItem || itemDistance < closestItemDistance || ( itemDistance == closestItemDistance && itemBounds.area() < closestItemArea ) )
    {
      closestItem = item;
      closestItemDistance = itemDistance;
      closestItemArea = itemBounds.area();
      bounds = itemBounds;
    }
  }
  return closestItem;
}

QgsAnnotationLayer *QgsAnnotationMapTool::annotationLayerFromId( const QString &layerId )
{
  QgsAnnotationLayer *layer = qobject_cast<QgsAnnotationLayer *>( QgsProject::instance()->mapLayer( layerId ) );
  if ( !layer && layerId == QgsProject::instance()->mainAnnotationLayer()->id() )
  {
    layer = QgsProject::instance()->mainAnnotationLayer();
  }
  return layer;
}

QgsAnnotationItem *QgsAnnotationMapTool::annotationItemFromId( const QString &layerId, const QString &itemId )
{
  QgsAnnotationLayer *layer = annotationLayerFromId( layerId );
  return layer ? layer->item( itemId ) : nullptr;
}
