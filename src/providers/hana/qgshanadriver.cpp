/***************************************************************************
   qgshanadriver.cpp
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
#include "qgshanadriver.h"
#include "qgslogger.h"

#include "odbc/Connection.h"
#include "odbc/Environment.h"

using namespace odbc;

QgsHanaDriver *QgsHanaDriver::sInstance = nullptr;

QgsHanaDriver::QgsHanaDriver()
{
  QgsDebugCall;
  mEnv = Environment::create();
#ifdef Q_OS_WIN
#ifdef Q_OS_WIN64
  mIsInstalled = mEnv->isDriverInstalled( "HDBODBC" );
#else
  mIsInstalled = mEnv->isDriverInstalled( "HDBODBC32" );
#endif
#else
  mIsInstalled = true;
#endif
}

QgsHanaDriver *QgsHanaDriver::instance()
{
  if ( !sInstance )
    sInstance = new QgsHanaDriver();
  return sInstance;
}

QgsHanaDriver::~QgsHanaDriver()
{
  QgsDebugCall;
}

void QgsHanaDriver::cleanupInstance()
{
  delete sInstance;
  sInstance = nullptr;
}

ConnectionRef QgsHanaDriver::createConnection()
{
  return mEnv->createConnection();
}
