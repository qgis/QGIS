/***************************************************************************
                             qgslayoutviewtooladditem.cpp
                             ----------------------------
    Date                 : July 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutviewtooladditem.h"
#include "qgsapplication.h"
#include "qgslayoutview.h"
#include "qgslayout.h"
#include "qgslayoutitemregistry.h"
#include "qgslayoutviewmouseevent.h"
#include "qgslogger.h"
#include "qgslayoutviewrubberband.h"
#include "qgsgui.h"
#include "qgslayoutitemguiregistry.h"
#include "qgslayoutnewitempropertiesdialog.h"
#include "qgssettings.h"
#include "qgslayoutundostack.h"
#include <QGraphicsRectItem>
#include <QPen>
#include <QBrush>
#include <QMouseEvent>

QgsLayoutViewToolAddItem::QgsLayoutViewToolAddItem( QgsLayoutView *view )
  : QgsLayoutViewTool( view, tr( "Add item" ) )
{
  setFlags( QgsLayoutViewTool::FlagSnaps );
  setCursor( Qt::CrossCursor );
}

void QgsLayoutViewToolAddItem::setItemMetadataId( int metadataId )
{
  mItemMetadataId = metadataId;
}

void QgsLayoutViewToolAddItem::layoutPressEvent( QgsLayoutViewMouseEvent *event )
{
  if ( event->button() != Qt::LeftButton )
  {
    event->ignore();
    return;
  }

  mDrawing = true;
  mMousePressStartPos = event->pos();
  mMousePressStartLayoutPos = event->layoutPoint();
  mRubberBand.reset( QgsGui::layoutItemGuiRegistry()->createItemRubberBand( mItemMetadataId, view() ) );
  if ( mRubberBand )
  {
    connect( mRubberBand.get(), &QgsLayoutViewRubberBand::sizeChanged, this, [ = ]( const QString & size )
    {
      view()->pushStatusMessage( size );
    } );
    mRubberBand->start( event->snappedPoint(), event->modifiers() );
  }
}

void QgsLayoutViewToolAddItem::layoutMoveEvent( QgsLayoutViewMouseEvent *event )
{
  if ( mDrawing && mRubberBand )
  {
    mRubberBand->update( event->snappedPoint(), event->modifiers() );
  }
  else
  {
    event->ignore();
  }
}

void QgsLayoutViewToolAddItem::layoutReleaseEvent( QgsLayoutViewMouseEvent *event )
{
  if ( event->button() != Qt::LeftButton || !mDrawing )
  {
    event->ignore();
    return;
  }
  mDrawing = false;

  const QRectF rect = mRubberBand ? mRubberBand->finish( event->snappedPoint(), event->modifiers() ) : QRectF();

  QString undoText;
  if ( QgsLayoutItemAbstractGuiMetadata *metadata = QgsGui::layoutItemGuiRegistry()->itemMetadata( mItemMetadataId ) )
  {
    undoText = tr( "Create %1" ).arg( metadata->visibleName() );
  }
  else
  {
    undoText = tr( "Create Item" );
  }
  layout()->undoStack()->beginMacro( undoText );

  QgsLayoutItem *item = QgsGui::layoutItemGuiRegistry()->createItem( mItemMetadataId, layout() );
  if ( !item )
  {
    layout()->undoStack()->endMacro();
    return;
  }

  // click? or click-and-drag?
  const bool clickOnly = !isClickAndDrag( mMousePressStartPos, event->pos() );
  if ( clickOnly && mRubberBand )
  {
    QgsLayoutItemPropertiesDialog dlg( view() );
    dlg.setLayout( layout() );
    dlg.setItemPosition( QgsLayoutPoint( event->snappedPoint(), layout()->units() ) );
    if ( dlg.exec() )
    {
      item->setReferencePoint( dlg.referencePoint() );
      item->attemptResize( dlg.itemSize() );
      item->attemptMove( dlg.itemPosition(), true, false, dlg.page() );
    }
    else
    {
      delete item;
      layout()->undoStack()->endMacro();
      return;
    }
  }
  else if ( mRubberBand )
  {
    item->attemptResize( QgsLayoutSize( rect.width(), rect.height(), QgsUnitTypes::LayoutMillimeters ) );
    item->attemptMove( QgsLayoutPoint( rect.left(), rect.top(), QgsUnitTypes::LayoutMillimeters ) );
  }
  else
  {
    // item type doesn't use rubber bands -- e.g. marker items
    item->attemptMove( QgsLayoutPoint( mMousePressStartLayoutPos, layout()->units() ) );
  }

  // record last created item size
  if ( mRubberBand )
  {
    QgsSettings settings;
    settings.setValue( QStringLiteral( "LayoutDesigner/lastItemWidth" ), item->sizeWithUnits().width() );
    settings.setValue( QStringLiteral( "LayoutDesigner/lastItemHeight" ), item->sizeWithUnits().height() );
    settings.setEnumValue( QStringLiteral( "LayoutDesigner/lastSizeUnit" ), item->sizeWithUnits().units() );
  }

  QgsGui::layoutItemGuiRegistry()->newItemAddedToLayout( mItemMetadataId, item, mCustomProperties );

  // it's possible (in certain circumstances, e.g. when adding frame items) that this item
  // has already been added to the layout
  if ( item->scene() != layout() )
    layout()->addLayoutItem( item );
  layout()->setSelectedItem( item );

  layout()->undoStack()->endMacro();
  emit createdItem();
}

void QgsLayoutViewToolAddItem::activate()
{
  QgsLayoutViewTool::activate();
  mCustomProperties.clear();
}

void QgsLayoutViewToolAddItem::deactivate()
{
  if ( mDrawing )
  {
    // canceled mid operation
    if ( mRubberBand )
      mRubberBand->finish();
    mDrawing = false;
  }
  QgsLayoutViewTool::deactivate();
}

QVariantMap QgsLayoutViewToolAddItem::customProperties() const
{
  return mCustomProperties;
}

void QgsLayoutViewToolAddItem::setCustomProperties( const QVariantMap &customProperties )
{
  mCustomProperties = customProperties;
}

int QgsLayoutViewToolAddItem::itemMetadataId() const
{
  return mItemMetadataId;
}
