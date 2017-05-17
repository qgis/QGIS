/***************************************************************************
                         qgslocatoroptionswidget.cpp
                         ---------------------------
    begin                : May 2017
    copyright            : (C) 2017 by Nyall Dawson
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


#include "qgslocatoroptionswidget.h"

QgsLocatorOptionsWidget::QgsLocatorOptionsWidget( QgsLocator *locator, QWidget *parent )
  : QWidget( parent )
  , mLocator( locator )
{
  setupUi( this );

  mModel = new QgsLocatorFiltersModel( mLocator, this );
  mFiltersTreeView->setModel( mModel );

  mFiltersTreeView->header()->setStretchLastSection( false );
  mFiltersTreeView->header()->setResizeMode( 0, QHeaderView::Stretch );
}


//
// QgsLocatorFiltersModel
//

#define HIDDEN_FILTER_OFFSET 1

QgsLocatorFiltersModel::QgsLocatorFiltersModel( QgsLocator *locator, QObject *parent )
  : QAbstractTableModel( parent )
  , mLocator( locator )
{
}

int QgsLocatorFiltersModel::rowCount( const QModelIndex & ) const
{
  return mLocator->filters().count() - HIDDEN_FILTER_OFFSET;
}

int QgsLocatorFiltersModel::columnCount( const QModelIndex & ) const
{
  return 4;
}

QVariant QgsLocatorFiltersModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() || index.row() < 0 || index.column() < 0 ||
       index.row() >= rowCount( QModelIndex() ) || index.column() >= columnCount( QModelIndex() ) )
    return QVariant();

  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::EditRole:
    {
      switch ( index.column() )
      {
        case Name:
          return filterForIndex( index )->displayName();

        case Prefix:
          return filterForIndex( index )->prefix();

        case Active:
        case Default:
          return QVariant();
      }
    }

    case Qt::CheckStateRole:
      switch ( index.column() )
      {
        case Name:
        case Prefix:
          return QVariant();

        case Active:
          return filterForIndex( index )->enabled() ? Qt::Checked : Qt::Unchecked;

        case Default:
          return filterForIndex( index )->useWithoutPrefix() ? Qt::Checked : Qt::Unchecked;
      }
  }

  return QVariant();
}

bool QgsLocatorFiltersModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() || index.row() < 0 || index.column() < 0 ||
       index.row() >= rowCount( QModelIndex() ) || index.column() >= columnCount( QModelIndex() ) )
    return false;

  switch ( role )
  {
    case Qt::CheckStateRole:
    {
      switch ( index.column() )
      {
        case Name:
        case Prefix:
          return false;

        case Active:
        {
          filterForIndex( index )->setEnabled( value.toInt() == Qt::Checked );
          emit dataChanged( index, index, QVector<int>() << Qt::EditRole << Qt::CheckStateRole );
          return true;
        }

        case Default:
        {
          filterForIndex( index )->setUseWithoutPrefix( value.toInt() == Qt::Checked );
          emit dataChanged( index, index, QVector<int>() << Qt::EditRole << Qt::CheckStateRole );
          return true;
        }
      }
    }
  }
  return false;
}

Qt::ItemFlags QgsLocatorFiltersModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() || index.row() < 0 || index.column() < 0 ||
       index.row() >= rowCount( QModelIndex() ) || index.column() >= columnCount( QModelIndex() ) )
    return QAbstractTableModel::flags( index );

  Qt::ItemFlags flags = QAbstractTableModel::flags( index );
  switch ( index.column() )
  {
    case Name:
    case Prefix:
      break;

    case Active:
    case Default:
      flags = flags | Qt::ItemIsUserCheckable;
      flags = flags | Qt::ItemIsEditable;
      break;
  }

  return flags;
}

QVariant QgsLocatorFiltersModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation == Qt::Horizontal && role == Qt::DisplayRole )
  {
    switch ( section )
    {
      case Name:
        return tr( "Filter" );

      case Prefix:
        return tr( "Prefix" );

      case Active:
        return tr( "Enabled" );

      case Default:
        return tr( "Default" );
    }
  }

  return QVariant();
}

QgsLocatorFilter *QgsLocatorFiltersModel::filterForIndex( const QModelIndex &index ) const
{
  return mLocator->filters().at( index.row() + HIDDEN_FILTER_OFFSET );
}
