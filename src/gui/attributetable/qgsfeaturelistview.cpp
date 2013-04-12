/***************************************************************************
     QgsAttributeTableView.cpp
     --------------------------------------
    Date                 : Feb 2009
    Copyright            : (C) 2009 Vita Cizek
    Email                : weetya (at) gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QKeyEvent>
#include <QSettings>
#include <QHeaderView>
#include <QMenu>

#include "qgsfeaturelistview.h"
#include "qgsattributetablemodel.h"
#include "qgsattributetabledelegate.h"
#include "qgsattributetablefiltermodel.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsfeaturelistviewdelegate.h"
#include "qgsfeaturelistmodel.h"
#include <QSet>

QgsFeatureListView::QgsFeatureListView( QWidget *parent )
    : QListView( parent ),
    mCurrentEditSelectionModel( NULL ),
    mItemDelegate( NULL ),
    mEditSelectionDrag( false )
{
  setSelectionMode( QAbstractItemView::ExtendedSelection );
}

QgsVectorLayerCache *QgsFeatureListView::layerCache()
{
  return mModel->layerCache();
}

void QgsFeatureListView::setModel( QgsFeatureListModel* featureListModel )
{
  QListView::setModel( featureListModel );
  mModel = featureListModel;

  mMasterSelection = featureListModel->masterSelection();

  connect( mMasterSelection, SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ), SLOT( onMasterSelectionChanged( QItemSelection, QItemSelection ) ) );
  connect( selectionModel(), SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ), SLOT( onSelectionChanged( QItemSelection, QItemSelection ) ) );

  connect( mModel->sourceModel(), SIGNAL( filterAboutToBeInvalidated() ), SLOT( onFilterAboutToBeInvalidated() ) );
  connect( mModel->sourceModel(), SIGNAL( filterInvalidated() ), SLOT( onFilterInvalidated() ) );

  mCurrentEditSelectionModel = new QItemSelectionModel( mModel->masterModel(), this );

  if ( mItemDelegate && mItemDelegate->parent() == this )
  {
    delete mItemDelegate;
  }

  mItemDelegate = new QgsFeatureListViewDelegate( mModel, this );
  mItemDelegate->setEditSelectionModel( mCurrentEditSelectionModel );
  setItemDelegate( mItemDelegate );

  connect( mCurrentEditSelectionModel, SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ), SLOT( editSelectionChanged( QItemSelection, QItemSelection ) ) );
}

bool QgsFeatureListView::setDisplayExpression( const QString expression )
{
  if ( mModel->setDisplayExpression( expression ) )
  {
    emit displayExpressionChanged( expression );
    return true;
  }
  else
  {
    return false;
  }
}

const QString& QgsFeatureListView::displayExpression() const
{
  return mModel->displayExpression();
}

QString QgsFeatureListView::parserErrorString()
{
  return mModel->parserErrorString();
}

void QgsFeatureListView::mouseMoveEvent( QMouseEvent *event )
{
  QPoint pos = event->pos();

  if ( mEditSelectionDrag )
  {
    QModelIndex index = mModel->mapToMaster( indexAt( pos ) );

    mCurrentEditSelectionModel->select( index, QItemSelectionModel::ClearAndSelect );
  }
  else
  {
    QListView::mouseMoveEvent( event );
  }
}

void QgsFeatureListView::mousePressEvent( QMouseEvent *event )
{
  QPoint pos = event->pos();

  if ( QgsFeatureListViewDelegate::EditElement == mItemDelegate->positionToElement( event->pos() ) )
  {
    mEditSelectionDrag = true;
    QModelIndex index = mModel->mapToMaster( indexAt( pos ) );

    mCurrentEditSelectionModel->select( index, QItemSelectionModel::ClearAndSelect );
  }
  else
  {
    mModel->disableSelectionSync();
    QListView::mousePressEvent( event );
  }
}

void QgsFeatureListView::mouseReleaseEvent( QMouseEvent *event )
{
  if ( mEditSelectionDrag )
  {
    mEditSelectionDrag = false;
  }
  else
  {
    QListView::mouseReleaseEvent( event );
    mModel->enableSelectionSync();
  }
}

void QgsFeatureListView::editSelectionChanged( QItemSelection deselected, QItemSelection selected )
{
  if ( isVisible() && updatesEnabled() )
  {
    QItemSelection localDeselected = mModel->mapSelectionFromMaster( deselected );
    QItemSelection localSelected = mModel->mapSelectionFromMaster( selected );
    viewport()->update( visualRegionForSelection( localDeselected ) | visualRegionForSelection( localSelected ) );
  }

  QItemSelection currentSelection = mCurrentEditSelectionModel->selection();
  if ( currentSelection.size() == 1 )
  {
    QgsFeature feat;
    mModel->featureByIndex( mModel->mapFromMaster( currentSelection.indexes().first() ), feat );

    emit currentEditSelectionChanged( feat );
  }
}

void QgsFeatureListView::onFilterAboutToBeInvalidated()
{
  disconnect( selectionModel(), SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ), this, SLOT( onSelectionChanged( QItemSelection, QItemSelection ) ) );
}

void QgsFeatureListView::onFilterInvalidated()
{
  QItemSelection localSelection = mModel->mapSelectionFromMaster( mMasterSelection->selection() );
  selectionModel()->select( localSelection, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
  connect( selectionModel(), SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ), this, SLOT( onSelectionChanged( QItemSelection, QItemSelection ) ) );
}

void QgsFeatureListView::onSelectionChanged( const QItemSelection& selected, const QItemSelection& deselected )
{
  disconnect( mMasterSelection, SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ), this, SLOT( onMasterSelectionChanged( QItemSelection, QItemSelection ) ) );
  QItemSelection masterSelected = mModel->mapSelectionToMaster( selected );
  QItemSelection masterDeselected = mModel->mapSelectionToMaster( deselected );

  mMasterSelection->select( masterSelected, QItemSelectionModel::Select );
  mMasterSelection->select( masterDeselected, QItemSelectionModel::Deselect );
  connect( mMasterSelection, SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ), SLOT( onMasterSelectionChanged( QItemSelection, QItemSelection ) ) );
}

void QgsFeatureListView::onMasterSelectionChanged( const QItemSelection& selected, const QItemSelection& deselected )
{
  Q_UNUSED( selected )
  Q_UNUSED( deselected )
  disconnect( selectionModel(), SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ), this, SLOT( onSelectionChanged( QItemSelection, QItemSelection ) ) );

  // Synchronizing the whole selection seems to work faster than using the deltas (Deselecting takes pretty long)
  QItemSelection localSelection = mModel->mapSelectionFromMaster( mMasterSelection->selection() );
  selectionModel()->select( localSelection, QItemSelectionModel::ClearAndSelect );

  connect( selectionModel(), SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ), this, SLOT( onSelectionChanged( QItemSelection, QItemSelection ) ) );
}

void QgsFeatureListView::selectAll()
{
  disconnect( selectionModel(), SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ), this, SLOT( onSelectionChanged( QItemSelection, QItemSelection ) ) );
  QItemSelection selection;
  selection.append( QItemSelectionRange( mModel->index( 0, 0 ), mModel->index( mModel->rowCount() - 1, 0 ) ) );
  selectionModel()->select( selection, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
  connect( selectionModel(), SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ), this, SLOT( onSelectionChanged( QItemSelection, QItemSelection ) ) );
}

void QgsFeatureListView::setEditSelection( const QgsFeatureIds &fids )
{
  QItemSelection selection;

  foreach ( QgsFeatureId fid, fids )
  {
    selection.append( QItemSelectionRange( mModel->mapToMaster( mModel->fidToIdx( fid ) ) ) );
  }

  mCurrentEditSelectionModel->select( selection, QItemSelectionModel::ClearAndSelect );
}
