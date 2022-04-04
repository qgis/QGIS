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

#include <QKeyEvent>
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
  : QgsTableView( parent )
  , mGCPListModel( new QgsGCPListModel( this ) )
  , mDmsAndDdDelegate( new QgsDmsAndDdDelegate( this ) )
  , mCoordDelegate( new QgsCoordDelegate( this ) )
{
  // Create a proxy model, which will handle dynamic sorting
  QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel( this );
  proxyModel->setSourceModel( mGCPListModel );
  proxyModel->setDynamicSortFilter( true );
  proxyModel->setSortRole( Qt::UserRole );
  setModel( proxyModel );
  setSortingEnabled( true );

  setContextMenuPolicy( Qt::CustomContextMenu );
  setFocusPolicy( Qt::ClickFocus );

  verticalHeader()->hide();
  setAlternatingRowColors( true );

  // set delegates for items
  setItemDelegateForColumn( static_cast< int >( QgsGCPListModel::Column::SourceX ), mCoordDelegate );
  setItemDelegateForColumn( static_cast< int >( QgsGCPListModel::Column::SourceY ), mCoordDelegate );
  setItemDelegateForColumn( static_cast< int >( QgsGCPListModel::Column::DestinationX ), mDmsAndDdDelegate );
  setItemDelegateForColumn( static_cast< int >( QgsGCPListModel::Column::DestinationY ), mDmsAndDdDelegate );

  connect( this, &QAbstractItemView::doubleClicked,
           this, &QgsGCPListWidget::itemDoubleClicked );
  connect( this, &QAbstractItemView::clicked,
           this, &QgsGCPListWidget::itemClicked );
  connect( this, &QWidget::customContextMenuRequested,
           this, &QgsGCPListWidget::showContextMenu );

  connect( mGCPListModel, &QgsGCPListModel::pointEnabled, this, [ = ]( QgsGeorefDataPoint * point, int row )
  {
    emit pointEnabled( point, row );
    adjustTableContent();
    return;
  } );
}

void QgsGCPListWidget::setGCPList( QgsGCPList *theGCPList )
{
  mGCPListModel->setGCPList( theGCPList );
  mGCPList = theGCPList;

  adjustTableContent();
}

void QgsGCPListWidget::setGeorefTransform( QgsGeorefTransform *georefTransform )
{
  mGCPListModel->setGeorefTransform( georefTransform );
  adjustTableContent();
}

void QgsGCPListWidget::setTargetCrs( const QgsCoordinateReferenceSystem &targetCrs, const QgsCoordinateTransformContext &context )
{
  mGCPListModel->setTargetCrs( targetCrs, context );
  adjustTableContent();
}

void QgsGCPListWidget::updateResiduals()
{
  mGCPListModel->updateResiduals();
  adjustTableContent();
}

void QgsGCPListWidget::closeEditors()
{
  const QModelIndexList selection = selectedIndexes();
  for ( const QModelIndex &index : selection )
  {
    closePersistentEditor( index );
  }
}

void QgsGCPListWidget::itemDoubleClicked( const QModelIndex &index )
{
  const QModelIndex sourceIndex = static_cast<const QSortFilterProxyModel *>( model() )->mapToSource( index );
  jumpToSourcePoint( sourceIndex );
}

void QgsGCPListWidget::itemClicked( const QModelIndex &index )
{
  const QModelIndex sourceIndex = static_cast<const QSortFilterProxyModel *>( model() )->mapToSource( index );

  mPrevRow = sourceIndex.row();
  mPrevColumn = sourceIndex.column();
}

void QgsGCPListWidget::keyPressEvent( QKeyEvent *e )
{
  if ( e->key() == Qt::Key_Delete )
  {
    const QModelIndex index = currentIndex();
    if ( index.isValid() )
    {
      removeRow();
      if ( model()->rowCount() > 0 )
      {
        setCurrentIndex( model()->index( index.row() == model()->rowCount() ? index.row() - 1 : index.row(), index.column() ) );
        return;
      }
    }
  }
  else if ( e->key() == Qt::Key_Up )
  {
    const QModelIndex index = currentIndex();
    if ( index.isValid() && index.row() > 0 )
    {
      setCurrentIndex( model()->index( index.row() - 1, index.column() ) );
      return;
    }
  }
  else if ( e->key() == Qt::Key_Down )
  {
    const QModelIndex index = currentIndex();
    if ( index.isValid() && index.row() < ( model()->rowCount() - 1 ) )
    {
      setCurrentIndex( model()->index( index.row() + 1, index.column() ) );
      return;
    }
  }
  else if ( e->key() == Qt::Key_Left )
  {
    const QModelIndex index = currentIndex();
    if ( index.isValid() && index.column() > 0 )
    {
      setCurrentIndex( model()->index( index.row(), index.column() - 1 ) );
      return;
    }
  }
  else if ( e->key() == Qt::Key_Right )
  {
    const QModelIndex index = currentIndex();
    if ( index.isValid() && index.column() < ( model()->columnCount() - 1 ) )
    {
      setCurrentIndex( model()->index( index.row(), index.column() + 1 ) );
      return;
    }
  }
  e->ignore();
}

void QgsGCPListWidget::showContextMenu( QPoint p )
{
  if ( !mGCPList || 0 == mGCPList->count() )
    return;

  QMenu m;// = new QMenu(this);
  const QModelIndex index = indexAt( p );
  if ( index == QModelIndex() )
    return;

  // Select the right-clicked item
  setCurrentIndex( index );

  QAction *jumpToPointAction = new QAction( tr( "Recenter" ), this );
  connect( jumpToPointAction, &QAction::triggered, this, [ = ]
  {
    const QModelIndex sourceIndex = static_cast<const QSortFilterProxyModel *>( model() )->mapToSource( currentIndex() );
    mPrevRow = sourceIndex.row();
    mPrevColumn = sourceIndex.column();
    jumpToSourcePoint( sourceIndex );
  } );
  m.addAction( jumpToPointAction );

  QAction *removeAction = new QAction( tr( "Remove" ), this );
  connect( removeAction, &QAction::triggered, this, &QgsGCPListWidget::removeRow );
  m.addAction( removeAction );
  m.exec( QCursor::pos(), removeAction );
}

void QgsGCPListWidget::removeRow()
{
  const QModelIndex index = static_cast<const QSortFilterProxyModel *>( model() )->mapToSource( currentIndex() );
  emit deleteDataPoint( index.row() );
}

void QgsGCPListWidget::jumpToSourcePoint( const QModelIndex &modelIndex )
{
  const QgsPointXY sourcePoint = mGCPListModel->data( modelIndex, static_cast< int >( QgsGCPListModel::Role::SourcePointRole ) ).value< QgsPointXY >();
  if ( !sourcePoint.isEmpty() )
  {
    emit jumpToGCP( sourcePoint );
  }
}

void QgsGCPListWidget::adjustTableContent()
{
  resizeColumnsToContents();
  resizeRowsToContents();
}
