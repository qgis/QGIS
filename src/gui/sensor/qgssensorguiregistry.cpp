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

#include "qgssensorwidget.h"

#include "moc_qgssensorguiregistry.cpp"

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

  addSensorGuiMetadata( new QgsSensorGuiMetadata( u"tcp_socket"_s, QObject::tr( "TCP socket sensor" ), QgsApplication::getThemeIcon( u"/mSensor.svg"_s ), []( QgsAbstractSensor *sensor ) -> QgsAbstractSensorWidget * {
    QgsTcpSocketSensorWidget *widget = new QgsTcpSocketSensorWidget( nullptr );
    widget->setSensor( sensor );
    return widget; }, nullptr ) );
  addSensorGuiMetadata( new QgsSensorGuiMetadata( u"udp_socket"_s, QObject::tr( "UDP socket sensor" ), QgsApplication::getThemeIcon( u"/mSensor.svg"_s ), []( QgsAbstractSensor *sensor ) -> QgsAbstractSensorWidget * {
    QgsUdpSocketSensorWidget *widget = new QgsUdpSocketSensorWidget( nullptr );
    widget->setSensor( sensor );
    return widget; }, nullptr ) );
#if defined( HAVE_QTSERIALPORT )
  addSensorGuiMetadata( new QgsSensorGuiMetadata( u"serial_port"_s, QObject::tr( "Serial port sensor" ), QgsApplication::getThemeIcon( u"/mSensor.svg"_s ), []( QgsAbstractSensor *sensor ) -> QgsAbstractSensorWidget * {
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
  auto it = mMetadata.constFind( type );
  if ( it == mMetadata.constEnd() )
    return nullptr;

  std::unique_ptr<QgsAbstractSensor> sensor( it.value()->createSensor( parent ) );
  if ( sensor )
    return sensor.release();

  return QgsApplication::sensorRegistry()->createSensor( type, parent );
}

QgsAbstractSensorWidget *QgsSensorGuiRegistry::createSensorWidget( QgsAbstractSensor *sensor ) const
{
  if ( !sensor )
    return nullptr;

  auto it = mMetadata.constFind( sensor->type() );
  if ( it == mMetadata.constEnd() )
    return nullptr;

  return it.value()->createSensorWidget( sensor );
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
