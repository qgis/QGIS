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
  QString fldName( fieldName ); // we may need a copy

  if ( mLayer )
  {
    // the name could be an alias
    // it would be better to have "display name" directly in QgsFields
    // rather than having to consult layer in various places in code!
    QString fieldNameWithAlias = mLayer->attributeAliases().key( fldName );
    if ( !fieldNameWithAlias.isNull() )
      fldName = fieldNameWithAlias;
  }

  int r = mFields.indexFromName( fldName );
  QModelIndex idx = index( r, 0 );
  if ( idx.isValid() )
  {
    return idx;
  }

  if ( mAllowExpression )
  {
    int exprIdx = mExpression.indexOf( fldName );
    if ( exprIdx != -1 )
    {
      return index( mFields.count() + exprIdx, 0 );
    }
  }

  return QModelIndex();
}

bool QgsFieldModel::isField( const QString& expression )
{
  int index = mFields.indexFromName( expression );
  return index >= 0;
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
  if ( mLayer )
  {
    QgsFields newFields = mLayer->fields();
    if ( mFields.toList() != newFields.toList() )
    {
      // Try to handle two special cases: addition of a new field and removal of a field.
      // It would be better to listen directly to attributeAdded/attributeDeleted
      // so we would not have to check for addition/removal here.

      if ( mFields.count() == newFields.count() - 1 )
      {
        QgsFields tmpNewFields = newFields;
        tmpNewFields.remove( tmpNewFields.count() - 1 );
        if ( mFields.toList() == tmpNewFields.toList() )
        {
          // the only change is a new field at the end
          beginInsertRows( QModelIndex(), mFields.count(), mFields.count() );
          mFields = newFields;
          endInsertRows();
          return;
        }
      }

      if ( mFields.count() == newFields.count() + 1 )
      {
        QgsFields tmpOldFields = mFields;
        tmpOldFields.remove( tmpOldFields.count() - 1 );
        if ( tmpOldFields.toList() == newFields.toList() )
        {
          // the only change is a field removed at the end
          beginRemoveRows( QModelIndex(), mFields.count() - 1, mFields.count() - 1 );
          mFields = newFields;
          endRemoveRows();
          return;
        }

        for ( int i = 0; i < newFields.count(); ++i )
        {
          if ( mFields.at( i ) != newFields.at( i ) )
          {
            QgsFields tmpOldFields = mFields;
            tmpOldFields.remove( i );
            if ( tmpOldFields.toList() != newFields.toList() )
              break; // the change is more complex - go with general case

            // the only change is a field removed at index i
            beginRemoveRows( QModelIndex(), i, i );
            mFields = newFields;
            endRemoveRows();
            return;
          }
        }
      }

      // general case with reset - not good - resets selections
      beginResetModel();
      mFields = mLayer->fields();
      endResetModel();
    }
    else
      emit dataChanged( index( 0, 0 ), index( rowCount(), 0 ) );
  }
  else
  {
    beginResetModel();
    mFields = QgsFields();
    endResetModel();
  }
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

void QgsFieldModel::setExpression( const QString &expression )
{
  if ( !mAllowExpression )
    return;

  QModelIndex idx = indexFromName( expression );
  if ( idx.isValid() )
    return;

  beginResetModel();
  mExpression = QList<QString>();
  if ( !expression.isEmpty() )
    mExpression << expression;
  endResetModel();
}

void QgsFieldModel::removeExpression()
{
  beginResetModel();
  mExpression = QList<QString>();
  endResetModel();
}

QModelIndex QgsFieldModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( hasIndex( row, column, parent ) )
  {
    return createIndex( row, column, row );
  }

  return QModelIndex();
}

QModelIndex QgsFieldModel::parent( const QModelIndex &child ) const
{
  Q_UNUSED( child );
  return QModelIndex();
}

int QgsFieldModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
  {
    return 0;
  }

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

  int exprIdx = index.row() - mFields.count();

  switch ( role )
  {
    case FieldNameRole:
    {
      if ( exprIdx >= 0 )
      {
        return "";
      }
      QgsField field = mFields[index.row()];
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
        QgsField field = mFields[index.row()];
        return field.name();
      }
    }

    case FieldIndexRole:
    {
      if ( exprIdx >= 0 )
      {
        return QVariant();
      }
      return index.row();
    }

    case IsExpressionRole:
    {
      return exprIdx >= 0;
    }

    case ExpressionValidityRole:
    {
      if ( exprIdx >= 0 )
      {
        QgsExpression exp( mExpression[exprIdx] );
        QgsExpressionContext context;
        if ( mLayer )
          context.setFields( mLayer->fields() );

        exp.prepare( &context );
        return !exp.hasParserError();
      }
      return true;
    }

    case FieldTypeRole:
    {
      if ( exprIdx < 0 )
      {
        QgsField field = mFields[index.row()];
        return ( int )field.type();
      }
      return QVariant();
    }

    case Qt::DisplayRole:
    case Qt::EditRole:
    {
      if ( exprIdx >= 0 )
      {
        return mExpression[exprIdx];
      }
      else if ( role == Qt::EditRole )
      {
        return mFields[index.row()].name();
      }
      else if ( mLayer )
      {
        return mLayer->attributeDisplayName( index.row() );
      }
      else
        return QVariant();
    }

    case Qt::ForegroundRole:
    {
      if ( exprIdx >= 0 )
      {
        // if expression, test validity
        QgsExpression exp( mExpression[exprIdx] );
        QgsExpressionContext context;
        if ( mLayer )
          context.setFields( mLayer->fields() );

        exp.prepare( &context );
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
