/***************************************************************************
    qgsmaptoolmodifyannotation.cpp
    ----------------
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolmodifyannotation.h"
#include "qgsrubberband.h"
#include "qgsmapmouseevent.h"
#include "qgsmapcanvas.h"
#include "qgsrendereditemresults.h"
#include "qgsrendereditemdetails.h"
#include "qgsannotationlayer.h"
#include "qgsproject.h"
#include "qgsrenderedannotationitemdetails.h"
#include "qgsannotationitem.h"
#include "qgsannotationitemnode.h"
#include "RTree.h"

///@cond PRIVATE
class QgsAnnotationItemNodesSpatialIndex : public RTree<int, float, 2, float>
{
  public:

    void insert( int index, const QgsRectangle &bounds )
    {
      std::array< float, 4 > scaledBounds = scaleBounds( bounds );
      this->Insert(
      {
        scaledBounds[0], scaledBounds[ 1]
      },
      {
        scaledBounds[2], scaledBounds[3]
      },
      index );
    }

    /**
     * Removes existing \a data from the spatial index, with the specified \a bounds.
     *
     * \a data is not deleted, and it is the caller's responsibility to ensure that
     * it is appropriately cleaned up.
     */
    void remove( int index, const QgsRectangle &bounds )
    {
      std::array< float, 4 > scaledBounds = scaleBounds( bounds );
      this->Remove(
      {
        scaledBounds[0], scaledBounds[ 1]
      },
      {
        scaledBounds[2], scaledBounds[3]
      },
      index );
    }

    /**
     * Performs an intersection check against the index, for data intersecting the specified \a bounds.
     *
     * The \a callback function will be called once for each matching data object encountered.
     */
    bool intersects( const QgsRectangle &bounds, const std::function< bool( int index )> &callback ) const
    {
      std::array< float, 4 > scaledBounds = scaleBounds( bounds );
      this->Search(
      {
        scaledBounds[0], scaledBounds[ 1]
      },
      {
        scaledBounds[2], scaledBounds[3]
      },
      callback );
      return true;
    }

  private:
    std::array<float, 4> scaleBounds( const QgsRectangle &bounds ) const
    {
      return
      {
        static_cast< float >( bounds.xMinimum() ),
        static_cast< float >( bounds.yMinimum() ),
        static_cast< float >( bounds.xMaximum() ),
        static_cast< float >( bounds.yMaximum() )
      };
    }
};
///@endcond


QgsMapToolModifyAnnotation::QgsMapToolModifyAnnotation( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget )
  : QgsMapToolAdvancedDigitizing( canvas, cadDockWidget )
{

}

QgsMapToolModifyAnnotation::~QgsMapToolModifyAnnotation() = default;

void QgsMapToolModifyAnnotation::deactivate()
{
  clearHoveredItem();
  QgsMapToolAdvancedDigitizing::deactivate();
}

void QgsMapToolModifyAnnotation::cadCanvasMoveEvent( QgsMapMouseEvent *event )
{
  const QgsPointXY mapPoint = event->mapPoint();

  QgsRectangle searchRect = QgsRectangle( mapPoint.x(), mapPoint.y(), mapPoint.x(), mapPoint.y() );
  searchRect.grow( searchRadiusMU( canvas() ) );

  const QgsRenderedItemResults *renderedItemResults = canvas()->renderedItemResults( false );
  if ( !renderedItemResults )
  {
    clearHoveredItem();
    return;
  }

  const QList<const QgsRenderedAnnotationItemDetails *> items = renderedItemResults->renderedAnnotationItemsInBounds( searchRect );
  if ( items.empty() )
  {
    clearHoveredItem();
    return;
  }

  // find closest item
  QgsRectangle itemBounds;
  const QgsRenderedAnnotationItemDetails *closestItem = findClosestItemToPoint( mapPoint, items, itemBounds );
  if ( !closestItem )
  {
    clearHoveredItem();
    return;
  }

  if ( closestItem->itemId() != mHoveredItemId || closestItem->layerId() != mHoveredItemLayerId )
  {
    setHoveredItem( closestItem, itemBounds );
  }

  // track hovered node too!... here we want to identify the closest node to the cursor position
  QgsAnnotationItemNode hoveredNode;
  double currentNodeDistance = std::numeric_limits< double >::max();
  mHoveredItemNodesSpatialIndex->intersects( searchRect, [&hoveredNode, &currentNodeDistance, &mapPoint, this]( int index )-> bool
  {
    const QgsAnnotationItemNode &thisNode = mHoveredItemNodes.at( index );
    const double nodeDistance = thisNode.point().sqrDist( mapPoint );
    if ( nodeDistance < currentNodeDistance )
    {
      hoveredNode = thisNode;
      currentNodeDistance = nodeDistance;
    }
    return true;
  } );

  if ( hoveredNode.point().isEmpty() )
  {
    // no hovered node
    if ( mHoveredNodeRubberBand )
      mHoveredNodeRubberBand->hide();
  }
  else
  {
    if ( !mHoveredNodeRubberBand )
      createHoveredNodeBand();

    mHoveredNodeRubberBand->reset( QgsWkbTypes::PointGeometry );
    mHoveredNodeRubberBand->addPoint( hoveredNode.point() );
    mHoveredNodeRubberBand->show();
  }

}

