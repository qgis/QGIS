/***************************************************************************
 qgssvgbrowserwidget.cpp

 ---------------------
 begin                : October 2020
 copyright            : (C) 2020 by Denis Rouzaud
 email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgis.h"
#include "qgssvgbrowserwidget.h"
#include "qgssvgselectorwidget.h"


QgsSvgBrowserWidget::QgsSvgBrowserWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );
  viewGroups->setHeaderHidden( true );


#if QT_VERSION < QT_VERSION_CHECK(5, 11, 0)
  mIconSize = std::max( 30, static_cast< int >( std::round( Qgis::UI_SCALE_FACTOR * fontMetrics().width( 'X' ) * 4 ) ) );
#else
  mIconSize = std::max( 30, static_cast< int >( std::round( Qgis::UI_SCALE_FACTOR * fontMetrics().horizontalAdvance( 'X' ) * 4 ) ) );
#endif
  viewImages->setGridSize( QSize( mIconSize * 1.2, mIconSize * 1.2 ) );

  connect( viewImages->selectionModel(), &QItemSelectionModel::currentChanged, this, &QgsSvgBrowserWidget::onSelectionChange );
}


void QgsSvgBrowserWidget::populateList()
{
  QAbstractItemModel *oldModel = viewGroups->model();
  QgsSvgSelectorGroupsModel *g = new QgsSvgSelectorGroupsModel( viewGroups );
  viewGroups->setModel( g );
  delete oldModel;

  // Set the tree expanded at the first level
  int rows = g->rowCount( g->indexFromItem( g->invisibleRootItem() ) );
  for ( int i = 0; i < rows; i++ )
  {
    viewGroups->setExpanded( g->indexFromItem( g->item( i ) ), true );
  }

  // Initially load the icons in the List view without any grouping
  oldModel = viewImages->model();
  QgsSvgSelectorListModel *m = new QgsSvgSelectorListModel( viewImages, mIconSize );
  viewImages->setModel( m );
  delete oldModel;

  connect( viewGroups->selectionModel(), &QItemSelectionModel::currentChanged, this, &QgsSvgBrowserWidget::populateIcons );

  mListPopulated = true;
  mSelectionDirty = true;
}

void QgsSvgBrowserWidget::updateSelection()
{
  QAbstractItemModel *m = viewImages->model();
  QItemSelectionModel *selModel = viewImages->selectionModel();
  if ( mPath.isEmpty() )
  {
    selModel->clearSelection();
    selModel->clearCurrentIndex();
  }
  else
  {
    for ( int i = 0; i < m->rowCount(); i++ )
    {
      QModelIndex idx( m->index( i, 0 ) );
      if ( m->data( idx ).toString() == mPath )
      {
        selModel->select( idx, QItemSelectionModel::SelectCurrent );
        selModel->setCurrentIndex( idx, QItemSelectionModel::SelectCurrent );
        break;
      }
    }
  }
  mSelectionDirty = false;
}

void QgsSvgBrowserWidget::populateIcons( const QModelIndex &idx )
{
  QString path = idx.data( Qt::UserRole + 1 ).toString();

  QAbstractItemModel *oldModel = viewImages->model();
  QgsSvgSelectorListModel *m = new QgsSvgSelectorListModel( viewImages, path, mIconSize );
  viewImages->setModel( m );
  delete oldModel;

  connect( viewImages->selectionModel(), &QItemSelectionModel::currentChanged, this, &QgsSvgBrowserWidget::onSelectionChange );
}

void QgsSvgBrowserWidget::setPath( const QString &path )
{
  if ( path == mPath )
    return;

  mPath = path;
  mSelectionDirty = true;

  if ( isVisible() )
    updateSelection();

  emit pathChanged( mPath );
}

void QgsSvgBrowserWidget::onSelectionChange( const QModelIndex index )
{
  QString path = index.data( Qt::UserRole ).toString();
  if ( mPath == path )
    return;

  mPath = path;
  emit pathChanged( mPath );
}


void QgsSvgBrowserWidget::showEvent( QShowEvent *event )
{
  QWidget::showEvent( event );

  if ( !mListPopulated )
    populateList();

  if ( mSelectionDirty )
    updateSelection();
}
