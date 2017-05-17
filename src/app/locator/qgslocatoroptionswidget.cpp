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
}


//
// QgsLocatorFiltersModel
//

QgsLocatorFiltersModel::QgsLocatorFiltersModel( QgsLocator *locator, QObject *parent )
  : QAbstractTableModel( parent )
  , mLocator( locator )
{

}

int QgsLocatorFiltersModel::rowCount( const QModelIndex & ) const
{
  return mLocator->filters().count();
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
          return mLocator->filters().at( index.row() )->displayName();

        case Prefix:
          return mLocator->filters().at( index.row() )->prefix();

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
          return Qt::Checked;

        case Default:
          return mLocator->filters().at( index.row() )->useWithoutPrefix() ? Qt::Checked : Qt::Unchecked;
      }


  }

  return QVariant();
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
      break;
  }

  return flags;
}
