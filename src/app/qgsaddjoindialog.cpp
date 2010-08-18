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

QgsAddJoinDialog::QgsAddJoinDialog( QgsVectorLayer* layer, QWidget * parent, Qt::WindowFlags f ): QDialog( parent, f ), mLayer( layer )
{
  setupUi( this );

  if( !mLayer )
  {
    return;
  }

  //insert possible vector layers into mJoinLayerComboBox

  mJoinLayerComboBox->blockSignals( true );
  const QMap<QString, QgsMapLayer*>& layerList = QgsMapLayerRegistry::instance()->mapLayers();
  QMap<QString, QgsMapLayer*>::const_iterator layerIt = layerList.constBegin();
  for(; layerIt != layerList.constEnd(); ++layerIt )
  {
    QgsMapLayer* currentLayer = layerIt.value();
    if( currentLayer->type() == QgsMapLayer::VectorLayer )
    {
      QgsVectorLayer* currentVectorLayer = dynamic_cast<QgsVectorLayer*>( currentLayer );
      if( currentVectorLayer && currentVectorLayer != mLayer )
      {
        if( currentVectorLayer->dataProvider() && currentVectorLayer->dataProvider()->supportsSubsetString() )
        mJoinLayerComboBox->addItem( currentLayer->name(), QVariant(currentLayer->getLayerID() ) );
      }
    }
  }
  mJoinLayerComboBox->blockSignals( false );
  on_mJoinLayerComboBox_currentIndexChanged( mJoinLayerComboBox->currentIndex() );

  //insert possible target fields
  const QgsFieldMap& layerFieldMap = mLayer->pendingFields();
  QgsFieldMap::const_iterator fieldIt = layerFieldMap.constBegin();
  for(; fieldIt != layerFieldMap.constEnd(); ++fieldIt )
  {
    mTargetFieldComboBox->addItem(fieldIt.value().name(), fieldIt.key() );
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

int QgsAddJoinDialog::joinField() const
{
  return mJoinFieldComboBox->itemData( mJoinFieldComboBox->currentIndex() ).toInt();
}

QString QgsAddJoinDialog::joinFieldName() const
{
   return mJoinFieldComboBox->itemText( mJoinFieldComboBox->currentIndex() );
}

int QgsAddJoinDialog::targetField() const
{
  return mTargetFieldComboBox->itemData( mTargetFieldComboBox->currentIndex() ).toInt();
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

void QgsAddJoinDialog::on_mJoinLayerComboBox_currentIndexChanged ( int index )
{
  mJoinFieldComboBox->clear();
  QString layerId = mJoinLayerComboBox->itemData( index ).toString();
  QgsMapLayer* layer = QgsMapLayerRegistry::instance()->mapLayer( layerId );
  if( !layer )
  {
    return;
  }
  QgsVectorLayer* vLayer = dynamic_cast<QgsVectorLayer*>( layer );
  if( !vLayer )
  {
    return;
  }

  const QgsFieldMap& layerFieldMap = vLayer->pendingFields();
  QgsFieldMap::const_iterator fieldIt = layerFieldMap.constBegin();
  for(; fieldIt != layerFieldMap.constEnd(); ++fieldIt )
  {
    mJoinFieldComboBox->addItem( fieldIt.value().name(), fieldIt.key() );
  }

  //does provider support creation of attribute indices?
  QgsVectorDataProvider* dp = vLayer->dataProvider();
  if( dp && (dp->capabilities() & QgsVectorDataProvider::CreateAttributeIndex) )
  {
    mCreateIndexCheckBox->setEnabled( true );
  }
  else
  {
    mCreateIndexCheckBox->setEnabled( false );
    mCreateIndexCheckBox->setChecked( false );
  }
}
