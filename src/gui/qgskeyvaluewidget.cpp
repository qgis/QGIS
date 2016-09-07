/***************************************************************************
    qgskeyvaluewidget.cpp
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

#include "qgskeyvaluewidget.h"

QgsKeyValueWidget::QgsKeyValueWidget( QWidget* parent )
    : QWidget( parent )
    , mModel( this )
{
  setupUi( this );
  tableView->setModel( &mModel );
  connect( tableView->selectionModel(), SIGNAL( selectionChanged( const QItemSelection &, const QItemSelection & ) ), this, SLOT( onSelectionChanged() ) );
  connect( &mModel, SIGNAL( dataChanged( const QModelIndex&, const QModelIndex& ) ), this, SLOT( onValueChanged() ) );
}

void QgsKeyValueWidget::setMap( const QVariantMap& map )
{
  removeButton->setEnabled( false );
  mModel.setMap( map );
}

void QgsKeyValueWidget::on_addButton_clicked()
{
  const QItemSelectionModel *select = tableView->selectionModel();
  const int pos = select->hasSelection() ? select->selectedRows()[0].row() : 0;
  mModel.insertRows( pos, 1 );
  const QModelIndex index = mModel.index( pos, 0 );
  tableView->scrollTo( index );
  tableView->edit( index );
  tableView->selectRow( pos );
}

void QgsKeyValueWidget::on_removeButton_clicked()
{
  const QItemSelectionModel *select = tableView->selectionModel();
  // The UI is configured to have single row selection.
  if ( select->hasSelection() )
  {
    mModel.removeRows( select->selectedRows()[0].row(), 1 );
  }
}

void QgsKeyValueWidget::onSelectionChanged()
{
  removeButton->setEnabled( tableView->selectionModel()->hasSelection() );
}

void QgsKeyValueWidget::onValueChanged()
{
  emit valueChanged( QVariant( map() ) );
}

///@cond PRIVATE
void QgsKeyValueModel::setMap( const QVariantMap& map )
{
  emit beginResetModel();
  mLines.clear();
  for ( QVariantMap::const_iterator it = map.constBegin(); it != map.constEnd(); ++it )
  {
    mLines.append( Line( it.key(), it.value() ) );
  }
  emit endResetModel();
}

QVariantMap QgsKeyValueModel::map() const
{
  QVariantMap ret;
  for ( QVector<Line>::const_iterator it = mLines.constBegin(); it != mLines.constEnd(); ++it )
  {
    if ( !it->first.isEmpty() )
    {
      ret[it->first] = it->second;
    }
  }
  return ret;
}

QgsKeyValueModel::QgsKeyValueModel( QObject *parent ) :
    QAbstractTableModel( parent )
{
}

int QgsKeyValueModel::rowCount( const QModelIndex& parent ) const
{
  Q_UNUSED( parent );
  return mLines.count();
}

int QgsKeyValueModel::columnCount( const QModelIndex & parent ) const
{
  Q_UNUSED( parent );
  return 2;
}

QVariant QgsKeyValueModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation == Qt::Horizontal && role == Qt::DisplayRole )
  {
    return QObject::tr( section == 0 ? "Key" : "Value" );
  }
  return QVariant();
}

QVariant QgsKeyValueModel::data( const QModelIndex& index, int role ) const
{
  if ( index.row() < 0 ||
       index.row() >= mLines.count() ||
       ( role != Qt::DisplayRole && role != Qt::EditRole ) )
  {
    return QVariant();
  }
  if ( index.column() == 0 )
    return mLines.at( index.row() ).first;
  if ( index.column() == 1 )
    return mLines.at( index.row() ).second;
  return QVariant();
}

bool QgsKeyValueModel::setData( const QModelIndex & index, const QVariant & value, int role )
{
  if ( index.row() < 0 || index.row() >= mLines.count() || role != Qt::EditRole )
  {
    return false;
  }
  if ( index.column() == 0 )
  {
    mLines[index.row()].first = value.toString();
  }
  else
  {
    mLines[index.row()].second = value;
  }
  emit dataChanged( index, index );
  return true;
}

Qt::ItemFlags QgsKeyValueModel::flags( const QModelIndex &index ) const
{
  return QAbstractTableModel::flags( index ) | Qt::ItemIsEditable;
}

bool QgsKeyValueModel::insertRows( int position, int rows, const QModelIndex & parent )
{
  Q_UNUSED( parent );
  beginInsertRows( QModelIndex(), position, position + rows - 1 );
  for ( int i = 0; i < rows; ++i )
  {
    mLines.insert( position, Line( "", QVariant() ) );
  }
  endInsertRows();
  return true;
}

bool QgsKeyValueModel::removeRows( int position, int rows, const QModelIndex &parent )
{
  Q_UNUSED( parent );
  beginRemoveRows( QModelIndex(), position, position + rows - 1 );
  mLines.remove( position, rows );
  endRemoveRows();
  return true;
}
///@endcond
