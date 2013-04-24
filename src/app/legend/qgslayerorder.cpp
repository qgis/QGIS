/***************************************************************************
                          qgslayerorder.cpp  -  description
                             -------------------
    begin                : 2011-11-09
    copyright            : (C) 2011 by Juergen E. Fischer, norBIT GmbH
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayerorder.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgslogger.h"
#include "qgslegend.h"
#include "qgslegendgroup.h"
#include "qgslegendlayer.h"
#include "qgslegendlayer.h"
#include "qgsproject.h"

#include <QMouseEvent>
#include <QTreeWidgetItem>

QgsLayerOrder::QgsLayerOrder( QgsLegend *legend, QWidget * parent, const char *name )
    : QListWidget( parent )
    , mLegend( legend )
    , mPressItem( 0 )
{
  setObjectName( name );

  // track if legend mode changes
  connect( mLegend, SIGNAL( updateDrawingOrderChecked( bool ) ),
           this, SLOT( updateDrawingOrderChecked( bool ) ) );

  connect( mLegend, SIGNAL( zOrderChanged() ),
           this, SLOT( refreshLayerList() ) );

  connect( mLegend, SIGNAL( invisibleLayerRemoved() ),
           this, SLOT( refreshLayerList() ) );

  connect( mLegend->canvas(), SIGNAL( layersChanged() ),
           this, SLOT( refreshLayerList() ) );

  // change visibility
  connect( this, SIGNAL( itemChanged( QListWidgetItem * ) ),
           this, SLOT( itemChanged( QListWidgetItem * ) ) );

  // Initialise the line indicator widget.
  mInsertionLine = new QWidget( viewport() );
  hideLine();
  mInsertionLine->setAutoFillBackground( true );
  QPalette pal = mInsertionLine->palette();
  pal.setColor( mInsertionLine->backgroundRole(), Qt::blue );
  mInsertionLine->setPalette( pal );
  mInsertRow = -1;

  setSortingEnabled( false );
  setSelectionMode( QAbstractItemView::ExtendedSelection );
  setDragEnabled( false );
  setAutoScroll( true );
  QFont f( "Arial", 10, QFont::Bold );
  setFont( f );
  QPalette palette;
  palette.setColor( backgroundRole(), QColor( 192, 192, 192 ) );
  setPalette( palette );

  updateDrawingOrderChecked( mLegend->updateDrawingOrder() );
}

QgsLayerOrder::~QgsLayerOrder()
{
  delete mInsertionLine;
}

void QgsLayerOrder::refreshLayerList()
{
  clear();

  QList<DrawingOrderInfo> drawingOrderList = mLegend->drawingOrder();
  QList<DrawingOrderInfo>::const_iterator it = drawingOrderList.constBegin();
  for ( ; it != drawingOrderList.constEnd(); ++it )
  {
    QListWidgetItem *item = new QListWidgetItem( it->name );
    item->setCheckState( it->checked ? Qt::Checked : Qt::Unchecked );
    item->setData( Qt::UserRole, it->id );
    item->setData( Qt::UserRole + 1, it->embeddedGroup );
    addItem( item );
  }
}

QListWidgetItem *QgsLayerOrder::layerItem( QgsMapLayer *layer ) const
{
  for ( int i = 0; i < count(); i++ )
  {
    QListWidgetItem *lwi = item( i );

    if ( layer == qobject_cast<QgsMapLayer *>( lwi->data( Qt::UserRole ).value<QObject*>() ) )
    {
      return lwi;
    }
  }

  return 0;
}

void QgsLayerOrder::itemChanged( QListWidgetItem *item )
{
  QString name = item->text();
  QString id = item->data( Qt::UserRole ).toString();
  bool embeddedGroup = item->data( Qt::UserRole + 1 ).toBool();
  if ( embeddedGroup )
  {
    QgsLegendGroup* grp = mLegend->findLegendGroup( name, id );
    if ( grp )
    {
      grp->setCheckState( 0, item->checkState() );
    }
  }
  else
  {
    QgsLegendLayer* ll = mLegend->findLegendLayer( id );
    if ( ll )
    {
      ll->setCheckState( 0, item->checkState() );
    }
  }
  updateLayerOrder();
}

void QgsLayerOrder::mousePressEvent( QMouseEvent * e )
{
  QgsDebugMsg( "Entering." );

  if ( e->button() == Qt::LeftButton )
  {
    mPressItem = itemAt( e->pos() );
  }

  QListWidget::mousePressEvent( e );
}

void QgsLayerOrder::mouseMoveEvent( QMouseEvent * e )
{
  if ( !mPressItem || count() == 0 )
    return;

  // start moving when cursor leaves the current item
  if ( itemAt( e->pos() ) == mPressItem )
    return;

  hideLine();

  if ( mItemsBeingMoved.isEmpty() && !selectedItems().isEmpty() )
  {
    for ( int i = 0; i < count(); i++ )
    {
      QListWidgetItem *lwi = item( i );
      if ( lwi->isSelected() )
      {
        QgsDebugMsg( QString( "Take item %1:%2" ).arg( i ).arg( lwi->text() ) );
        mItemsBeingMoved << takeItem( i-- );
      }
    }

    setCursor( Qt::SizeVerCursor );
  }

  if ( mItemsBeingMoved.isEmpty() )
  {
    setCursor( Qt::ArrowCursor );
    mInsertRow = -1;
  }
  else
  {
    QListWidgetItem *rowItem = itemAt( e->pos() );

    if ( rowItem )
    {
      mInsertRow = row( rowItem );
      QgsDebugMsg( QString( "Move to row %1" ).arg( mInsertRow ) );
      scrollToItem( rowItem );
    }
    else if ( e->pos().y() < visualItemRect( item( 0 ) ).bottom() )
    {
      mInsertRow = 0;
      QgsDebugMsg( QString( "Insert top row %1" ).arg( mInsertRow ) );
      scrollToTop();
    }
    else
    {
      mInsertRow = count();
      QgsDebugMsg( QString( "Append bottom row %1" ).arg( mInsertRow ) );
      scrollToBottom();
    }

    int y;
    if ( mInsertRow < count() )
    {
      y = visualItemRect( rowItem ).top();
    }
    else
    {
      y = visualItemRect( item( count() - 1 ) ).bottom();
    }

    mInsertionLine->setGeometry( visualItemRect( item( 0 ) ).left(), y, viewport()->width(), 2 );
  }
}

void QgsLayerOrder::mouseReleaseEvent( QMouseEvent * e )
{
  QgsDebugMsg( "Entering." );
  QListWidget::mouseReleaseEvent( e );
  mPressItem = 0;
  hideLine();

  if ( mItemsBeingMoved.isEmpty() )
    return;

  setCursor( QCursor( Qt::ArrowCursor ) );

  foreach ( QListWidgetItem *item, mItemsBeingMoved )
  {
    if ( mInsertRow >= count() )
    {
      QgsDebugMsg( QString( "Adding item at %1:%2" ).arg( mInsertRow ).arg( item->text() ) );
      addItem( item );
    }
    else
    {
      QgsDebugMsg( QString( "Inserting item at %1:%2" ).arg( mInsertRow ).arg( item->text() ) );
      insertItem( mInsertRow, item );
    }

    mInsertRow++;
  }

  mItemsBeingMoved.clear();
  mInsertRow = -1;

  updateLayerOrder();
}

void QgsLayerOrder::updateLayerOrder()
{
  if ( !isEnabled() )
    return;

  QList<DrawingOrderInfo> drawingOrder;

  for ( int i = 0; i < count(); i++ )
  {
    QListWidgetItem* listItem = item( i );
    if ( !listItem )
    {
      continue;
    }
    DrawingOrderInfo info;
    info.name = listItem->text();
    info.id = listItem->data( Qt::UserRole ).toString();
    info.checked = listItem->checkState() == Qt::Checked;
    info.embeddedGroup = listItem->data( Qt::UserRole + 1 ).toBool();
    drawingOrder.push_back( info );
  }

  mLegend->setDrawingOrder( drawingOrder );
}

void QgsLayerOrder::hideLine()
{
  mInsertionLine->setGeometry( 0, -100, 1, 1 );
}

void QgsLayerOrder::updateDrawingOrderChecked( bool enabled )
{
  setDisabled( enabled );
}
