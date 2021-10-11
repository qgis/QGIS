/***************************************************************************
    qgsprocessingtoolboxtreeview.cpp
    -------------------------------
    begin                : July 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingtoolboxtreeview.h"
#include "qgsprocessingtoolboxmodel.h"

#include <QKeyEvent>

///@cond PRIVATE

QgsProcessingToolboxTreeView::QgsProcessingToolboxTreeView( QWidget *parent,
    QgsProcessingRegistry *registry,
    QgsProcessingRecentAlgorithmLog *recentLog )
  : QTreeView( parent )
{
  mModel = new QgsProcessingToolboxProxyModel( this, registry, recentLog );
  mToolboxModel = mModel->toolboxModel();
  setModel( mModel );
}

void QgsProcessingToolboxTreeView::setRegistry( QgsProcessingRegistry *registry, QgsProcessingRecentAlgorithmLog *recentLog )
{
  QgsProcessingToolboxProxyModel *newModel = new QgsProcessingToolboxProxyModel( this, registry, recentLog );
  mToolboxModel = newModel->toolboxModel();
  setModel( newModel );
  mModel->deleteLater();
  mModel = newModel;
}

void QgsProcessingToolboxTreeView::setToolboxProxyModel( QgsProcessingToolboxProxyModel *model )
{
  mToolboxModel = mModel->toolboxModel();
  setModel( model );
  mModel->deleteLater();
  mModel = model;
}

void QgsProcessingToolboxTreeView::setFilterString( const QString &filter )
{
  const QString text = filter.trimmed().toLower();
  mModel->setFilterString( text );
  if ( !text.isEmpty() )
  {
    expandAll();
    if ( !selectedAlgorithm() )
    {
      // if previously selected item was hidden, auto select the first visible algorithm
      const QModelIndex firstVisibleIndex = findFirstVisibleAlgorithm( QModelIndex() );
      if ( firstVisibleIndex.isValid() )
        selectionModel()->setCurrentIndex( firstVisibleIndex, QItemSelectionModel::ClearAndSelect );
    }
  }
  else
  {
    collapseAll();
  }
}

const QgsProcessingAlgorithm *QgsProcessingToolboxTreeView::algorithmForIndex( const QModelIndex &index )
{
  const QModelIndex sourceIndex = mModel->mapToSource( index );
  if ( mToolboxModel->isAlgorithm( sourceIndex ) )
    return mToolboxModel->algorithmForIndex( sourceIndex );
  else
    return nullptr;
}

const QgsProcessingAlgorithm *QgsProcessingToolboxTreeView::selectedAlgorithm()
{
  if ( selectionModel()->hasSelection() )
  {
    const QModelIndex index = selectionModel()->selectedIndexes().at( 0 );
    return algorithmForIndex( index );
  }
  else
  {
    return nullptr;
  }
}

void QgsProcessingToolboxTreeView::setFilters( QgsProcessingToolboxProxyModel::Filters filters )
{
  mModel->setFilters( filters );
}

QgsProcessingToolboxProxyModel::Filters QgsProcessingToolboxTreeView::filters() const
{
  return mModel->filters();
}

void QgsProcessingToolboxTreeView::setInPlaceLayer( QgsVectorLayer *layer )
{
  mModel->setInPlaceLayer( layer );
}

QModelIndex QgsProcessingToolboxTreeView::findFirstVisibleAlgorithm( const QModelIndex &parent )
{
  for ( int r = 0; r < mModel->rowCount( parent ); ++r )
  {
    QModelIndex proxyIndex = mModel->index( r, 0, parent );
    const QModelIndex sourceIndex = mModel->mapToSource( proxyIndex );
    if ( mToolboxModel->isAlgorithm( sourceIndex ) )
      return proxyIndex;

    QModelIndex index = findFirstVisibleAlgorithm( proxyIndex );
    if ( index.isValid() )
      return index;
  }
  return QModelIndex();
}

void QgsProcessingToolboxTreeView::keyPressEvent( QKeyEvent *event )
{
  if ( event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter )
    emit doubleClicked( currentIndex() );
  else
    QTreeView::keyPressEvent( event );
}

///@endcond
