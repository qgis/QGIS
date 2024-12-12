/***************************************************************************
    qgsmeshdatasetgrouptreewidget.cpp
    -------------------------------
    begin                : May 2020
    copyright            : (C) 2020 by Vincent Cloarec
    email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmeshdatasetgrouptreewidget.h"
#include "moc_qgsmeshdatasetgrouptreewidget.cpp"

#include <QFileDialog>
#include <QMessageBox>

#include "qgsmeshdatasetgrouptreeview.h"
#include "qgsmeshlayer.h"
#include "qgsmeshlayertemporalproperties.h"
#include "qgsproject.h"
#include "qgsprojecttimesettings.h"
#include "qgsproviderregistry.h"
#include "qgssettings.h"


QgsMeshDatasetGroupTreeWidget::QgsMeshDatasetGroupTreeWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  connect( mAddDatasetButton, &QToolButton::clicked, this, &QgsMeshDatasetGroupTreeWidget::addDataset );
  connect( mRemoveDatasetButton, &QToolButton::clicked, this, &QgsMeshDatasetGroupTreeWidget::removeDataset );
  connect( mCollapseButton, &QToolButton::clicked, mDatasetGroupTreeView, &QTreeView::collapseAll );
  connect( mExpandButton, &QToolButton::clicked, mDatasetGroupTreeView, &QTreeView::expandAll );
  connect( mCheckAllButton, &QToolButton::clicked, mDatasetGroupTreeView, &QgsMeshDatasetGroupTreeView::selectAllGroups );
  connect( mUnCheckAllButton, &QToolButton::clicked, mDatasetGroupTreeView, &QgsMeshDatasetGroupTreeView::deselectAllGroups );
  connect( mResetDefaultButton, &QToolButton::clicked, this, [this] {
    this->mDatasetGroupTreeView->resetDefault( this->mMeshLayer );
  } );

  connect( mDatasetGroupTreeView->selectionModel(), &QItemSelectionModel::currentChanged, this, [this]() {
    const QModelIndex index = mDatasetGroupTreeView->currentIndex();
    const QgsMeshDatasetGroupTreeItem *meshGroupItem = mDatasetGroupTreeView->datasetGroupTreeRootItem()->childFromDatasetGroupIndex( index.row() );
    if ( meshGroupItem )
    {
      if ( mMeshLayer->dataProvider()->dataSourceUri().contains( meshGroupItem->description() ) )
      {
        mRemoveDatasetButton->setEnabled( false );
      }
      else
      {
        mRemoveDatasetButton->setEnabled( true );
      }
    }
  } );

  connect( mDatasetGroupTreeView, &QgsMeshDatasetGroupTreeView::apply, this, &QgsMeshDatasetGroupTreeWidget::apply );
}

void QgsMeshDatasetGroupTreeWidget::syncToLayer( QgsMeshLayer *meshLayer )
{
  mMeshLayer = meshLayer;
  mDatasetGroupTreeView->syncToLayer( meshLayer );
}

void QgsMeshDatasetGroupTreeWidget::apply()
{
  if ( mMeshLayer )
    mMeshLayer->setDatasetGroupTreeRootItem( mDatasetGroupTreeView->datasetGroupTreeRootItem() );
}

void QgsMeshDatasetGroupTreeWidget::removeDataset()
{
  const QModelIndex index = mDatasetGroupTreeView->currentIndex();
  const QgsMeshDatasetGroupTreeItem *meshGroupItem = mDatasetGroupTreeView->datasetGroupTreeRootItem()->child( index.row() );
  const QString datasetGroupName = meshGroupItem->defaultName();
  if ( mMeshLayer->removeDatasets( datasetGroupName ) )
  {
    QMessageBox::warning( this, tr( "Remove mesh datasets" ), tr( "Dataset Group removed from mesh." ) );
    emit datasetGroupsChanged();
  }
  else
  {
    QMessageBox::warning( this, tr( "Remove mesh datasets" ), tr( "Could not remove mesh dataset group." ) );
  }

  mDatasetGroupTreeView->resetDefault( mMeshLayer );
}

void QgsMeshDatasetGroupTreeWidget::addDataset()
{
  if ( !mMeshLayer->dataProvider() )
    return;

  QgsSettings settings;
  const QString openFileDir = settings.value( QStringLiteral( "lastMeshDatasetDir" ), QDir::homePath(), QgsSettings::App ).toString();
  const QString openFileString = QFileDialog::getOpenFileName( nullptr, tr( "Load mesh datasets" ), openFileDir, QgsProviderRegistry::instance()->fileMeshDatasetFilters() );

  if ( openFileString.isEmpty() )
  {
    return; // canceled by the user
  }

  if ( !mMeshLayer->datasetsPathUnique( openFileString ) )
  {
    QMessageBox::warning( this, tr( "Load mesh datasets" ), tr( "Could not add dataset from path that is already added to the mesh." ) );
    return;
  }

  const QFileInfo openFileInfo( openFileString );
  settings.setValue( QStringLiteral( "lastMeshDatasetDir" ), openFileInfo.absolutePath(), QgsSettings::App );
  const QFile datasetFile( openFileString );

  if ( mMeshLayer->addDatasets( openFileString, QgsProject::instance()->timeSettings()->temporalRange().begin() ) )
  {
    QMessageBox::information( this, tr( "Load mesh datasets" ), tr( "Datasets successfully added to the mesh layer" ) );
    emit datasetGroupsChanged();
  }
  else
  {
    QMessageBox::warning( this, tr( "Load mesh datasets" ), tr( "Could not read mesh dataset." ) );
  }
}
