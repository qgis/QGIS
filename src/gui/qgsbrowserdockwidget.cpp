/***************************************************************************
    qgsbrowserdockwidget.cpp
    ---------------------
    begin                : July 2011
    copyright            : (C) 2011 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsbrowserdockwidget.h"
#include "qgsbrowserdockwidget_p.h"
#include "qgsbrowserwidget.h"
#include "qgsbrowserproxymodel.h"
#include "qgsbrowserguimodel.h"
#include "qgsdirectoryitem.h"
#include "qgsprojectitem.h"
#include "qgslayeritem.h"

#include <QVBoxLayout>
#include <QFileDialog>

QgsBrowserDockWidget::QgsBrowserDockWidget( const QString &name, QgsBrowserGuiModel *browserModel, QWidget *parent )
  : QgsDockWidget( parent )
{
  QVBoxLayout *layout = new QVBoxLayout();
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->setSpacing( 0 );
  QWidget *container = new QWidget();
  container->setLayout( layout );
  setWidget( container );

  setWindowTitle( name );

  mWidget = new QgsBrowserWidget( browserModel );
  layout->addWidget( mWidget );

  connect( mWidget, &QgsBrowserWidget::openFile, this, &QgsBrowserDockWidget::openFile );
  connect( mWidget, &QgsBrowserWidget::handleDropUriList, this, &QgsBrowserDockWidget::handleDropUriList );
  connect( mWidget, &QgsBrowserWidget::connectionsChanged, this, &QgsBrowserDockWidget::connectionsChanged );
}

QgsBrowserDockWidget::~QgsBrowserDockWidget() = default;

QgsBrowserWidget *QgsBrowserDockWidget::browserWidget()
{
  return mWidget;
}

void QgsBrowserDockWidget::showContextMenu( QPoint pt )
{
  mWidget->showContextMenu( pt );
}

void QgsBrowserDockWidget::addFavorite()
{
  const QModelIndex index = mWidget->mProxyModel->mapToSource( mWidget->mBrowserView->currentIndex() );
  QgsDataItem *item = mWidget->mModel->dataItem( index );
  if ( !item )
    return;

  QgsDirectoryItem *dirItem = qobject_cast<QgsDirectoryItem *>( item );
  if ( !dirItem )
    return;

  Q_NOWARN_DEPRECATED_PUSH
  addFavoriteDirectory( dirItem->dirPath() );
  Q_NOWARN_DEPRECATED_POP
}

void QgsBrowserDockWidget::addFavoriteDirectory()
{
  const QString directory = QFileDialog::getExistingDirectory( this, tr( "Add directory to favorites" ) );
  if ( !directory.isEmpty() )
  {
    Q_NOWARN_DEPRECATED_PUSH
    addFavoriteDirectory( directory );
    Q_NOWARN_DEPRECATED_POP
  }
}

void QgsBrowserDockWidget::addFavoriteDirectory( const QString &favDir, const QString &name )
{
  mWidget->mModel->addFavoriteDirectory( favDir, name );
}

void QgsBrowserDockWidget::setMessageBar( QgsMessageBar *bar )
{
  mWidget->setMessageBar( bar );
}

QgsMessageBar *QgsBrowserDockWidget::messageBar()
{
  return mWidget->messageBar();
}

void QgsBrowserDockWidget::setDisabledDataItemsKeys( const QStringList &filter )
{
  mWidget->setDisabledDataItemsKeys( filter );
}

void QgsBrowserDockWidget::removeFavorite()
{
  mWidget->mModel->removeFavorite( mWidget->mProxyModel->mapToSource( mWidget->mBrowserView->currentIndex() ) );
}

void QgsBrowserDockWidget::refresh()
{
  mWidget->refreshModel( QModelIndex() );
}

bool QgsBrowserDockWidget::addLayerAtIndex( const QModelIndex &index )
{
  QgsDebugMsg( QStringLiteral( "rowCount() = %1" ).arg( mWidget->mModel->rowCount( mWidget->mProxyModel->mapToSource( index ) ) ) );
  QgsDataItem *item = mWidget->mModel->dataItem( mWidget->mProxyModel->mapToSource( index ) );

  if ( item && item->type() == Qgis::BrowserItemType::Project )
  {
    QgsProjectItem *projectItem = qobject_cast<QgsProjectItem *>( item );
    if ( projectItem )
    {
      QApplication::setOverrideCursor( Qt::WaitCursor );
      emit openFile( projectItem->path(), QStringLiteral( "project" ) );
      QApplication::restoreOverrideCursor();
    }
    return true;
  }
  else if ( item && item->type() == Qgis::BrowserItemType::Layer )
  {
    QgsLayerItem *layerItem = qobject_cast<QgsLayerItem *>( item );
    if ( layerItem )
    {
      QApplication::setOverrideCursor( Qt::WaitCursor );
      mWidget->addLayer( layerItem );
      QApplication::restoreOverrideCursor();
    }
    return true;
  }
  return false;
}

void QgsBrowserDockWidget::addSelectedLayers()
{
  mWidget->addSelectedLayers();
}

void QgsBrowserDockWidget::hideItem()
{
  mWidget->hideItem();
}

void QgsBrowserDockWidget::showProperties()
{
  mWidget->showProperties();
}

void QgsBrowserDockWidget::toggleFastScan()
{
  const QModelIndex index = mWidget->mProxyModel->mapToSource( mWidget->mBrowserView->currentIndex() );
  QgsDataItem *item = mWidget->mModel->dataItem( index );
  if ( ! item )
    return;

  if ( item->type() == Qgis::BrowserItemType::Directory )
  {
    QgsSettings settings;
    QStringList fastScanDirs = settings.value( QStringLiteral( "qgis/scanItemsFastScanUris" ),
                               QStringList() ).toStringList();
    const int idx = fastScanDirs.indexOf( item->path() );
    if ( idx != -1 )
    {
      fastScanDirs.removeAt( idx );
    }
    else
    {
      fastScanDirs << item->path();
    }
    settings.setValue( QStringLiteral( "qgis/scanItemsFastScanUris" ), fastScanDirs );
  }
}

void QgsBrowserDockWidget::showFilterWidget( bool visible )
{
  mWidget->showFilterWidget( visible );
}

void QgsBrowserDockWidget::setFilter()
{
  mWidget->setFilter();
}

void QgsBrowserDockWidget::updateProjectHome()
{
  mWidget->updateProjectHome();
}

void QgsBrowserDockWidget::setFilterSyntax( QAction *action )
{
  mWidget->setFilterSyntax( action );
}

void QgsBrowserDockWidget::setCaseSensitive( bool caseSensitive )
{
  mWidget->setCaseSensitive( caseSensitive );
}

void QgsBrowserDockWidget::selectionChanged( const QItemSelection &selected, const QItemSelection &deselected )
{
  mWidget->selectionChanged( selected, deselected );
}

void QgsBrowserDockWidget::enablePropertiesWidget( bool enable )
{
  mWidget->enablePropertiesWidget( enable );
}

void QgsBrowserDockWidget::setActiveIndex( const QModelIndex &index )
{
  mWidget->setActiveIndex( index );
}

void QgsBrowserDockWidget::splitterMoved()
{
}
