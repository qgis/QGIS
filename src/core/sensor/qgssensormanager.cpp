/***************************************************************************
    qgssensormanager.cpp
    ------------------
    Date                 : March 2023
    Copyright            : (C) 2023 Mathieu Pellerin
    Email                : mathieu at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssensormanager.h"
#include "moc_qgssensormanager.cpp"

#include "qgsapplication.h"
#include "qgssensorregistry.h"

QgsSensorManager::QgsSensorManager( QObject *parent )
  : QObject( parent )
{
}

QgsSensorManager::~QgsSensorManager()
{
  clear();
}

void QgsSensorManager::clear()
{
  const QList<QgsAbstractSensor *> sensors = mSensors;
  for ( QgsAbstractSensor *sensor : sensors )
  {
    if ( sensor )
    {
      removeSensor( sensor->id() );
    }
  }
  mSensors.clear();
  mSensorsData.clear();
}

QList<QgsAbstractSensor *> QgsSensorManager::sensors() const
{
  return mSensors;
}

QgsAbstractSensor *QgsSensorManager::sensor( const QString &id ) const
{
  for ( QgsAbstractSensor *sensor : mSensors )
  {
    if ( sensor->id() == id )
    {
      return sensor;
    }
  }

  return nullptr;
}

QgsAbstractSensor::SensorData QgsSensorManager::sensorData( const QString &name ) const
{
  return mSensorsData.value( name );
}

QMap<QString, QgsAbstractSensor::SensorData> QgsSensorManager::sensorsData() const
{
  return mSensorsData;
}

QStringList QgsSensorManager::sensorNames() const
{
  QStringList names;
  for ( const QgsAbstractSensor *sensor : std::as_const( mSensors ) )
  {
    names << sensor->name();
  }
  return names;
}

void QgsSensorManager::addSensor( QgsAbstractSensor *sensor )
{
  if ( !sensor || mSensors.contains( sensor ) )
    return;

  connect( sensor, &QgsAbstractSensor::nameChanged, this, &QgsSensorManager::handleSensorNameChanged );
  connect( sensor, &QgsAbstractSensor::statusChanged, this, &QgsSensorManager::handleSensorStatusChanged );
  connect( sensor, &QgsAbstractSensor::dataChanged, this, &QgsSensorManager::captureSensorData );
  connect( sensor, &QgsAbstractSensor::errorOccurred, this, &QgsSensorManager::handleSensorErrorOccurred );
  mSensors << sensor;

  emit sensorAdded( sensor->id() );

  return;
}

bool QgsSensorManager::removeSensor( const QString &id )
{
  for ( QgsAbstractSensor *sensor : mSensors )
  {
    if ( sensor->id() == id )
    {
      emit sensorAboutToBeRemoved( id );
      mSensors.removeAll( sensor );
      mSensorsData.remove( sensor->name() );
      sensor->disconnectSensor();
      sensor->deleteLater();
      emit sensorRemoved( id );
      return true;
    }
  }

  return false;
}

void QgsSensorManager::handleSensorNameChanged()
{
  const QgsAbstractSensor *sensor = qobject_cast<QgsAbstractSensor *>( sender() );
  if ( !sensor )
    return;

  emit sensorNameChanged( sensor->id() );
}

void QgsSensorManager::handleSensorStatusChanged()
{
  const QgsAbstractSensor *sensor = qobject_cast<QgsAbstractSensor *>( sender() );
  if ( !sensor )
    return;

  if ( sensor->status() == Qgis::DeviceConnectionStatus::Disconnected )
  {
    mSensorsData.remove( sensor->name() );
  }

  emit sensorStatusChanged( sensor->id() );
}

void QgsSensorManager::captureSensorData()
{
  const QgsAbstractSensor *sensor = qobject_cast<QgsAbstractSensor *>( sender() );
  if ( !sensor )
    return;

  mSensorsData.insert( sensor->name(), sensor->data() );
  emit sensorDataCaptured( sensor->id() );
}

void QgsSensorManager::handleSensorErrorOccurred( const QString & )
{
  const QgsAbstractSensor *sensor = qobject_cast<QgsAbstractSensor *>( sender() );
  if ( !sensor )
    return;

  emit sensorErrorOccurred( sensor->id() );
}

bool QgsSensorManager::readXml( const QDomElement &element, const QDomDocument &document )
{
  clear();

  QDomElement sensorsElem = element;
  if ( element.tagName() != QLatin1String( "Sensors" ) )
  {
    sensorsElem = element.firstChildElement( QStringLiteral( "Sensors" ) );
  }

  QDomNodeList sensorNodes = element.elementsByTagName( QStringLiteral( "Sensor" ) );
  for ( int i = 0; i < sensorNodes.size(); ++i )
  {
    const QDomElement sensorElement = sensorNodes.at( i ).toElement();
    const QString sensorType = sensorElement.attribute( QStringLiteral( "type" ) );
    QgsAbstractSensor *sensor = QgsApplication::sensorRegistry()->createSensor( sensorType, this );
    if ( !sensor )
    {
      continue;
    }

    sensor->readXml( sensorElement, document );
    addSensor( sensor );
  }

  return true;
}

QDomElement QgsSensorManager::writeXml( QDomDocument &document ) const
{
  QDomElement sensorsElem = document.createElement( QStringLiteral( "Sensors" ) );

  for ( const QgsAbstractSensor *sensor : mSensors )
  {
    sensor->writeXml( sensorsElem, document );
  }

  return sensorsElem;
}
