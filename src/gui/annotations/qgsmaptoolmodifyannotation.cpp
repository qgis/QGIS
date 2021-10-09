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
#include "qgsannotationitemeditoperation.h"
#include "qgssnapindicator.h"
#include "RTree.h"
#include <QTransform>
#include <QWindow>
#include <QScreen>

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
  , mSnapIndicator( new QgsSnapIndicator( canvas ) )
{

  connect( QgsMapToolModifyAnnotation::canvas(), &QgsMapCanvas::mapCanvasRefreshed, this, &QgsMapToolModifyAnnotation::onCanvasRefreshed );
}

QgsMapToolModifyAnnotation::~QgsMapToolModifyAnnotation() = default;

void QgsMapToolModifyAnnotation::deactivate()
{
  mSnapIndicator->setMatch( QgsPointLocator::Match() );

  clearHoveredItem();
  QgsMapToolAdvancedDigitizing::deactivate();
}

void QgsMapToolModifyAnnotation::cadCanvasMoveEvent( QgsMapMouseEvent *event )
{
  event->snapPoint();
  mSnapIndicator->setMatch( event->mapPointMatch() );

  const QgsPointXY mapPoint = event->mapPoint();

  switch ( mCurrentAction )
  {
    case Action::NoAction:
    {
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
      if ( closestItem->itemId() == mSelectedItemId && closestItem->layerId() == mSelectedItemLayerId )
      {
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
      }

      if ( hoveredNode.point().isEmpty() )
      {
        // no hovered node
        if ( mHoveredNodeRubberBand )
          mHoveredNodeRubberBand->hide();
        setCursor( mHoveredItemId == mSelectedItemId && mHoveredItemLayerId == mSelectedItemLayerId ? Qt::OpenHandCursor : Qt::ArrowCursor );
      }
      else
      {
        if ( !mHoveredNodeRubberBand )
          createHoveredNodeBand();

        mHoveredNodeRubberBand->reset( QgsWkbTypes::PointGeometry );
        mHoveredNodeRubberBand->addPoint( hoveredNode.point() );
        mHoveredNodeRubberBand->show();

        setCursor( Qt::ArrowCursor );
      }
      break;
    }

    case Action::MoveItem:
    {
      if ( QgsAnnotationItem *item = annotationItemFromId( mSelectedItemLayerId, mSelectedItemId ) )
      {
        QgsAnnotationLayer *layer = annotationLayerFromId( mSelectedItemLayerId );
        const QgsVector delta = toLayerCoordinates( layer, event->mapPoint() ) - mMoveStartPointLayerCrs;

        QgsAnnotationItemEditOperationTranslateItem operation( mSelectedItemId, delta.x(), delta.y() );
        std::unique_ptr< QgsAnnotationItemEditOperationTransientResults > operationResults( item->transientEditResults( &operation ) );
        if ( operationResults )
        {
          mTemporaryRubberBand.reset( new QgsRubberBand( mCanvas, operationResults->representativeGeometry().type() ) );
          const double scaleFactor = canvas()->fontMetrics().xHeight() * .2;
          mTemporaryRubberBand->setWidth( scaleFactor );
          mTemporaryRubberBand->setToGeometry( operationResults->representativeGeometry(), layer->crs() );
        }
        else
        {
          mTemporaryRubberBand.reset();
        }
      }
      break;
    }

    case Action::MoveNode:
    {
      if ( QgsAnnotationItem *item = annotationItemFromId( mSelectedItemLayerId, mSelectedItemId ) )
      {
        QgsAnnotationLayer *layer = annotationLayerFromId( mSelectedItemLayerId );
        const QgsPointXY endPointLayer = toLayerCoordinates( layer, event->mapPoint() );
        QgsAnnotationItemEditOperationMoveNode operation( mSelectedItemId, mTargetNode.id(), QgsPoint( mTargetNode.point() ), QgsPoint( endPointLayer ) );
        std::unique_ptr< QgsAnnotationItemEditOperationTransientResults > operationResults( item->transientEditResults( &operation ) );
        if ( operationResults )
        {
          mTemporaryRubberBand.reset( new QgsRubberBand( mCanvas, operationResults->representativeGeometry().type() ) );
          const double scaleFactor = canvas()->fontMetrics().xHeight() * .2;
          mTemporaryRubberBand->setWidth( scaleFactor );
          mTemporaryRubberBand->setToGeometry( operationResults->representativeGeometry(), layer->crs() );
        }
        else
        {
          mTemporaryRubberBand.reset();
        }
      }
      break;
    }
  }

}

