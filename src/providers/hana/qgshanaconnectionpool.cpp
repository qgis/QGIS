/***************************************************************************
   qgshanaconnectionpool.cpp
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maksim Rylov
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#include "qgshanaconnection.h"
#include "qgshanaconnectionpool.h"

QgsHanaConnectionPoolGroup::QgsHanaConnectionPoolGroup(const QString &name)
  : QgsConnectionPoolGroup<QgsHanaConnection*>(name)
{
  initTimer(this);
}

QgsHanaConnectionPool* QgsHanaConnectionPool::sInstance = nullptr;

QgsHanaConnectionPool* QgsHanaConnectionPool::instance()
{
  if ( !sInstance )
    sInstance = new QgsHanaConnectionPool();
  return sInstance;
}

bool QgsHanaConnectionPool::hasInstance()
{
  return sInstance != nullptr;
}

void QgsHanaConnectionPool::cleanupInstance()
{
  delete sInstance;
  sInstance = nullptr;
}

QgsHanaConnectionPool::QgsHanaConnectionPool()
  : QgsConnectionPool<QgsHanaConnection*, QgsHanaConnectionPoolGroup>()
{
  QgsDebugCall;
}

QgsHanaConnectionPool::~QgsHanaConnectionPool()
{
  QgsDebugCall;
}
