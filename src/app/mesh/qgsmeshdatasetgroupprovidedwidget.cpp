/***************************************************************************
    qgsmeshdatasetgroupprovidedwidget.cpp
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

#include "qgsmeshdatasetgroupprovidedwidget.h"

#include <QFileDialog>
#include <QMessageBox>

#include "qgsmeshdatasetgrouptreeview.h"
#include "qgsmeshlayer.h"
#include "qgsmeshlayertemporalproperties.h"
#include "qgsproject.h"
#include "qgsprojecttimesettings.h"
#include "qgsproviderregistry.h"
#include "qgssettings.h"


QgsMeshDatasetGroupProvidedWidget::QgsMeshDatasetGroupProvidedWidget( QWidget *parent ):
  QWidget( parent )
{
  setupUi( this );

  connect( mAddDatasetButton, &QPushButton::clicked, this, &QgsMeshDatasetGroupProvidedWidget::addDataset );
  connect( mCollapseButton, &QPushButton::clicked, mDatasetGroupProvidedTreeView, &QTreeView::collapseAll );
  connect( mExpandButton, &QPushButton::clicked, mDatasetGroupProvidedTreeView, &QTreeView::expandAll );
  connect( mCheckAllButton, &QPushButton::clicked, mDatasetGroupProvidedTreeView, &QgsMeshDatasetGroupProvidedTreeView::selectAllGroups );
  connect( mUnCheckAllButton, &QPushButton::clicked, mDatasetGroupProvidedTreeView, &QgsMeshDatasetGroupProvidedTreeView::unselectAllGroups );
  connect( mResetDefaultButton, &QPushButton::clicked, this, [this]
  {
    this->mDatasetGroupProvidedTreeView->resetDefault( this->mMeshLayer );
  } );

}

void QgsMeshDatasetGroupProvidedWidget::syncToLayer( QgsMeshLayer *meshLayer )
{
  mMeshLayer = meshLayer;
  mDatasetGroupProvidedTreeView->syncToLayer( meshLayer );
}

QMap<int, QgsMeshDatasetGroupState> QgsMeshDatasetGroupProvidedWidget::datasetGroupStates() const
{
  return  mDatasetGroupProvidedTreeView->groupStates();
}

void QgsMeshDatasetGroupProvidedWidget::addDataset()
{
  if ( !mMeshLayer->dataProvider() )
    return;

  QgsSettings settings;
  QString openFileDir = settings.value( QStringLiteral( "lastMeshDatasetDir" ), QDir::homePath(), QgsSettings::App ).toString();
  QString openFileString = QFileDialog::getOpenFileName( nullptr,
                           tr( "Load mesh datasets" ),
                           openFileDir,
                           QgsProviderRegistry::instance()->fileMeshDatasetFilters() );

  if ( openFileString.isEmpty() )
  {
    return; // canceled by the user
  }

  QFileInfo openFileInfo( openFileString );
  settings.setValue( QStringLiteral( "lastMeshDatasetDir" ), openFileInfo.absolutePath(), QgsSettings::App );
  QFile datasetFile( openFileString );

  bool isTemporalBefore = mMeshLayer->dataProvider()->temporalCapabilities()->hasTemporalCapabilities();
  bool ok = mMeshLayer->dataProvider()->addDataset( openFileString );
  if ( ok )
  {
    QgsMeshLayerTemporalProperties *temporalProperties = qobject_cast< QgsMeshLayerTemporalProperties * >( mMeshLayer->temporalProperties() );
    if ( !isTemporalBefore && mMeshLayer->dataProvider()->temporalCapabilities()->hasTemporalCapabilities() )
    {
      mMeshLayer->temporalProperties()->setDefaultsFromDataProviderTemporalCapabilities(
        mMeshLayer->dataProvider()->temporalCapabilities() );

      if ( ! temporalProperties->referenceTime().isValid() )
      {
        QDateTime referenceTime = QgsProject::instance()->timeSettings()->temporalRange().begin();
        if ( !referenceTime.isValid() ) // If project reference time is invalid, use current date
          referenceTime = QDateTime( QDate::currentDate(), QTime( 0, 0, 0, Qt::UTC ) );
        temporalProperties->setReferenceTime( referenceTime, mMeshLayer->dataProvider()->temporalCapabilities() );
      }

      mMeshLayer->temporalProperties()->setIsActive( true );
    }
    QMessageBox::information( this, tr( "Load mesh datasets" ), tr( "Datasets successfully added to the mesh layer" ) );
    emit mMeshLayer->dataSourceChanged();
    emit datasetGroupAdded();
  }
  else
  {
    QMessageBox::warning( this, tr( "Load mesh datasets" ), tr( "Could not read mesh dataset." ) );
  }
}

