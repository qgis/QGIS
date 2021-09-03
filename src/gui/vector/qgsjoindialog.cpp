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
#include "qgsproject.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerjoininfo.h"
#include "qgsmaplayercombobox.h"
#include "qgsfieldcombobox.h"
#include "qgshelp.h"

#include <QStandardItemModel>
#include <QPushButton>

QgsJoinDialog::QgsJoinDialog( QgsVectorLayer *layer, QList<QgsMapLayer *> alreadyJoinedLayers, QWidget *parent, Qt::WindowFlags f )
  : QDialog( parent, f )
  , mLayer( layer )
{
  setupUi( this );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this,  [ = ]
  {
    QgsHelp::openHelp( QStringLiteral( "working_with_vector/vector_properties.html#joins-properties" ) );
  } );

  if ( !mLayer )
  {
    return;
  }
  // adds self layer to the joined layer (cannot join to itself)
  alreadyJoinedLayers.append( layer );

  mTargetFieldComboBox->setLayer( mLayer );

  mDynamicFormCheckBox->setToolTip( tr( "This option allows values of the joined fields to be automatically reloaded when the \"Target Field\" is changed" ) );

  mEditableJoinLayer->setToolTip( tr( "This option allows values of the joined layers to be editable if they're themselves editable" ) );
  mUpsertOnEditCheckBox->setToolTip( tr( "Automatically adds a matching row to the joined table, but if one already exists then update that matching row instead" ) );
  mDeleteCascadeCheckBox->setToolTip( tr( "Automatically delete the corresponding feature of the linked layer if one exists" ) );

  mJoinLayerComboBox->setFilters( QgsMapLayerProxyModel::VectorLayer );
  mJoinLayerComboBox->setExceptedLayerList( alreadyJoinedLayers );
  connect( mJoinLayerComboBox, &QgsMapLayerComboBox::layerChanged, mJoinFieldComboBox, &QgsFieldComboBox::setLayer );
  connect( mJoinLayerComboBox, &QgsMapLayerComboBox::layerChanged, this, &QgsJoinDialog::joinedLayerChanged );

  mCacheInMemoryCheckBox->setChecked( true );
  mCacheEnabled = mCacheInMemoryCheckBox->isChecked();

  QgsMapLayer *joinLayer = mJoinLayerComboBox->currentLayer();
  if ( joinLayer && joinLayer->isValid() )
  {
    mJoinFieldComboBox->setLayer( joinLayer );
    joinedLayerChanged( joinLayer );
  }

  connect( mJoinLayerComboBox, &QgsMapLayerComboBox::layerChanged, this, &QgsJoinDialog::checkDefinitionValid );
  connect( mJoinFieldComboBox, &QgsFieldComboBox::fieldChanged, this, &QgsJoinDialog::checkDefinitionValid );
  connect( mTargetFieldComboBox, &QgsFieldComboBox::fieldChanged, this, &QgsJoinDialog::checkDefinitionValid );
  connect( mEditableJoinLayer, &QGroupBox::toggled, this, &QgsJoinDialog::editableJoinLayerChanged );

  checkDefinitionValid();
}

