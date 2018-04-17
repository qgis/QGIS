/***************************************************************************
  qgsopenclutils.cpp - QgsOpenClUtils

 ---------------------
 begin                : 11.4.2018
 copyright            : (C) 2018 by elpaso
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsopenclutils.h"
#include "qgssettings.h"
#include "qgsmessagelog.h"
#include "qgslogger.h"

#include <QTextStream>
#include <QFile>
#include <QDebug>

QLatin1String QgsOpenClUtils::SETTINGS_KEY = QLatin1Literal( "OpenClEnabled" );
QLatin1String QgsOpenClUtils::LOGMESSAGE_TAG = QLatin1Literal( "OpenCL" );
bool QgsOpenClUtils::sAvailable = false;


void QgsOpenClUtils::init()
{
  static bool initialized = false;
  if ( ! initialized )
  {
    std::vector<cl::Platform> platforms;
    cl::Platform::get( &platforms );
    cl::Platform plat;
    cl::Device dev;
    for ( auto &p : platforms )
    {
      std::string platver = p.getInfo<CL_PLATFORM_VERSION>();
      QgsDebugMsg( QStringLiteral( "Found OpenCL platform %1: %2" ).arg( QString::fromStdString( platver ), QString::fromStdString( p.getInfo<CL_PLATFORM_NAME>() ) ) );
      if ( platver.find( "OpenCL 1." ) != std::string::npos )
      {
        std::vector<cl::Device> devices;
        // Check for a GPU device
        p.getDevices( CL_DEVICE_TYPE_GPU, &devices );
        if ( devices.size() > 0 )
        {
          // Got one!
          plat = p;
          dev = devices[0];
          break;
        }
      }
    }
    if ( plat() == 0 )
    {
      QgsMessageLog::logMessage( QObject::tr( "No OpenCL 1.x platform with GPU found." ), LOGMESSAGE_TAG, Qgis::Warning );
      sAvailable = false;
    }
    else
    {
      cl::Platform newP = cl::Platform::setDefault( plat );
      if ( newP != plat )
      {
        QgsMessageLog::logMessage( QObject::tr( "Error setting default platform." ), LOGMESSAGE_TAG, Qgis::Warning );
        sAvailable = false;
      }
      else
      {
        cl::Device::setDefault( dev );
        QgsDebugMsg( QStringLiteral( "Found OpenCL device %1" ).arg( QString::fromStdString( dev.getInfo<CL_DEVICE_NAME>() ) ) );
        sAvailable = true;
      }
    }
    initialized = true;
  }
}


bool QgsOpenClUtils::enabled()
{
  return QgsSettings().value( SETTINGS_KEY, true, QgsSettings::Section::Core ).toBool();
}

bool QgsOpenClUtils::available()
{
  init();
  return sAvailable;
}

void QgsOpenClUtils::setEnabled( bool enabled )
{
  QgsSettings().setValue( SETTINGS_KEY, enabled, QgsSettings::Section::Core );
}

QString QgsOpenClUtils::sourceFromPath( const QString &path )
{
  // TODO: check for compatibility with current platform ( cl_khr_fp64 )
  // Try to load the program sources
  QString source_str;
  QFile file( path );
  if ( file.open( QFile::ReadOnly | QFile::Text ) )
  {
    QTextStream in( &file );
    source_str = in.readAll();
    file.close();
  }
  else
  {
    QgsMessageLog::logMessage( QObject::tr( "Could not load OpenCL program from path %1." ).arg( path ), LOGMESSAGE_TAG, Qgis::Warning );
  }
  return source_str;
}

QString QgsOpenClUtils::buildLog( cl::BuildError &e )
{
  cl::BuildLogType build_logs = e.getBuildLog();
  QString build_log;
  if ( build_logs.size() > 0 )
    build_log = QString::fromStdString( build_logs[0].second );
  return build_log;
}