void QgsMapToolModifyAnnotation::cadCanvasPressEvent( QgsMapMouseEvent *event )
{
  if ( event->button() != Qt::LeftButton )
    return;

  if ( mHoveredItemId.isEmpty() || !mHoverRubberBand )
  {
    if ( mSelectedRubberBand )
      mSelectedRubberBand->hide();
    mSelectedItemId.clear();
    mSelectedItemLayerId.clear();
  }
  else if ( mHoveredItemId != mSelectedItemId || mHoveredItemLayerId != mSelectedItemLayerId )
  {
    mSelectedItemId = mHoveredItemId;
    mSelectedItemLayerId = mHoveredItemLayerId;

    if ( !mSelectedRubberBand )
      createSelectedItemBand();

    mSelectedRubberBand->copyPointsFrom( mHoverRubberBand );
    mSelectedRubberBand->show();

    emit itemSelected( annotationLayerFromId( mSelectedItemLayerId ), mSelectedItemId );
  }
}

void QgsMapToolModifyAnnotation::setHoveredItem( const QgsRenderedAnnotationItemDetails *item, const QgsRectangle &itemMapBounds )
{
  mHoveredItemNodeRubberBands.clear();
  mHoveredItemId = item->itemId();
  mHoveredItemLayerId = item->layerId();
  if ( !mHoverRubberBand )
    createHoverBand();

  mHoverRubberBand->show();

  mHoverRubberBand->reset( QgsWkbTypes::LineGeometry );
  mHoverRubberBand->addPoint( QgsPointXY( itemMapBounds.xMinimum(), itemMapBounds.yMinimum() ) );
  mHoverRubberBand->addPoint( QgsPointXY( itemMapBounds.xMaximum(), itemMapBounds.yMinimum() ) );
  mHoverRubberBand->addPoint( QgsPointXY( itemMapBounds.xMaximum(), itemMapBounds.yMaximum() ) );
  mHoverRubberBand->addPoint( QgsPointXY( itemMapBounds.xMinimum(), itemMapBounds.yMaximum() ) );
  mHoverRubberBand->addPoint( QgsPointXY( itemMapBounds.xMinimum(), itemMapBounds.yMinimum() ) );

  QgsAnnotationLayer *layer = annotationLayerFromId( item->layerId() );
  const QgsAnnotationItem *annotationItem = annotationItemFromId( item->layerId(), item->itemId() );
  if ( !annotationItem )
    return;

  QgsCoordinateTransform layerToMapTransform = QgsCoordinateTransform( layer->crs(), canvas()->mapSettings().destinationCrs(), canvas()->mapSettings().transformContext() );

  const double scaleFactor = canvas()->fontMetrics().xHeight() * .2;

  const QList< QgsAnnotationItemNode > itemNodes = annotationItem->nodes();
  QgsRubberBand *vertexNodeBand = new QgsRubberBand( mCanvas, QgsWkbTypes::PointGeometry );

  vertexNodeBand->setIcon( QgsRubberBand::ICON_BOX );
  vertexNodeBand->setWidth( scaleFactor );
  vertexNodeBand->setIconSize( scaleFactor * 5 );
  vertexNodeBand->setColor( QColor( 200, 0, 120, 255 ) );

  // store item nodes in a spatial index for quick searching
  mHoveredItemNodesSpatialIndex = std::make_unique< QgsAnnotationItemNodesSpatialIndex >();
  int index = 0;
  mHoveredItemNodes.clear();
  mHoveredItemNodes.reserve( itemNodes.size() );
  for ( const QgsAnnotationItemNode &node : itemNodes )
  {
    QgsPointXY nodeMapPoint;
    try
    {
      nodeMapPoint = layerToMapTransform.transform( node.point() );
    }
    catch ( QgsCsException & )
    {
      continue;
    }

    switch ( node.type() )
    {
      case Qgis::AnnotationItemNodeType::VertexHandle:
        vertexNodeBand->addPoint( nodeMapPoint );
        break;
    }

    mHoveredItemNodesSpatialIndex->insert( index, QgsRectangle( nodeMapPoint.x(), nodeMapPoint.y(),
                                           nodeMapPoint.x(), nodeMapPoint.y() ) );

    QgsAnnotationItemNode transformedNode = node;
    transformedNode.setPoint( nodeMapPoint );
    mHoveredItemNodes.append( transformedNode );

    index++;
  }

  mHoveredItemNodeRubberBands.emplace_back( vertexNodeBand );
}