void QgsMapToolModifyAnnotation::cadCanvasPressEvent( QgsMapMouseEvent *event )
{
  switch ( mCurrentAction )
  {
    case Action::NoAction:
    {
      if ( event->button() != Qt::LeftButton )
        return;

      if ( mHoveredItemId.isEmpty() || !mHoverRubberBand )
      {
        clearSelectedItem();
      }
      if ( mHoveredItemId == mSelectedItemId && mHoveredItemLayerId == mSelectedItemLayerId )
      {
        // press is on selected item => move that item
        if ( QgsAnnotationLayer *layer = annotationLayerFromId( mSelectedItemLayerId ) )
        {
          const QgsPointXY mapPoint = event->mapPoint();
          QgsRectangle searchRect = QgsRectangle( mapPoint.x(), mapPoint.y(), mapPoint.x(), mapPoint.y() );
          searchRect.grow( searchRadiusMU( canvas() ) );

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

          mMoveStartPointCanvasCrs = mapPoint;
          mMoveStartPointLayerCrs = toLayerCoordinates( layer, mMoveStartPointCanvasCrs );
          if ( mHoverRubberBand )
            mHoverRubberBand->hide();
          if ( mSelectedRubberBand )
            mSelectedRubberBand->hide();

          if ( hoveredNode.point().isEmpty() )
          {
            mCurrentAction = Action::MoveItem;
          }
          else
          {
            mCurrentAction = Action::MoveNode;
            mTargetNode = hoveredNode;
          }
        }
      }
      else
      {
        // press is on a different item to selected item => select that item
        mSelectedItemId = mHoveredItemId;
        mSelectedItemLayerId = mHoveredItemLayerId;

        if ( !mSelectedRubberBand )
          createSelectedItemBand();

        mSelectedRubberBand->copyPointsFrom( mHoverRubberBand );
        mSelectedRubberBand->show();

        setCursor( Qt::OpenHandCursor );

        emit itemSelected( annotationLayerFromId( mSelectedItemLayerId ), mSelectedItemId );
      }
      break;
    }

    case Action::MoveItem:
    {
      if ( event->button() == Qt::RightButton )
      {
        mCurrentAction = Action::NoAction;
        mTemporaryRubberBand.reset();
        if ( mSelectedRubberBand )
        {
          mSelectedRubberBand->setTranslationOffset( 0, 0 );
          mSelectedRubberBand->show();
        }
        mHoveredItemNodeRubberBands.clear();
        setCursor( Qt::ArrowCursor );
      }
      else if ( event->button() == Qt::LeftButton )
      {
        // apply move
        if ( QgsAnnotationLayer *layer = annotationLayerFromId( mSelectedItemLayerId ) )
        {
          const QgsVector delta = toLayerCoordinates( layer, event->mapPoint() ) - mMoveStartPointLayerCrs;

          QgsAnnotationItemEditOperationTranslateItem operation( mSelectedItemId, delta.x(), delta.y() );
          switch ( layer->applyEdit( &operation ) )
          {
            case Qgis::AnnotationItemEditOperationResult::Success:
              QgsProject::instance()->setDirty( true );
              mRefreshSelectedItemAfterRedraw = true;
              break;
            case Qgis::AnnotationItemEditOperationResult::Invalid:
            case Qgis::AnnotationItemEditOperationResult::ItemCleared:
              break;
          }
        }

        mTemporaryRubberBand.reset();
        mCurrentAction = Action::NoAction;
        setCursor( Qt::ArrowCursor );
      }
      break;
    }

    case Action::MoveNode:
    {
      if ( event->button() == Qt::RightButton )
      {
        mCurrentAction = Action::NoAction;
        mTemporaryRubberBand.reset();
        mHoveredItemNodeRubberBands.clear();
        mTemporaryRubberBand.reset();
        setCursor( Qt::ArrowCursor );
      }
      else if ( event->button() == Qt::LeftButton )
      {
        if ( QgsAnnotationLayer *layer = annotationLayerFromId( mSelectedItemLayerId ) )
        {
          const QgsPointXY endPointLayer = toLayerCoordinates( layer, event->mapPoint() );
          QgsAnnotationItemEditOperationMoveNode operation( mSelectedItemId, mTargetNode.id(), QgsPoint( mTargetNode.point() ), QgsPoint( endPointLayer ) );
          switch ( layer->applyEdit( &operation ) )
          {
            case Qgis::AnnotationItemEditOperationResult::Success:
              QgsProject::instance()->setDirty( true );
              mRefreshSelectedItemAfterRedraw = true;
              break;

            case Qgis::AnnotationItemEditOperationResult::Invalid:
            case Qgis::AnnotationItemEditOperationResult::ItemCleared:
              break;
          }
        }

        mTemporaryRubberBand.reset();
        mHoveredItemNodeRubberBands.clear();
        mHoveredItemNodes.clear();
        mTemporaryRubberBand.reset();
        mCurrentAction = Action::NoAction;
        setCursor( Qt::ArrowCursor );
      }
      break;
    }
  }
}

