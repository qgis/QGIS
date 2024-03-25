/***************************************************************************
    qgslistwidget.cpp
     --------------------------------------
    Date                 : 08.2016
    Copyright            : (C) 2016 Patrick Valsecchi
    Email                : patrick.valsecchi@camptocamp.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslistwidget.h"

QgsListWidget::QgsListWidget( QVariant::Type subType, QWidget *parent )
  : QgsTableWidgetBase( parent )
  , mModel( subType, this )
  , mSubType( subType )
{
  init( &mModel );
}

void QgsListWidget::setList( const QVariantList &list )
{
  removeButton->setEnabled( false );
  mModel.setList( list );
}

void QgsListWidget::setReadOnly( bool readOnly )
{
  mModel.setReadOnly( readOnly );
  QgsTableWidgetBase::setReadOnly( readOnly );
}


///@cond PRIVATE
QgsListModel::QgsListModel( QVariant::Type subType, QObject *parent ) :
  QAbstractTableModel( parent ),
  mSubType( subType )
{
}

void QgsListModel::setList( const QVariantList &list )
{
  beginResetModel();
  mLines = list;
  endResetModel();
}

QVariantList QgsListModel::list() const
{
  QVariantList result;
  for ( QVariantList::const_iterator it = mLines.constBegin(); it != mLines.constEnd(); ++it )
  {
    QVariant cur = *it;
    if ( cur.convert( mSubType ) )
      result.append( cur );
  }
  return result;
}

bool QgsListModel::valid() const
{
  for ( QVariantList::const_iterator it = mLines.constBegin(); it != mLines.constEnd(); ++it )
  {
    QVariant cur = *it;
    if ( !cur.convert( mSubType ) ) return false;
  }
  return true;
}

int QgsListModel::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return mLines.count();
}

int QgsListModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return 1;
}

QVariant QgsListModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation == Qt::Horizontal && role == Qt::DisplayRole && section == 0 )
  {
    return QObject::tr( "Value" );
  }
  return QVariant();
}

QVariant QgsListModel::data( const QModelIndex &index, int role ) const
{
  if ( index.row() < 0 ||
       index.row() >= mLines.count() ||
       ( role != Qt::DisplayRole && role != Qt::EditRole ) ||
       index.column() != 0 )
  {
    return QVariant( mSubType );
  }
  return mLines.at( index.row() );
}

bool QgsListModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( mReadOnly )
    return false;

  if ( index.row() < 0 || index.row() >= mLines.count() ||
       index.column() != 0 || role != Qt::EditRole )
  {
    return false;
  }
  mLines[index.row()] = value.toString();
  emit dataChanged( index, index );
  return true;
}

Qt::ItemFlags QgsListModel::flags( const QModelIndex &index ) const
{
  if ( !mReadOnly )
    return QAbstractTableModel::flags( index ) | Qt::ItemIsEditable;
  else
    return QAbstractTableModel::flags( index );
}

bool QgsListModel::insertRows( int position, int rows, const QModelIndex &parent )
{
  if ( mReadOnly )
    return false;

  Q_UNUSED( parent )
  beginInsertRows( QModelIndex(), position, position + rows - 1 );
  for ( int i = 0; i < rows; ++i )
  {
    mLines.insert( position, QVariant( mSubType ) );
  }
  endInsertRows();
  return true;
}

bool QgsListModel::removeRows( int position, int rows, const QModelIndex &parent )
{
  if ( mReadOnly )
    return false;

  Q_UNUSED( parent )
  beginRemoveRows( QModelIndex(), position, position + rows - 1 );
  for ( int i = 0; i < rows; ++i )
    mLines.removeAt( position );
  endRemoveRows();
  return true;
}

void QgsListModel::setReadOnly( bool readOnly )
{
  mReadOnly = readOnly;
}
///@endcond
