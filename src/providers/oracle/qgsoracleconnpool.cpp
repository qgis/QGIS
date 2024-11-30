/***************************************************************************
    qgsoracleconnpool.cpp
    ---------------------
    begin                : November 2015
    copyright            : (C) 2015 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsoracleconnpool.h"
#include "moc_qgsoracleconnpool.cpp"
#include "qgsoracleconn.h"
#include "qgslogger.h"

QgsOracleConnPool *QgsOracleConnPool::sInstance = nullptr;

QgsOracleConnPool *QgsOracleConnPool::instance()
{
  if ( !sInstance )
    sInstance = new QgsOracleConnPool();
  return sInstance;
}

void QgsOracleConnPool::cleanupInstance()
{
  delete sInstance;
  sInstance = nullptr;
}

QgsOracleConnPool::QgsOracleConnPool()
  : QgsConnectionPool<QgsOracleConn *, QgsOracleConnPoolGroup>()
{
  QgsDebugCall;
}

QgsOracleConnPool::~QgsOracleConnPool()
{
  QgsDebugCall;
}
