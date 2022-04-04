/***************************************************************************
                         qgspointcloudattributemodel.cpp
                         ---------------------
    begin                : November 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointcloudattributemodel.h"
#include "qgspointcloudlayer.h"
#include "qgspointcloudindex.h"
#include "qgsapplication.h"

QgsPointCloudAttributeModel::QgsPointCloudAttributeModel( QObject *parent )
  : QAbstractItemModel( parent )
{

}

void QgsPointCloudAttributeModel::setLayer( QgsPointCloudLayer *layer )
{
  if ( layer )
  {
    mLayer = layer;
    setAttributes( layer->attributes() );
  }
  else
    setAttributes( QgsPointCloudAttributeCollection() );
}

QgsPointCloudLayer *QgsPointCloudAttributeModel::layer()
{
  return mLayer;
}

void QgsPointCloudAttributeModel::setAttributes( const QgsPointCloudAttributeCollection &attributes )
{
  beginResetModel();
  mAttributes = attributes;
  endResetModel();
}

void QgsPointCloudAttributeModel::setAllowEmptyAttributeName( bool allowEmpty )
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

QModelIndex QgsPointCloudAttributeModel::indexFromName( const QString &name )
{
  if ( !name.isEmpty() )
  {
    const int idx = mAttributes.indexOf( name );
    if ( idx >= 0 )
    {
      if ( mAllowEmpty )
        return index( 1 + idx, 0 );
      else
        return index( idx, 0 );
    }
  }

  if ( mAllowEmpty && name.isEmpty() )
    return index( 0, 0 );

  return QModelIndex();
}

QModelIndex QgsPointCloudAttributeModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( hasIndex( row, column, parent ) )
  {
    return createIndex( row, column, row );
  }

  return QModelIndex();
}

QModelIndex QgsPointCloudAttributeModel::parent( const QModelIndex &child ) const
{
  Q_UNUSED( child )
  return QModelIndex();
}

int QgsPointCloudAttributeModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
  {
    return 0;
  }

  return ( mAllowEmpty ? 1 : 0 ) + mAttributes.count();
}

int QgsPointCloudAttributeModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return 1;
}

QVariant QgsPointCloudAttributeModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  const bool isEmpty = mAllowEmpty && index.row() == 0;
  const int fieldOffset = mAllowEmpty ? 1 : 0;

  if ( !isEmpty && ( index.row() - fieldOffset >= mAttributes.count() ) )
    return QVariant();

  switch ( role )
  {
    case AttributeNameRole:
    {
      if ( isEmpty )
      {
        return QVariant();
      }
      return mAttributes.at( index.row() - fieldOffset ).name();
    }

    case AttributeIndexRole:
    {
      if ( isEmpty )
      {
        return QVariant();
      }
      return index.row() - fieldOffset;
    }

    case AttributeSizeRole:
    {
      if ( isEmpty )
      {
        return QVariant();
      }
      return static_cast< int >( mAttributes.at( index.row() - fieldOffset ).size() );
    }

    case AttributeTypeRole:
    {
      if ( isEmpty )
      {
        return QVariant();
      }
      return static_cast< int >( mAttributes.at( index.row() - fieldOffset ).type() );
    }

    case IsEmptyRole:
    {
      return isEmpty;
    }

    case IsNumericRole:
    {
      if ( isEmpty )
      {
        return QVariant();
      }
      return QgsPointCloudAttribute::isNumeric( mAttributes.at( index.row() - fieldOffset ).type() );
    }

    case Qt::DisplayRole:
    case Qt::EditRole:
    case Qt::ToolTipRole:
    {
      if ( isEmpty )
      {
        return QVariant();
      }
      else if ( role == Qt::ToolTipRole )
      {
        return attributeToolTip( mAttributes.at( index.row() - fieldOffset ) );
      }
      else
        return mAttributes.at( index.row() - fieldOffset ).name();
    }

    case Qt::DecorationRole:
    {
      if ( !isEmpty )
      {
        return iconForAttributeType( mAttributes.at( index.row() - fieldOffset ).type() );
      }
      return QIcon();
    }

    default:
      return QVariant();
  }
}

QString QgsPointCloudAttributeModel::attributeToolTip( const QgsPointCloudAttribute &attribute )
{
  QString toolTip = QStringLiteral( "<b>%1</b>" ).arg( attribute.name() );

  toolTip += QStringLiteral( "<br><font style='font-family:monospace; white-space: nowrap;'>%3</font>" ).arg( attribute.displayType() );

  return toolTip;
}

QIcon QgsPointCloudAttributeModel::iconForAttributeType( QgsPointCloudAttribute::DataType type )
{
  switch ( type )
  {
    case QgsPointCloudAttribute::Short:
    case QgsPointCloudAttribute::UShort:
    case QgsPointCloudAttribute::Int32:
    case QgsPointCloudAttribute::Int64:
    case QgsPointCloudAttribute::UInt32:
    case QgsPointCloudAttribute::UInt64:
    {
      return QgsApplication::getThemeIcon( "/mIconFieldInteger.svg" );
    }
    case QgsPointCloudAttribute::Float:
    case QgsPointCloudAttribute::Double:
    {
      return QgsApplication::getThemeIcon( "/mIconFieldFloat.svg" );
    }
    case QgsPointCloudAttribute::Char:
    case QgsPointCloudAttribute::UChar:
    {
      return QgsApplication::getThemeIcon( "/mIconFieldText.svg" );
    }

  }
  return QIcon();
}

//
// QgsPointCloudAttributeProxyModel
//

QgsPointCloudAttributeProxyModel::QgsPointCloudAttributeProxyModel( QgsPointCloudAttributeModel *source, QObject *parent )
  : QSortFilterProxyModel( parent )
  , mModel( source )
{
  setSourceModel( mModel );
}

QgsPointCloudAttributeProxyModel *QgsPointCloudAttributeProxyModel::setFilters( QgsPointCloudAttributeProxyModel::Filters filters )
{
  mFilters = filters;
  invalidateFilter();
  return this;
}

bool QgsPointCloudAttributeProxyModel::filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const
{
  const QModelIndex index = sourceModel()->index( source_row, 0, source_parent );

  if ( mFilters.testFlag( AllTypes ) )
    return true;

  const QVariant typeVar = mModel->data( index, QgsPointCloudAttributeModel::AttributeTypeRole );
  if ( typeVar.isNull() )
    return true;

  bool ok;
  const QgsPointCloudAttribute::DataType type = static_cast< QgsPointCloudAttribute::DataType >( typeVar.toInt( &ok ) );
  if ( !ok )
    return true;

  if ( ( mFilters.testFlag( Char ) && type == QgsPointCloudAttribute::Char ) ||
       ( mFilters.testFlag( Char ) && type == QgsPointCloudAttribute::UChar ) ||
       ( mFilters.testFlag( Short ) && type == QgsPointCloudAttribute::Short ) ||
       ( mFilters.testFlag( Short ) && type == QgsPointCloudAttribute::UShort ) ||
       ( mFilters.testFlag( Int32 ) && type == QgsPointCloudAttribute::Int32 ) ||
       ( mFilters.testFlag( Int32 ) && type == QgsPointCloudAttribute::UInt32 ) ||
       ( mFilters.testFlag( Int32 ) && type == QgsPointCloudAttribute::Int64 ) ||
       ( mFilters.testFlag( Int32 ) && type == QgsPointCloudAttribute::UInt64 ) ||
       ( mFilters.testFlag( Float ) && type == QgsPointCloudAttribute::Float ) ||
       ( mFilters.testFlag( Double ) && type == QgsPointCloudAttribute::Double ) )
    return true;

  return false;
}

bool QgsPointCloudAttributeProxyModel::lessThan( const QModelIndex &left, const QModelIndex &right ) const
{
  // empty field is always first
  if ( sourceModel()->data( left, QgsPointCloudAttributeModel::IsEmptyRole ).toBool() )
    return true;
  else if ( sourceModel()->data( right, QgsPointCloudAttributeModel::IsEmptyRole ).toBool() )
    return false;

  // order is attribute order
  bool lok, rok;
  const int leftId = sourceModel()->data( left, QgsPointCloudAttributeModel::AttributeIndexRole ).toInt( &lok );
  const int rightId = sourceModel()->data( right, QgsPointCloudAttributeModel::AttributeIndexRole ).toInt( &rok );

  if ( !lok )
    return false;
  if ( !rok )
    return true;

  return leftId < rightId;
}
