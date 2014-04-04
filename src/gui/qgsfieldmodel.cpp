/***************************************************************************
   qgsfieldmodel.cpp
    --------------------------------------
   Date                 : 01.04.2014
   Copyright            : (C) 2014 Denis Rouzaud
   Email                : denis.rouzaud@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qgsfieldmodel.h"
#include "qgsmaplayermodel.h"
#include "qgsmaplayerproxymodel.h"
#include "qgslogger.h"


QgsFieldModel::QgsFieldModel( QObject *parent )
    : QAbstractItemModel( parent )
    , mLayer( NULL )
{
}

QModelIndex QgsFieldModel::indexFromName( QString fieldName )
{
  int r = mFields.indexFromName( fieldName );
  return index( r, 0 );
}


void QgsFieldModel::setLayer( QgsMapLayer *layer )
{
  if ( mLayer )
  {
    disconnect( mLayer, SIGNAL( updatedFields() ), this, SLOT( updateFields() ) );
    disconnect( mLayer, SIGNAL( layerDeleted() ), this, SLOT( layerDeleted() ) );
  }
  QgsVectorLayer* vl = dynamic_cast<QgsVectorLayer*>( layer );
  if ( vl )
  {
    mLayer = vl;
    connect( mLayer, SIGNAL( updatedFields() ), this, SLOT( updateFields() ) );
    connect( mLayer, SIGNAL( layerDeleted() ), this, SLOT( layerDeleted() ) );
  }
  else
  {
    mLayer = 0;
  }
  updateFields();
}

void QgsFieldModel::layerDeleted()
{
  mLayer = 0;
  updateFields();
}

void QgsFieldModel::updateFields()
{
  beginResetModel();
  if ( mLayer )
    mFields = mLayer->pendingFields();
  else
    mFields = QgsFields();
  endResetModel();
}

QModelIndex QgsFieldModel::index( int row, int column, const QModelIndex &parent ) const
{
  Q_UNUSED( parent );
  if ( row < 0 || row >= mFields.count() )
    return QModelIndex();

  return createIndex( row, column, row );
}

QModelIndex QgsFieldModel::parent( const QModelIndex &child ) const
{
  Q_UNUSED( child );
  return QModelIndex();
}

int QgsFieldModel::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent );
  return mFields.count();
}

int QgsFieldModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent );
  return 1;
}

QVariant QgsFieldModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  if ( !mLayer )
    return QVariant();

  if ( role == FieldNameRole )
  {
    QgsField field = mFields[index.internalId()];
    return field.name();
  }

  if ( role == FieldIndexRole )
  {
    return index.row();
  }

  if ( role == Qt::DisplayRole )
  {
    QgsField field = mFields[index.internalId()];
    const QMap< QString, QString > aliases = mLayer->attributeAliases();
    QString alias = aliases.value( field.name(), field.name() );
    return alias;
  }

  if ( role == Qt::UserRole )
  {
    QgsField field = mFields[index.internalId()];
    return field.name();
  }

  return QVariant();
}

