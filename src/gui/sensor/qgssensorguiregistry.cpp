/***************************************************************************
                            qgssensorguiregistry.h
                            --------------------------
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

#include "qgssensorguiregistry.h"
#include "moc_qgssensorguiregistry.cpp"
#include "qgssensorwidget.h"

QgsSensorGuiRegistry::QgsSensorGuiRegistry( QObject *parent )
  : QObject( parent )
{
}

QgsSensorGuiRegistry::~QgsSensorGuiRegistry()
{
  qDeleteAll( mMetadata );
}

bool QgsSensorGuiRegistry::populate()
{
  if ( !mMetadata.isEmpty() )
    return false;

  addSensorGuiMetadata( new QgsSensorGuiMetadata( QStringLiteral( "tcp_socket" ), QObject::tr( "TCP socket sensor" ), QgsApplication::getThemeIcon( QStringLiteral( "/mSensor.svg" ) ), [=]( QgsAbstractSensor *sensor ) -> QgsAbstractSensorWidget * {
    QgsTcpSocketSensorWidget *widget = new QgsTcpSocketSensorWidget( nullptr );
    widget->setSensor( sensor );
    return widget; }, nullptr ) );
  addSensorGuiMetadata( new QgsSensorGuiMetadata( QStringLiteral( "udp_socket" ), QObject::tr( "UDP socket sensor" ), QgsApplication::getThemeIcon( QStringLiteral( "/mSensor.svg" ) ), [=]( QgsAbstractSensor *sensor ) -> QgsAbstractSensorWidget * {
    QgsUdpSocketSensorWidget *widget = new QgsUdpSocketSensorWidget( nullptr );
    widget->setSensor( sensor );
    return widget; }, nullptr ) );
#if defined( HAVE_QTSERIALPORT )
  addSensorGuiMetadata( new QgsSensorGuiMetadata( QStringLiteral( "serial_port" ), QObject::tr( "Serial port sensor" ), QgsApplication::getThemeIcon( QStringLiteral( "/mSensor.svg" ) ), [=]( QgsAbstractSensor *sensor ) -> QgsAbstractSensorWidget * {
    QgsSerialPortSensorWidget *widget = new QgsSerialPortSensorWidget( nullptr );
    widget->setSensor( sensor );
    return widget; }, nullptr ) );
#endif
  return true;
}

QgsSensorAbstractGuiMetadata *QgsSensorGuiRegistry::sensorMetadata( const QString &type ) const
{
  return mMetadata.value( type );
}

bool QgsSensorGuiRegistry::addSensorGuiMetadata( QgsSensorAbstractGuiMetadata *metadata )
{
  if ( !metadata || mMetadata.contains( metadata->type() ) )
    return false;

  mMetadata[metadata->type()] = metadata;
  emit sensorAdded( metadata->type(), metadata->visibleName() );
  return true;
}

QgsAbstractSensor *QgsSensorGuiRegistry::createSensor( const QString &type, QObject *parent ) const
{
  if ( !mMetadata.contains( type ) )
    return nullptr;

  std::unique_ptr<QgsAbstractSensor> sensor( mMetadata.value( type )->createSensor( parent ) );
  if ( sensor )
    return sensor.release();

  return QgsApplication::sensorRegistry()->createSensor( type, parent );
}

QgsAbstractSensorWidget *QgsSensorGuiRegistry::createSensorWidget( QgsAbstractSensor *sensor ) const
{
  if ( !sensor || !mMetadata.contains( sensor->type() ) )
    return nullptr;

  return mMetadata[sensor->type()]->createSensorWidget( sensor );
}

QMap<QString, QString> QgsSensorGuiRegistry::sensorTypes() const
{
  QMap<QString, QString> types;
  for ( auto it = mMetadata.constBegin(); it != mMetadata.constEnd(); ++it )
  {
    types.insert( it.key(), it.value()->visibleName() );
  }

  return types;
}
