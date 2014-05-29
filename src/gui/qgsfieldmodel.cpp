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

#include <QFont>

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

QModelIndex QgsFieldModel::indexFromName( const QString &fieldName )
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
  }

  return QModelIndex();
}

void QgsFieldModel::setLayer( QgsVectorLayer *layer )
{
  if ( mLayer )
  {
    disconnect( mLayer, SIGNAL( updatedFields() ), this, SLOT( updateModel() ) );
    disconnect( mLayer, SIGNAL( layerDeleted() ), this, SLOT( layerDeleted() ) );
  }

  if ( !layer )
  {
    mLayer = 0;
    updateModel();
    return;
  }

  mLayer = layer;
  connect( mLayer, SIGNAL( updatedFields() ), this, SLOT( updateModel() ) );
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

QModelIndex QgsFieldModel::setExpression( const QString &expression )
{
  if ( !mAllowExpression )
    return QModelIndex();

  beginResetModel();
  mExpression = QList<QString>() << expression;
  endResetModel();

  return index( mFields.count() , 0 );
}

void QgsFieldModel::removeExpression()
{
  beginResetModel();
  mExpression = QList<QString>();
  endResetModel();
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

  int exprIdx = index.internalId() - mFields.count();

  switch ( role )
  {
    case FieldNameRole:
    {
      if ( exprIdx >= 0 )
      {
        return "";
      }
      QgsField field = mFields[index.internalId()];
      return field.name();
    }

    case ExpressionRole:
    {
      if ( exprIdx >= 0 )
      {
        return mExpression[exprIdx];
      }
      else
      {
        QgsField field = mFields[index.internalId()];
        return field.name();
      }
    }

    case FieldIndexRole:
    {
      if ( exprIdx >= 0 )
      {
        return QVariant();
      }
      return index.internalId();
    }

    case IsExpressionRole:
    {
      return exprIdx >= 0;
    }

    case ExpressionValidityRole:
    {
      if ( exprIdx >= 0 )
      {
        if ( !mLayer )
          return false;
        QgsExpression exp( mExpression[exprIdx] );
        exp.prepare( mLayer->pendingFields() );
        return !exp.hasParserError();
      }
      return true;
    }

    case FieldTypeRole:
    {
      if ( exprIdx < 0 )
      {
        QgsField field = mFields[index.internalId()];
        return ( int )field.type();
      }
    }

    case Qt::DisplayRole:
    case Qt::EditRole:
    {
      if ( exprIdx >= 0 )
      {
        return mExpression[exprIdx];
      }
      QgsField field = mFields[index.internalId()];
      const QMap< QString, QString > aliases = mLayer->attributeAliases();
      QString alias = aliases.value( field.name(), field.name() );
      return alias;
    }

    case Qt::ForegroundRole:
    {
      if ( exprIdx >= 0 )
      {
        // if expression, test validity
        if ( !mLayer )
          return false;
        QgsExpression exp( mExpression[exprIdx] );
        exp.prepare( mLayer->pendingFields() );
        if ( exp.hasParserError() )
        {
          return QBrush( QColor( Qt::red ) );
        }
      }
      return QVariant();
    }

    case Qt::FontRole:
    {
      if ( exprIdx >= 0 )
      {
        // if the line is an expression, set it as italic
        QFont font = QFont();
        font.setItalic( true );
        return font;
      }
      return QVariant();
    }

    default:
      return QVariant();
  }
}