void QgsMapToolModifyAnnotation::canvasDoubleClickEvent( QgsMapMouseEvent *event )
{
  switch ( mCurrentAction )
  {
    case Action::NoAction:
    case Action::MoveItem:
    {
      if ( event->button() != Qt::LeftButton )
        return;

      mCurrentAction = Action::NoAction;
      if ( mHoveredItemId == mSelectedItemId && mHoveredItemLayerId == mSelectedItemLayerId )
      {
        // double-click on selected item => add node
        if ( QgsAnnotationLayer *layer = annotationLayerFromId( mSelectedItemLayerId ) )
        {
          const QgsPointXY layerPoint = toLayerCoordinates( layer, event->mapPoint() );
          QgsAnnotationItemEditOperationAddNode operation( mSelectedItemId, QgsPoint( layerPoint ) );
          switch ( layer->applyEdit( &operation ) )
          {
            case Qgis::AnnotationItemEditOperationResult::Success:
              QgsProject::instance()->setDirty( true );
              mRefreshSelectedItemAfterRedraw = true;
              break;

            case Qgis::AnnotationItemEditOperationResult::Invalid:
            case Qgis::AnnotationItemEditOperationResult::ItemCleared:
              break;
          }
        }
      }
      else
      {
        // press is on a different item to selected item => select that item
        mSelectedItemId = mHoveredItemId;
        mSelectedItemLayerId = mHoveredItemLayerId;

        if ( !mSelectedRubberBand )
          createSelectedItemBand();

        mSelectedRubberBand->copyPointsFrom( mHoverRubberBand );
        mSelectedRubberBand->show();

        setCursor( Qt::OpenHandCursor );

        emit itemSelected( annotationLayerFromId( mSelectedItemLayerId ), mSelectedItemId );
      }
      break;
    }

    case Action::MoveNode:
      break;
  }
}

void QgsMapToolModifyAnnotation::keyPressEvent( QKeyEvent *event )
{
  switch ( mCurrentAction )
  {
    case Action::NoAction:
    {
      if ( event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete )
      {
        QgsAnnotationLayer *layer = annotationLayerFromId( mSelectedItemLayerId );
        if ( !layer || mSelectedItemId.isEmpty() )
          return;

        layer->removeItem( mSelectedItemId );
        clearSelectedItem();
        clearHoveredItem();
        event->ignore(); // disable default shortcut handling
      }
      else if ( event->key() == Qt::Key_Left
                || event->key() == Qt::Key_Right
                || event->key() == Qt::Key_Up
                || event->key() == Qt::Key_Down )
      {
        QgsAnnotationLayer *layer = annotationLayerFromId( mSelectedItemLayerId );
        if ( !layer )
          return;

        const QSizeF deltaLayerCoordinates = deltaForKeyEvent( layer, mSelectedRubberBand->asGeometry().centroid().asPoint(), event );

        QgsAnnotationItemEditOperationTranslateItem operation( mSelectedItemId, deltaLayerCoordinates.width(), deltaLayerCoordinates.height() );
        switch ( layer->applyEdit( &operation ) )
        {
          case Qgis::AnnotationItemEditOperationResult::Success:
            QgsProject::instance()->setDirty( true );
            mRefreshSelectedItemAfterRedraw = true;
            break;
          case Qgis::AnnotationItemEditOperationResult::Invalid:
          case Qgis::AnnotationItemEditOperationResult::ItemCleared:
            break;
        }
        event->ignore(); // disable default shortcut handling (move map)
      }
      break;
    }

    case Action::MoveNode:
    {
      if ( event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace )
      {
        if ( QgsAnnotationLayer *layer = annotationLayerFromId( mSelectedItemLayerId ) )
        {
          QgsAnnotationItemEditOperationDeleteNode operation( mSelectedItemId, mTargetNode.id(), QgsPoint( mTargetNode.point() ) );
          switch ( layer->applyEdit( &operation ) )
          {
            case Qgis::AnnotationItemEditOperationResult::Success:
              QgsProject::instance()->setDirty( true );
              mRefreshSelectedItemAfterRedraw = true;
              break;
            case Qgis::AnnotationItemEditOperationResult::Invalid:
              break;
            case Qgis::AnnotationItemEditOperationResult::ItemCleared:
              QgsProject::instance()->setDirty( true );
              break;
          }
        }

        mTemporaryRubberBand.reset();
        mHoveredItemNodeRubberBands.clear();
        mHoveredItemNodes.clear();
        mTemporaryRubberBand.reset();
        mCurrentAction = Action::NoAction;
        setCursor( Qt::ArrowCursor );
        event->ignore(); // disable default shortcut handling (delete vector feature)
        break;
      }
      FALLTHROUGH
    }

    case Action::MoveItem:
    {
      // warning -- fallthrough above!
      if ( event->key() == Qt::Key_Escape )
      {
        mCurrentAction = Action::NoAction;
        mTemporaryRubberBand.reset();
        if ( mSelectedRubberBand )
        {
          mSelectedRubberBand->setTranslationOffset( 0, 0 );
          mSelectedRubberBand->show();
        }
        mHoveredItemNodeRubberBands.clear();

        setCursor( Qt::ArrowCursor );
      }
      break;
    }
  }
}

