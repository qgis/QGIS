/***************************************************************************
                             qgsabstractsensor.cpp
                             ---------------------------
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

#include "qgsabstractsensor.h"

#include <QUuid>

#include "moc_qgsabstractsensor.cpp"

QgsAbstractSensor::QgsAbstractSensor( QObject *parent )
  : QObject( parent )
  , mId( QUuid::createUuid().toString() )
{
}

QString QgsAbstractSensor::name() const
{
  return mName;
}

void QgsAbstractSensor::setName( const QString &name )
{
  if ( mName == name )
    return;

  mName = name;
  emit nameChanged();
}

Qgis::DeviceConnectionStatus QgsAbstractSensor::status() const
{
  return mStatus;
}

void QgsAbstractSensor::setStatus( Qgis::DeviceConnectionStatus status )
{
  if ( mStatus == status )
    return;

  mStatus = status;
  emit statusChanged();
}

QgsAbstractSensor::SensorData QgsAbstractSensor::data() const
{
  return mData;
}

void QgsAbstractSensor::setData( const QgsAbstractSensor::SensorData &data )
{
  mData = data;
  emit dataChanged();
}

QString QgsAbstractSensor::errorString() const
{
  return mErrorString;
}

void QgsAbstractSensor::connectSensor()
{
  setStatus( Qgis::DeviceConnectionStatus::Connecting );
  handleConnect();
}

void QgsAbstractSensor::disconnectSensor()
{
  handleDisconnect();
  setStatus( Qgis::DeviceConnectionStatus::Disconnected );
}

bool QgsAbstractSensor::writePropertiesToElement( QDomElement &, QDomDocument & ) const
{
  return true;
}

bool QgsAbstractSensor::readPropertiesFromElement( const QDomElement &, const QDomDocument & )
{
  return true;
}

bool QgsAbstractSensor::writeXml( QDomElement &parentElement, QDomDocument &document ) const
{
  QDomElement element = document.createElement( u"Sensor"_s );
  element.setAttribute( u"id"_s, id() );
  element.setAttribute( u"type"_s, type() );
  element.setAttribute( u"name"_s, name() );

  writePropertiesToElement( element, document );
  parentElement.appendChild( element );

  return true;
}

bool QgsAbstractSensor::readXml( const QDomElement &element, const QDomDocument &document )
{
  if ( element.nodeName() != "Sensor"_L1 )
  {
    return false;
  }

  mId = element.attribute( u"id"_s, QUuid::createUuid().toString() );
  mName = element.attribute( u"name"_s );
  readPropertiesFromElement( element, document );

  return true;
}
