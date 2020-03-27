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

#include <QDir>
#include <QFileInfo>

#include "odbc/Connection.h"
#include "odbc/Environment.h"

using namespace odbc;

QgsHanaDriver *QgsHanaDriver::sInstance = nullptr;

static QString detectDriverPath( EnvironmentRef &env, const QString &libName, const QString &defaultPath )
{
  QString path = defaultPath + QDir::separator() + libName;
  if ( QFileInfo::exists( path ) )
    return path;

  std::vector<DriverInformation> drivers = env->getDrivers();
  for ( const DriverInformation &drv : drivers )
  {
    for ( const DriverInformation::Attribute &attr : drv.attributes )
    {
      if ( QString::compare( QString::fromStdString( attr.name ), "DRIVER", Qt::CaseInsensitive ) == 0 )
      {
        QString path = QString::fromStdString( attr.value );
        if ( path.endsWith( libName ) && QFileInfo::exists( path ) )
          return path;
      }
    }
  }
  return QString();
}

QgsHanaDriver::QgsHanaDriver()
  : mEnv( Environment::create() )
{
  QgsDebugCall;
#if defined(Q_OS_WIN)
#if defined(Q_OS_WIN64)
  mDriver = mEnv->isDriverInstalled( "HDBODBC" ) ? "HDBODBC" : "";
#else
  mDriver = mEnv->isDriverInstalled( "HDBODBC32" ) ? "HDBODBC32" : "";
#endif
#elif defined(Q_OS_MAC)
  mDriver = detectDriverPath( mEnv, "libodbcHDB.dylib", "/Applications/sap/hdbclient" );
#else
  mDriver = detectDriverPath( mEnv, "libodbcHDB.so", "/usr/sap/hdbclient" );
#endif
}

QgsHanaDriver::~QgsHanaDriver()
{
  QgsDebugCall;
}

QgsHanaDriver *QgsHanaDriver::instance()
{
  if ( !sInstance )
    sInstance = new QgsHanaDriver();
  return sInstance;
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

QString QgsHanaDriver::getDriver() const
{
  return mDriver;
}
