/***************************************************************************
   qgsredshiftconnpool.cpp
   --------------------------------------
   Date      : 16.02.2021
   Copyright : (C) 2021 Amazon Inc. or its affiliates
   Author    : Marcel Bezdrighin
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#include "qgsredshiftconnpool.h"

#include "qgslogger.h"
#include "qgsredshiftconn.h"

QgsRedshiftConnPool *QgsRedshiftConnPool::sInstance = nullptr;

QgsRedshiftConnPool *QgsRedshiftConnPool::instance()
{
  if ( !sInstance )
    sInstance = new QgsRedshiftConnPool();
  return sInstance;
}

void QgsRedshiftConnPool::cleanupInstance()
{
  delete sInstance;
  sInstance = nullptr;
}

QgsRedshiftConnPool::QgsRedshiftConnPool() : QgsConnectionPool<QgsRedshiftConn *, QgsRedshiftConnPoolGroup>()
{
  QgsDebugCall;
}

QgsRedshiftConnPool::~QgsRedshiftConnPool()
{
  QgsDebugCall;
}
