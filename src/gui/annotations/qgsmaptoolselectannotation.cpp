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
#include "moc_qgsmaptoolselectannotation.cpp"
#include "qgsapplication.h"
#include "qgsrubberband.h"
#include "qgsmapmouseevent.h"
#include "qgsmapcanvas.h"
#include "qgsrendereditemresults.h"
#include "qgsrendereditemdetails.h"
#include "qgsannotationlayer.h"
#include "qgsproject.h"
#include "qgsrenderedannotationitemdetails.h"
#include "qgsmaptoolselectannotationmousehandles.h"
#include "qgsannotationitem.h"
#include "qgsannotationitemnode.h"
#include "qgsannotationitemeditoperation.h"
#include "qgssnapindicator.h"

#include <QGraphicsSceneHoverEvent>
#include <QTransform>
#include <QWindow>
#include <QScreen>


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

  mNeedsUpdatedBoundingBox = true;
}

QgsAnnotationItemRubberBand::~QgsAnnotationItemRubberBand()
{
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
  addPoint( QgsPointXY( boundingBox.xMinimum(), boundingBox.yMinimum() ) );
  addPoint( QgsPointXY( boundingBox.xMaximum(), boundingBox.yMinimum() ) );
  addPoint( QgsPointXY( boundingBox.xMaximum(), boundingBox.yMaximum() ) );
  addPoint( QgsPointXY( boundingBox.xMinimum(), boundingBox.yMaximum() ) );
  addPoint( QgsPointXY( boundingBox.xMinimum(), boundingBox.yMinimum() ) );

  show();
  setSelected( true );
}

void QgsAnnotationItemRubberBand::attemptMoveBy( double deltaX, double deltaY )
{
  if ( QgsAnnotationItem *annotationItem = item() )
  {
    const double mupp = mMapCanvas->mapSettings().mapUnitsPerPixel();
    QgsVector translation( deltaX * mupp, -deltaY * mupp );
    QgsAnnotationLayer *annotationLayer = layer();

    QgsAnnotationItemEditContext context;
    context.setCurrentItemBounds( mBoundingBox );
    context.setRenderContext( QgsRenderContext::fromMapSettings( mMapCanvas->mapSettings() ) );

    const QList<QgsAnnotationItemNode> itemNodes = annotationItem->nodesV2( context );
    for ( const QgsAnnotationItemNode &node : itemNodes )
    {
      QgsPointXY mapPoint = mMapCanvas->mapSettings().layerToMapCoordinates( annotationLayer, node.point() );
      mapPoint += translation;
      QgsPointXY modifiedPoint = mMapCanvas->mapSettings().mapToLayerCoordinates( annotationLayer, mapPoint );

      QgsAnnotationItemEditOperationMoveNode operation( mItemId, node.id(), QgsPoint( node.point() ), QgsPoint( modifiedPoint ) );
      switch ( annotationLayer->applyEditV2( &operation, context ) )
      {
        case Qgis::AnnotationItemEditOperationResult::Success:
          QgsProject::instance()->setDirty( true );
          mNeedsUpdatedBoundingBox = true;
          break;

        case Qgis::AnnotationItemEditOperationResult::Invalid:
        case Qgis::AnnotationItemEditOperationResult::ItemCleared:
          break;
      }
    }
    mBoundingBox += translation;
  }
}

void QgsAnnotationItemRubberBand::attemptRotateBy( double deltaDegree )
{
  if ( QgsAnnotationItem *annotationItem = item() )
  {
    QgsAnnotationLayer *annotationLayer = layer();

    const double deltaRadian = -deltaDegree * M_PI / 180;
    const QgsRectangle boundingBox = mMapCanvas->mapSettings().mapToLayerCoordinates( annotationLayer, mBoundingBox );
    const QgsPointXY centroid = boundingBox.center();

    QgsAnnotationItemEditContext context;
    context.setCurrentItemBounds( boundingBox );
    context.setRenderContext( QgsRenderContext::fromMapSettings( mMapCanvas->mapSettings() ) );

    const QList<QgsAnnotationItemNode> itemNodes = annotationItem->nodesV2( context );
    for ( const QgsAnnotationItemNode &node : itemNodes )
    {
      const double translatedX = node.point().x() - centroid.x();
      const double translatedY = node.point().y() - centroid.y();
      const double rotatedX = translatedX * std::cos( deltaRadian ) - translatedY * std::sin( deltaRadian );
      const double rotatedY = translatedX * std::sin( deltaRadian ) + translatedY * std::cos( deltaRadian );
      QgsPointXY modifiedPoint( rotatedX + centroid.x(), rotatedY + centroid.y() );
      QgsAnnotationItemEditOperationMoveNode operation( mItemId, node.id(), QgsPoint( node.point() ), QgsPoint( modifiedPoint ) );
      switch ( annotationLayer->applyEditV2( &operation, context ) )
      {
        case Qgis::AnnotationItemEditOperationResult::Success:
          QgsProject::instance()->setDirty( true );
          mNeedsUpdatedBoundingBox = true;
          break;

        case Qgis::AnnotationItemEditOperationResult::Invalid:
        case Qgis::AnnotationItemEditOperationResult::ItemCleared:
          break;
      }
    }
  }
}

