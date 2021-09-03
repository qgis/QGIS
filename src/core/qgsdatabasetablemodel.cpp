/***************************************************************************
   qgsdatabasetablemodel.cpp
    ------------------------
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
#include "qgsdatabasetablemodel.h"
#include "qgsproviderregistry.h"
#include "qgsprovidermetadata.h"
#include "qgsabstractdatabaseproviderconnection.h"
#include "qgsiconutils.h"
#include <QIcon>

QgsDatabaseTableModel::QgsDatabaseTableModel( const QString &provider, const QString &connection, const QString &schema, QObject *parent )
  : QAbstractItemModel( parent )
  , mSchema( schema )
{
  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( provider );
  Q_ASSERT( metadata );

  mConnection.reset( dynamic_cast<QgsAbstractDatabaseProviderConnection *>( metadata->createConnection( connection ) ) );
  Q_ASSERT( mConnection );
  init();
}

QgsDatabaseTableModel::QgsDatabaseTableModel( QgsAbstractDatabaseProviderConnection *connection, const QString &schema, QObject *parent )
  : QAbstractItemModel( parent )
  , mConnection( connection )
  , mSchema( schema )
{
  Q_ASSERT( mConnection );
  init();
}

void QgsDatabaseTableModel::init()
{
  Q_ASSERT( mConnection->capabilities() & QgsAbstractDatabaseProviderConnection::Capability::Tables );
  mTables = mConnection->tables( mSchema );
}

QModelIndex QgsDatabaseTableModel::parent( const QModelIndex &child ) const
{
  Q_UNUSED( child )
  return QModelIndex();
}


int QgsDatabaseTableModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;

  return mTables.count() + ( mAllowEmpty ? 1 : 0 );
}

int QgsDatabaseTableModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return 1;
}


QVariant QgsDatabaseTableModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  if ( index.row() == 0 && mAllowEmpty )
  {
    if ( role == RoleEmpty )
      return true;

    return QVariant();
  }

  if ( index.row() - ( mAllowEmpty ? 1 : 0 ) >= mTables.count() )
    return QVariant();

  const QgsAbstractDatabaseProviderConnection::TableProperty &table = mTables[ index.row() - ( mAllowEmpty ? 1 : 0 ) ];
  switch ( role )
  {
    case RoleEmpty:
      return false;

    case Qt::DisplayRole:
    case Qt::ToolTipRole:
    case Qt::EditRole:
    {
      return mSchema.isEmpty() && !table.schema().isEmpty() ? QStringLiteral( "%1.%2" ).arg( table.schema(), table.tableName() ) : table.tableName();
    }

    case RoleTableName:
    {
      return table.tableName();
    }

    case Qt::DecorationRole:
    case RoleWkbType:
    case RoleCrs:
    {
      if ( table.geometryColumnTypes().empty() )
      {
        if ( role == Qt::DecorationRole )
          return QgsIconUtils::iconTable();
        else
          return QVariant();
      }

      if ( role == Qt::DecorationRole )
      {
        const QgsWkbTypes::GeometryType geomType = QgsWkbTypes::geometryType( table.geometryColumnTypes().at( 0 ).wkbType );
        switch ( geomType )
        {
          case QgsWkbTypes::PointGeometry:
          {
            return QgsIconUtils::iconPoint();
          }
          case QgsWkbTypes::PolygonGeometry:
          {
            return QgsIconUtils::iconPolygon();
          }
          case QgsWkbTypes::LineGeometry:
          {
            return QgsIconUtils::iconLine();
          }
          case QgsWkbTypes::UnknownGeometry:
          {
            return QgsIconUtils::iconGeometryCollection();
          }
          case QgsWkbTypes::NullGeometry:
            return QgsIconUtils::iconTable();
        }

        return QgsIconUtils::iconTable();
      }
      else if ( role == RoleWkbType )
        return table.geometryColumnTypes().at( 0 ).wkbType;
      else if ( role == RoleCrs )
        return table.geometryColumnTypes().at( 0 ).crs;

      return QVariant();
    }

    case RoleSchema:
      return table.schema();

    case RoleTableFlags:
      return static_cast< int >( table.flags() );

    case RoleComment:
      return table.comment();

    case RoleCustomInfo:
      return table.info();

  }

  return QVariant();
}

QModelIndex QgsDatabaseTableModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( hasIndex( row, column, parent ) )
  {
    return createIndex( row, column, row );
  }

  return QModelIndex();
}

void QgsDatabaseTableModel::setAllowEmptyTable( bool allowEmpty )
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

void QgsDatabaseTableModel::refresh()
{
  const QList< QgsAbstractDatabaseProviderConnection::TableProperty > newTables = mConnection->tables( mSchema );
  const QList< QgsAbstractDatabaseProviderConnection::TableProperty > oldTables = mTables;

  for ( const QgsAbstractDatabaseProviderConnection::TableProperty &oldTable : oldTables )
  {
    if ( !newTables.contains( oldTable ) )
    {
      const int r = mTables.indexOf( oldTable );
      beginRemoveRows( QModelIndex(), r + ( mAllowEmpty ? 1 : 0 ), r + ( mAllowEmpty ? 1 : 0 ) );
      mTables.removeAt( r );
      endRemoveRows();
    }
  }

  for ( const  QgsAbstractDatabaseProviderConnection::TableProperty &newTable : newTables )
  {
    if ( !mTables.contains( newTable ) )
    {
      beginInsertRows( QModelIndex(), mTables.count() + ( mAllowEmpty ? 1 : 0 ), mTables.count() + ( mAllowEmpty ? 1 : 0 ) );
      mTables.append( newTable );
      endInsertRows();
    }
  }
}
