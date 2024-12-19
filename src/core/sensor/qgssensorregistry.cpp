/***************************************************************************
                            qgssensorregistry.cpp
                            ------------------------
    begin                : March 2023
    copyright            : (C) 2023 by Mathieu Pellerin
    email                : mathieu at opengis dot ch
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsconfig.h"

#include "qgssensorregistry.h"
#include "moc_qgssensorregistry.cpp"
#include "qgsiodevicesensor.h"
#include "qgsproject.h"
#include "qgssensormanager.h"

QgsSensorRegistry::QgsSensorRegistry( QObject *parent )
  : QObject( parent )
{
}

QgsSensorRegistry::~QgsSensorRegistry()
{
  qDeleteAll( mMetadata );
}

bool QgsSensorRegistry::populate()
{
  if ( !mMetadata.isEmpty() )
    return false;

  addSensorType( new QgsSensorMetadata( QLatin1String( "tcp_socket" ), QObject::tr( "TCP socket sensor" ), QgsTcpSocketSensor::create ) );
  addSensorType( new QgsSensorMetadata( QLatin1String( "udp_socket" ), QObject::tr( "UDP socket sensor" ), QgsUdpSocketSensor::create ) );
#if defined( HAVE_QTSERIALPORT )
  addSensorType( new QgsSensorMetadata( QLatin1String( "serial_port" ), QObject::tr( "Serial port sensor" ), QgsSerialPortSensor::create ) );
#endif

  return true;
}

QgsSensorAbstractMetadata *QgsSensorRegistry::sensorMetadata( const QString &type ) const
{
  return mMetadata.value( type );
}

bool QgsSensorRegistry::addSensorType( QgsSensorAbstractMetadata *metadata )
{
  if ( !metadata || mMetadata.contains( metadata->type() ) )
    return false;

  mMetadata[metadata->type()] = metadata;
  emit sensorAdded( metadata->type(), metadata->visibleName() );
  return true;
}

bool QgsSensorRegistry::removeSensorType( const QString &type )
{
  if ( !mMetadata.contains( type ) )
    return false;

  // remove any sensor of this type in the project sensor manager
  const QList<QgsAbstractSensor *> sensors = QgsProject::instance()->sensorManager()->sensors(); // skip-keyword-check
  for ( QgsAbstractSensor *sensor : sensors )
  {
    if ( sensor->type() == type )
    {
      QgsProject::instance()->sensorManager()->removeSensor( sensor->id() ); // skip-keyword-check
    }
  }

  delete mMetadata.take( type );
  return true;
}

QgsAbstractSensor *QgsSensorRegistry::createSensor( const QString &type, QObject *parent ) const
{
  if ( !mMetadata.contains( type ) )
    return nullptr;

  return mMetadata[type]->createSensor( parent );
}

QMap<QString, QString> QgsSensorRegistry::sensorTypes() const
{
  QMap<QString, QString> types;
  for ( auto it = mMetadata.constBegin(); it != mMetadata.constEnd(); ++it )
  {
    types.insert( it.key(), it.value()->visibleName() );
  }

  return types;
}