void QgsAnnotationItemRubberBand::attemptSetSceneRect( const QRectF &rect )
{
  const double widthRatio = rect.width() / boundingRect().width();
  const double heightRatio = rect.height() / boundingRect().height();
  const double deltaX = rect.x() - x() + 1;
  const double deltaY = rect.y() - y() + 1;
  attemptMoveBy( deltaX, deltaY );

  if ( QgsAnnotationItem *annotationItem = item() )
  {
    QgsAnnotationLayer *annotationLayer = layer();

    const QgsRectangle boundingBox = mMapCanvas->mapSettings().mapToLayerCoordinates( annotationLayer, mBoundingBox );
    const QgsRectangle modifiedBoundingBox( boundingBox.xMinimum(), boundingBox.yMaximum() - boundingBox.height() * heightRatio, boundingBox.xMinimum() + boundingBox.width() * widthRatio, boundingBox.yMaximum() );

    QgsAnnotationItemEditContext context;
    context.setCurrentItemBounds( boundingBox );
    context.setRenderContext( QgsRenderContext::fromMapSettings( mMapCanvas->mapSettings() ) );

    const QList<QgsAnnotationItemNode> itemNodes = annotationItem->nodesV2( context );
    for ( const QgsAnnotationItemNode &node : itemNodes )
    {
      const double modifiedX = modifiedBoundingBox.xMinimum() + modifiedBoundingBox.width() * ( ( node.point().x() - boundingBox.xMinimum() ) / boundingBox.width() );
      const double modifiedY = modifiedBoundingBox.yMaximum() - modifiedBoundingBox.height() * ( ( boundingBox.yMaximum() - node.point().y() ) / boundingBox.height() );
      QgsPointXY modifiedPoint( modifiedX, modifiedY );
      QgsAnnotationItemEditOperationMoveNode operation( mItemId, node.id(), QgsPoint( node.point() ), QgsPoint( modifiedPoint ) );
      switch ( annotationLayer->applyEditV2( &operation, context ) )
      {
        case Qgis::AnnotationItemEditOperationResult::Success:
          QgsProject::instance()->setDirty( true );
          mNeedsUpdatedBoundingBox = true;
          break;

        case Qgis::AnnotationItemEditOperationResult::Invalid:
        case Qgis::AnnotationItemEditOperationResult::ItemCleared:
          break;
      }
    }
  }
}

bool QgsAnnotationItemRubberBand::operator==( const QgsAnnotationItemRubberBand &other ) const
{
  return mLayerId == other.mLayerId && mItemId == other.mItemId;
}


QgsMapToolSelectAnnotation::QgsMapToolSelectAnnotation( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget )
  : QgsMapToolAdvancedDigitizing( canvas, cadDockWidget )
{
  connect( QgsMapToolSelectAnnotation::canvas(), &QgsMapCanvas::mapCanvasRefreshed, this, &QgsMapToolSelectAnnotation::onCanvasRefreshed );
  connect( this, &QgsMapToolSelectAnnotation::selectedItemsChanged, this, &QgsMapToolSelectAnnotation::updateSelectedItem );
}

QgsMapToolSelectAnnotation::~QgsMapToolSelectAnnotation()
{
  mSelectionRubberBand.reset();
  mMouseHandles.reset();

  qDeleteAll( mSelectedItems );
}

void QgsMapToolSelectAnnotation::activate()
{
  mMouseHandles.reset( new QgsMapToolSelectAnnotationMouseHandles( this, mCanvas ) );

  QgsMapToolAdvancedDigitizing::activate();
}

