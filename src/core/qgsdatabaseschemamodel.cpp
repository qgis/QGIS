/***************************************************************************
   qgsdatabaseschemamodel.cpp
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
#include "qgsdatabaseschemamodel.h"
#include "qgsproviderregistry.h"
#include "qgsprovidermetadata.h"
#include "qgsabstractdatabaseproviderconnection.h"

QgsDatabaseSchemaModel::QgsDatabaseSchemaModel( const QString &provider, const QString &connection, QObject *parent )
  : QAbstractItemModel( parent )
{
  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( provider );
  Q_ASSERT( metadata );

  mConnection.reset( dynamic_cast<QgsAbstractDatabaseProviderConnection *>( metadata->createConnection( connection ) ) );
  Q_ASSERT( mConnection );
  init();
}

QgsDatabaseSchemaModel::QgsDatabaseSchemaModel( QgsAbstractDatabaseProviderConnection *connection, QObject *parent )
  : QAbstractItemModel( parent )
  , mConnection( connection )
{
  Q_ASSERT( mConnection );
  init();
}

void QgsDatabaseSchemaModel::init()
{
  Q_ASSERT( mConnection->capabilities() & QgsAbstractDatabaseProviderConnection::Capability::Schemas );
  mSchemas = mConnection->schemas();
}

QModelIndex QgsDatabaseSchemaModel::parent( const QModelIndex &child ) const
{
  Q_UNUSED( child )
  return QModelIndex();
}


int QgsDatabaseSchemaModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;

  return mSchemas.count() + ( mAllowEmpty ? 1 : 0 );
}

int QgsDatabaseSchemaModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return 1;
}


QVariant QgsDatabaseSchemaModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  if ( index.row() == 0 && mAllowEmpty )
  {
    if ( role == RoleEmpty )
      return true;

    return QVariant();
  }

  const QString schemaName = mSchemas.value( index.row() - ( mAllowEmpty ? 1 : 0 ) );
  switch ( role )
  {
    case RoleEmpty:
      return false;

    case Qt::DisplayRole:
    case Qt::EditRole:
    case Qt::ToolTipRole:
    {
      return schemaName;
    }
  }

  return QVariant();
}

QModelIndex QgsDatabaseSchemaModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( hasIndex( row, column, parent ) )
  {
    return createIndex( row, column, row );
  }

  return QModelIndex();
}

void QgsDatabaseSchemaModel::setAllowEmptySchema( bool allowEmpty )
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

void QgsDatabaseSchemaModel::refresh()
{
  const QStringList newSchemas = mConnection->schemas();
  const QStringList oldSchemas = mSchemas;

  for ( const QString &oldSchema : oldSchemas )
  {
    if ( !newSchemas.contains( oldSchema ) )
    {
      const int r = mSchemas.indexOf( oldSchema ) ;
      beginRemoveRows( QModelIndex(), r + ( mAllowEmpty ? 1 : 0 ), r + ( mAllowEmpty ? 1 : 0 ) );
      mSchemas.removeAt( r );
      endRemoveRows();
    }
  }

  for ( const QString &newSchema : newSchemas )
  {
    if ( !mSchemas.contains( newSchema ) )
    {
      beginInsertRows( QModelIndex(), mSchemas.count() + ( mAllowEmpty ? 1 : 0 ), mSchemas.count() + ( mAllowEmpty ? 1 : 0 ) );
      mSchemas.append( newSchema );
      endInsertRows();
    }
  }
}
