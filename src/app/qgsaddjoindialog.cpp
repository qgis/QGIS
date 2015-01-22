/***************************************************************************
                              qgsaddjoindialog.cpp
                              --------------------
  begin                : July 10, 2010
  copyright            : (C) 2010 by Marco Hugentobler
  email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsaddjoindialog.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"

#include <QStandardItemModel>

QgsAddJoinDialog::QgsAddJoinDialog( QgsVectorLayer* layer, QWidget * parent, Qt::WindowFlags f ): QDialog( parent, f ), mLayer( layer )
{
  setupUi( this );

  if ( !mLayer )
  {
    return;
  }

  //insert possible vector layers into mJoinLayerComboBox

  mJoinLayerComboBox->blockSignals( true );
  const QMap<QString, QgsMapLayer*>& layerList = QgsMapLayerRegistry::instance()->mapLayers();
  QMap<QString, QgsMapLayer*>::const_iterator layerIt = layerList.constBegin();
  for ( ; layerIt != layerList.constEnd(); ++layerIt )
  {
    QgsMapLayer* currentLayer = layerIt.value();
    if ( currentLayer->type() == QgsMapLayer::VectorLayer )
    {
      QgsVectorLayer* currentVectorLayer = dynamic_cast<QgsVectorLayer*>( currentLayer );
      if ( currentVectorLayer && currentVectorLayer != mLayer )
      {
        if ( currentVectorLayer->dataProvider() && currentVectorLayer->dataProvider()->supportsSubsetString() )
          mJoinLayerComboBox->addItem( currentLayer->name(), QVariant( currentLayer->id() ) );
      }
    }
  }
  mJoinLayerComboBox->blockSignals( false );
  on_mJoinLayerComboBox_currentIndexChanged( mJoinLayerComboBox->currentIndex() );

  //insert possible target fields
  QgsVectorDataProvider* provider = mLayer->dataProvider();
  if ( provider )
  {
    const QgsFields& layerFields = provider->fields();
    for ( int idx = 0; idx < layerFields.count(); ++idx )
    {
      mTargetFieldComboBox->addItem( layerFields[idx].name(), idx );
    }
  }

  mCacheInMemoryCheckBox->setChecked( true );
}

QgsAddJoinDialog::~QgsAddJoinDialog()
{
}

QString QgsAddJoinDialog::joinedLayerId() const
{
  return mJoinLayerComboBox->itemData( mJoinLayerComboBox->currentIndex() ).toString();
}

QString QgsAddJoinDialog::joinFieldName() const
{
  return mJoinFieldComboBox->itemText( mJoinFieldComboBox->currentIndex() );
}

QString QgsAddJoinDialog::targetFieldName() const
{
  return mTargetFieldComboBox->itemText( mTargetFieldComboBox->currentIndex() );
}

bool QgsAddJoinDialog::cacheInMemory() const
{
  return mCacheInMemoryCheckBox->isChecked();
}

bool QgsAddJoinDialog::createAttributeIndex() const
{
  return mCreateIndexCheckBox->isChecked();
}

bool QgsAddJoinDialog::hasJoinFieldsSubset() const
{
  return mUseJoinFieldsSubset->isChecked();
}

QStringList QgsAddJoinDialog::joinFieldsSubset() const
{
  QStringList lst;
  QAbstractItemModel* model = mJoinFieldsSubsetView->model();
  if ( !model )
    return lst;

  for ( int i = 0; i < model->rowCount(); ++i )
  {
    QModelIndex index = model->index( i, 0 );
    if ( model->data( index, Qt::CheckStateRole ).toInt() == Qt::Checked )
      lst << model->data( index ).toString();
  }
  return lst;
}

bool QgsAddJoinDialog::hasCustomPrefix() const
{
  return mUseCustomPrefix;
}

const QString QgsAddJoinDialog::customPrefix() const
{
  return mCustomPrefix->text();
}

void QgsAddJoinDialog::on_mJoinLayerComboBox_currentIndexChanged( int index )
{
  mJoinFieldComboBox->clear();
  QString layerId = mJoinLayerComboBox->itemData( index ).toString();
  QgsMapLayer* layer = QgsMapLayerRegistry::instance()->mapLayer( layerId );
  QgsVectorLayer* vLayer = dynamic_cast<QgsVectorLayer*>( layer );
  if ( !vLayer )
  {
    return;
  }

  QStandardItemModel* subsetModel = new QStandardItemModel( this );

  const QgsFields& layerFields = vLayer->pendingFields();
  for ( int idx = 0; idx < layerFields.count(); ++idx )
  {
    mJoinFieldComboBox->addItem( layerFields[idx].name(), idx );
    QStandardItem* subsetItem = new QStandardItem( layerFields[idx].name() );
    subsetItem->setCheckable( true );
    //subsetItem->setFlags( subsetItem->flags() | Qt::ItemIsUserCheckable );
    subsetModel->appendRow( subsetItem );
  }

  //does provider support creation of attribute indices?
  QgsVectorDataProvider* dp = vLayer->dataProvider();
  if ( dp && ( dp->capabilities() & QgsVectorDataProvider::CreateAttributeIndex ) )
  {
    mCreateIndexCheckBox->setEnabled( true );
  }
  else
  {
    mCreateIndexCheckBox->setEnabled( false );
    mCreateIndexCheckBox->setChecked( false );
  }

  mJoinFieldsSubsetView->setModel( subsetModel );

  if ( !mUseCustomPrefix->isChecked() )
  {
    mCustomPrefix->setText( layer->name() + "_" );
  }
}