void QgsMapToolSelectAnnotation::deactivate()
{
  mSelectionRubberBand.reset();
  mMouseHandles.reset();

  qDeleteAll( mSelectedItems );
  mSelectedItems.clear();

  mCopiedItems.clear();
  mCopiedItemsTopLeft = QPointF();

  QgsMapToolAdvancedDigitizing::deactivate();
}

void QgsMapToolSelectAnnotation::cadCanvasMoveEvent( QgsMapMouseEvent *event )
{
  const QPointF scenePos = mCanvas->mapToScene( event->pos() );

  if ( ( event->buttons() == Qt::NoButton ) )
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
  else if ( ( event->buttons() == Qt::LeftButton ) )
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
        mSelectionRubberBand.reset( new QgsRubberBand( mCanvas, Qgis::GeometryType::Polygon ) );
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

  if ( !mSelectedItems.isEmpty() && mMouseHandles->sceneBoundingRect().contains( scenePos ) )
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

    mMouseHandles->setSelected( !mSelectedItems.isEmpty() );
  }
}

void QgsMapToolSelectAnnotation::keyPressEvent( QKeyEvent *event )
{
  if ( mMouseHandles->isDragging() || mMouseHandles->isResizing() || mMouseHandles->isRotating() )
  {
    return;
  }

  if ( mSelectedItems.isEmpty() )
  {
    return;
  }

  if ( event->key() == Qt::Key_C || event->key() == Qt::Key_X )
  {
    mCopiedItems.clear();
    mCopiedItemsTopLeft = toMapCoordinates( QPoint( mMouseHandles->sceneBoundingRect().topLeft().x(), mMouseHandles->sceneBoundingRect().topLeft().y() ) );
    for ( QgsAnnotationItemRubberBand *selectedItem : mSelectedItems )
    {
      mCopiedItems << qMakePair( selectedItem->layerId(), selectedItem->itemId() );
    }
    if ( event->key() == Qt::Key_C )
    {
      event->ignore();
      return;
    }
  }
  else if ( event->key() == Qt::Key_V )
  {
    const QgsPointXY copiedItemsSceneTopLeft = mCanvas->mapSettings().mapToPixel().transform( mCopiedItemsTopLeft );
    const double deltaX = mLastScenePos.x() - copiedItemsSceneTopLeft.x();
    const double deltaY = mLastScenePos.y() - copiedItemsSceneTopLeft.y();
    if ( !mSelectedItems.isEmpty() )
    {
      qDeleteAll( mSelectedItems );
      mSelectedItems.clear();
    }

    for ( const QPair<QString, QString> &copiedItem : mCopiedItems )
    {
      if ( QgsAnnotationItem *annotationItem = annotationItemFromId( copiedItem.first, copiedItem.second ) )
      {
        QgsAnnotationLayer *annotationLayer = annotationLayerFromId( copiedItem.first );
        QString pastedItemId = annotationLayer->addItem( annotationItem->clone() );

        mSelectedItems << new QgsAnnotationItemRubberBand( copiedItem.first, pastedItemId, mCanvas );
        mSelectedItems.last()->attemptMoveBy( deltaX, deltaY );
      }
    }
    emit selectedItemsChanged();
    event->ignore();
    return;
  }

  if ( event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete || event->key() == Qt::Key_X )
  {
    while ( !mSelectedItems.isEmpty() )
    {
      QgsAnnotationItemRubberBand *selectedItem = mSelectedItems.takeLast();
      if ( QgsAnnotationLayer *annotationLayer = selectedItem->layer() )
      {
        annotationLayer->removeItem( selectedItem->itemId() );
      }
      delete selectedItem;
    }
    emit selectedItemsChanged();
    event->ignore();
  }
  else if ( event->key() == Qt::Key_Left
            || event->key() == Qt::Key_Right
            || event->key() == Qt::Key_Up
            || event->key() == Qt::Key_Down )
  {
    const int pixels = event->modifiers() & Qt::ShiftModifier ? 1 : 50;
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

    for ( QgsAnnotationItemRubberBand *selectedItem : mSelectedItems )
    {
      selectedItem->attemptMoveBy( deltaX, deltaY );
    }
    event->ignore();
  }
}

