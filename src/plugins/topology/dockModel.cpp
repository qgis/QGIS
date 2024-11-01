/***************************************************************************
  dockModel.cpp
  TOPOLogy checker
  -------------------
         date                 : May 2009
         copyright            : (C) 2009 by Vita Cizek
         email                : weetya (at) gmail.com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "dockModel.h"
#include "moc_dockModel.cpp"
#include "topolError.h"
#include "qgsvectorlayer.h"
#include <qlogging.h>

DockModel::DockModel( QObject *parent )
{
  Q_UNUSED( parent )
  mHeader << QObject::tr( "Error" ) << QObject::tr( "Layer" ) << QObject::tr( "Feature ID" );
}

void DockModel::setErrors( const ErrorList &errorList )
{
  beginResetModel();
  mErrorlist = errorList;
  endResetModel();
}

int DockModel::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return mErrorlist.count();
}

int DockModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return 3;
}

QVariant DockModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( role == Qt::DisplayRole )
  {
    if ( orientation == Qt::Vertical ) //row
    {
      return QVariant( section );
    }
    else if ( section >= 0 && section < mHeader.count() )
    {
      return mHeader[section];
    }
  }

  return QAbstractItemModel::headerData( section, orientation, role );
}

QVariant DockModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() || ( role != Qt::TextAlignmentRole && role != Qt::DisplayRole && role != Qt::EditRole ) )
    return QVariant();

  const int row = index.row();
  //  if(!row)
  //    {
  //      return QVariant();
  //    }
  const int column = index.column();

  if ( role == Qt::TextAlignmentRole )
  {
    if ( column )
      return static_cast<Qt::Alignment::Int>( Qt::AlignRight );
    else
      return static_cast<Qt::Alignment::Int>( Qt::AlignLeft );
  }

  QVariant val;
  switch ( column )
  {
    case 0:
      val = mErrorlist[row]->name();
      break;
    case 1:
      if ( !mErrorlist[row]->featurePairs().first().layer )
        val = QStringLiteral( "Unknown" );
      else
        val = mErrorlist[row]->featurePairs().first().layer->name();
      break;
    case 2:
      val = mErrorlist[row]->featurePairs().first().feature.id();
      break;
    default:
      val = QVariant();
  }

  if ( QgsVariantUtils::isNull( val ) )
  {
    return QVariant();
  }

  // convert to QString from some other representation
  // this prevents displaying greater numbers in exponential format
  return val.toString();
}

bool DockModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  Q_UNUSED( index )
  Q_UNUSED( value )
  Q_UNUSED( role )
  return false;
}

Qt::ItemFlags DockModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return Qt::ItemIsEnabled;

  Qt::ItemFlags flags = QAbstractTableModel::flags( index );
  return flags;
}

void DockModel::reload( const QModelIndex &index1, const QModelIndex &index2 )

{
  emit dataChanged( index1, index2 );
}

DockFilterModel::DockFilterModel( QObject *parent )
  : QSortFilterProxyModel( parent )
  , mDockModel( new DockModel( parent ) )
{
  setSourceModel( mDockModel );
  setFilterKeyColumn( 0 );
}

void DockFilterModel::setErrors( const ErrorList &errorList )
{
  mDockModel->setErrors( errorList );
}

void DockFilterModel::reload( const QModelIndex &index1, const QModelIndex &index2 )
{
  mDockModel->reload( index1, index2 );
}
