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
  connect( mDisplayScalarsCheckBox, &QCheckBox::stateChanged, this, &QgsMeshRendererActiveDatasetWidget::onScalarChecked );
  connect( mDisplayVectorsCheckBox, &QCheckBox::stateChanged, this, &QgsMeshRendererActiveDatasetWidget::onVectorChecked );
  connect( mDisplayNativeMeshCheckBox, &QCheckBox::stateChanged, this, &QgsMeshRendererActiveDatasetWidget::onNativeMeshChecked );
  connect( mDisplayTriangularMeshCheckBox, &QCheckBox::stateChanged, this, &QgsMeshRendererActiveDatasetWidget::onTringularMeshChecked );
}

void QgsMeshRendererActiveDatasetWidget::setLayer( QgsMeshLayer *layer )
{
  if ( layer != mMeshLayer )
  {
    mMeshLayer = layer;
  }

  setEnabled( mMeshLayer );
  syncToLayer();

  mDatasetGroupTreeView->setLayer( layer );
}

int QgsMeshRendererActiveDatasetWidget::activeScalarDataset() const
{
  if ( isEnabled() &&
       mDisplayScalarsCheckBox->isEnabled() &&
       mDisplayScalarsCheckBox->isChecked() )
    return datasetIndex();
  else
    return -1;
}

int QgsMeshRendererActiveDatasetWidget::activeVectorDataset() const
{
  if ( isEnabled() &&
       mDisplayVectorsCheckBox->isEnabled() &&
       mDisplayVectorsCheckBox->isChecked() )
    return  datasetIndex();
  else
    return -1;
}

bool QgsMeshRendererActiveDatasetWidget::isNativeMeshEnabled() const
{
  return isEnabled() && mDisplayNativeMeshCheckBox->isChecked();
}

bool QgsMeshRendererActiveDatasetWidget::isTriangularMeshEnabled() const
{
  return isEnabled() && mDisplayTriangularMeshCheckBox->isChecked();
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
  if ( datasets.size() < value || !mMeshLayer || !mMeshLayer->dataProvider() )
  {
    mDisplayScalarsCheckBox->setEnabled( false );
    mDisplayVectorsCheckBox->setEnabled( false );
  }
  else
  {
    datasetIndex = datasets[value];
    const QgsMeshDatasetMetadata meta = mMeshLayer->dataProvider()->datasetMetadata( datasetIndex );
    mDisplayScalarsCheckBox->setEnabled( true );
    mDisplayVectorsCheckBox->setEnabled( meta.isVector() );
  }

  updateMetadata( datasetIndex );

  emit activeScalarDatasetChanged( activeScalarDataset() );
  emit activeVectorDatasetChanged( activeVectorDataset() );

  emit widgetChanged();
}

void QgsMeshRendererActiveDatasetWidget::onScalarChecked( int toggle )
{
  Q_UNUSED( toggle );
  emit activeScalarDatasetChanged( activeScalarDataset() );
  emit widgetChanged();
}

void QgsMeshRendererActiveDatasetWidget::onVectorChecked( int toggle )
{
  Q_UNUSED( toggle );
  emit activeVectorDatasetChanged( activeVectorDataset() );
  emit widgetChanged();
}

void QgsMeshRendererActiveDatasetWidget::onNativeMeshChecked( int toggle )
{
  Q_UNUSED( toggle );
  emit nativeMeshEnabledChanged( isNativeMeshEnabled() );
  emit widgetChanged();
}

void QgsMeshRendererActiveDatasetWidget::onTringularMeshChecked( int toggle )
{
  Q_UNUSED( toggle );
  emit triangularMeshEnabledChanged( isTriangularMeshEnabled() );
  emit widgetChanged();
}

void QgsMeshRendererActiveDatasetWidget::updateMetadata( int datasetIndex )
{
  if ( datasetIndex == -1 )
  {
    mActiveDatasetMetadata->setText( tr( "N/A" ) );
  }
  else
  {
    const QgsMeshDatasetMetadata meta = mMeshLayer->dataProvider()->datasetMetadata( datasetIndex );
    QString msg;
    msg += QStringLiteral( "<table>" );
    msg += QStringLiteral( "<tr><td>%1</td><td>%2</td></tr>" ).arg( tr( "is on vertices" ) ).arg( meta.isOnVertices() );
    msg += QStringLiteral( "<tr><td>%1</td><td>%2</td></tr>" ).arg( tr( "is vector" ) ).arg( meta.isVector() );
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
    whileBlocking( mDisplayNativeMeshCheckBox )->setChecked( mMeshLayer->rendererNativeMeshSettings().isEnabled() );
    whileBlocking( mDisplayTriangularMeshCheckBox )->setChecked( mMeshLayer->rendererTriangularMeshSettings().isEnabled() );
    whileBlocking( mDisplayScalarsCheckBox )->setChecked( mMeshLayer->activeScalarDataset() != -1 );
    whileBlocking( mDisplayVectorsCheckBox )->setChecked( mMeshLayer->activeVectorDataset() != -1 );
  }
}
