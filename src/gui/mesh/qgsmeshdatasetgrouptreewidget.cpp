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

#include <QFileDialog>
#include <QMessageBox>

#include "qgsmeshdatasetgrouptreeview.h"
#include "qgsmeshlayer.h"
#include "qgsmeshlayertemporalproperties.h"
#include "qgsproject.h"
#include "qgsprojecttimesettings.h"
#include "qgsproviderregistry.h"
#include "qgssettings.h"


QgsMeshDatasetGroupTreeWidget::QgsMeshDatasetGroupTreeWidget( QWidget *parent ):
  QWidget( parent )
{
  setupUi( this );

  connect( mAddDatasetButton, &QToolButton::clicked, this, &QgsMeshDatasetGroupTreeWidget::addDataset );
  connect( mCollapseButton, &QToolButton::clicked, mDatasetGroupTreeView, &QTreeView::collapseAll );
  connect( mExpandButton, &QToolButton::clicked, mDatasetGroupTreeView, &QTreeView::expandAll );
  connect( mCheckAllButton, &QToolButton::clicked, mDatasetGroupTreeView, &QgsMeshDatasetGroupTreeView::selectAllGroups );
  connect( mUnCheckAllButton, &QToolButton::clicked, mDatasetGroupTreeView, &QgsMeshDatasetGroupTreeView::deselectAllGroups );
  connect( mResetDefaultButton, &QToolButton::clicked, this, [this]
  {
    this->mDatasetGroupTreeView->resetDefault( this->mMeshLayer );
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

void QgsMeshDatasetGroupTreeWidget::addDataset()
{
  if ( !mMeshLayer->dataProvider() )
    return;

  QgsSettings settings;
  const QString openFileDir = settings.value( QStringLiteral( "lastMeshDatasetDir" ), QDir::homePath(), QgsSettings::App ).toString();
  const QString openFileString = QFileDialog::getOpenFileName( nullptr,
                                 tr( "Load mesh datasets" ),
                                 openFileDir,
                                 QgsProviderRegistry::instance()->fileMeshDatasetFilters() );

  if ( openFileString.isEmpty() )
  {
    return; // canceled by the user
  }

  const QFileInfo openFileInfo( openFileString );
  settings.setValue( QStringLiteral( "lastMeshDatasetDir" ), openFileInfo.absolutePath(), QgsSettings::App );
  const QFile datasetFile( openFileString );

  if ( mMeshLayer->addDatasets( openFileString, QgsProject::instance()->timeSettings()->temporalRange().begin() ) )
  {
    QMessageBox::information( this, tr( "Load mesh datasets" ), tr( "Datasets successfully added to the mesh layer" ) );
    emit datasetGroupAdded();
  }
  else
  {
    QMessageBox::warning( this, tr( "Load mesh datasets" ), tr( "Could not read mesh dataset." ) );
  }
}
