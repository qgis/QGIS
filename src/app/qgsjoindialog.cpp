/***************************************************************************
                              qgsjoindialog.cpp
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

#include "qgsjoindialog.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsmaplayercombobox.h"
#include "qgsfieldcombobox.h"

#include <QStandardItemModel>

QgsJoinDialog::QgsJoinDialog( QgsVectorLayer* layer, QList<QgsMapLayer*> alreadyJoinedLayers, QWidget * parent, Qt::WindowFlags f )
    : QDialog( parent, f )
    , mLayer( layer )
{
  setupUi( this );

  if ( !mLayer )
  {
    return;
  }
  // adds self layer to the joined layer (cannot join to itself)
  alreadyJoinedLayers.append( layer );

  mTargetFieldComboBox->setLayer( mLayer );

  mJoinLayerComboBox->setExceptedLayerList( alreadyJoinedLayers );
  connect( mJoinLayerComboBox, SIGNAL( layerChanged( QgsMapLayer* ) ), mJoinFieldComboBox, SLOT( setLayer( QgsMapLayer* ) ) );
  connect( mJoinLayerComboBox, SIGNAL( layerChanged( QgsMapLayer* ) ), this, SLOT( joinedLayerChanged( QgsMapLayer* ) ) );

  mCacheInMemoryCheckBox->setChecked( true );
}

QgsJoinDialog::~QgsJoinDialog()
{
}

void QgsJoinDialog::setJoinInfo( const QgsVectorJoinInfo& joinInfo )
{
  mJoinLayerComboBox->setLayer( QgsMapLayerRegistry::instance()->mapLayer( joinInfo.joinLayerId ) );
  mJoinFieldComboBox->setField( joinInfo.joinFieldName );
  mTargetFieldComboBox->setField( joinInfo.targetFieldName );
  mCacheInMemoryCheckBox->setChecked( joinInfo.memoryCache );
  if ( joinInfo.prefix.isNull() )
  {
    mUseCustomPrefix->setChecked( false );
  }
  else
  {
    mUseCustomPrefix->setChecked( true );
    mCustomPrefix->setText( joinInfo.prefix );
  }

  QStringList* lst = joinInfo.joinFieldNamesSubset();
  mUseJoinFieldsSubset->setChecked( lst && lst->count() > 0 );
  QAbstractItemModel* model = mJoinFieldsSubsetView->model();
  if ( model )
  {
    for ( int i = 0; i < model->rowCount(); ++i )
    {
      QModelIndex index = model->index( i, 0 );
      if ( lst && lst->contains( model->data( index, Qt::DisplayRole ).toString() ) )
      {
        model->setData( index, Qt::Checked, Qt::CheckStateRole );
      }
      else
      {
        model->setData( index, Qt::Unchecked, Qt::CheckStateRole );
      }
    }
  }
}

QgsVectorJoinInfo QgsJoinDialog::joinInfo() const
{
  QgsVectorJoinInfo info;
  info.joinLayerId = mJoinLayerComboBox->currentLayer()->id();
  info.joinFieldName = mJoinFieldComboBox->currentField();
  info.targetFieldName = mTargetFieldComboBox->currentField();
  info.memoryCache = mCacheInMemoryCheckBox->isChecked();

  if ( mUseCustomPrefix->isChecked() )
    info.prefix = mCustomPrefix->text();
  else
    info.prefix = QString::null;

  if ( mUseJoinFieldsSubset->isChecked() )
  {
    QStringList lst;
    QAbstractItemModel* model = mJoinFieldsSubsetView->model();
    if ( model )
    {
      for ( int i = 0; i < model->rowCount(); ++i )
      {
        QModelIndex index = model->index( i, 0 );
        if ( model->data( index, Qt::CheckStateRole ).toInt() == Qt::Checked )
          lst << model->data( index ).toString();
      }
    }
    info.setJoinFieldNamesSubset( new QStringList( lst ) );
  }

  return info;
}

bool QgsJoinDialog::createAttributeIndex() const
{
  return mCreateIndexCheckBox->isChecked();
}

void QgsJoinDialog::joinedLayerChanged( QgsMapLayer* layer )
{

  mJoinFieldComboBox->clear();

  QgsVectorLayer* vLayer = dynamic_cast<QgsVectorLayer*>( layer );
  if ( !vLayer )
  {
    return;
  }

  mUseJoinFieldsSubset->setChecked( false );
  QStandardItemModel* subsetModel = new QStandardItemModel( this );
  const QgsFields& layerFields = vLayer->pendingFields();
  for ( int idx = 0; idx < layerFields.count(); ++idx )
  {
    QStandardItem* subsetItem = new QStandardItem( layerFields[idx].name() );
    subsetItem->setCheckable( true );
    //subsetItem->setFlags( subsetItem->flags() | Qt::ItemIsUserCheckable );
    subsetModel->appendRow( subsetItem );
  }
  mJoinFieldsSubsetView->setModel( subsetModel );

  QgsVectorDataProvider* dp = vLayer->dataProvider();
  bool canCreateAttrIndex = dp && ( dp->capabilities() & QgsVectorDataProvider::CreateAttributeIndex );
  if ( canCreateAttrIndex )
  {
    mCreateIndexCheckBox->setEnabled( true );
  }
  else
  {
    mCreateIndexCheckBox->setEnabled( false );
    mCreateIndexCheckBox->setChecked( false );
  }

  if ( !mUseCustomPrefix->isChecked() )
  {
    mCustomPrefix->setText( layer->name() + "_" );
  }
}
