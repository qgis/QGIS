/***************************************************************************
   qgshanadriver.cpp
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maxim Rylov
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
#include <QLibrary>

#include "odbc/Connection.h"
#include "odbc/Environment.h"

using namespace NS_ODBC;

static QString detectDriverPath( EnvironmentRef &env, const QString &libName, const QString &defaultPath )
{
  QString path = defaultPath + QDir::separator() + libName;
  if ( QFileInfo::exists( path ) )
    return path;

  const std::vector<DriverInformation> drivers = env->getDrivers();
  for ( const DriverInformation &drv : drivers )
  {
    for ( const DriverInformation::Attribute &attr : drv.attributes )
    {
      if ( QString::compare( QString::fromStdString( attr.name ), QLatin1String( "DRIVER" ), Qt::CaseInsensitive ) == 0 )
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
  mDriver = mEnv->isDriverInstalled( "HDBODBC" ) ? QStringLiteral( "HDBODBC" ) : QString();
#else
  mDriver = mEnv->isDriverInstalled( "HDBODBC32" ) ? QStringLiteral( "HDBODBC32" ) : QString();
#endif
#elif defined(Q_OS_MAC)
  mDriver = detectDriverPath( mEnv, QStringLiteral( "libodbcHDB.dylib" ), QStringLiteral( "/Applications/sap/hdbclient" ) );
#else
  mDriver = detectDriverPath( mEnv, QStringLiteral( "libodbcHDB.so" ), QStringLiteral( "/usr/sap/hdbclient" ) );
#endif
}

QgsHanaDriver::~QgsHanaDriver()
{
  QgsDebugCall;
}

ConnectionRef QgsHanaDriver::createConnection()
{
  return mEnv->createConnection();
}

const QString &QgsHanaDriver::driver() const
{
  return mDriver;
}

QgsHanaDriver *QgsHanaDriver::instance()
{
  static QgsHanaDriver instance;
  return &instance;
}

bool QgsHanaDriver::isInstalled( const QString &name )
{
  EnvironmentRef env = Environment::create();
  return env->isDriverInstalled( name.toStdString().c_str() );
}

bool QgsHanaDriver::isValidPath( const QString &path )
{
  if ( !QLibrary::isLibrary( path ) )
    return false;

  QLibrary lib( path );
  if ( !lib.load() )
    return false;
  const bool ret = lib.resolve( "SQLConnect" ) != nullptr;
  lib.unload();
  return ret;
}