void QgsJoinDialog::setJoinInfo( const QgsVectorLayerJoinInfo &joinInfo )
{
  mJoinLayerComboBox->setLayer( joinInfo.joinLayer() );
  mJoinFieldComboBox->setField( joinInfo.joinFieldName() );
  mTargetFieldComboBox->setField( joinInfo.targetFieldName() );

  mCacheEnabled = joinInfo.isUsingMemoryCache();
  mCacheInMemoryCheckBox->setChecked( joinInfo.isUsingMemoryCache() );

  mDynamicFormCheckBox->setChecked( joinInfo.isDynamicFormEnabled() );
  mEditableJoinLayer->setChecked( joinInfo.isEditable() );
  mUpsertOnEditCheckBox->setChecked( joinInfo.hasUpsertOnEdit() );
  mDeleteCascadeCheckBox->setChecked( joinInfo.hasCascadedDelete() );

  if ( joinInfo.prefix().isNull() )
  {
    mUseCustomPrefix->setChecked( false );
  }
  else
  {
    mUseCustomPrefix->setChecked( true );
    mCustomPrefix->setText( joinInfo.prefix() );
  }

  QStringList *lst = joinInfo.joinFieldNamesSubset();
  mUseJoinFieldsSubset->setChecked( lst && !lst->isEmpty() );
  QAbstractItemModel *model = mJoinFieldsSubsetView->model();
  if ( model )
  {
    for ( int i = 0; i < model->rowCount(); ++i )
    {
      const QModelIndex index = model->index( i, 0 );
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

  editableJoinLayerChanged();
}

QgsVectorLayerJoinInfo QgsJoinDialog::joinInfo() const
{
  QgsVectorLayerJoinInfo info;
  info.setJoinLayer( qobject_cast<QgsVectorLayer *>( mJoinLayerComboBox->currentLayer() ) );
  info.setJoinFieldName( mJoinFieldComboBox->currentField() );
  info.setTargetFieldName( mTargetFieldComboBox->currentField() );
  info.setUsingMemoryCache( mCacheInMemoryCheckBox->isChecked() );
  info.setDynamicFormEnabled( mDynamicFormCheckBox->isChecked() );

  info.setEditable( mEditableJoinLayer->isChecked() );
  if ( info.isEditable() )
  {
    info.setUpsertOnEdit( mUpsertOnEditCheckBox->isChecked() );
    info.setCascadedDelete( mDeleteCascadeCheckBox->isChecked() );
  }

  if ( mUseCustomPrefix->isChecked() )
    info.setPrefix( mCustomPrefix->text() );
  else
    info.setPrefix( QString() );

  if ( mUseJoinFieldsSubset->isChecked() )
  {
    QStringList lst;
    QAbstractItemModel *model = mJoinFieldsSubsetView->model();
    if ( model )
    {
      for ( int i = 0; i < model->rowCount(); ++i )
      {
        const QModelIndex index = model->index( i, 0 );
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

void QgsJoinDialog::joinedLayerChanged( QgsMapLayer *layer )
{
  mJoinFieldComboBox->clear();

  QgsVectorLayer *vLayer = qobject_cast<QgsVectorLayer *>( layer );
  if ( !vLayer )
  {
    return;
  }

  mUseJoinFieldsSubset->setChecked( false );
  QStandardItemModel *subsetModel = new QStandardItemModel( this );
  const QgsFields layerFields = vLayer->fields();
  for ( const QgsField &field : layerFields )
  {
    QStandardItem *subsetItem = new QStandardItem( field.name() );
    subsetItem->setCheckable( true );
    //subsetItem->setFlags( subsetItem->flags() | Qt::ItemIsUserCheckable );
    subsetModel->appendRow( subsetItem );
  }
  mJoinFieldsSubsetView->setModel( subsetModel );

  QgsVectorDataProvider *dp = vLayer->dataProvider();
  const bool canCreateAttrIndex = dp && ( dp->capabilities() & QgsVectorDataProvider::CreateAttributeIndex );
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
    mCustomPrefix->setText( layer->name() + '_' );
  }
}

void QgsJoinDialog::checkDefinitionValid()
{
  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( mJoinLayerComboBox->currentIndex() != -1
      && mJoinFieldComboBox->currentIndex() != -1
      && mTargetFieldComboBox->currentIndex() != -1 );
}

void QgsJoinDialog::editableJoinLayerChanged()
{
  if ( mEditableJoinLayer->isChecked() )
  {
    mCacheInMemoryCheckBox->setEnabled( false );
    mCacheInMemoryCheckBox->setToolTip( tr( "Caching can not be enabled if editable join layer is enabled" ) );
    mCacheEnabled = mCacheInMemoryCheckBox->isChecked();
    mCacheInMemoryCheckBox->setChecked( false );
  }
  else
  {
    mCacheInMemoryCheckBox->setEnabled( true );
    mCacheInMemoryCheckBox->setToolTip( QString() );
    mCacheInMemoryCheckBox->setChecked( mCacheEnabled );
  }
}
