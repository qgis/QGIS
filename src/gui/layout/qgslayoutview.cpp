/***************************************************************************
                             qgslayoutview.cpp
                             -----------------
    Date                 : July 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutview.h"
#include "qgslayoutframe.h"
#include "qgslayoutmultiframe.h"
#include "qgslayoutviewtool.h"
#include "qgslayoutviewmouseevent.h"
#include "qgslayoutviewtooltemporarykeypan.h"
#include "qgslayoutviewtooltemporarykeyzoom.h"
#include "qgslayoutviewtooltemporarymousepan.h"
#include "qgslayoutruler.h"
#include "qgslayoutmodel.h"
#include "qgssettings.h"
#include "qgsrectangle.h"
#include "qgsproject.h"
#include "qgslayoutitemgroup.h"
#include "qgslayoutpagecollection.h"
#include "qgslayoutundostack.h"
#include "qgslayoutreportsectionlabel.h"
#include "qgsreadwritecontext.h"
#include "qgsscreenhelper.h"

#include <memory>
#include <QMenu>
#include <QClipboard>
#include <QMimeData>

#define MIN_VIEW_SCALE 0.05
#define MAX_VIEW_SCALE 1000.0

QgsLayoutView::QgsLayoutView( QWidget *parent )
  : QGraphicsView( parent )
{
  setResizeAnchor( QGraphicsView::AnchorViewCenter );
  setMouseTracking( true );
  viewport()->setMouseTracking( true );

  // set the "scene" background on the view using stylesheets
  // we don't want to use QGraphicsScene::setBackgroundBrush because we want to keep
  // a transparent background for exports, and it's only a cosmetic thing for the view only
  // ALSO - only set it on the viewport - we don't want scrollbars/etc affected by this
  viewport()->setStyleSheet( QStringLiteral( "background-color:#d7d7d7;" ) );

  mSpacePanTool = new QgsLayoutViewToolTemporaryKeyPan( this );
  mMidMouseButtonPanTool = new QgsLayoutViewToolTemporaryMousePan( this );
  mSpaceZoomTool = new QgsLayoutViewToolTemporaryKeyZoom( this );

  mPreviewEffect = new QgsPreviewEffect( this );
  viewport()->setGraphicsEffect( mPreviewEffect );

  connect( this, &QgsLayoutView::zoomLevelChanged, this, &QgsLayoutView::invalidateCachedRenders );

  mScreenHelper = new QgsScreenHelper( this );
}

QgsLayoutView::~QgsLayoutView()
{
  emit willBeDeleted();
}

QgsLayout *QgsLayoutView::currentLayout()
{
  return qobject_cast<QgsLayout *>( scene() );
}

const QgsLayout *QgsLayoutView::currentLayout() const
{
  return qobject_cast<const QgsLayout *>( scene() );
}

void QgsLayoutView::setCurrentLayout( QgsLayout *layout )
{
  setScene( layout );

  connect( layout->pageCollection(), &QgsLayoutPageCollection::changed, this, &QgsLayoutView::viewChanged );
  connect( layout, &QgsLayout::selectedItemChanged, this, &QgsLayoutView::itemFocused );

  viewChanged();

  // IMPORTANT!
  // previous snap markers, snap lines are owned by previous layout - so don't delete them here!
  mSnapMarker = new QgsLayoutViewSnapMarker();
  mSnapMarker->hide();
  layout->addItem( mSnapMarker );
  mHorizontalSnapLine = createSnapLine();
  mHorizontalSnapLine->hide();
  layout->addItem( mHorizontalSnapLine );
  mVerticalSnapLine = createSnapLine();
  mVerticalSnapLine->hide();
  layout->addItem( mVerticalSnapLine );
  mSectionLabel = nullptr;

  if ( mHorizontalRuler )
  {
    connect( &layout->guides(), &QAbstractItemModel::dataChanged, mHorizontalRuler, [ = ] { mHorizontalRuler->update(); } );
    connect( &layout->guides(), &QAbstractItemModel::rowsInserted, mHorizontalRuler, [ = ] { mHorizontalRuler->update(); } );
    connect( &layout->guides(), &QAbstractItemModel::rowsRemoved, mHorizontalRuler, [ = ] { mHorizontalRuler->update(); } );
    connect( &layout->guides(), &QAbstractItemModel::modelReset, mHorizontalRuler, [ = ] { mHorizontalRuler->update(); } );
  }
  if ( mVerticalRuler )
  {
    connect( &layout->guides(), &QAbstractItemModel::dataChanged, mVerticalRuler, [ = ] { mVerticalRuler->update(); } );
    connect( &layout->guides(), &QAbstractItemModel::rowsInserted, mVerticalRuler, [ = ] { mVerticalRuler->update(); } );
    connect( &layout->guides(), &QAbstractItemModel::rowsRemoved, mVerticalRuler, [ = ] { mVerticalRuler->update(); } );
    connect( &layout->guides(), &QAbstractItemModel::modelReset, mVerticalRuler, [ = ] { mVerticalRuler->update(); } );
  }

  //emit layoutSet, so that designer dialogs can update for the new layout
  emit layoutSet( layout );
}

QgsLayoutViewTool *QgsLayoutView::tool()
{
  return mTool;
}

void QgsLayoutView::setTool( QgsLayoutViewTool *tool )
{
  if ( !tool )
    return;

  if ( mTool )
  {
    mTool->deactivate();
    disconnect( mTool, &QgsLayoutViewTool::itemFocused, this, &QgsLayoutView::itemFocused );
  }

  if ( mSnapMarker )
    mSnapMarker->hide();
  if ( mHorizontalSnapLine )
    mHorizontalSnapLine->hide();
  if ( mVerticalSnapLine )
    mVerticalSnapLine->hide();

  // activate new tool before setting it - gives tools a chance
  // to respond to whatever the current tool is
  tool->activate();
  mTool = tool;
  connect( mTool, &QgsLayoutViewTool::itemFocused, this, &QgsLayoutView::itemFocused );
  emit toolSet( mTool );
}

void QgsLayoutView::unsetTool( QgsLayoutViewTool *tool )
{
  if ( mTool && mTool == tool )
  {
    mTool->deactivate();
    emit toolSet( nullptr );
    setCursor( Qt::ArrowCursor );
  }
}

void QgsLayoutView::setPreviewModeEnabled( bool enabled )
{
  mPreviewEffect->setEnabled( enabled );
}

bool QgsLayoutView::previewModeEnabled() const
{
  return mPreviewEffect->isEnabled();
}

void QgsLayoutView::setPreviewMode( QgsPreviewEffect::PreviewMode mode )
{
  mPreviewEffect->setMode( mode );
}

QgsPreviewEffect::PreviewMode QgsLayoutView::previewMode() const
{
  return mPreviewEffect->mode();
}

void QgsLayoutView::scaleSafe( double scale )
{
  double currentScale = transform().m11();
  scale *= currentScale;
  scale = std::clamp( scale, MIN_VIEW_SCALE, MAX_VIEW_SCALE );
  setTransform( QTransform::fromScale( scale, scale ) );
  emit zoomLevelChanged();
  viewChanged();
}

void QgsLayoutView::setZoomLevel( double level )
{
  if ( !currentLayout() )
    return;

  if ( currentLayout()->units() == QgsUnitTypes::LayoutPixels )
  {
    setTransform( QTransform::fromScale( level, level ) );
  }
  else
  {
    double dpi = mScreenHelper->screenDpi();
    //monitor dpi is not always correct - so make sure the value is sane
    if ( ( dpi < 60 ) || ( dpi > 1200 ) )
      dpi = 72;

    //desired pixel width for 1mm on screen
    level = std::clamp( level, MIN_VIEW_SCALE, MAX_VIEW_SCALE );
    double mmLevel = currentLayout()->convertFromLayoutUnits( level, QgsUnitTypes::LayoutMillimeters ).length() * dpi / 25.4;
    setTransform( QTransform::fromScale( mmLevel, mmLevel ) );
  }
  emit zoomLevelChanged();
  viewChanged();
}

void QgsLayoutView::setHorizontalRuler( QgsLayoutRuler *ruler )
{
  mHorizontalRuler = ruler;
  ruler->setLayoutView( this );
  if ( QgsLayout *layout = currentLayout() )
  {
    connect( &layout->guides(), &QAbstractItemModel::dataChanged, ruler, [ = ] { mHorizontalRuler->update(); } );
    connect( &layout->guides(), &QAbstractItemModel::rowsInserted, ruler, [ = ] { mHorizontalRuler->update(); } );
    connect( &layout->guides(), &QAbstractItemModel::rowsRemoved, ruler, [ = ] { mHorizontalRuler->update(); } );
    connect( &layout->guides(), &QAbstractItemModel::modelReset, ruler, [ = ] { mHorizontalRuler->update(); } );
  }
  viewChanged();
}

void QgsLayoutView::setVerticalRuler( QgsLayoutRuler *ruler )
{
  mVerticalRuler = ruler;
  ruler->setLayoutView( this );
  if ( QgsLayout *layout = currentLayout() )
  {
    connect( &layout->guides(), &QAbstractItemModel::dataChanged, ruler, [ = ] { mVerticalRuler->update(); } );
    connect( &layout->guides(), &QAbstractItemModel::rowsInserted, ruler, [ = ] { mVerticalRuler->update(); } );
    connect( &layout->guides(), &QAbstractItemModel::rowsRemoved, ruler, [ = ] { mVerticalRuler->update(); } );
    connect( &layout->guides(), &QAbstractItemModel::modelReset, ruler, [ = ] { mVerticalRuler->update(); } );
  }
  viewChanged();
}

void QgsLayoutView::setMenuProvider( QgsLayoutViewMenuProvider *provider )
{
  mMenuProvider.reset( provider );
}

QgsLayoutViewMenuProvider *QgsLayoutView::menuProvider() const
{
  return mMenuProvider.get();
}

QList<QgsLayoutItemPage *> QgsLayoutView::visiblePages() const
{
  if ( !currentLayout() )
    return QList< QgsLayoutItemPage *>();

  //get current visible part of scene
  QRect viewportRect( 0, 0, viewport()->width(), viewport()->height() );
  QRectF visibleRect = mapToScene( viewportRect ).boundingRect();
  return currentLayout()->pageCollection()->visiblePages( visibleRect );
}

QList<int> QgsLayoutView::visiblePageNumbers() const
{
  if ( !currentLayout() )
    return QList< int >();

  //get current visible part of scene
  QRect viewportRect( 0, 0, viewport()->width(), viewport()->height() );
  QRectF visibleRect = mapToScene( viewportRect ).boundingRect();
  return currentLayout()->pageCollection()->visiblePageNumbers( visibleRect );
}

void QgsLayoutView::alignSelectedItems( QgsLayoutAligner::Alignment alignment )
{
  if ( !currentLayout() )
    return;

  const QList<QgsLayoutItem *> selectedItems = currentLayout()->selectedLayoutItems();
  QgsLayoutAligner::alignItems( currentLayout(), selectedItems, alignment );
}

void QgsLayoutView::distributeSelectedItems( QgsLayoutAligner::Distribution distribution )
{
  if ( !currentLayout() )
    return;

  const QList<QgsLayoutItem *> selectedItems = currentLayout()->selectedLayoutItems();
  QgsLayoutAligner::distributeItems( currentLayout(), selectedItems, distribution );
}

void QgsLayoutView::resizeSelectedItems( QgsLayoutAligner::Resize resize )
{
  if ( !currentLayout() )
    return;

  const QList<QgsLayoutItem *> selectedItems = currentLayout()->selectedLayoutItems();
  QgsLayoutAligner::resizeItems( currentLayout(), selectedItems, resize );
}

void QgsLayoutView::copySelectedItems( QgsLayoutView::ClipboardOperation operation )
{
  copyItems( currentLayout()->selectedLayoutItems(), operation );
}

void QgsLayoutView::copyItems( const QList<QgsLayoutItem *> &items, QgsLayoutView::ClipboardOperation operation )
{
  if ( !currentLayout() )
    return;

  QgsReadWriteContext context;
  QDomDocument doc;
  QDomElement documentElement = doc.createElement( QStringLiteral( "LayoutItemClipboard" ) );
  if ( operation == ClipboardCut )
    currentLayout()->undoStack()->beginMacro( tr( "Cut Items" ) );

  QSet< QgsLayoutMultiFrame * > copiedMultiFrames;

  for ( QgsLayoutItem *item : items )
  {
    // copy every child from a group
    if ( QgsLayoutItemGroup *itemGroup = qobject_cast<QgsLayoutItemGroup *>( item ) )
    {
      const QList<QgsLayoutItem *> groupedItems = itemGroup->items();
      for ( const QgsLayoutItem *groupedItem : groupedItems )
      {
        groupedItem->writeXml( documentElement, doc, context );
      }
    }
    else if ( QgsLayoutFrame *frame = qobject_cast<QgsLayoutFrame *>( item ) )
    {
      // copy multiframe too
      if ( frame->multiFrame() && !copiedMultiFrames.contains( frame->multiFrame() ) )
      {
        frame->multiFrame()->writeXml( documentElement, doc, context );
        copiedMultiFrames.insert( frame->multiFrame() );
      }
    }
    item->writeXml( documentElement, doc, context );
    if ( operation == ClipboardCut )
      currentLayout()->removeLayoutItem( item );
  }
  doc.appendChild( documentElement );
  if ( operation == ClipboardCut )
  {
    currentLayout()->undoStack()->endMacro();
    currentLayout()->update();
  }

  //remove the UUIDs since we don't want any duplicate UUID
  QDomNodeList itemsNodes = doc.elementsByTagName( QStringLiteral( "LayoutItem" ) );
  for ( int i = 0; i < itemsNodes.count(); ++i )
  {
    QDomNode itemNode = itemsNodes.at( i );
    if ( itemNode.isElement() )
    {
      itemNode.toElement().removeAttribute( QStringLiteral( "uuid" ) );
      itemNode.toElement().removeAttribute( QStringLiteral( "groupUuid" ) );
    }
  }
  QDomNodeList multiFrameNodes = doc.elementsByTagName( QStringLiteral( "LayoutMultiFrame" ) );
  for ( int i = 0; i < multiFrameNodes.count(); ++i )
  {
    QDomNode multiFrameNode = multiFrameNodes.at( i );
    if ( multiFrameNode.isElement() )
    {
      multiFrameNode.toElement().removeAttribute( QStringLiteral( "uuid" ) );
      QDomNodeList frameNodes = multiFrameNode.toElement().elementsByTagName( QStringLiteral( "childFrame" ) );
      for ( int j = 0; j < frameNodes.count(); ++j )
      {
        QDomNode itemNode = frameNodes.at( j );
        if ( itemNode.isElement() )
        {
          itemNode.toElement().removeAttribute( QStringLiteral( "uuid" ) );
        }
      }
    }
  }

  QMimeData *mimeData = new QMimeData;
  mimeData->setData( QStringLiteral( "text/xml" ), doc.toByteArray() );
  QClipboard *clipboard = QApplication::clipboard();
  clipboard->setMimeData( mimeData );
}

QList< QgsLayoutItem * > QgsLayoutView::pasteItems( QgsLayoutView::PasteMode mode )
{
  if ( !currentLayout() )
    return QList< QgsLayoutItem * >();

  QList< QgsLayoutItem * > pastedItems;
  QDomDocument doc;
  QClipboard *clipboard = QApplication::clipboard();
  if ( doc.setContent( clipboard->mimeData()->data( QStringLiteral( "text/xml" ) ) ) )
  {
    QDomElement docElem = doc.documentElement();
    if ( docElem.tagName() == QLatin1String( "LayoutItemClipboard" ) )
    {
      QPointF pt;
      switch ( mode )
      {
        case PasteModeCursor:
        case PasteModeInPlace:
        {
          // place items at cursor position
          pt = mapToScene( mapFromGlobal( QCursor::pos() ) );
          break;
        }
        case PasteModeCenter:
        {
          // place items in center of viewport
          pt = mapToScene( viewport()->rect().center() );
          break;
        }
      }
      bool pasteInPlace = ( mode == PasteModeInPlace );
      currentLayout()->undoStack()->beginMacro( tr( "Paste Items" ) );
      currentLayout()->undoStack()->beginCommand( currentLayout(), tr( "Paste Items" ) );
      pastedItems = currentLayout()->addItemsFromXml( docElem, doc, QgsReadWriteContext(), &pt, pasteInPlace );
      currentLayout()->undoStack()->endCommand();
      currentLayout()->undoStack()->endMacro();
    }
  }
  return pastedItems;
}

QList<QgsLayoutItem *> QgsLayoutView::pasteItems( QPointF layoutPoint )
{
  if ( !currentLayout() )
    return QList<QgsLayoutItem *>();

  QList< QgsLayoutItem * > pastedItems;
  QDomDocument doc;
  QClipboard *clipboard = QApplication::clipboard();
  if ( doc.setContent( clipboard->mimeData()->data( QStringLiteral( "text/xml" ) ) ) )
  {
    QDomElement docElem = doc.documentElement();
    if ( docElem.tagName() == QLatin1String( "LayoutItemClipboard" ) )
    {
      currentLayout()->undoStack()->beginMacro( tr( "Paste Items" ) );
      currentLayout()->undoStack()->beginCommand( currentLayout(), tr( "Paste Items" ) );
      pastedItems = currentLayout()->addItemsFromXml( docElem, doc, QgsReadWriteContext(), &layoutPoint, false );
      currentLayout()->undoStack()->endCommand();
      currentLayout()->undoStack()->endMacro();
    }
  }
  return pastedItems;
}

bool QgsLayoutView::hasItemsInClipboard() const
{
  QDomDocument doc;
  QClipboard *clipboard = QApplication::clipboard();
  if ( doc.setContent( clipboard->mimeData()->data( QStringLiteral( "text/xml" ) ) ) )
  {
    QDomElement docElem = doc.documentElement();
    if ( docElem.tagName() == QLatin1String( "LayoutItemClipboard" ) )
      return true;
  }
  return false;
}

QPointF QgsLayoutView::deltaForKeyEvent( QKeyEvent *event )
{
  // increment used for cursor key item movement
  double increment = 1.0;
  if ( event->modifiers() & Qt::ShiftModifier )
  {
    //holding shift while pressing cursor keys results in a big step
    increment = 10.0;
  }
  else if ( event->modifiers() & Qt::AltModifier )
  {
    //holding alt while pressing cursor keys results in a 1 pixel step
    double viewScale = transform().m11();
    if ( viewScale > 0 )
    {
      increment = 1 / viewScale;
    }
  }

  double deltaX = 0;
  double deltaY = 0;
  switch ( event->key() )
  {
    case Qt::Key_Left:
      deltaX = -increment;
      break;
    case Qt::Key_Right:
      deltaX = increment;
      break;
    case Qt::Key_Up:
      deltaY = -increment;
      break;
    case Qt::Key_Down:
      deltaY = increment;
      break;
    default:
      break;
  }

  return QPointF( deltaX, deltaY );
}

void QgsLayoutView::setPaintingEnabled( bool enabled )
{
  mPaintingEnabled = enabled;
  if ( enabled )
    update();
}

void QgsLayoutView::setSectionLabel( const QString &label )
{
  if ( !currentLayout() )
    return;

  if ( !mSectionLabel )
  {
    mSectionLabel = new QgsLayoutReportSectionLabel( currentLayout(), this );
    currentLayout()->addItem( mSectionLabel );
    mSectionLabel->setRect( 0, -200, 1000, 200 );
    mSectionLabel->setZValue( -1 );
  }
  mSectionLabel->setLabel( label );
}

void QgsLayoutView::zoomFull()
{
  if ( !scene() )
    return;

  fitInView( scene()->sceneRect(), Qt::KeepAspectRatio );
  viewChanged();
  emit zoomLevelChanged();
}

void QgsLayoutView::zoomWidth()
{
  if ( !scene() )
    return;

  //get current visible part of scene
  QRect viewportRect( 0, 0, viewport()->width(), viewport()->height() );
  QRectF visibleRect = mapToScene( viewportRect ).boundingRect();

  double verticalCenter = ( visibleRect.top() + visibleRect.bottom() ) / 2.0;
  // expand out visible rect to include left/right edges of scene
  // centered on current visible vertical center
  // note that we can't have a 0 height rect - fitInView doesn't handle that
  // so we just set a very small height instead.
  const double tinyHeight = 0.01;
  QRectF targetRect( scene()->sceneRect().left(),
                     verticalCenter - tinyHeight,
                     scene()->sceneRect().width(),
                     tinyHeight * 2 );

  fitInView( targetRect, Qt::KeepAspectRatio );
  emit zoomLevelChanged();
  viewChanged();
}

void QgsLayoutView::zoomIn()
{
  scaleSafe( 2 );
}

void QgsLayoutView::zoomOut()
{
  scaleSafe( 0.5 );
}

void QgsLayoutView::zoomActual()
{
  setZoomLevel( 1.0 );
}

void QgsLayoutView::emitZoomLevelChanged()
{
  emit zoomLevelChanged();
}

void QgsLayoutView::selectAll()
{
  if ( !currentLayout() )
  {
    return;
  }

  //select all items in layout
  QgsLayoutItem *focusedItem = nullptr;
  const QList<QGraphicsItem *> itemList = currentLayout()->items();
  for ( QGraphicsItem *graphicsItem : itemList )
  {
    QgsLayoutItem *item = dynamic_cast<QgsLayoutItem *>( graphicsItem );
    QgsLayoutItemPage *paperItem = dynamic_cast<QgsLayoutItemPage *>( graphicsItem );
    if ( item && !paperItem )
    {
      if ( !item->isLocked() )
      {
        item->setSelected( true );
        if ( !focusedItem )
          focusedItem = item;
      }
      else
      {
        //deselect all locked items
        item->setSelected( false );
      }
    }
  }
  emit itemFocused( focusedItem );
}

void QgsLayoutView::deselectAll()
{
  if ( !currentLayout() )
  {
    return;
  }

  currentLayout()->deselectAll();
}

void QgsLayoutView::invertSelection()
{
  if ( !currentLayout() )
  {
    return;
  }

  QgsLayoutItem *focusedItem = nullptr;
  //check all items in layout
  const QList<QGraphicsItem *> itemList = currentLayout()->items();
  for ( QGraphicsItem *graphicsItem : itemList )
  {
    QgsLayoutItem *item = dynamic_cast<QgsLayoutItem *>( graphicsItem );
    QgsLayoutItemPage *paperItem = dynamic_cast<QgsLayoutItemPage *>( graphicsItem );
    if ( item && !paperItem )
    {
      //flip selected state for items (and deselect any locked items)
      if ( item->isSelected() || item->isLocked() )
      {
        item->setSelected( false );
      }
      else
      {
        item->setSelected( true );
        if ( !focusedItem )
          focusedItem = item;
      }
    }
  }
  if ( focusedItem )
    emit itemFocused( focusedItem );
}


void selectNextByZOrder( QgsLayout *layout, bool above )
{
  if ( !layout )
    return;

  QgsLayoutItem *previousSelectedItem = nullptr;
  const QList<QgsLayoutItem *> selectedItems = layout->selectedLayoutItems();
  if ( !selectedItems.isEmpty() )
  {
    previousSelectedItem = selectedItems.at( 0 );
  }

  if ( !previousSelectedItem )
  {
    return;
  }

  //select item with target z value
  QgsLayoutItem *selectedItem = nullptr;
  if ( !above )
    selectedItem = layout->itemsModel()->findItemBelow( previousSelectedItem );
  else
    selectedItem = layout->itemsModel()->findItemAbove( previousSelectedItem );

  if ( !selectedItem )
  {
    return;
  }

  //OK, found a good target item
  layout->setSelectedItem( selectedItem );
}

void QgsLayoutView::selectNextItemAbove()
{
  selectNextByZOrder( currentLayout(), true );
}

void QgsLayoutView::selectNextItemBelow()
{
  selectNextByZOrder( currentLayout(), false );
}

void QgsLayoutView::raiseSelectedItems()
{
  if ( !currentLayout() )
    return;

  const QList<QgsLayoutItem *> selectedItems = currentLayout()->selectedLayoutItems();
  bool itemsRaised = false;
  for ( QgsLayoutItem *item : selectedItems )
  {
    itemsRaised = itemsRaised | currentLayout()->raiseItem( item, true );
  }

  if ( !itemsRaised )
  {
    //no change
    return;
  }

  //update all positions
  currentLayout()->updateZValues();
  currentLayout()->update();
}

void QgsLayoutView::lowerSelectedItems()
{
  if ( !currentLayout() )
    return;

  const QList<QgsLayoutItem *> selectedItems = currentLayout()->selectedLayoutItems();
  bool itemsLowered = false;
  for ( QgsLayoutItem *item : selectedItems )
  {
    itemsLowered  = itemsLowered  | currentLayout()->lowerItem( item, true );
  }

  if ( !itemsLowered )
  {
    //no change
    return;
  }

  //update all positions
  currentLayout()->updateZValues();
  currentLayout()->update();
}

void QgsLayoutView::moveSelectedItemsToTop()
{
  if ( !currentLayout() )
    return;

  const QList<QgsLayoutItem *> selectedItems = currentLayout()->selectedLayoutItems();
  bool itemsRaised = false;
  for ( QgsLayoutItem *item : selectedItems )
  {
    itemsRaised = itemsRaised | currentLayout()->moveItemToTop( item, true );
  }

  if ( !itemsRaised )
  {
    //no change
    return;
  }

  //update all positions
  currentLayout()->updateZValues();
  currentLayout()->update();
}

void QgsLayoutView::moveSelectedItemsToBottom()
{
  if ( !currentLayout() )
    return;

  const QList<QgsLayoutItem *> selectedItems = currentLayout()->selectedLayoutItems();
  bool itemsLowered = false;
  for ( QgsLayoutItem *item : selectedItems )
  {
    itemsLowered = itemsLowered | currentLayout()->moveItemToBottom( item, true );
  }

  if ( !itemsLowered )
  {
    //no change
    return;
  }

  //update all positions
  currentLayout()->updateZValues();
  currentLayout()->update();
}

void QgsLayoutView::lockSelectedItems()
{
  if ( !currentLayout() )
    return;

  currentLayout()->undoStack()->beginMacro( tr( "Lock Items" ) );
  const QList<QgsLayoutItem *> selectionList = currentLayout()->selectedLayoutItems();
  for ( QgsLayoutItem *item : selectionList )
  {
    item->setLocked( true );
  }

  currentLayout()->deselectAll();
  currentLayout()->undoStack()->endMacro();
}

void QgsLayoutView::unlockAllItems()
{
  if ( !currentLayout() )
    return;

  //unlock all items in layout
  currentLayout()->undoStack()->beginMacro( tr( "Unlock Items" ) );

  //first, clear the selection
  currentLayout()->deselectAll();

  QgsLayoutItem *focusItem = nullptr;

  const QList<QGraphicsItem *> itemList = currentLayout()->items();
  for ( QGraphicsItem *graphicItem : itemList )
  {
    QgsLayoutItem *item = dynamic_cast<QgsLayoutItem *>( graphicItem );
    if ( item && item->isLocked() )
    {
      focusItem = item;
      item->setLocked( false );
      //select unlocked items, same behavior as illustrator
      item->setSelected( true );
    }
  }
  currentLayout()->undoStack()->endMacro();

  emit itemFocused( focusItem );
}

void QgsLayoutView::deleteSelectedItems()
{
  if ( !currentLayout() )
    return;

  deleteItems( currentLayout()->selectedLayoutItems() );
}

void QgsLayoutView::deleteItems( const QList<QgsLayoutItem *> &items )
{
  if ( !currentLayout() )
    return;

  if ( items.empty() )
    return;

  currentLayout()->undoStack()->beginMacro( tr( "Delete Items" ) );
  //delete selected items
  for ( QgsLayoutItem *item : items )
  {
    currentLayout()->removeLayoutItem( item );
  }
  currentLayout()->undoStack()->endMacro();
  currentLayout()->project()->setDirty( true );
}

void QgsLayoutView::groupSelectedItems()
{
  if ( !currentLayout() )
  {
    return;
  }

  //group selected items
  const QList<QgsLayoutItem *> selectionList = currentLayout()->selectedLayoutItems();
  QgsLayoutItemGroup *itemGroup = currentLayout()->groupItems( selectionList );

  if ( !itemGroup )
  {
    //group could not be created
    return;
  }

  for ( QgsLayoutItem *item : selectionList )
  {
    item->setSelected( false );
  }

  currentLayout()->setSelectedItem( itemGroup );
}

void QgsLayoutView::ungroupSelectedItems()
{
  if ( !currentLayout() )
  {
    return;
  }

  QList< QgsLayoutItem * > ungroupedItems;
  //hunt through selection for any groups, and ungroup them
  const QList<QgsLayoutItem *> selectionList = currentLayout()->selectedLayoutItems();
  for ( QgsLayoutItem *item : selectionList )
  {
    if ( item->type() == QgsLayoutItemRegistry::LayoutGroup )
    {
      QgsLayoutItemGroup *itemGroup = static_cast<QgsLayoutItemGroup *>( item );
      ungroupedItems.append( currentLayout()->ungroupItems( itemGroup ) );
    }
  }

  if ( !ungroupedItems.empty() )
  {
    for ( QgsLayoutItem *item : std::as_const( ungroupedItems ) )
    {
      item->setSelected( true );
    }
    emit itemFocused( ungroupedItems.at( 0 ) );
  }
}

void QgsLayoutView::mousePressEvent( QMouseEvent *event )
{
  if ( !currentLayout() )
    return;

  if ( mSnapMarker )
    mSnapMarker->setVisible( false );

  if ( mTool )
  {
    std::unique_ptr<QgsLayoutViewMouseEvent> me( new QgsLayoutViewMouseEvent( this, event, mTool->flags() & QgsLayoutViewTool::FlagSnaps ) );
    mTool->layoutPressEvent( me.get() );
    event->setAccepted( me->isAccepted() );
  }

  if ( !mTool || !event->isAccepted() )
  {
    if ( event->button() == Qt::MiddleButton )
    {
      // Pan layout with middle mouse button
      setTool( mMidMouseButtonPanTool );
      event->accept();
    }
    else if ( event->button() == Qt::RightButton && mMenuProvider )
    {
      QMenu *menu = mMenuProvider->createContextMenu( this, currentLayout(), mapToScene( event->pos() ) );
      if ( menu )
      {
        menu->exec( event->globalPos() );
        delete menu;
      }
    }
    else
    {
      QGraphicsView::mousePressEvent( event );
    }
  }
}

void QgsLayoutView::mouseReleaseEvent( QMouseEvent *event )
{
  if ( !currentLayout() )
    return;

  if ( mTool )
  {
    std::unique_ptr<QgsLayoutViewMouseEvent> me( new QgsLayoutViewMouseEvent( this, event, mTool->flags() & QgsLayoutViewTool::FlagSnaps ) );
    mTool->layoutReleaseEvent( me.get() );
    event->setAccepted( me->isAccepted() );
  }

  if ( !mTool || !event->isAccepted() )
    QGraphicsView::mouseReleaseEvent( event );
}

void QgsLayoutView::mouseMoveEvent( QMouseEvent *event )
{
  if ( !currentLayout() )
    return;

  mMouseCurrentXY = event->pos();

  QPointF cursorPos = mapToScene( mMouseCurrentXY );
  if ( mTool )
  {
    std::unique_ptr<QgsLayoutViewMouseEvent> me( new QgsLayoutViewMouseEvent( this, event, false ) );
    if ( mTool->flags() & QgsLayoutViewTool::FlagSnaps )
    {
      me->snapPoint( mHorizontalSnapLine, mVerticalSnapLine, mTool->ignoredSnapItems() );
    }
    if ( mTool->flags() & QgsLayoutViewTool::FlagSnaps )
    {
      //draw snapping point indicator
      if ( me->isSnapped() )
      {
        cursorPos = me->snappedPoint();
        if ( mSnapMarker )
        {
          mSnapMarker->setPos( me->snappedPoint() );
          mSnapMarker->setVisible( true );
        }
      }
      else if ( mSnapMarker )
      {
        mSnapMarker->setVisible( false );
      }
    }
    mTool->layoutMoveEvent( me.get() );
    event->setAccepted( me->isAccepted() );
  }

  //update cursor position in status bar
  emit cursorPosChanged( cursorPos );

  if ( !mTool || !event->isAccepted() )
    QGraphicsView::mouseMoveEvent( event );
}

void QgsLayoutView::mouseDoubleClickEvent( QMouseEvent *event )
{
  if ( !currentLayout() )
    return;

  if ( mTool )
  {
    std::unique_ptr<QgsLayoutViewMouseEvent> me( new QgsLayoutViewMouseEvent( this, event, mTool->flags() & QgsLayoutViewTool::FlagSnaps ) );
    mTool->layoutDoubleClickEvent( me.get() );
    event->setAccepted( me->isAccepted() );
  }

  if ( !mTool || !event->isAccepted() )
    QGraphicsView::mouseDoubleClickEvent( event );
}

void QgsLayoutView::wheelEvent( QWheelEvent *event )
{
  if ( !currentLayout() )
    return;

  if ( mTool )
  {
    mTool->wheelEvent( event );
  }

  if ( !mTool || !event->isAccepted() )
  {
    event->accept();
    wheelZoom( event );
  }
}

void QgsLayoutView::keyPressEvent( QKeyEvent *event )
{
  if ( !currentLayout() )
    return;

  if ( mTool )
  {
    mTool->keyPressEvent( event );
  }

  if ( mTool && event->isAccepted() )
    return;

  if ( event->key() == Qt::Key_Space && ! event->isAutoRepeat() )
  {
    if ( !( event->modifiers() & Qt::ControlModifier ) )
    {
      // Pan layout with space bar
      setTool( mSpacePanTool );
    }
    else
    {
      //ctrl+space pressed, so switch to temporary keyboard based zoom tool
      setTool( mSpaceZoomTool );
    }
    event->accept();
  }
  else if ( event->key() == Qt::Key_Left
            || event->key() == Qt::Key_Right
            || event->key() == Qt::Key_Up
            || event->key() == Qt::Key_Down )
  {
    QgsLayout *l = currentLayout();
    const QList<QgsLayoutItem *> layoutItemList = l->selectedLayoutItems();

    QPointF delta = deltaForKeyEvent( event );

    l->undoStack()->beginMacro( tr( "Move Item" ) );
    for ( QgsLayoutItem *item : layoutItemList )
    {
      l->undoStack()->beginCommand( item, tr( "Move Item" ), QgsLayoutItem::UndoIncrementalMove );
      item->attemptMoveBy( delta.x(), delta.y() );
      l->undoStack()->endCommand();
    }
    l->undoStack()->endMacro();
    event->accept();
  }
}

void QgsLayoutView::keyReleaseEvent( QKeyEvent *event )
{
  if ( !currentLayout() )
    return;

  if ( mTool )
  {
    mTool->keyReleaseEvent( event );
  }

  if ( !mTool || !event->isAccepted() )
    QGraphicsView::keyReleaseEvent( event );
}

void QgsLayoutView::resizeEvent( QResizeEvent *event )
{
  QGraphicsView::resizeEvent( event );
  emit zoomLevelChanged();
  viewChanged();
}

void QgsLayoutView::scrollContentsBy( int dx, int dy )
{
  QGraphicsView::scrollContentsBy( dx, dy );
  viewChanged();
}

void QgsLayoutView::dragEnterEvent( QDragEnterEvent *e )
{
  // By default graphics view delegates the drag events to graphics items.
  // But we do not want that and by ignoring the drag enter we let the
  // parent (e.g. QgsLayoutDesignerDialog) to handle drops of files.
  e->ignore();
}

void QgsLayoutView::paintEvent( QPaintEvent *event )
{
  if ( mPaintingEnabled )
  {
    QGraphicsView::paintEvent( event );
    event->accept();
  }
  else
  {
    event->ignore();
  }
}

void QgsLayoutView::invalidateCachedRenders()
{
  if ( !currentLayout() )
    return;

  //redraw cached map items
  QList< QgsLayoutItem *> items;
  currentLayout()->layoutItems( items );

  for ( QgsLayoutItem *item : std::as_const( items ) )
  {
    item->invalidateCache();
  }
}

void QgsLayoutView::viewChanged()
{
  if ( mHorizontalRuler )
  {
    mHorizontalRuler->setSceneTransform( viewportTransform() );
  }
  if ( mVerticalRuler )
  {
    mVerticalRuler->setSceneTransform( viewportTransform() );
  }

  // determine page at center of view
  QRect viewportRect( 0, 0, viewport()->width(), viewport()->height() );
  QRectF visibleRect = mapToScene( viewportRect ).boundingRect();
  QPointF centerVisible = visibleRect.center();

  if ( currentLayout() && currentLayout()->pageCollection() )
  {
    int newPage = currentLayout()->pageCollection()->pageNumberForPoint( centerVisible );
    if ( newPage != mCurrentPage )
    {
      mCurrentPage = newPage;
      emit pageChanged( mCurrentPage );
    }
  }
}

void QgsLayoutView::pushStatusMessage( const QString &message )
{
  emit statusMessage( message );
}

void QgsLayoutView::wheelZoom( QWheelEvent *event )
{
  //get mouse wheel zoom behavior settings
  QgsSettings settings;
  double zoomFactor = settings.value( QStringLiteral( "qgis/zoom_factor" ), 2 ).toDouble();

  // "Normal" mouse have an angle delta of 120, precision mouses provide data faster, in smaller steps
  zoomFactor = 1.0 + ( zoomFactor - 1.0 ) / 120.0 * std::fabs( event->angleDelta().y() );

  if ( event->modifiers() & Qt::ControlModifier )
  {
    //holding ctrl while wheel zooming results in a finer zoom
    zoomFactor = 1.0 + ( zoomFactor - 1.0 ) / 20.0;
  }

  //calculate zoom scale factor
  bool zoomIn = event->angleDelta().y() > 0;
  double scaleFactor = ( zoomIn ? 1 / zoomFactor : zoomFactor );

  //get current visible part of scene
  QRect viewportRect( 0, 0, viewport()->width(), viewport()->height() );
  QgsRectangle visibleRect = QgsRectangle( mapToScene( viewportRect ).boundingRect() );

  //transform the mouse pos to scene coordinates
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  QPointF scenePoint = mapToScene( event->pos() );
#else
  QPointF scenePoint = mapToScene( event->position().x(), event->position().y() );
#endif

  //adjust view center
  QgsPointXY oldCenter( visibleRect.center() );
  QgsPointXY newCenter( scenePoint.x() + ( ( oldCenter.x() - scenePoint.x() ) * scaleFactor ),
                        scenePoint.y() + ( ( oldCenter.y() - scenePoint.y() ) * scaleFactor ) );
  centerOn( newCenter.x(), newCenter.y() );

  //zoom layout
  if ( zoomIn )
  {
    scaleSafe( zoomFactor );
  }
  else
  {
    scaleSafe( 1 / zoomFactor );
  }
}

QGraphicsLineItem *QgsLayoutView::createSnapLine() const
{
  std::unique_ptr< QGraphicsLineItem>  item( new QGraphicsLineItem( nullptr ) );
  QPen pen = QPen( QColor( Qt::blue ) );
  pen.setStyle( Qt::DotLine );
  pen.setWidthF( 0.0 );
  item->setPen( pen );
  item->setZValue( QgsLayout::ZSmartGuide );
  return item.release();
}

//
// QgsLayoutViewSnapMarker
//

///@cond PRIVATE
QgsLayoutViewSnapMarker::QgsLayoutViewSnapMarker()
  : QGraphicsRectItem( QRectF( 0, 0, 0, 0 ) )
{
  QFont f;
  QFontMetrics fm( f );
  mSize = fm.horizontalAdvance( 'X' );
  setPen( QPen( Qt::transparent, mSize ) );

  setFlags( flags() | QGraphicsItem::ItemIgnoresTransformations );
  setZValue( QgsLayout::ZSnapIndicator );
}

void QgsLayoutViewSnapMarker::paint( QPainter *p, const QStyleOptionGraphicsItem *, QWidget * )
{
  QPen pen( QColor( 255, 0, 0 ) );
  pen.setWidth( 0 );
  p->setPen( pen );
  p->setBrush( Qt::NoBrush );

  double halfSize = mSize / 2.0;
  p->drawLine( QLineF( -halfSize, -halfSize, halfSize, halfSize ) );
  p->drawLine( QLineF( -halfSize, halfSize, halfSize, -halfSize ) );
}

///@endcond
