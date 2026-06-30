/***************************************************************************
    qgsmaptoolselectannotation.cpp
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

#include "qgsmaptoolselectannotation.h"

#include <algorithm>
#include <limits>

#include "qgsannotationitem.h"
#include "qgsannotationitemeditoperation.h"
#include "qgsannotationitemnode.h"
#include "qgsannotationlayer.h"
#include "qgsannotationrectitem.h"
#include "qgsapplication.h"
#include "qgscoordinatetransform.h"
#include "qgsgeometry.h"
#include "qgsmapcanvas.h"
#include "qgsmaptoolselectannotationmousehandles.h"
#include "qgsmaptopixel.h"
#include "qgsproject.h"
#include "qgsrendercontext.h"
#include "qgsrenderedannotationitemdetails.h"
#include "qgsrendereditemdetails.h"
#include "qgsrendereditemresults.h"
#include "qgsrubberband.h"
#include "qgssnapindicator.h"

#include <QGraphicsSceneHoverEvent>
#include <QKeySequence>
#include <QScreen>
#include <QTransform>
#include <QWindow>

#include "moc_qgsmaptoolselectannotation.cpp"

QgsAnnotationItemRubberBand::QgsAnnotationItemRubberBand( const QString &layerId, const QString &itemId, QgsMapCanvas *canvas )
  : QgsRubberBand( canvas, Qgis::GeometryType::Line )
  , mLayerId( layerId )
  , mItemId( itemId )
{
  setFlags( flags() | QGraphicsItem::ItemIsSelectable );

  setWidth( canvas->fontMetrics().xHeight() * .2 );
  setSecondaryStrokeColor( QColor( 255, 255, 255, 100 ) );
  setColor( QColor( 50, 50, 50, 200 ) );
  setZValue( 10 );
  setSelected( false );
}

QgsAnnotationLayer *QgsAnnotationItemRubberBand::layer() const
{
  QgsAnnotationLayer *lyr = qobject_cast<QgsAnnotationLayer *>( QgsProject::instance()->mapLayer( mLayerId ) );
  if ( !lyr && mLayerId == QgsProject::instance()->mainAnnotationLayer()->id() )
  {
    lyr = QgsProject::instance()->mainAnnotationLayer();
  }
  return lyr;
}

QgsAnnotationItem *QgsAnnotationItemRubberBand::item() const
{
  QgsAnnotationLayer *lyr = layer();
  return lyr ? lyr->item( mItemId ) : nullptr;
}

void QgsAnnotationItemRubberBand::updateBoundingBox( const QgsRectangle &boundingBox )
{
  mBoundingBox = boundingBox;
  QgsAnnotationItem *annotationItem = item();
  mNeedsUpdatedBoundingBox = annotationItem && ( annotationItem->flags() & Qgis::AnnotationItemFlag::ScaleDependentBoundingBox );

  reset( Qgis::GeometryType::Line );

  const QgsMapToPixel *transform = mMapCanvas->getCoordinateTransform();

  const QgsAnnotationRectItem *rectItem = dynamic_cast< const QgsAnnotationRectItem * >( annotationItem );
  const double mapRotation = mMapCanvas->mapSettings().rotation();
  const double frameRotation = rectItem ? rectItem->appliedRotation( mapRotation ) - mapRotation : 0;

  if ( rectItem && !qgsDoubleNear( frameRotation, 0 ) )
  {
    double minX = std::numeric_limits<double>::max();
    double minY = std::numeric_limits<double>::max();
    double maxX = std::numeric_limits<double>::lowest();
    double maxY = std::numeric_limits<double>::lowest();
    auto includePixel = [&]( const QPointF &p ) {
      minX = std::min( minX, p.x() );
      minY = std::min( minY, p.y() );
      maxX = std::max( maxX, p.x() );
      maxY = std::max( maxY, p.y() );
    };

    // rotated item corners, as drawn on screen
    if ( QgsAnnotationLayer *lyr = layer() )
    {
      const QgsCoordinateTransform layerToMap( lyr->crs(), mMapCanvas->mapSettings().destinationCrs(), mMapCanvas->mapSettings().transformContext() );
      const QgsRenderContext renderContext = QgsRenderContext::fromMapSettings( mMapCanvas->mapSettings() );
      const QPolygonF corners = rectItem->rotatedBoundsGeometry( rectItem->bounds(), renderContext ).asQPolygonF();
      for ( const QPointF &corner : corners )
      {
        QgsPointXY mapPoint( corner.x(), corner.y() );
        try
        {
          mapPoint = layerToMap.transform( mapPoint );
        }
        catch ( QgsCsException & )
        {}
        includePixel( toCanvasCoordinates( mapPoint ) );
      }
    }

    // callout anchor and everything else reported by the incoming bounding box
    const QgsPointXY bboxCorners[4] = {
      QgsPointXY( boundingBox.xMinimum(), boundingBox.yMinimum() ),
      QgsPointXY( boundingBox.xMaximum(), boundingBox.yMinimum() ),
      QgsPointXY( boundingBox.xMaximum(), boundingBox.yMaximum() ),
      QgsPointXY( boundingBox.xMinimum(), boundingBox.yMaximum() ),
    };
    for ( const QgsPointXY &corner : bboxCorners )
      includePixel( toCanvasCoordinates( corner ) );

    const QgsPointXY topLeft = transform->toMapCoordinates( minX, minY );
    const QgsPointXY topRight = transform->toMapCoordinates( maxX, minY );
    const QgsPointXY bottomRight = transform->toMapCoordinates( maxX, maxY );
    const QgsPointXY bottomLeft = transform->toMapCoordinates( minX, maxY );
    addPoint( topLeft );
    addPoint( topRight );
    addPoint( bottomRight );
    addPoint( bottomLeft );
    addPoint( topLeft );
  }
  else
  {
    // Build the rubber band as a screen-aligned rectangle matching the item's
    // on-screen size.
    const QPointF centerCanvas = toCanvasCoordinates( boundingBox.center() );
    const double mupp = mMapCanvas->mapSettings().mapUnitsPerPixel();
    const double halfWidth = 0.5 * boundingBox.width() / mupp;
    const double halfHeight = 0.5 * boundingBox.height() / mupp;
    const QgsPointXY topLeft = transform->toMapCoordinates( centerCanvas.x() - halfWidth, centerCanvas.y() - halfHeight );
    const QgsPointXY topRight = transform->toMapCoordinates( centerCanvas.x() + halfWidth, centerCanvas.y() - halfHeight );
    const QgsPointXY bottomRight = transform->toMapCoordinates( centerCanvas.x() + halfWidth, centerCanvas.y() + halfHeight );
    const QgsPointXY bottomLeft = transform->toMapCoordinates( centerCanvas.x() - halfWidth, centerCanvas.y() + halfHeight );
    addPoint( topLeft );
    addPoint( topRight );
    addPoint( bottomRight );
    addPoint( bottomLeft );
    addPoint( topLeft );
  }

  show();
  setSelected( true );
}

void QgsAnnotationItemRubberBand::setNeedsUpdatedBoundingBox( bool needsUpdatedBoundingBox )
{
  mNeedsUpdatedBoundingBox = needsUpdatedBoundingBox;
}


QgsMapToolSelectAnnotation::QgsMapToolSelectAnnotation( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget )
  : QgsAnnotationMapTool( canvas, cadDockWidget )
{
  connect( QgsMapToolSelectAnnotation::canvas(), &QgsMapCanvas::mapCanvasRefreshed, this, &QgsMapToolSelectAnnotation::onCanvasRefreshed );

  connect( QgsMapToolSelectAnnotation::canvas(), &QgsMapCanvas::rotationChanged, this, [this] {
    for ( std::unique_ptr<QgsAnnotationItemRubberBand> &selectedItem : mSelectedItems )
      selectedItem->updateBoundingBox( selectedItem->boundingBox() );
  } );

  setAdvancedDigitizingAllowed( false );
}

QgsMapToolSelectAnnotation::~QgsMapToolSelectAnnotation()
{
  mSelectionRubberBand.reset();
  mMouseHandles.reset();
}

void QgsMapToolSelectAnnotation::activate()
{
  mMouseHandles = make_qobject_unique<QgsMapToolSelectAnnotationMouseHandles>( this, mCanvas );

  QgsMapToolAdvancedDigitizing::activate();
}

void QgsMapToolSelectAnnotation::deactivate()
{
  mSelectionRubberBand.reset();
  mMouseHandles.reset();

  mSelectedItems.clear();

  mCopiedItems.clear();
  mCopiedItemsTopLeft = QPointF();

  QgsMapToolAdvancedDigitizing::deactivate();
}

void QgsMapToolSelectAnnotation::cadCanvasMoveEvent( QgsMapMouseEvent *event )
{
  const QPointF scenePos = mCanvas->mapToScene( event->pos() );

  if ( event->buttons() == Qt::NoButton )
  {
    if ( mMouseHandles->isDragging() )
    {
      QGraphicsSceneMouseEvent forwardedEvent( QEvent::GraphicsSceneMouseMove );
      forwardedEvent.setPos( mMouseHandles->mapFromScene( scenePos ) );
      forwardedEvent.setScenePos( scenePos );
      forwardedEvent.setLastScenePos( mLastScenePos );
      forwardedEvent.setButton( Qt::LeftButton );
      mMouseHandles->mouseMoveEvent( &forwardedEvent );
    }
    else
    {
      if ( mMouseHandles->sceneBoundingRect().contains( scenePos ) )
      {
        QGraphicsSceneHoverEvent forwardedEvent( QEvent::GraphicsSceneHoverMove );
        forwardedEvent.setPos( mMouseHandles->mapFromScene( scenePos ) );
        forwardedEvent.setScenePos( scenePos );
        mMouseHandles->hoverMoveEvent( &forwardedEvent );
        mHoveringMouseHandles = true;
      }
      else if ( mHoveringMouseHandles )
      {
        QGraphicsSceneHoverEvent forwardedEvent( QEvent::GraphicsSceneHoverLeave );
        forwardedEvent.setPos( mMouseHandles->mapFromScene( scenePos ) );
        forwardedEvent.setScenePos( scenePos );
        mMouseHandles->hoverMoveEvent( &forwardedEvent );
        mHoveringMouseHandles = false;
      }
    }
  }
  else if ( event->buttons() == Qt::LeftButton )
  {
    if ( mMouseHandles->shouldBlockEvent( event ) )
    {
      QGraphicsSceneMouseEvent forwardedEvent( QEvent::GraphicsSceneMouseMove );
      forwardedEvent.setPos( mMouseHandles->mapFromScene( scenePos ) );
      forwardedEvent.setScenePos( scenePos );
      forwardedEvent.setLastScenePos( mLastScenePos );
      forwardedEvent.setButton( Qt::LeftButton );
      mMouseHandles->mouseMoveEvent( &forwardedEvent );
    }
    else
    {
      if ( !mDragging )
      {
        mDragging = true;
        mSelectionRubberBand = make_qobject_unique<QgsRubberBand>( mCanvas, Qgis::GeometryType::Polygon );
        QColor color( Qt::blue );
        color.setAlpha( 63 );
        mSelectionRubberBand->setColor( color );
        mSelectionRect.setTopLeft( event->pos() );
      }

      mSelectionRect.setBottomRight( event->pos() );
      if ( mSelectionRubberBand )
      {
        mSelectionRubberBand->setToCanvasRectangle( mSelectionRect );
        mSelectionRubberBand->show();
      }
    }
  }

  mLastScenePos = scenePos;
}

void QgsMapToolSelectAnnotation::cadCanvasPressEvent( QgsMapMouseEvent *event )
{
  if ( event->button() != Qt::LeftButton )
  {
    return;
  }

  QPointF scenePos = mCanvas->mapToScene( event->pos() );
  const bool toggleSelection = event->modifiers() & Qt::ShiftModifier;

  if ( !toggleSelection && !mSelectedItems.empty() && mMouseHandles->sceneBoundingRect().contains( scenePos ) )
  {
    QGraphicsSceneMouseEvent forwardedEvent( QEvent::GraphicsSceneMousePress );
    forwardedEvent.setPos( mMouseHandles->mapFromScene( scenePos ) );
    forwardedEvent.setScenePos( scenePos );
    forwardedEvent.setLastScenePos( mLastScenePos );
    forwardedEvent.setButton( Qt::LeftButton );
    mMouseHandles->mousePressEvent( &forwardedEvent );
  }
  else
  {
    mSelectionRect.setTopLeft( event->pos() );
    mSelectionRect.setBottomRight( event->pos() );
  }

  mLastScenePos = scenePos;
}

void QgsMapToolSelectAnnotation::cadCanvasReleaseEvent( QgsMapMouseEvent *event )
{
  if ( event->button() != Qt::LeftButton )
  {
    return;
  }

  QPointF scenePos = mCanvas->mapToScene( event->pos() );

  if ( mMouseHandles->shouldBlockEvent( event ) )
  {
    QGraphicsSceneMouseEvent forwardedEvent( QEvent::GraphicsSceneMouseRelease );
    forwardedEvent.setPos( mMouseHandles->mapFromScene( scenePos ) );
    forwardedEvent.setScenePos( scenePos );
    forwardedEvent.setLastScenePos( mLastScenePos );
    forwardedEvent.setButton( Qt::LeftButton );
    mMouseHandles->mouseReleaseEvent( &forwardedEvent );
  }
  else
  {
    if ( mCanceled )
    {
      mCanceled = false;
      mDragging = false;
      return;
    }

    if ( mDragging )
    {
      mDragging = false;
      mSelectionRubberBand.reset();

      const QgsPointXY topLeft = toMapCoordinates( mSelectionRect.topLeft() );
      const QgsPointXY bottomRight = toMapCoordinates( mSelectionRect.bottomRight() );
      const QgsRectangle searchRect( topLeft, bottomRight );

      setSelectedItemsFromRect( searchRect, ( event->modifiers() & Qt::ShiftModifier ) );
    }
    else
    {
      setSelectedItemFromPoint( event->mapPoint(), ( event->modifiers() & Qt::ShiftModifier ) );
    }

    mMouseHandles->setSelected( !mSelectedItems.empty() );
  }
}

void QgsMapToolSelectAnnotation::keyPressEvent( QKeyEvent *event )
{
  if ( mMouseHandles->isDragging() || mMouseHandles->isResizing() || mMouseHandles->isRotating() )
  {
    return;
  }

  if ( mSelectedItems.empty() )
  {
    return;
  }

  if ( event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete )
  {
    while ( !mSelectedItems.empty() )
    {
      if ( QgsAnnotationLayer *annotationLayer = mSelectedItems.back()->layer() )
      {
        annotationLayer->removeItem( mSelectedItems.back()->itemId() );
      }
      mSelectedItems.pop_back();
    }
    emit selectedItemsChanged();
    updateSelectedItem();
    event->ignore();
  }
  else if ( event->key() == Qt::Key_Left || event->key() == Qt::Key_Right || event->key() == Qt::Key_Up || event->key() == Qt::Key_Down )
  {
    const int pixels = ( event->modifiers() & Qt::ShiftModifier ) ? 1 : 50;
    int deltaX = 0;
    int deltaY = 0;
    if ( event->key() == Qt::Key_Up )
    {
      deltaY = -pixels;
    }
    else if ( event->key() == Qt::Key_Down )
    {
      deltaY = pixels;
    }
    else if ( event->key() == Qt::Key_Left )
    {
      deltaX = -pixels;
    }
    else if ( event->key() == Qt::Key_Right )
    {
      deltaX = pixels;
    }

    for ( std::unique_ptr<QgsAnnotationItemRubberBand> &selectedItem : mSelectedItems )
    {
      attemptMoveBy( selectedItem.get(), deltaX, deltaY );
    }
    event->ignore();
  }
}

bool QgsMapToolSelectAnnotation::shortcutEvent( QKeyEvent *event )
{
  if ( mMouseHandles->isDragging() || mMouseHandles->isResizing() || mMouseHandles->isRotating() )
  {
    return true;
  }
  // NOLINTBEGIN(bugprone-narrowing-conversions)
  QKeySequence keySequence( event->key() | event->modifiers() );
  // NOLINTEND(bugprone-narrowing-conversions)
  if ( keySequence == QKeySequence::Copy || keySequence == QKeySequence::Cut )
  {
    mCopiedItems.clear();
    mCopiedItemsTopLeft = toMapCoordinates( QPoint( mMouseHandles->sceneBoundingRect().topLeft().x(), mMouseHandles->sceneBoundingRect().topLeft().y() ) );
    for ( std::unique_ptr<QgsAnnotationItemRubberBand> &selectedItem : mSelectedItems )
    {
      if ( QgsAnnotationItem *annotationItem = selectedItem->item() )
      {
        std::unique_ptr<QgsAnnotationItem> copiedItem;
        copiedItem.reset( annotationItem->clone() );
        mCopiedItems.push_back( std::move( copiedItem ) );
      }
    }

    if ( keySequence == QKeySequence::Cut )
    {
      while ( !mSelectedItems.empty() )
      {
        if ( QgsAnnotationLayer *annotationLayer = mSelectedItems.back()->layer() )
        {
          annotationLayer->removeItem( mSelectedItems.back()->itemId() );
        }
        mSelectedItems.pop_back();
      }
      emit selectedItemsChanged();
    }

    return true;
  }
  else if ( keySequence == QKeySequence::Paste )
  {
    const QgsPointXY copiedItemsSceneTopLeft = mCanvas->mapSettings().mapToPixel().transform( mCopiedItemsTopLeft );
    const double deltaX = mLastScenePos.x() - copiedItemsSceneTopLeft.x();
    const double deltaY = mLastScenePos.y() - copiedItemsSceneTopLeft.y();
    if ( !mSelectedItems.empty() )
    {
      mSelectedItems.clear();
    }

    for ( std::unique_ptr<QgsAnnotationItem> &copiedItem : mCopiedItems )
    {
      QgsAnnotationLayer *annotationLayer = dynamic_cast<QgsAnnotationLayer *>( layer() );
      if ( !annotationLayer )
      {
        annotationLayer = QgsProject::instance()->mainAnnotationLayer();
      }
      QString pastedItemId = annotationLayer->addItem( copiedItem->clone() );
      mSelectedItems.push_back( std::make_unique<QgsAnnotationItemRubberBand>( annotationLayer->id(), pastedItemId, mCanvas ) );
      attemptMoveBy( mSelectedItems.back().get(), deltaX, deltaY );
      mSelectedItems.back().get()->setNeedsUpdatedBoundingBox( true );
    }
    emit selectedItemsChanged();

    return true;
  }

  return false;
}

QList<QgsAnnotationItemRubberBand *> QgsMapToolSelectAnnotation::selectedItems() const
{
  QList<QgsAnnotationItemRubberBand *> items;
  for ( const std::unique_ptr<QgsAnnotationItemRubberBand> &selectedItem : mSelectedItems )
  {
    if ( !selectedItem.get()->boundingBox().isEmpty() )
    {
      items << selectedItem.get();
    }
  }
  return items;
}

void QgsMapToolSelectAnnotation::onCanvasRefreshed()
{
  const QgsRenderedItemResults *renderedItemResults = canvas()->renderedItemResults( false );
  if ( !renderedItemResults )
  {
    return;
  }

  const bool interactiveOperation = mMouseHandles && ( mMouseHandles->isDragging() || mMouseHandles->isResizing() || mMouseHandles->isRotating() );

  const QList<QgsRenderedItemDetails *> items = renderedItemResults->renderedItems();
  bool needsSelectedItemsUpdate = false;
  for ( std::unique_ptr<QgsAnnotationItemRubberBand> &selectedItem : mSelectedItems )
  {
    if ( interactiveOperation && !selectedItem->needsUpdatedBoundingBox() )
      continue;

    auto it = std::find_if( items.begin(), items.end(), [&selectedItem]( const QgsRenderedItemDetails *item ) {
      if ( const QgsRenderedAnnotationItemDetails *annotationItem = dynamic_cast<const QgsRenderedAnnotationItemDetails *>( item ) )
      {
        if ( annotationItem->itemId() == selectedItem->itemId() && annotationItem->layerId() == selectedItem->layerId() )
        {
          return true;
        }
      }
      return false;
    } );

    if ( it != items.end() )
    {
      needsSelectedItemsUpdate = true;
      selectedItem->updateBoundingBox( ( *it )->boundingBox() );
    }
  }

  if ( needsSelectedItemsUpdate )
  {
    emit selectedItemsChanged();
  }
}

long long QgsMapToolSelectAnnotation::annotationItemRubberBandIndexFromId( const QString &layerId, const QString &itemId )
{
  if ( mSelectedItems.empty() )
  {
    return -1;
  }

  auto it = std::find_if( mSelectedItems.begin(), mSelectedItems.end(), [&layerId, &itemId]( auto &item ) { return item->layerId() == layerId && item->itemId() == itemId; } );
  return it != mSelectedItems.end() ? std::distance( mSelectedItems.begin(), it ) : -1;
}

void QgsMapToolSelectAnnotation::setSelectedItemsFromRect( const QgsRectangle &mapRect, bool toggleSelection )
{
  const QgsRenderedItemResults *renderedItemResults = canvas()->renderedItemResults( false );
  if ( !renderedItemResults )
  {
    if ( !toggleSelection )
    {
      clearSelectedItems();
    }
    return;
  }

  const QList<const QgsRenderedAnnotationItemDetails *> items = renderedItemResults->renderedAnnotationItemsInBounds( mapRect );
  if ( items.empty() )
  {
    if ( !toggleSelection )
    {
      clearSelectedItems();
    }
    return;
  }

  if ( !toggleSelection )
  {
    mSelectedItems.clear();
  }
  for ( const QgsRenderedAnnotationItemDetails *item : items )
  {
    QgsAnnotationItem *annotationItem = annotationItemFromId( item->layerId(), item->itemId() );
    if ( annotationItem )
    {
      if ( toggleSelection )
      {
        long long index = annotationItemRubberBandIndexFromId( item->layerId(), item->itemId() );
        if ( index >= 0 )
        {
          mSelectedItems.erase( mSelectedItems.begin() + index );
          continue;
        }
      }
      mSelectedItems.push_back( std::make_unique<QgsAnnotationItemRubberBand>( item->layerId(), item->itemId(), mCanvas ) );
      mSelectedItems.back()->updateBoundingBox( item->boundingBox() );
    }
  }
  emit selectedItemsChanged();
  updateSelectedItem();
}

void QgsMapToolSelectAnnotation::setSelectedItemFromPoint( const QgsPointXY &mapPoint, bool toggleSelection )
{
  QgsRectangle searchRect = QgsRectangle( mapPoint.x(), mapPoint.y(), mapPoint.x(), mapPoint.y() );
  searchRect.grow( searchRadiusMU( canvas() ) );

  const QgsRenderedItemResults *renderedItemResults = canvas()->renderedItemResults( false );
  if ( !renderedItemResults )
  {
    clearSelectedItems();
    return;
  }

  const QList<const QgsRenderedAnnotationItemDetails *> items = renderedItemResults->renderedAnnotationItemsInBounds( searchRect );
  if ( items.empty() )
  {
    if ( !toggleSelection )
    {
      clearSelectedItems();
    }
    return;
  }

  QgsRectangle itemBounds;
  const QgsRenderedAnnotationItemDetails *closestItem = findClosestItemToPoint( mapPoint, items, itemBounds );
  if ( !closestItem )
  {
    if ( !toggleSelection )
    {
      clearSelectedItems();
    }
    return;
  }

  long long index = annotationItemRubberBandIndexFromId( closestItem->layerId(), closestItem->itemId() );
  if ( index >= 0 )
  {
    if ( toggleSelection )
    {
      mSelectedItems.erase( mSelectedItems.begin() + index );
    }
  }
  else
  {
    if ( !toggleSelection )
    {
      mSelectedItems.clear();
    }

    mSelectedItems.push_back( std::make_unique<QgsAnnotationItemRubberBand>( closestItem->layerId(), closestItem->itemId(), mCanvas ) );
    mSelectedItems.back()->updateBoundingBox( closestItem->boundingBox() );
  }
  emit selectedItemsChanged();
  updateSelectedItem();
}

void QgsMapToolSelectAnnotation::updateSelectedItem()
{
  if ( mSelectedItems.size() > 1 )
  {
    emit multipleItemsSelected();
  }
  else if ( mSelectedItems.size() == 1 )
  {
    emit singleItemSelected( mSelectedItems[0]->layer(), mSelectedItems[0]->itemId() );
  }
  else
  {
    emit selectionCleared();
  }
}

void QgsMapToolSelectAnnotation::clearSelectedItems()
{
  const bool hadSelection = !mSelectedItems.empty();
  mSelectedItems.clear();

  if ( hadSelection )
  {
    emit selectedItemsChanged();
    updateSelectedItem();
  }
}

void QgsMapToolSelectAnnotation::attemptMoveBy( QgsAnnotationItemRubberBand *annotationItemRubberBand, double deltaX, double deltaY )
{
  if ( QgsAnnotationItem *annotationItem = annotationItemRubberBand->item() )
  {
    QgsRectangle boundingBox = annotationItemRubberBand->boundingBox();
    // take map rotation into account in translation
    const QgsMapToPixel *transform = mCanvas->getCoordinateTransform();
    const QgsPointXY mapOrigin = transform->toMapCoordinates( 0.0, 0.0 );
    const QgsPointXY mapMoved = transform->toMapCoordinates( deltaX, deltaY );
    const QgsVector translation( mapMoved.x() - mapOrigin.x(), mapMoved.y() - mapOrigin.y() );
    QgsAnnotationLayer *annotationLayer = annotationItemRubberBand->layer();

    QgsAnnotationItemEditContext context;
    context.setCurrentItemBounds( boundingBox );
    context.setRenderContext( QgsRenderContext::fromMapSettings( mCanvas->mapSettings() ) );

    const QList<QgsAnnotationItemNode> itemNodes = annotationItem->nodesV2( context );
    for ( const QgsAnnotationItemNode &node : itemNodes )
    {
      QgsPointXY mapPoint = mCanvas->mapSettings().layerToMapCoordinates( annotationLayer, node.point() );
      mapPoint += translation;
      QgsPointXY modifiedPoint = mCanvas->mapSettings().mapToLayerCoordinates( annotationLayer, mapPoint );

      QgsAnnotationItemEditOperationMoveNode operation( annotationItemRubberBand->itemId(), node.id(), QgsPoint( node.point() ), QgsPoint( modifiedPoint ) );
      switch ( annotationLayer->applyEditV2( &operation, context ) )
      {
        case Qgis::AnnotationItemEditOperationResult::Success:
          QgsProject::instance()->setDirty( true );
          annotationItemRubberBand->setNeedsUpdatedBoundingBox( true );
          break;

        case Qgis::AnnotationItemEditOperationResult::Invalid:
        case Qgis::AnnotationItemEditOperationResult::ItemCleared:
          break;
      }
    }
    boundingBox += translation;
    annotationItemRubberBand->updateBoundingBox( boundingBox );
  }
}

void QgsMapToolSelectAnnotation::attemptRotateBy( QgsAnnotationItemRubberBand *annotationItemRubberBand, double deltaDegree )
{
  if ( QgsAnnotationLayer *annotationLayer = annotationItemRubberBand->layer() )
  {
    const QgsRectangle boundingBox = mCanvas->mapSettings().mapToLayerCoordinates( annotationLayer, annotationItemRubberBand->boundingBox() );
    QgsAnnotationItemEditContext context;
    context.setCurrentItemBounds( boundingBox );
    context.setRenderContext( QgsRenderContext::fromMapSettings( mCanvas->mapSettings() ) );

    QgsAnnotationItemEditOperationRotateItem operation( annotationItemRubberBand->itemId(), deltaDegree );
    switch ( annotationLayer->applyEditV2( &operation, context ) )
    {
      case Qgis::AnnotationItemEditOperationResult::Success:
        QgsProject::instance()->setDirty( true );
        annotationItemRubberBand->setNeedsUpdatedBoundingBox( true );
        break;

      case Qgis::AnnotationItemEditOperationResult::Invalid:
      case Qgis::AnnotationItemEditOperationResult::ItemCleared:
        break;
    }
  }
}

void QgsMapToolSelectAnnotation::attemptSetSceneRect( QgsAnnotationItemRubberBand *annotationItemRubberBand, const QRectF &rect )
{
  QgsAnnotationItem *annotationItem = annotationItemRubberBand->item();
  if ( !annotationItem )
    return;

  QgsAnnotationLayer *annotationLayer = annotationItemRubberBand->layer();
  if ( !annotationLayer )
    return;

  // on-screen rotation = the item's own rotation plus the map rotation when
  // the item follows it.
  double rotation = 0;
  if ( const QgsAnnotationRectItem *rectItem = dynamic_cast<const QgsAnnotationRectItem *>( annotationItem ) )
  {
    rotation = rectItem->appliedRotation( mCanvas->mapSettings().rotation() );
  }

  // Rebuild the unrotated (screen-aligned) rectangle so that, after rotating
  // around its center, the dragged corner stays where the user placed it.
  const double w = rect.width();
  const double h = rect.height();
  const QPointF halfDiagonal( w / 2.0, h / 2.0 );
  QTransform itemRotation;
  itemRotation.rotate( rotation );
  const QPointF unrotatedTopLeftScene = rect.topLeft() + itemRotation.map( halfDiagonal ) - halfDiagonal;
  const QRectF newSceneRect( unrotatedTopLeftScene, QSizeF( w, h ) );

  // Derive the bounds from the center and pixel size, which stays correct
  // when the map is rotated.
  const QgsMapToPixel *transform = mCanvas->getCoordinateTransform();
  const QgsRectangle newMapBounds = QgsAnnotationRectItem::boundsFromPixelRect( transform, newSceneRect.center(), w, h );

  const QgsRectangle oldBounds = mCanvas->mapSettings().mapToLayerCoordinates( annotationLayer, annotationItemRubberBand->boundingBox() );
  const QgsRectangle newBounds = mCanvas->mapSettings().mapToLayerCoordinates( annotationLayer, newMapBounds );

  if ( oldBounds.width() == 0 || oldBounds.height() == 0 )
    return;

  QgsAnnotationItemEditContext context;
  context.setCurrentItemBounds( oldBounds );
  context.setRenderContext( QgsRenderContext::fromMapSettings( mCanvas->mapSettings() ) );

  const QList<QgsAnnotationItemNode> itemNodes = annotationItem->nodesV2( context );
  for ( const QgsAnnotationItemNode &node : itemNodes )
  {
    const double modifiedX = newBounds.xMinimum() + newBounds.width() * ( ( node.point().x() - oldBounds.xMinimum() ) / oldBounds.width() );
    const double modifiedY = newBounds.yMaximum() - newBounds.height() * ( ( oldBounds.yMaximum() - node.point().y() ) / oldBounds.height() );
    QgsPointXY modifiedPoint( modifiedX, modifiedY );
    QgsAnnotationItemEditOperationMoveNode operation( annotationItemRubberBand->itemId(), node.id(), QgsPoint( node.point() ), QgsPoint( modifiedPoint ) );
    switch ( annotationLayer->applyEditV2( &operation, context ) )
    {
      case Qgis::AnnotationItemEditOperationResult::Success:
        QgsProject::instance()->setDirty( true );
        annotationItemRubberBand->setNeedsUpdatedBoundingBox( true );
        break;

      case Qgis::AnnotationItemEditOperationResult::Invalid:
      case Qgis::AnnotationItemEditOperationResult::ItemCleared:
        break;
    }
  }

  annotationItemRubberBand->updateBoundingBox( newMapBounds );
}
