/***************************************************************************
   qgsproviderconnectionmodel.cpp
    --------------------------------------
   Date                 : March 2020
   Copyright            : (C) 2020 Nyall Dawson
   Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
#include "qgsproviderconnectionmodel.h"
#include "qgsproviderregistry.h"
#include "qgsprovidermetadata.h"

QgsProviderConnectionModel::QgsProviderConnectionModel( const QString &provider, QObject *parent )
  : QAbstractItemModel( parent )
  , mProvider( provider )
  , mMetadata( QgsProviderRegistry::instance()->providerMetadata( provider ) )
{
  Q_ASSERT( mMetadata );

  connect( mMetadata, &QgsProviderMetadata::connectionCreated, this, &QgsProviderConnectionModel::addConnection );
  connect( mMetadata, &QgsProviderMetadata::connectionDeleted, this, &QgsProviderConnectionModel::removeConnection );

  mConnections = mMetadata->connections().keys();
}

void QgsProviderConnectionModel::removeConnection( const QString &connection )
{
  int index = mConnections.indexOf( connection );
  if ( index < 0 )
    return;

  beginRemoveRows( QModelIndex(), index, index );
  mConnections.removeAt( index );
  endRemoveRows();
}

void QgsProviderConnectionModel::addConnection( const QString &connection )
{
  beginInsertRows( QModelIndex(), mConnections.count(), mConnections.count() );
  mConnections.append( connection );
  endInsertRows();
}

QModelIndex QgsProviderConnectionModel::parent( const QModelIndex &child ) const
{
  Q_UNUSED( child )
  return QModelIndex();
}


int QgsProviderConnectionModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;

  return mConnections.count();
}

int QgsProviderConnectionModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return 1;
}


QVariant QgsProviderConnectionModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  const QString connectionName = mConnections.value( index.row() );
  switch ( role )
  {
    case Qt::DisplayRole:
    case RoleConnectionName:
    {
      return connectionName;
    }

    case Qt::DecorationRole:
      if ( const QgsAbstractProviderConnection *connection =  mMetadata->findConnection( connectionName ) )
      {
        return connection->icon();
      }
      else
      {
        return QIcon();
      }

    case Qt::ToolTipRole:
    case RoleUri:
    {
      if ( const QgsAbstractProviderConnection *connection =  mMetadata->findConnection( connectionName ) )
      {
        return connection->uri();
      }
      else
      {
        return QString();
      }
    }

    case RoleConfiguration:
    {
      if ( const QgsAbstractProviderConnection *connection =  mMetadata->findConnection( connectionName ) )
      {
        return connection->configuration();
      }
      else
      {
        return QVariant();
      }
    }

  }

  return QVariant();
}

QModelIndex QgsProviderConnectionModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( hasIndex( row, column, parent ) )
  {
    return createIndex( row, column, row );
  }

  return QModelIndex();
}
