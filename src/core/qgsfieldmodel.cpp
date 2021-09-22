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
#include <QIcon>

#include "qgsfieldmodel.h"
#include "qgsmaplayermodel.h"
#include "qgsmaplayerproxymodel.h"
#include "qgslogger.h"
#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerjoinbuffer.h"

QgsFieldModel::QgsFieldModel( QObject *parent )
  : QAbstractItemModel( parent )
{
}

QModelIndex QgsFieldModel::indexFromName( const QString &fieldName )
{
  QString fldName( fieldName ); // we may need a copy

  // only non-empty names should be used here, as by default all fields
  // have no aliases set and calling key() fill return just first value
  // from the aliases map
  if ( mLayer && !fldName.isEmpty() )
  {
    // the name could be an alias
    // it would be better to have "display name" directly in QgsFields
    // rather than having to consult layer in various places in code!
    const QString fieldNameWithAlias = mLayer->attributeAliases().key( fldName );
    if ( !fieldNameWithAlias.isNull() )
      fldName = fieldNameWithAlias;
  }

  if ( mAllowEmpty && fieldName.isEmpty() )
    return index( 0, 0 );

  int r = mFields.lookupField( fldName );
  if ( r >= 0 )
  {
    if ( mAllowEmpty )
      r++;

    QModelIndex idx = index( r, 0 );
    if ( idx.isValid() )
    {
      return idx;
    }
  }

  if ( mAllowExpression )
  {
    const int exprIdx = mExpression.indexOf( fldName );
    if ( exprIdx != -1 )
    {
      return index( ( mAllowEmpty ? 1 : 0 ) + mFields.count() + exprIdx, 0 );
    }
  }

  return QModelIndex();
}

bool QgsFieldModel::isField( const QString &expression ) const
{
  const int index = mFields.indexFromName( expression );
  return index >= 0;
}

void QgsFieldModel::setLayer( QgsVectorLayer *layer )
{
  if ( mLayer )
  {
    disconnect( mLayer, &QgsVectorLayer::updatedFields, this, &QgsFieldModel::updateModel );
    disconnect( mLayer, &QObject::destroyed, this, &QgsFieldModel::layerDeleted );
  }

  mLayer = layer;

  if ( mLayer )
  {
    connect( mLayer, &QgsVectorLayer::updatedFields, this, &QgsFieldModel::updateModel );
    connect( mLayer, &QObject::destroyed, this, &QgsFieldModel::layerDeleted );
  }

  updateModel();
}

void QgsFieldModel::layerDeleted()
{
  mLayer = nullptr;
  updateModel();
}

void QgsFieldModel::updateModel()
{
  const int offset = mAllowEmpty ? 1 : 0;
  if ( mLayer )
  {
    const QgsFields newFields = mLayer->fields();
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
          beginInsertRows( QModelIndex(), mFields.count() + offset, mFields.count() + offset );
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
          beginRemoveRows( QModelIndex(), mFields.count() - 1 + offset, mFields.count() - 1 + offset );
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
            beginRemoveRows( QModelIndex(), i + offset, i + offset );
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
      emit dataChanged( index( 0 + offset, 0 ), index( rowCount(), 0 ) );
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
    const int start = mFields.count();
    const int end = start + mExpression.count() - 1;
    beginRemoveRows( QModelIndex(), start, end );
    mExpression = QList<QString>();
    endRemoveRows();
  }
}

void QgsFieldModel::setAllowEmptyFieldName( bool allowEmpty )
{
  if ( allowEmpty == mAllowEmpty )
    return;

  if ( allowEmpty )
  {
    beginInsertRows( QModelIndex(), 0, 0 );
    mAllowEmpty = true;
    endInsertRows();
  }
  else
  {
    beginRemoveRows( QModelIndex(), 0, 0 );
    mAllowEmpty = false;
    endRemoveRows();
  }
}


void QgsFieldModel::setExpression( const QString &expression )
{
  if ( !mAllowExpression )
    return;

  const QModelIndex idx = indexFromName( expression );
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
  Q_UNUSED( child )
  return QModelIndex();
}

int QgsFieldModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
  {
    return 0;
  }

  return ( mAllowEmpty ? 1 : 0 ) + ( mAllowExpression ? mFields.count() + mExpression.count() : mFields.count() );
}

int QgsFieldModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return 1;
}

