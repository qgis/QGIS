/***************************************************************************
                             qgslayoutmousehandles.cpp
                             ------------------------
    begin                : September 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall.dawson@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutmousehandles.h"
#include "qgis.h"
#include "qgslayout.h"
#include "qgslayoutitem.h"
#include "qgslayoututils.h"
#include "qgslayoutview.h"
#include "qgslayoutviewtoolselect.h"
#include "qgslayoutsnapper.h"
#include "qgslayoutitemgroup.h"
#include "qgslayoutundostack.h"
#include "qgslayoutrendercontext.h"
#include <QGraphicsView>
#include <QGraphicsSceneHoverEvent>
#include <QPainter>
#include <QWidget>
#include <limits>


#define CACHE_SIZE_LIMIT 5000

///@cond PRIVATE

QgsLayoutMouseHandles::QgsLayoutMouseHandles( QgsLayout *layout, QgsLayoutView *view )
  : QgsGraphicsViewMouseHandles( view )
  , mLayout( layout )
  , mView( view )
{
  //listen for selection changes, and update handles accordingly
  connect( mLayout, &QGraphicsScene::selectionChanged, this, &QgsLayoutMouseHandles::selectionChanged );

  mHorizontalSnapLine = mView->createSnapLine();
  mHorizontalSnapLine->hide();
  layout->addItem( mHorizontalSnapLine );
  mVerticalSnapLine = mView->createSnapLine();
  mVerticalSnapLine->hide();
  layout->addItem( mVerticalSnapLine );
}

void QgsLayoutMouseHandles::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
  paintInternal( painter, mLayout->renderContext().isPreviewRender(),
                 mLayout->renderContext().boundingBoxesVisible(), true, option, widget );
}

void QgsLayoutMouseHandles::selectionChanged()
{
  //listen out for selected items' size and rotation changed signals
  const QList<QGraphicsItem *> itemList = layout()->items();
  for ( QGraphicsItem *graphicsItem : itemList )
  {
    QgsLayoutItem *item = dynamic_cast<QgsLayoutItem *>( graphicsItem );
    if ( !item )
      continue;

    if ( item->isSelected() )
    {
      connect( item, &QgsLayoutItem::sizePositionChanged, this, &QgsLayoutMouseHandles::selectedItemSizeChanged );
      connect( item, &QgsLayoutItem::rotationChanged, this, &QgsLayoutMouseHandles::selectedItemRotationChanged );
      connect( item, &QgsLayoutItem::frameChanged, this, &QgsLayoutMouseHandles::selectedItemSizeChanged );
      connect( item, &QgsLayoutItem::lockChanged, this, &QgsLayoutMouseHandles::selectedItemSizeChanged );
    }
    else
    {
      disconnect( item, &QgsLayoutItem::sizePositionChanged, this, &QgsLayoutMouseHandles::selectedItemSizeChanged );
      disconnect( item, &QgsLayoutItem::rotationChanged, this, &QgsLayoutMouseHandles::selectedItemRotationChanged );
      disconnect( item, &QgsLayoutItem::frameChanged, this, &QgsLayoutMouseHandles::selectedItemSizeChanged );
      disconnect( item, &QgsLayoutItem::lockChanged, this, &QgsLayoutMouseHandles::selectedItemSizeChanged );
    }
  }

  resetStatusBar();
  mItemCachedImage = QImage();
  updateHandles();
}

void QgsLayoutMouseHandles::setViewportCursor( Qt::CursorShape cursor )
{
  //workaround qt bug #3732 by setting cursor for QGraphicsView viewport,
  //rather then setting it directly here

  if ( qobject_cast< QgsLayoutViewToolSelect *>( mView->tool() ) )
  {
    mView->viewport()->setCursor( cursor );
  }
}

QList<QGraphicsItem *> QgsLayoutMouseHandles::sceneItemsAtPoint( QPointF scenePoint )
{
  QList< QGraphicsItem * > items;
  if ( QgsLayoutViewToolSelect *tool = qobject_cast< QgsLayoutViewToolSelect *>( mView->tool() ) )
  {
    const double searchTolerance = tool->searchToleranceInLayoutUnits();
    const QRectF area( scenePoint.x() - searchTolerance, scenePoint.y() - searchTolerance, 2 * searchTolerance, 2 * searchTolerance );
    items = mLayout->items( area );
  }
  else
  {
    items = mLayout->items( scenePoint );
  }
  items.erase( std::remove_if( items.begin(), items.end(), []( QGraphicsItem * item )
  {
    return !dynamic_cast<QgsLayoutItem *>( item );
  } ), items.end() );

  return items;
}

QList<QGraphicsItem *> QgsLayoutMouseHandles::selectedSceneItems( bool includeLockedItems ) const
{
  QList<QGraphicsItem *> res;
  const QList<QgsLayoutItem *> layoutItems = mLayout->selectedLayoutItems( includeLockedItems );
  res.reserve( layoutItems.size() );
  for ( QgsLayoutItem *item : layoutItems )
    res << item;
  return res;
}

bool QgsLayoutMouseHandles::itemIsLocked( QGraphicsItem *item )
{
  if ( QgsLayoutItem *layoutItem = dynamic_cast<QgsLayoutItem *>( item ) )
    return layoutItem->isLocked();
  else
    return false;
}

bool QgsLayoutMouseHandles::itemIsGroupMember( QGraphicsItem *item )
{
  if ( QgsLayoutItem *layoutItem = dynamic_cast<QgsLayoutItem *>( item ) )
    return layoutItem->isGroupMember();
  else
    return false;
}

QRectF QgsLayoutMouseHandles::itemRect( QGraphicsItem *item ) const
{
  if ( QgsLayoutItem *layoutItem = dynamic_cast<QgsLayoutItem *>( item ) )
    return layoutItem->rectWithFrame();
  else
    return QRectF();
}

QPointF QgsLayoutMouseHandles::snapPoint( QPointF originalPoint, QgsLayoutMouseHandles::SnapGuideMode mode, bool snapHorizontal, bool snapVertical )
{
  bool snapped = false;

  const QList< QGraphicsItem * > selectedItems = selectedSceneItems();
  QList< QGraphicsItem * > itemsToExclude;
  expandItemList( selectedItems, itemsToExclude );

  QList< QgsLayoutItem * > layoutItemsToExclude;
  for ( QGraphicsItem *item : itemsToExclude )
    layoutItemsToExclude << dynamic_cast< QgsLayoutItem * >( item );

  //depending on the mode, we either snap just the single point, or all the bounds of the selection
  QPointF snappedPoint;
  switch ( mode )
  {
    case Item:
      snappedPoint = mLayout->snapper().snapRect( rect().translated( originalPoint ), mView->transform().m11(), snapped, snapHorizontal ? mHorizontalSnapLine : nullptr,
                     snapVertical ? mVerticalSnapLine : nullptr, &layoutItemsToExclude ).topLeft();
      break;
    case Point:
      snappedPoint = mLayout->snapper().snapPoint( originalPoint, mView->transform().m11(), snapped, snapHorizontal ? mHorizontalSnapLine : nullptr,
                     snapVertical ? mVerticalSnapLine : nullptr, &layoutItemsToExclude );
      break;
  }

  return snapped ? snappedPoint : originalPoint;
}

void QgsLayoutMouseHandles::createItemCommand( QGraphicsItem *item )
{
  mItemCommand.reset( qgis::down_cast< QgsLayoutItem * >( item )->createCommand( QString(), 0 ) );
  mItemCommand->saveBeforeState();
}

void QgsLayoutMouseHandles::endItemCommand( QGraphicsItem * )
{
  mItemCommand->saveAfterState();
  mLayout->undoStack()->push( mItemCommand.release() );
}

void QgsLayoutMouseHandles::startMacroCommand( const QString &text )
{
  mLayout->undoStack()->beginMacro( text );

}

void QgsLayoutMouseHandles::endMacroCommand()
{
  mLayout->undoStack()->endMacro();
}

void QgsLayoutMouseHandles::drawMovePreview( QPainter *painter )
{
  const QList<QGraphicsItem *> selectedItems = selectedSceneItems( false );
  if ( selectedItems.isEmpty() )
  {
    return;
  }

  double destinationDpi = QgsLayoutUtils::scaleFactorFromItemStyle( nullptr, painter ) * 25.4;
  double widthInPixels = boundingRect().width() * destinationDpi;
  double heightInPixels = boundingRect().height() * destinationDpi;

  // limit size of image for better performance
  if ( ( widthInPixels > CACHE_SIZE_LIMIT || heightInPixels > CACHE_SIZE_LIMIT ) )
  {
    double scale = 1.0;
    if ( widthInPixels > heightInPixels )
    {
      scale = widthInPixels / CACHE_SIZE_LIMIT;
      widthInPixels = CACHE_SIZE_LIMIT;
      heightInPixels /= scale;
    }
    else
    {
      scale = heightInPixels / CACHE_SIZE_LIMIT;
      heightInPixels = CACHE_SIZE_LIMIT;
      widthInPixels /= scale;
    }
    destinationDpi = destinationDpi / scale;
  }

  // create image if not already cached
  if ( mItemCachedImage.isNull() )
  {
    QImage image = QImage( static_cast<int>( widthInPixels ), static_cast<int>( heightInPixels ), QImage::Format_ARGB32 );
    image.fill( Qt::transparent );
    image.setDotsPerMeterX( static_cast<int>( 1000 * destinationDpi * 25.4 ) );
    image.setDotsPerMeterY( static_cast<int>( 1000 * destinationDpi * 25.4 ) );
    QPainter p( &image );
    p.setRenderHint( QPainter::Antialiasing, true );
    QgsRenderContext context = QgsLayoutUtils::createRenderContextForLayout( mLayout, &p, destinationDpi );
    // painter is already scaled to dots
    // need to translate so that item origin is at 0,0 in painter coordinates (not bounding rect origin)
    p.translate( -boundingRect().x() * context.scaleFactor(), -boundingRect().y() * context.scaleFactor() );
    // scale to layout units for background and frame rendering
    p.scale( context.scaleFactor(), context.scaleFactor() );

    QList<QGraphicsItem *> itemsToDraw;
    expandItemList( selectedItems, itemsToDraw );

    // Draw a semi-transparent version of the selected items
    // painter->setOpacity( 0.5 );
    // the MouseHandles can be rotated if a single rotated item is selected, so we need
    // so we need to compensate for the rotation before applying the translation offset
    p.rotate( -rotation() );
    p.translate( transform().dx(), transform().dy() );
    p.rotate( rotation() );

    // Sort items by z value, so that items with a higher z value are drawn on top of items with a lower z value
    std::sort( itemsToDraw.begin(), itemsToDraw.end(), []( QGraphicsItem * a, QGraphicsItem * b )
    {
      return a->zValue() < b->zValue();
    } );

    for ( QGraphicsItem *item : itemsToDraw )
    {
      QgsScopedQPainterState innerState( &p );
      // Apply the item's transform to the painter
      p.setTransform( item->itemTransform( this ), true );
      // Draw the item
      item->paint( &p, nullptr, nullptr );
    }

    p.scale( 1 / context.scaleFactor(), 1 / context.scaleFactor() );
    p.end();
    mItemCachedImage = image;
  }

  // Draw the cached image as a semi-transparent overlay
  const QgsRenderContext context = QgsLayoutUtils::createRenderContextForLayout( mLayout, painter, destinationDpi );
  const QgsScopedQPainterState painterState( painter );
  painter->setOpacity( 0.5 );
  painter->setRenderHint( QPainter::Antialiasing, true );
  painter->scale( 1 / context.scaleFactor(), 1 / context.scaleFactor() );
  painter->drawImage( QPointF( boundingRect().x() * context.scaleFactor(),
                               boundingRect().y() * context.scaleFactor() ), mItemCachedImage );
}

void QgsLayoutMouseHandles::mouseReleaseEvent( QGraphicsSceneMouseEvent *event )
{
  QgsGraphicsViewMouseHandles::mouseReleaseEvent( event );
  mItemCachedImage = QImage();
}

void QgsLayoutMouseHandles::hideAlignItems()
{
  mHorizontalSnapLine->hide();
  mVerticalSnapLine->hide();
}

void QgsLayoutMouseHandles::expandItemList( const QList<QGraphicsItem *> &items, QList<QGraphicsItem *> &collected ) const
{
  for ( QGraphicsItem *item : items )
  {
    if ( item->type() == QgsLayoutItemRegistry::LayoutGroup )
    {
      // if a group is selected, we don't draw the bounds of the group - instead we draw the bounds of the grouped items
      const QList<QgsLayoutItem *> groupItems = static_cast< QgsLayoutItemGroup * >( item )->items();
      expandItemList( groupItems, collected );
    }
    else
    {
      collected << item;
    }
  }
}


void QgsLayoutMouseHandles::expandItemList( const QList<QgsLayoutItem *> &items, QList<QGraphicsItem *> &collected ) const
{
  for ( QGraphicsItem *item : items )
  {
    if ( item->type() == QgsLayoutItemRegistry::LayoutGroup )
    {
      // if a group is selected, we don't draw the bounds of the group - instead we draw the bounds of the grouped items
      const QList<QgsLayoutItem *> groupItems = static_cast< QgsLayoutItemGroup * >( item )->items();
      expandItemList( groupItems, collected );
    }
    else
    {
      collected << item;
    }
  }
}

void QgsLayoutMouseHandles::moveItem( QGraphicsItem *item, double deltaX, double deltaY )
{
  qgis::down_cast< QgsLayoutItem * >( item )->attemptMoveBy( deltaX, deltaY );
}

void QgsLayoutMouseHandles::setItemRect( QGraphicsItem *item, QRectF rect )
{
  QgsLayoutItem *layoutItem = dynamic_cast< QgsLayoutItem * >( item );
  layoutItem->attemptSetSceneRect( rect, true );
}

void QgsLayoutMouseHandles::showStatusMessage( const QString &message )
{
  if ( !mView )
    return;

  mView->pushStatusMessage( message );
}


///@endcond PRIVATE
