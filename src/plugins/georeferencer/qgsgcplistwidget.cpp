/***************************************************************************
    qgsgcplistwidget.cpp - Widget for GCP list display
     --------------------------------------
    Date                 : 27-Feb-2009
    Copyright            : (c) 2009 by Manuel Massing
    Email                : m.massing at warped-space.de
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QHeaderView>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QMenu>
#include <QSortFilterProxyModel>

#include "qgsgeorefdelegates.h"
#include "qgsgeorefdatapoint.h"
#include "qgsgcplist.h"
#include "qgsgcplistwidget.h"
#include "qgsgcplistmodel.h"

QgsGCPListWidget::QgsGCPListWidget( QWidget *parent )
    : QTableView( parent )
    , mGCPList( 0 )
    , mGCPListModel( new QgsGCPListModel( this ) )
    , mNonEditableDelegate( new QgsNonEditableDelegate( this ) )
    , mDmsAndDdDelegate( new QgsDmsAndDdDelegate( this ) )
    , mCoordDelegate( new QgsCoordDelegate( this ) )
    , mPrevRow( 0 )
    , mPrevColumn( 0 )
{
  // Create a proxy model, which will handle dynamic sorting
  QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel( this );
  proxyModel->setSourceModel( mGCPListModel );
  proxyModel->setDynamicSortFilter( true );
  proxyModel->setSortRole( Qt::UserRole );
  setModel( proxyModel );
  setSortingEnabled( true );

  setContextMenuPolicy( Qt::CustomContextMenu );
  setFocusPolicy( Qt::NoFocus );

  verticalHeader()->hide();
  setAlternatingRowColors( true );

  // set delegates for items
  setItemDelegateForColumn( 1, mNonEditableDelegate ); // id
  setItemDelegateForColumn( 2, mCoordDelegate ); // srcX
  setItemDelegateForColumn( 3, mCoordDelegate ); // srcY
  setItemDelegateForColumn( 4, mDmsAndDdDelegate ); // dstX
  setItemDelegateForColumn( 5, mDmsAndDdDelegate ); // dstY
  setItemDelegateForColumn( 6, mNonEditableDelegate ); // dX
  setItemDelegateForColumn( 7, mNonEditableDelegate ); // dY
  setItemDelegateForColumn( 8, mNonEditableDelegate ); // residual

  connect( this, SIGNAL( doubleClicked( QModelIndex ) ),
           this, SLOT( itemDoubleClicked( QModelIndex ) ) );
  connect( this, SIGNAL( clicked( QModelIndex ) ),
           this, SLOT( itemClicked( QModelIndex ) ) );
  connect( this, SIGNAL( customContextMenuRequested( QPoint ) ),
           this, SLOT( showContextMenu( QPoint ) ) );

  connect( this, SIGNAL( replaceDataPoint( QgsGeorefDataPoint*, int ) ),
           mGCPListModel, SLOT( replaceDataPoint( QgsGeorefDataPoint*, int ) ) );

  connect( mDmsAndDdDelegate, SIGNAL( closeEditor( QWidget* ) ),
           this, SLOT( updateItemCoords( QWidget* ) ) );
  connect( mCoordDelegate, SIGNAL( closeEditor( QWidget* ) ),
           this, SLOT( updateItemCoords( QWidget* ) ) );
}

void QgsGCPListWidget::setGCPList( QgsGCPList *theGCPList )
{
  mGCPListModel->setGCPList( theGCPList );
  mGCPList = theGCPList;

  adjustTableContent();
}

void QgsGCPListWidget::setGeorefTransform( QgsGeorefTransform *theGeorefTransform )
{
  mGCPListModel->setGeorefTransform( theGeorefTransform );
  adjustTableContent();
}

void QgsGCPListWidget::updateGCPList()
{
  mGCPListModel->updateModel();
  adjustTableContent();
}

void QgsGCPListWidget::closeEditors()
{
  Q_FOREACH ( const QModelIndex& index, selectedIndexes() )
  {
    closePersistentEditor( index );
  }
}

void QgsGCPListWidget::itemDoubleClicked( QModelIndex index )
{
  index = static_cast<const QSortFilterProxyModel*>( model() )->mapToSource( index );
  QStandardItem *item = mGCPListModel->item( index.row(), 1 );
  bool ok;
  int id = item->text().toInt( &ok );

  if ( ok )
  {
    emit jumpToGCP( id );
  }
}

void QgsGCPListWidget::itemClicked( QModelIndex index )
{
  index = static_cast<const QSortFilterProxyModel*>( model() )->mapToSource( index );
  QStandardItem *item = mGCPListModel->item( index.row(), index.column() );
  if ( item->isCheckable() )
  {
    QgsGeorefDataPoint *p = mGCPList->at( index.row() );
    if ( item->checkState() == Qt::Checked )
    {
      p->setEnabled( true );
    }
    else // Qt::Unchecked
    {
      p->setEnabled( false );
    }

    mGCPListModel->updateModel();
    emit pointEnabled( p, index.row() );
    adjustTableContent();
  }

  mPrevRow = index.row();
  mPrevColumn = index.column();
}

void QgsGCPListWidget::updateItemCoords( QWidget *editor )
{
  QLineEdit *lineEdit = qobject_cast<QLineEdit *>( editor );
  QgsGeorefDataPoint *dataPoint = mGCPList->at( mPrevRow );
  if ( lineEdit )
  {
    double value = lineEdit->text().toDouble();
    QgsPoint newMapCoords( dataPoint->mapCoords() );
    QgsPoint newPixelCoords( dataPoint->pixelCoords() );
    if ( mPrevColumn == 2 ) // srcX
    {
      newPixelCoords.setX( value );
    }
    else if ( mPrevColumn == 3 ) // srcY
    {
      newPixelCoords.setY( value );
    }
    else if ( mPrevColumn == 4 ) // dstX
    {
      newMapCoords.setX( value );
    }
    else if ( mPrevColumn == 5 ) // dstY
    {
      newMapCoords.setY( value );
    }
    else
    {
      return;
    }

    dataPoint->setPixelCoords( newPixelCoords );
    dataPoint->setMapCoords( newMapCoords );
  }

  dataPoint->updateCoords();
  updateGCPList();
}

void QgsGCPListWidget::showContextMenu( QPoint p )
{
  if ( !mGCPList || 0 == mGCPList->count() )
    return;

  QMenu m;// = new QMenu(this);
  QModelIndex index = indexAt( p );
  if ( index == QModelIndex() )
    return;

  // Select the right-clicked item
  setCurrentIndex( index );

  QAction *jumpToPointAction = new QAction( tr( "Recenter" ), this );
  connect( jumpToPointAction, SIGNAL( triggered() ), this, SLOT( jumpToPoint() ) );
  m.addAction( jumpToPointAction );

  QAction *removeAction = new QAction( tr( "Remove" ), this );
  connect( removeAction, SIGNAL( triggered() ), this, SLOT( removeRow() ) );
  m.addAction( removeAction );
  m.exec( QCursor::pos(), removeAction );

  index = static_cast<const QSortFilterProxyModel*>( model() )->mapToSource( index );
  mPrevRow = index.row();
  mPrevColumn = index.column();
}

void QgsGCPListWidget::removeRow()
{
  QModelIndex index = static_cast<const QSortFilterProxyModel*>( model() )->mapToSource( currentIndex() );
  emit deleteDataPoint( index.row() );
}

void QgsGCPListWidget::editCell()
{
  edit( currentIndex() );
}

void QgsGCPListWidget::jumpToPoint()
{
  QModelIndex index = static_cast<const QSortFilterProxyModel*>( model() )->mapToSource( currentIndex() );
  emit jumpToGCP( index.row() );
}

void QgsGCPListWidget::adjustTableContent()
{
  resizeColumnsToContents();
  resizeRowsToContents();
}