const QgsRenderedAnnotationItemDetails *QgsMapToolModifyAnnotation::findClosestItemToPoint( const QgsPointXY &mapPoint, const QList<const QgsRenderedAnnotationItemDetails *> &items, QgsRectangle &bounds )
{
  const QgsRenderedAnnotationItemDetails *closestItem = nullptr;
  double closestItemDistance = std::numeric_limits< double >::max();
  int closestItemZ = 0;

  for ( const QgsRenderedAnnotationItemDetails *item : items )
  {
    const QgsAnnotationItem *annotationItem = annotationItemFromId( item->layerId(), item->itemId() );
    if ( !annotationItem )
      continue;

    const QgsRectangle itemBounds = item->boundingBox();
    const double itemDistance = itemBounds.contains( mapPoint ) ? 0 : itemBounds.distance( mapPoint );
    if ( !closestItem || itemDistance < closestItemDistance || ( itemDistance == closestItemDistance && annotationItem->zIndex() > closestItemZ ) )
    {
      closestItem = item;
      closestItemDistance = itemDistance;
      closestItemZ = annotationItem->zIndex();
      bounds = itemBounds;
    }
  }
  return closestItem;
}

QgsAnnotationLayer *QgsMapToolModifyAnnotation::annotationLayerFromId( const QString &layerId )
{
  QgsAnnotationLayer *layer = qobject_cast< QgsAnnotationLayer * >( QgsProject::instance()->mapLayer( layerId ) );
  if ( !layer && layerId == QgsProject::instance()->mainAnnotationLayer()->id() )
    layer = QgsProject::instance()->mainAnnotationLayer();
  return layer;
}

QgsAnnotationItem *QgsMapToolModifyAnnotation::annotationItemFromId( const QString &layerId, const QString &itemId )
{
  QgsAnnotationLayer *layer = annotationLayerFromId( layerId );
  return layer ? layer->item( itemId ) : nullptr;
}

void QgsMapToolModifyAnnotation::clearHoveredItem()
{
  if ( mHoverRubberBand )
    mHoverRubberBand->hide();
  if ( mHoveredNodeRubberBand )
    mHoveredNodeRubberBand->hide();

  mHoveredItemId.clear();
  mHoveredItemLayerId.clear();
  mHoveredItemNodeRubberBands.clear();
  mHoveredItemNodesSpatialIndex.reset();
}

void QgsMapToolModifyAnnotation::createHoverBand()
{
  const double scaleFactor = canvas()->fontMetrics().xHeight() * .2;

  mHoverRubberBand.reset( new QgsRubberBand( mCanvas, QgsWkbTypes::LineGeometry ) );
  mHoverRubberBand->setWidth( scaleFactor );
  mHoverRubberBand->setSecondaryStrokeColor( QColor( 255, 255, 255, 100 ) );
  mHoverRubberBand->setColor( QColor( 100, 100, 100, 155 ) );
}

void QgsMapToolModifyAnnotation::createHoveredNodeBand()
{
  const double scaleFactor = canvas()->fontMetrics().xHeight() * .2;

  mHoveredNodeRubberBand.reset( new QgsRubberBand( mCanvas, QgsWkbTypes::PointGeometry ) );
  mHoveredNodeRubberBand->setIcon( QgsRubberBand::ICON_FULL_BOX );
  mHoveredNodeRubberBand->setWidth( scaleFactor );
  mHoveredNodeRubberBand->setIconSize( scaleFactor * 5 );
  mHoveredNodeRubberBand->setColor( QColor( 200, 0, 120, 255 ) );
}

void QgsMapToolModifyAnnotation::createSelectedItemBand()
{
  const double scaleFactor = canvas()->fontMetrics().xHeight() * .2;

  mSelectedRubberBand.reset( new QgsRubberBand( mCanvas, QgsWkbTypes::LineGeometry ) );
  mSelectedRubberBand->setWidth( scaleFactor );
  mSelectedRubberBand->setSecondaryStrokeColor( QColor( 255, 255, 255, 100 ) );
  mSelectedRubberBand->setColor( QColor( 50, 50, 50, 200 ) );
}

