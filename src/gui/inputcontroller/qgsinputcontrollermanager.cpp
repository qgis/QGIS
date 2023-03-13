/***************************************************************************
    qgsinputcontroller.cpp
    ---------------------
    begin                : March 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsinputcontrollermanager.h"
#include "qgs2dmapcontroller.h"
#include "qgs3dmapcontroller.h"

#include <QRegularExpression>
#include <QRegularExpressionMatch>

//
// QgsInputControllerManager
//

QgsInputControllerManager::QgsInputControllerManager( QObject *parent )
  : QObject( parent )
{

}

QgsInputControllerManager::~QgsInputControllerManager()
{
  qDeleteAll( m2DMapControllers );
  qDeleteAll( m3DMapControllers );
}

bool QgsInputControllerManager::register2DMapController( QgsAbstract2DMapController *controller )
{
  if ( !controller )
    return false;

  if ( m2DMapControllers.contains( controller->deviceId() ) )
  {
    delete controller;
    return false;
  }

  m2DMapControllers.insert( controller->deviceId(), controller );
  return true;
}

bool QgsInputControllerManager::register3DMapController( QgsAbstract3DMapController *controller )
{
  if ( !controller )
    return false;

  if ( m3DMapControllers.contains( controller->deviceId() ) )
  {
    delete controller;
    return false;
  }

  m3DMapControllers.insert( controller->deviceId(), controller );
  return true;
}

QStringList QgsInputControllerManager::available2DMapControllers() const
{
  QStringList devices = m2DMapControllers.keys();
  return devices;
}

QgsAbstract2DMapController *QgsInputControllerManager::create2DMapController( const QString &deviceId ) const
{
  auto it = m2DMapControllers.constFind( deviceId );
  if ( it == m2DMapControllers.constEnd() )
    return nullptr;

  return qgis::down_cast< QgsAbstract2DMapController *>( it.value()->clone() );
}

QStringList QgsInputControllerManager::available3DMapControllers() const
{
  QStringList devices = m3DMapControllers.keys();
  return devices;
}

QgsAbstract3DMapController *QgsInputControllerManager::create3DMapController( const QString &deviceId ) const
{
  auto it = m3DMapControllers.constFind( deviceId );
  if ( it == m3DMapControllers.constEnd() )
    return nullptr;

  return qgis::down_cast< QgsAbstract3DMapController *>( it.value()->clone() );
}

