/***************************************************************************
    qgsmeshrendereractivedatasetwidget.cpp
    ---------------------------------------
    begin                : June 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmeshrendereractivedatasetwidget.h"

#include "qgis.h"
#include "qgsmeshlayer.h"
#include "qgsmessagelog.h"
#include "qgsmeshrenderersettings.h"

QgsMeshRendererActiveDatasetWidget::QgsMeshRendererActiveDatasetWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );
  connect( mDatasetGroupTreeView, &QgsMeshDatasetGroupTreeView::activeGroupChanged, this, &QgsMeshRendererActiveDatasetWidget::onActiveGroupChanged );
  connect( mDatasetSlider, &QSlider::valueChanged, this, &QgsMeshRendererActiveDatasetWidget::onActiveDatasetChanged );
}

void QgsMeshRendererActiveDatasetWidget::setLayer( QgsMeshLayer *layer )
{
  if ( layer != mMeshLayer )
  {
    mMeshLayer = layer;
  }

  mDatasetGroupTreeView->setLayer( layer );
  setEnabled( mMeshLayer );
  syncToLayer();
}

int QgsMeshRendererActiveDatasetWidget::activeScalarDataset() const
{
  return mActiveScalarDataset;
}

int QgsMeshRendererActiveDatasetWidget::activeVectorDataset() const
{
  return mActiveVectorDataset;
}

void QgsMeshRendererActiveDatasetWidget::onActiveGroupChanged()
{
  const QVector<int> datasets = mDatasetGroupTreeView->datasetsInActiveGroup();

  mDatasetSlider->setMinimum( 0 );
  mDatasetSlider->setMaximum( datasets.size() - 1 );
  mDatasetSlider->setValue( 0 );
}

void QgsMeshRendererActiveDatasetWidget::onActiveDatasetChanged( int value )
{
  int datasetIndex = -1;
  const QVector<int> datasets = mDatasetGroupTreeView->datasetsInActiveGroup();
  mActiveScalarDataset = -1;
  mActiveVectorDataset = -1;

  if ( datasets.size() > value && mMeshLayer && mMeshLayer->dataProvider() )
  {
    datasetIndex = datasets[value];
    const QgsMeshDatasetMetadata meta = mMeshLayer->dataProvider()->datasetMetadata( datasetIndex );
    mActiveScalarDataset = datasetIndex;
    if ( meta.isVector() )
      mActiveVectorDataset = datasetIndex;
  }

  updateMetadata( datasetIndex );

  emit activeScalarDatasetChanged( activeScalarDataset() );
  emit activeVectorDatasetChanged( activeVectorDataset() );

  emit widgetChanged();
}

void QgsMeshRendererActiveDatasetWidget::updateMetadata( int datasetIndex )
{
  if ( datasetIndex == -1 )
  {
    mActiveDatasetMetadata->setText( tr( "No dataset selected" ) );
  }
  else
  {
    const QgsMeshDatasetMetadata meta = mMeshLayer->dataProvider()->datasetMetadata( datasetIndex );
    QString msg;
    msg += QStringLiteral( "<table>" );

    msg += QStringLiteral( "<tr><td>%1</td><td>%2</td></tr>" )
           .arg( tr( "Is on vertices" ) )
           .arg( meta.isOnVertices() ? tr( "Yes" ) : tr( "No" ) );

    msg += QStringLiteral( "<tr><td>%1</td><td>%2</td></tr>" )
           .arg( tr( "Is vector" ) )
           .arg( meta.isVector() ? tr( "Yes" ) : tr( "No" ) );

    for ( auto it = meta.extraOptions().constBegin(); it != meta.extraOptions().constEnd(); ++it )
    {
      msg += QStringLiteral( "<tr><td>%1</td><td>%2</td></tr>" ).arg( it.key() ).arg( it.value() );
    }

    msg += QStringLiteral( "</table>" );
    mActiveDatasetMetadata->setText( msg );
  }

}

int QgsMeshRendererActiveDatasetWidget::datasetIndex() const
{
  const QVector<int> datasets = mDatasetGroupTreeView->datasetsInActiveGroup();
  int value = mDatasetSlider->value();
  int datasetIndex = -1;
  if ( value < datasets.size() )
  {
    datasetIndex = datasets[value];
  }
  return datasetIndex;
}

void QgsMeshRendererActiveDatasetWidget::syncToLayer()
{
  mDatasetGroupTreeView->syncToLayer();

  if ( mMeshLayer )
  {
    mActiveScalarDataset = mMeshLayer->activeScalarDataset();
    mActiveVectorDataset = mMeshLayer->activeVectorDataset();
  }
  else
  {
    mActiveScalarDataset = -1;
    mActiveVectorDataset = -1;
  }

  if ( mActiveScalarDataset != -1 )
    whileBlocking( mDatasetSlider )->setValue( mActiveScalarDataset );
}