QVariant QgsFieldModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  int exprIdx = index.row() - mFields.count();
  if ( mAllowEmpty )
    exprIdx--;
  const bool isEmpty = mAllowEmpty && index.row() == 0;
  const int fieldOffset = mAllowEmpty ? 1 : 0;

  switch ( role )
  {
    case FieldNameRole:
    {
      if ( isEmpty || exprIdx >= 0 )
      {
        return QString();
      }
      const QgsField field = mFields.at( index.row() - fieldOffset );
      return field.name();
    }

    case ExpressionRole:
    {
      if ( exprIdx >= 0 )
      {
        return mExpression.at( exprIdx );
      }
      else if ( isEmpty )
      {
        return QVariant();
      }
      else
      {
        const QgsField field = mFields.at( index.row() - fieldOffset );
        return field.name();
      }
    }

    case FieldIndexRole:
    {
      if ( isEmpty || exprIdx >= 0 )
      {
        return QVariant();
      }
      return index.row() - fieldOffset;
    }

    case IsExpressionRole:
    {
      return exprIdx >= 0;
    }

    case ExpressionValidityRole:
    {
      if ( exprIdx >= 0 )
      {
        QgsExpression exp( mExpression.at( exprIdx ) );
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
      if ( exprIdx < 0 && !isEmpty )
      {
        const QgsField field = mFields.at( index.row() - fieldOffset );
        return static_cast< int >( field.type() );
      }
      return QVariant();
    }

    case FieldOriginRole:
    {
      if ( exprIdx < 0 && !isEmpty )
      {
        return static_cast< int >( mFields.fieldOrigin( index.row() - fieldOffset ) );
      }
      return QVariant();
    }

    case IsEmptyRole:
    {
      return isEmpty;
    }

    case EditorWidgetType:
    {
      if ( exprIdx < 0 && !isEmpty )
      {
        return mFields.at( index.row() - fieldOffset ).editorWidgetSetup().type();
      }
      return QVariant();
    }

    case JoinedFieldIsEditable:
    {
      if ( exprIdx < 0 && !isEmpty )
      {
        if ( mLayer && mFields.fieldOrigin( index.row() - fieldOffset ) == QgsFields::OriginJoin )
        {
          int srcFieldIndex;
          const QgsVectorLayerJoinInfo *info = mLayer->joinBuffer()->joinForFieldIndex( index.row() - fieldOffset, mLayer->fields(), srcFieldIndex );

          if ( !info || !info->isEditable() )
            return false;

          return true;
        }
      }
      return QVariant();
    }

    case FieldIsWidgetEditable:
    {
      return !( mLayer->editFormConfig().readOnly( index.row() - fieldOffset ) );
    }


    case Qt::DisplayRole:
    case Qt::EditRole:
    case Qt::ToolTipRole:
    {
      if ( isEmpty )
      {
        return QVariant();
      }
      else if ( exprIdx >= 0 )
      {
        return mExpression.at( exprIdx );
      }
      else if ( role == Qt::EditRole )
      {
        return mFields.at( index.row() - fieldOffset ).name();
      }
      else if ( role == Qt::ToolTipRole )
      {
        return fieldToolTip( mFields.at( index.row() - fieldOffset ) );
      }
      else if ( mLayer )
      {
        return mLayer->attributeDisplayName( index.row() - fieldOffset );
      }
      else if ( mFields.size() > index.row() - fieldOffset )
      {
        return mFields.field( index.row() - fieldOffset ).displayName();
      }
      else
        return QVariant();
    }

    case Qt::ForegroundRole:
    {
      if ( !isEmpty && exprIdx >= 0 )
      {
        // if expression, test validity
        QgsExpression exp( mExpression.at( exprIdx ) );
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
      if ( !isEmpty && exprIdx >= 0 )
      {
        // if the line is an expression, set it as italic
        QFont font = QFont();
        font.setItalic( true );
        return font;
      }
      return QVariant();
    }

    case Qt::DecorationRole:
    {
      if ( !isEmpty && exprIdx < 0 )
      {
        return mFields.iconForField( index.row() - fieldOffset );
      }
      return QIcon();
    }

    default:
      return QVariant();
  }
}

QString QgsFieldModel::fieldToolTip( const QgsField &field )
{
  QString toolTip;
  if ( !field.alias().isEmpty() )
  {
    toolTip = QStringLiteral( "<b>%1</b> (%2)" ).arg( field.alias(), field.name() );
  }
  else
  {
    toolTip = QStringLiteral( "<b>%1</b>" ).arg( field.name() );
  }

  toolTip += QStringLiteral( "<br><font style='font-family:monospace; white-space: nowrap;'>%3</font>" ).arg( field.displayType( true ) );

  const QString comment = field.comment();

  if ( ! comment.isEmpty() )
  {
    toolTip += QStringLiteral( "<br><em>%1</em>" ).arg( comment );
  }

  return toolTip;
}

QString QgsFieldModel::fieldToolTipExtended( const QgsField &field, const QgsVectorLayer *layer )
{
  QString toolTip = QgsFieldModel::fieldToolTip( field );
  const QgsFields fields = layer->fields();
  const int fieldIdx = fields.indexOf( field.name() );

  if ( fieldIdx < 0 )
    return QString();

  const QString expressionString = fields.fieldOrigin( fieldIdx ) == QgsFields::OriginExpression
                                   ? layer->expressionField( fieldIdx )
                                   : QString();

  if ( !expressionString.isEmpty() )
  {
    toolTip += QStringLiteral( "<br><font style='font-family:monospace;'>%3</font>" ).arg( expressionString );
  }

  return toolTip;
}

void QgsFieldModel::setFields( const QgsFields &fields )
{
  setLayer( nullptr );
  beginResetModel();
  mFields = fields;
  endResetModel();
}

QgsFields QgsFieldModel::fields() const
{
  return mFields;
}