void QgsMapToolModifyAnnotation::onCanvasRefreshed()
{
  if ( mRefreshSelectedItemAfterRedraw )
  {
    const QgsRenderedItemResults *renderedItemResults = canvas()->renderedItemResults( false );
    if ( !renderedItemResults )
    {
      return;
    }

    const QList<QgsRenderedItemDetails *> items = renderedItemResults->renderedItems();
    auto it = std::find_if( items.begin(), items.end(), [this]( const QgsRenderedItemDetails * item )
    {
      if ( const QgsRenderedAnnotationItemDetails *annotationItem = dynamic_cast< const QgsRenderedAnnotationItemDetails *>( item ) )
      {
        if ( annotationItem->itemId() == mSelectedItemId && annotationItem->layerId() == mSelectedItemLayerId )
          return true;
      }
      return false;
    } );
    if ( it != items.end() )
    {
      const QgsRectangle itemBounds = ( *it )->boundingBox();

      setHoveredItem( dynamic_cast< const QgsRenderedAnnotationItemDetails *>( *it ), itemBounds );
      if ( !mSelectedRubberBand )
        createSelectedItemBand();

      mSelectedRubberBand->copyPointsFrom( mHoverRubberBand );
      mSelectedRubberBand->show();
    }
  }
  mRefreshSelectedItemAfterRedraw = false;
}

void QgsMapToolModifyAnnotation::setHoveredItem( const QgsRenderedAnnotationItemDetails *item, const QgsRectangle &itemMapBounds )
{
  mHoveredItemNodeRubberBands.clear();
  if ( mHoveredNodeRubberBand )
    mHoveredNodeRubberBand->hide();
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

QSizeF QgsMapToolModifyAnnotation::deltaForKeyEvent( QgsAnnotationLayer *layer, const QgsPointXY &originalCanvasPoint, QKeyEvent *event )
{
  const double canvasDpi = canvas()->window()->windowHandle()->screen()->physicalDotsPerInch();

  // increment used for cursor key item movement
  double incrementPixels = 0.0;
  if ( event->modifiers() & Qt::ShiftModifier )
  {
    //holding shift while pressing cursor keys results in a big step - 20 mm
    incrementPixels = 20.0 / 25.4 * canvasDpi;
  }
  else if ( event->modifiers() & Qt::AltModifier )
  {
    //holding alt while pressing cursor keys results in a 1 pixel step
    incrementPixels = 1;
  }
  else
  {
    // 5 mm
    incrementPixels = 5.0 / 25.4 * canvasDpi;
  }

  double deltaXPixels = 0;
  double deltaYPixels = 0;
  switch ( event->key() )
  {
    case Qt::Key_Left:
      deltaXPixels = -incrementPixels;
      break;
    case Qt::Key_Right:
      deltaXPixels = incrementPixels;
      break;
    case Qt::Key_Up:
      deltaYPixels = -incrementPixels;
      break;
    case Qt::Key_Down:
      deltaYPixels = incrementPixels;
      break;
    default:
      break;
  }

  const QgsPointXY beforeMoveMapPoint = canvas()->getCoordinateTransform()->toMapCoordinates( originalCanvasPoint.x(), originalCanvasPoint.y() );
  const QgsPointXY beforeMoveLayerPoint = toLayerCoordinates( layer, beforeMoveMapPoint );

  const QgsPointXY afterMoveCanvasPoint( originalCanvasPoint.x() + deltaXPixels, originalCanvasPoint.y() + deltaYPixels );
  const QgsPointXY afterMoveMapPoint = canvas()->getCoordinateTransform()->toMapCoordinates( afterMoveCanvasPoint.x(), afterMoveCanvasPoint.y() );
  const QgsPointXY afterMoveLayerPoint = toLayerCoordinates( layer, afterMoveMapPoint );

  return QSizeF( afterMoveLayerPoint.x() - beforeMoveLayerPoint.x(), afterMoveLayerPoint.y() - beforeMoveLayerPoint.y() );
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

  setCursor( Qt::ArrowCursor );
}

void QgsMapToolModifyAnnotation::clearSelectedItem()
{
  if ( mSelectedRubberBand )
    mSelectedRubberBand->hide();

  const bool hadSelection = !mSelectedItemId.isEmpty();
  mSelectedItemId.clear();
  mSelectedItemLayerId.clear();
  if ( hadSelection )
    emit selectionCleared();
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

