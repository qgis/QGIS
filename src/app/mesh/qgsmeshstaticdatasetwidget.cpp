/***************************************************************************
    qgsmeshstaticdatasetwidget.cpp
    -------------------------------------
    begin                : March 2020
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

#include "qgsmeshstaticdatasetwidget.h"

#include "qgsmeshlayer.h"

QgsMeshStaticDatasetWidget::QgsMeshStaticDatasetWidget( QWidget *parent ): QWidget( parent )
{
  setupUi( this );

  mDatasetScalarModel = new QgsMeshDatasetListModel( this );
  mScalarDatasetComboBox->setModel( mDatasetScalarModel );
  mDatasetVectorModel = new QgsMeshDatasetListModel( this );
  mVectorDatasetComboBox->setModel( mDatasetVectorModel );
}

void QgsMeshStaticDatasetWidget::setLayer( QgsMeshLayer *layer )
{
  mLayer = layer;
}

void QgsMeshStaticDatasetWidget::syncToLayer()
{
  if ( !mLayer )
    return;

  mScalarDatasetGroup = mLayer->rendererSettings().activeScalarDatasetGroup();
  mVectorDatasetGroup = mLayer->rendererSettings().activeVectorDatasetGroup();
  mDatasetScalarModel->setMeshLayer( mLayer );
  mDatasetScalarModel->setDatasetGroup( mScalarDatasetGroup );
  mDatasetVectorModel->setMeshLayer( mLayer );
  mDatasetVectorModel->setDatasetGroup( mVectorDatasetGroup );

  mScalarDatasetComboBox->setCurrentIndex( mLayer->staticScalarDatasetIndex().dataset() + 1 );
  mVectorDatasetComboBox->setCurrentIndex( mLayer->staticVectorDatasetIndex().dataset() + 1 );
}

void QgsMeshStaticDatasetWidget::apply()
{
  if ( !mLayer )
    return;

  mLayer->setStaticScalarDatasetIndex( QgsMeshDatasetIndex( mScalarDatasetGroup, mScalarDatasetComboBox->currentIndex() - 1 ) );
  mLayer->setStaticVectorDatasetIndex( QgsMeshDatasetIndex( mVectorDatasetGroup, mVectorDatasetComboBox->currentIndex() - 1 ) );
}

void QgsMeshStaticDatasetWidget::setScalarDatasetGroup( int index )
{
  mScalarDatasetGroup = index;
  mDatasetScalarModel->setDatasetGroup( index );
  mScalarName->setText( mLayer->dataProvider()->datasetGroupMetadata( index ).name() );
}

void QgsMeshStaticDatasetWidget::setVectorDatasetGroup( int index )
{
  mVectorDatasetGroup = index;
  mDatasetVectorModel->setDatasetGroup( index );
  mVectorName->setText( mLayer->dataProvider()->datasetGroupMetadata( index ).name() );
}

QgsMeshDatasetListModel::QgsMeshDatasetListModel( QObject *parent ): QAbstractListModel( parent )
{}

void QgsMeshDatasetListModel::setMeshLayer( QgsMeshLayer *layer )
{
  beginResetModel();
  mLayer = layer;
  endResetModel();
}

void QgsMeshDatasetListModel::setDatasetGroup( int group )
{
  beginResetModel();
  mDatasetGroup = group;
  endResetModel();
}

int QgsMeshDatasetListModel::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )

  if ( mLayer && mLayer->dataProvider() )
    return  mLayer->dataProvider()->datasetCount( mDatasetGroup ) + 1;
  else
    return 0;
}

QVariant QgsMeshDatasetListModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  if ( role == Qt::DisplayRole )
  {
    if ( !mLayer || !mLayer->dataProvider() || mDatasetGroup < 0 || index.row() == 0 )
      return tr( "none" );

    else if ( index.row() == 1 && mLayer->dataProvider()->datasetCount( mDatasetGroup ) == 1 )
    {
      return tr( "Display dataset" );
    }
    else
    {
      qint64 time = mLayer->dataProvider()->temporalCapabilities()->datasetTime( QgsMeshDatasetIndex( mDatasetGroup, index.row() - 1 ) );
      return mLayer->formatTime( time / 3600.0 / 1000.0 );
    }
  }

  return QVariant();
}
