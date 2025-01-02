/***************************************************************************
    qgsspatialiteconnpool.cpp
    ---------------------
    begin                : February 2014
    copyright            : (C) 2014 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QMutex>
#include <QMutexLocker>

#include "qgsspatialiteconnpool.h"
#include "moc_qgsspatialiteconnpool.cpp"

QgsSpatiaLiteConnPool *QgsSpatiaLiteConnPool::sInstance = nullptr;

QgsSpatiaLiteConnPool *QgsSpatiaLiteConnPool::instance()
{
  if ( !sInstance )
  {
    static QMutex sMutex;
    QMutexLocker locker( &sMutex );
    // cppcheck-suppress identicalInnerCondition
    if ( !sInstance )
    {
      sInstance = new QgsSpatiaLiteConnPool();
    }
  }
  return sInstance;
}

// static public
void QgsSpatiaLiteConnPool::cleanupInstance()
{
  delete sInstance;
  sInstance = nullptr;
}
