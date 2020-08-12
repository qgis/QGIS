/***************************************************************************
    qgsogrconnpool.cpp
    ---------------------
    begin                : May 2015
    copyright            : (C) 2015 by Sandro Mani
    email                : smani at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsogrconnpool.h"
///@cond PRIVATE

#include "qgslogger.h"

QgsOgrConnPool *QgsOgrConnPool::sInstance = nullptr;

// static public
QgsOgrConnPool *QgsOgrConnPool::instance()
{
  if ( ! sInstance ) sInstance = new QgsOgrConnPool();
  return sInstance;
}

// static public
void QgsOgrConnPool::cleanupInstance()
{
  delete sInstance;
  sInstance = nullptr;
}

QgsOgrConnPool::QgsOgrConnPool() : QgsConnectionPool<QgsOgrConn *, QgsOgrConnPoolGroup>()
{
  QgsDebugCall;
}

QgsOgrConnPool::~QgsOgrConnPool()
{
  QgsDebugCall;
}

///@endcond
