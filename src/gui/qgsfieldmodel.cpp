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
    , mAllowExpression( false )
{
}

QModelIndex QgsFieldModel::indexFromName( QString fieldName )
{
  int r = mFields.indexFromName( fieldName );
  QModelIndex idx = index( r, 0 );
  if ( idx.isValid() )
  {
    return idx;
  }

  if ( mAllowExpression )
  {
    int exprIdx = mExpression.indexOf( fieldName );
    if ( exprIdx != -1 )
    {
      return index( mFields.count() + exprIdx , 0 );
    }
    else
    {
      return setExpression( fieldName );
    }
  }

  return QModelIndex();
}

void QgsFieldModel::setLayer( QgsMapLayer *layer )
{
  if ( mLayer )
  {
    disconnect( mLayer, SIGNAL( updatedFields() ), this, SLOT( updateFields() ) );
    disconnect( mLayer, SIGNAL( layerDeleted() ), this, SLOT( layerDeleted() ) );
  }

  if ( !layer )
  {
    mLayer = 0;
    updateModel();
    return;
  }
  QgsVectorLayer* vl = dynamic_cast<QgsVectorLayer*>( layer );
  if ( !vl )
  {
    mLayer = 0;
    updateModel();
    return;
  }

  mLayer = vl;
  connect( mLayer, SIGNAL( updatedFields() ), this, SLOT( updateFields() ) );
  connect( mLayer, SIGNAL( layerDeleted() ), this, SLOT( layerDeleted() ) );
  updateModel();
}

void QgsFieldModel::layerDeleted()
{
  mLayer = 0;
  updateModel();
}

void QgsFieldModel::updateModel()
{
  beginResetModel();
  mExpression = QList<QString>();
  if ( mLayer )
    mFields = mLayer->pendingFields();
  else
    mFields = QgsFields();
  endResetModel();
}

void QgsFieldModel::setAllowExpression( bool allowExpression )
{
  if ( allowExpression == mAllowExpression )
    return;

  mAllowExpression = allowExpression;

  if ( !mAllowExpression )
  {
    int start = mFields.count();
    int end = start + mExpression.count() - 1;
    beginRemoveRows( QModelIndex(), start, end );
    mExpression = QList<QString>();
    endRemoveRows();
  }
}

QModelIndex QgsFieldModel::setExpression( QString expression )
{
  if ( !mAllowExpression )
    return QModelIndex();

  beginResetModel();
  mExpression = QList<QString>() << expression;
  endResetModel();

  return index( mFields.count() , 0 );
}

QModelIndex QgsFieldModel::index( int row, int column, const QModelIndex &parent ) const
{
  Q_UNUSED( parent );
  if ( row < 0 || row >= rowCount() )
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
  return mAllowExpression ? mFields.count() + mExpression.count() : mFields.count();
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
    int exprIdx = index.internalId() - mFields.count();
    if ( exprIdx >= 0 )
    {
      return "";
    }
    QgsField field = mFields[index.internalId()];
    return field.name();
  }

  if ( role == ExpressionRole )
  {
    int exprIdx = index.internalId() - mFields.count();
    if ( exprIdx >= 0 )
    {
      return mExpression[exprIdx];
    }
    return "";
  }

  if ( role == FieldIndexRole )
  {
    if ( index.internalId() >= mFields.count() )
    {
      return QVariant();
    }
    return index.internalId();
  }

  if ( role == Qt::DisplayRole )
  {
    int exprIdx = index.internalId() - mFields.count();
    if ( exprIdx >= 0 )
    {
      return mExpression[exprIdx];
    }
    QgsField field = mFields[index.internalId()];
    const QMap< QString, QString > aliases = mLayer->attributeAliases();
    QString alias = aliases.value( field.name(), field.name() );
    return alias;
  }

  if ( role == Qt::FontRole && index.internalId() >= mFields.count() )
  {
    // if the line is an expression, set it as italic
    QFont font;
    font.setItalic( true );
    return font;
  }

  return QVariant();
}

