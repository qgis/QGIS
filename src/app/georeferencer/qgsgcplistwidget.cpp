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

#include "qgsproject.h"
#include "qgsexception.h"
#include "qgscoordinatetransform.h"
#include "qgsgeorefdelegates.h"
#include "qgsgeorefdatapoint.h"
#include "qgsgcplist.h"
#include "qgsgcplistwidget.h"
#include "qgsgcplistmodel.h"

QgsGCPListWidget::QgsGCPListWidget( QWidget *parent )
  : QTableView( parent )
  , mGCPListModel( new QgsGCPListModel( this ) )
  , mNonEditableDelegate( new QgsNonEditableDelegate( this ) )
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
  setItemDelegateForColumn( 1, mNonEditableDelegate ); // id
  setItemDelegateForColumn( 2, mNonEditableDelegate ); // crs
  setItemDelegateForColumn( 3, mCoordDelegate ); // srcX
  setItemDelegateForColumn( 4, mCoordDelegate ); // srcY
  setItemDelegateForColumn( 5, mCoordDelegate ); // gcpX
  setItemDelegateForColumn( 6, mCoordDelegate ); // gcpY
  setItemDelegateForColumn( 7, mCoordDelegate ); // dstX
  setItemDelegateForColumn( 8, mCoordDelegate ); // dstY
  setItemDelegateForColumn( 9, mNonEditableDelegate ); // dX
  setItemDelegateForColumn( 10, mNonEditableDelegate ); // dY
  setItemDelegateForColumn( 11, mNonEditableDelegate ); // residual

  connect( this, &QAbstractItemView::doubleClicked,
           this, &QgsGCPListWidget::itemDoubleClicked );
  connect( this, &QAbstractItemView::clicked,
           this, &QgsGCPListWidget::itemClicked );
  connect( this, &QWidget::customContextMenuRequested,
           this, &QgsGCPListWidget::showContextMenu );

  connect( mDmsAndDdDelegate, &QAbstractItemDelegate::closeEditor,
           this, &QgsGCPListWidget::updateItemCoords );
  connect( mCoordDelegate, &QAbstractItemDelegate::closeEditor,
           this, &QgsGCPListWidget::updateItemCoords );
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

void QgsGCPListWidget::updateGCPList()
{
  mGCPListModel->updateModel();
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

void QgsGCPListWidget::itemDoubleClicked( QModelIndex index )
{
  index = static_cast<const QSortFilterProxyModel *>( model() )->mapToSource( index );
  QStandardItem *item = mGCPListModel->item( index.row(), 1 );
  bool ok;
  const int id = item->text().toInt( &ok );

  if ( ok )
  {
    emit jumpToGCP( id );
  }
}

void QgsGCPListWidget::itemClicked( QModelIndex index )
{
  index = static_cast<const QSortFilterProxyModel *>( model() )->mapToSource( index );
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
  else if ( e->key() == Qt::Key_Space )
  {
    const QModelIndex index = currentIndex();
    if ( index.isValid() )
    {
      const QModelIndex sourceIndex = static_cast<const QSortFilterProxyModel *>( model() )->mapToSource( index );
      QgsGeorefDataPoint *p = mGCPList->at( sourceIndex.row() );
      p->setEnabled( !p->isEnabled() );

      mGCPListModel->updateModel();
      emit pointEnabled( p, sourceIndex.row() );
      adjustTableContent();
      setCurrentIndex( model()->index( index.row(), index.column() ) );
      return;
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

void QgsGCPListWidget::updateItemCoords( QWidget *editor )
{
  QLineEdit *lineEdit = qobject_cast<QLineEdit *>( editor );
  QgsGeorefDataPoint *dataPoint = mGCPList->at( mPrevRow );
  if ( lineEdit )
  {
    bool changeX = false;
    const double value = lineEdit->text().toDouble();
    QgsPointXY tempPoint;

    switch ( mPrevColumn )
    {
      case 3:
        changeX = true;
      case 4:
        tempPoint = dataPoint->pixelCoords();
        if ( changeX )
          tempPoint.setX( value );
        else
          tempPoint.setY( value );
        dataPoint->setPixelCoords( tempPoint );
        break;

      case 5:
        changeX = true;
      case 6:
        tempPoint = dataPoint->mapCoords();
        if ( changeX )
          tempPoint.setX( value );
        else
          tempPoint.setY( value );
        dataPoint->setMapCoords( tempPoint );
        break;

      case 7:
        changeX = true;
      case 8:
        tempPoint = dataPoint->transCoords();
        if ( changeX )
          tempPoint.setX( value );
        else
          tempPoint.setY( value );
        try
        {
          tempPoint =  QgsCoordinateTransform( mGCPList->targetCRS(), dataPoint->crs(),
                                                 QgsProject::instance() ).transform( tempPoint );
        }
        catch ( const QgsException &e )
        {
          Q_UNUSED( e );
          return;
        }
        dataPoint->setMapCoords( tempPoint );
        break;

      default:
        return;
    }
  }

  dataPoint->updateCoords();
  updateGCPList();
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
  connect( jumpToPointAction, &QAction::triggered, this, &QgsGCPListWidget::jumpToPoint );
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

void QgsGCPListWidget::editCell()
{
  edit( currentIndex() );
}

void QgsGCPListWidget::jumpToPoint()
{
  const QModelIndex index = static_cast<const QSortFilterProxyModel *>( model() )->mapToSource( currentIndex() );
  mPrevRow = index.row();
  mPrevColumn = index.column();
  emit jumpToGCP( index.row() );
}

void QgsGCPListWidget::adjustTableContent()
{
  resizeColumnsToContents();
  resizeRowsToContents();
}
