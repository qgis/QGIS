/***************************************************************************
    qgssensormodel.cpp
    ---------------
    begin                : March 2023
    copyright            : (C) 2023 by Mathieu pellerin
    email                : mathieu at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssensormodel.h"
#include "moc_qgssensormodel.cpp"

#include "qgis.h"
#include "qgssensormanager.h"
#include "qgsabstractsensor.h"


QgsSensorModel::QgsSensorModel( QgsSensorManager *manager, QObject *parent )
  : QAbstractItemModel( parent )
  , mSensorManager( manager )
{
  connect( mSensorManager, &QgsSensorManager::sensorAdded, this, &QgsSensorModel::sensorAdded );
  connect( mSensorManager, &QgsSensorManager::sensorAboutToBeRemoved, this, &QgsSensorModel::sensorRemoved );

  connect( mSensorManager, &QgsSensorManager::sensorNameChanged, this, &QgsSensorModel::sensorNameChanged );
  connect( mSensorManager, &QgsSensorManager::sensorStatusChanged, this, &QgsSensorModel::sensorStatusChanged );
  connect( mSensorManager, &QgsSensorManager::sensorDataCaptured, this, &QgsSensorModel::sensorDataCaptured );

  beginResetModel();
  const QList<QgsAbstractSensor *> sensors = manager->sensors();
  for ( const QgsAbstractSensor *sensor : sensors )
  {
    if ( !mSensorIds.contains( sensor->id() ) )
    {
      mSensorIds << sensor->id();
    }
  }
  endResetModel();
}

QVariant QgsSensorModel::data( const QModelIndex &index, int role ) const
{
  if ( index.row() < 0 || index.row() >= rowCount( QModelIndex() ) )
    return QVariant();

  QgsAbstractSensor *sensor = mSensorManager->sensor( mSensorIds[index.row()] );
  if ( !sensor )
    return QVariant();

  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
    case Qt::EditRole:
    {
      switch ( index.column() )
      {
        case static_cast<int>( Column::Name ):
        {
          return sensor->name();
        }

        case static_cast<int>( Column::LastValue ):
        {
          switch ( sensor->status() )
          {
            case Qgis::DeviceConnectionStatus::Disconnected:
              return tr( "Disconnected" );

            case Qgis::DeviceConnectionStatus::Connecting:
              return tr( "Connecting" );

            case Qgis::DeviceConnectionStatus::Connected:
              return sensor->data().lastValue.toString();
          }
        }

        default:
          break;
      }

      return QVariant();
    }

    case static_cast< int >( CustomRole::SensorType ):
    {
      return sensor->type();
    }

    case static_cast< int >( CustomRole::SensorId ):
    {
      return sensor->id();
    }

    case static_cast< int >( CustomRole::SensorName ):
    {
      return sensor->name();
    }

    case static_cast< int >( CustomRole::SensorStatus ):
    {
      return QVariant::fromValue<Qgis::DeviceConnectionStatus>( sensor->status() );
    }

    case static_cast< int >( CustomRole::SensorLastValue ):
    {
      return sensor->data().lastValue;
    }

    case static_cast< int >( CustomRole::SensorLastTimestamp ):
    {
      return sensor->data().lastTimestamp;
    }

    case static_cast< int >( CustomRole::Sensor ):
    {
      return QVariant::fromValue<QgsAbstractSensor *>( sensor );
    }

    default:
      return QVariant();
  }
  BUILTIN_UNREACHABLE
}

bool QgsSensorModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( index.row() < 0 || index.row() >= rowCount( QModelIndex() ) || role != Qt::EditRole )
    return false;

  QgsAbstractSensor *sensor = mSensorManager->sensor( mSensorIds[index.row()] );
  if ( !sensor )
    return false;

  switch ( index.column() )
  {
    case static_cast<int>( Column::Name ):
    {
      sensor->setName( value.toString() );
      return true;
    }

    default:
      break;
  }

  return false;
}

Qt::ItemFlags QgsSensorModel::flags( const QModelIndex &index ) const
{
  Qt::ItemFlags flags = QAbstractItemModel::flags( index );
  if ( index.isValid() && index.column() == static_cast<int>( Column::Name ) )
  {
    QgsAbstractSensor *sensor = mSensorManager->sensor( mSensorIds[index.row()] );
    if ( sensor )
    {
      return flags | Qt::ItemIsEditable;
    }
  }
  return flags;
}

QVariant QgsSensorModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( role == Qt::DisplayRole )
  {
    if ( orientation == Qt::Vertical ) //row
    {
      return QVariant( section + 1 );
    }
    else
    {
      switch ( section )
      {
        case static_cast<int>( Column::Name ):
          return QVariant( tr( "Name" ) );

        case static_cast<int>( Column::LastValue ):
          return QVariant( tr( "Last Value" ) );

        default:
          return QVariant();
      }
    }
  }
  else
  {
    return QVariant();
  }
}

QModelIndex QgsSensorModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( !hasIndex( row, column, parent ) )
    return QModelIndex();

  if ( !parent.isValid() )
  {
    return createIndex( row, column );
  }

  return QModelIndex();
}

QModelIndex QgsSensorModel::parent( const QModelIndex & ) const
{
  return QModelIndex();
}

int QgsSensorModel::rowCount( const QModelIndex &parent ) const
{
  if ( !parent.isValid() )
  {
    return mSensorIds.size();
  }
  return 0;
}

int QgsSensorModel::columnCount( const QModelIndex & ) const
{
  return 2;
}

void QgsSensorModel::sensorAdded( const QString &id )
{
  beginInsertRows( QModelIndex(), mSensorIds.size(), mSensorIds.size() );
  mSensorIds << id;
  endInsertRows();
}

void QgsSensorModel::sensorRemoved( const QString &id )
{
  const int sensorIndex = mSensorIds.indexOf( id );
  if ( sensorIndex < 0 )
    return;

  beginRemoveRows( QModelIndex(), sensorIndex, sensorIndex );
  mSensorIds.removeAt( sensorIndex );
  endRemoveRows();
}

void QgsSensorModel::sensorNameChanged( const QString &id )
{
  const int sensorIndex = mSensorIds.indexOf( id );
  if ( sensorIndex < 0 )
    return;

  emit dataChanged( index( sensorIndex, static_cast<int>( Column::Name ) ), index( sensorIndex, static_cast<int>( Column::Name ) ), QVector< int >() << Qt::DisplayRole << static_cast< int >( CustomRole::SensorName ) );
}

void QgsSensorModel::sensorStatusChanged( const QString &id )
{
  const int sensorIndex = mSensorIds.indexOf( id );
  if ( sensorIndex < 0 )
    return;

  emit dataChanged( index( sensorIndex, static_cast<int>( Column::LastValue ) ), index( sensorIndex, static_cast<int>( Column::LastValue ) ), QVector< int >() << static_cast< int >( CustomRole::SensorStatus ) );
}

void QgsSensorModel::sensorDataCaptured( const QString &id )
{
  const int sensorIndex = mSensorIds.indexOf( id );
  if ( sensorIndex < 0 )
    return;

  emit dataChanged( index( sensorIndex, static_cast<int>( Column::LastValue ) ), index( sensorIndex, static_cast<int>( Column::LastValue ) ), QVector< int >() << static_cast< int >( CustomRole::SensorLastValue ) << static_cast< int >( CustomRole::SensorLastTimestamp ) );
}