void QgsMapToolSelectAnnotation::onCanvasRefreshed()
{
  const QgsRenderedItemResults *renderedItemResults = canvas()->renderedItemResults( false );
  if ( !renderedItemResults )
  {
    return;
  }

  const QList<QgsRenderedItemDetails *> items = renderedItemResults->renderedItems();
  bool needsSelectedItemsUpdate = false;
  for ( QgsAnnotationItemRubberBand *selectedItem : mSelectedItems )
  {
    if ( selectedItem->needsUpdatedBoundingBox() )
    {
      needsSelectedItemsUpdate = true;

      auto it = std::find_if( items.begin(), items.end(), [this, selectedItem]( const QgsRenderedItemDetails *item ) {
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
        selectedItem->updateBoundingBox( ( *it )->boundingBox() );
      }
    }
  }

  if ( needsSelectedItemsUpdate )
  {
    emit selectedItemsChanged();
  }
}

const QgsRenderedAnnotationItemDetails *QgsMapToolSelectAnnotation::findClosestItemToPoint( const QgsPointXY &mapPoint, const QList<const QgsRenderedAnnotationItemDetails *> &items, QgsRectangle &bounds )
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

QgsAnnotationLayer *QgsMapToolSelectAnnotation::annotationLayerFromId( const QString &layerId )
{
  QgsAnnotationLayer *layer = qobject_cast<QgsAnnotationLayer *>( QgsProject::instance()->mapLayer( layerId ) );
  if ( !layer && layerId == QgsProject::instance()->mainAnnotationLayer()->id() )
  {
    layer = QgsProject::instance()->mainAnnotationLayer();
  }
  return layer;
}

QgsAnnotationItem *QgsMapToolSelectAnnotation::annotationItemFromId( const QString &layerId, const QString &itemId )
{
  QgsAnnotationLayer *layer = annotationLayerFromId( layerId );
  return layer ? layer->item( itemId ) : nullptr;
}

QgsAnnotationItemRubberBand *QgsMapToolSelectAnnotation::annotationItemRubberBandFromId( const QString &layerId, const QString &itemId )
{
  if ( mSelectedItems.isEmpty() )
  {
    return nullptr;
  }

  auto it = std::find_if( mSelectedItems.begin(), mSelectedItems.end(), [&layerId, &itemId]( QgsAnnotationItemRubberBand *item ) { return item->layerId() == layerId && item->itemId() == itemId; } );
  return it != mSelectedItems.end() ? *it : nullptr;
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
    qDeleteAll( mSelectedItems );
    mSelectedItems.clear();
  }
  for ( const QgsRenderedAnnotationItemDetails *item : items )
  {
    QgsAnnotationItem *annotationItem = annotationItemFromId( item->layerId(), item->itemId() );
    if ( annotationItem )
    {
      if ( toggleSelection )
      {
        if ( QgsAnnotationItemRubberBand *preexistingItem = annotationItemRubberBandFromId( item->layerId(), item->itemId() ) )
        {
          delete mSelectedItems.takeAt( mSelectedItems.indexOf( preexistingItem ) );
          continue;
        }
      }
      std::unique_ptr<QgsAnnotationItemRubberBand> rubberband = std::make_unique<QgsAnnotationItemRubberBand>( item->layerId(), item->itemId(), mCanvas );
      rubberband->updateBoundingBox( item->boundingBox() );
      mSelectedItems << rubberband.release();
    }
  }
  emit selectedItemsChanged();
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

  if ( QgsAnnotationItemRubberBand *preexistingItem = annotationItemRubberBandFromId( closestItem->layerId(), closestItem->itemId() ) )
  {
    if ( toggleSelection )
    {
      delete mSelectedItems.takeAt( mSelectedItems.indexOf( preexistingItem ) );
    }
  }
  else
  {
    if ( !toggleSelection )
    {
      qDeleteAll( mSelectedItems );
      mSelectedItems.clear();
    }
    std::unique_ptr<QgsAnnotationItemRubberBand> rubberband = std::make_unique<QgsAnnotationItemRubberBand>( closestItem->layerId(), closestItem->itemId(), mCanvas );
    rubberband->updateBoundingBox( closestItem->boundingBox() );
    mSelectedItems << rubberband.release();
  }
  emit selectedItemsChanged();
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
  const bool hadSelection = !mSelectedItems.isEmpty();
  qDeleteAll( mSelectedItems );
  mSelectedItems.clear();
  if ( hadSelection )
  {
    emit selectedItemsChanged();
  }
}
