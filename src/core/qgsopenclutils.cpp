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
cl::Platform QgsOpenClUtils::sPlatform = cl::Platform();
cl::Device QgsOpenClUtils::sDevice = cl::Device();
QString QgsOpenClUtils::sSourcePath = QString();

void QgsOpenClUtils::init()
{
  static bool initialized = false;
  if ( ! initialized )
  {
    try
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
          try
          {
            p.getDevices( CL_DEVICE_TYPE_GPU, &devices );
          }
          catch ( cl::Error &e )
          {
            QgsDebugMsgLevel( QStringLiteral( "Error %1 on platform %3 searching for OpenCL device: %2" )
                              .arg( errorText( e.err() ),
                                    QString::fromStdString( e.what() ),
                                    QString::fromStdString( p.getInfo<CL_PLATFORM_NAME>() ) ), 2 );
          }
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
          QgsDebugMsgLevel( QStringLiteral( "Found OpenCL device %1" )
                            .arg( QString::fromStdString( dev.getInfo<CL_DEVICE_NAME>() ) ), 2 );
          sAvailable = true;
          sDevice = dev;
          sPlatform = plat;
        }
      }
    }
    catch ( cl::Error &e )
    {
      QgsMessageLog::logMessage( QObject::tr( "Error %1 searching for OpenCL device: %2" )
                                 .arg( errorText( e.err() ), QString::fromStdString( e.what() ) ),
                                 LOGMESSAGE_TAG, Qgis::Critical );
      sAvailable = false;
    }
    initialized = true;
  }
}

QString QgsOpenClUtils::sourcePath()
{
  return sSourcePath.isEmpty() ? QStringLiteral( "./" ) : sSourcePath;
}

void QgsOpenClUtils::setSourcePath( const QString &value )
{
  sSourcePath = value;
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

QString QgsOpenClUtils::sourceFromBaseName( const QString &baseName )
{
  QString path = QStringLiteral( "%1/%2.cl" ).arg( sourcePath(), baseName );
  return sourceFromPath( path );
}

QString QgsOpenClUtils::buildLog( cl::BuildError &e )
{
  cl::BuildLogType build_logs = e.getBuildLog();
  QString build_log;
  if ( build_logs.size() > 0 )
    build_log = QString::fromStdString( build_logs[0].second );
  return build_log;
}

QString QgsOpenClUtils::errorText( const int errorCode )
{
  switch ( errorCode )
  {
    case 0: return QStringLiteral( "CL_SUCCESS" );
    case -1: return QStringLiteral( "CL_DEVICE_NOT_FOUND" );
    case -2: return QStringLiteral( "CL_DEVICE_NOT_AVAILABLE" );
    case -3: return QStringLiteral( "CL_COMPILER_NOT_AVAILABLE" );
    case -4: return QStringLiteral( "CL_MEM_OBJECT_ALLOCATION_FAILURE" );
    case -5: return QStringLiteral( "CL_OUT_OF_RESOURCES" );
    case -6: return QStringLiteral( "CL_OUT_OF_HOST_MEMORY" );
    case -7: return QStringLiteral( "CL_PROFILING_INFO_NOT_AVAILABLE" );
    case -8: return QStringLiteral( "CL_MEM_COPY_OVERLAP" );
    case -9: return QStringLiteral( "CL_IMAGE_FORMAT_MISMATCH" );
    case -10: return QStringLiteral( "CL_IMAGE_FORMAT_NOT_SUPPORTED" );
    case -12: return QStringLiteral( "CL_MAP_FAILURE" );
    case -13: return QStringLiteral( "CL_MISALIGNED_SUB_BUFFER_OFFSET" );
    case -14: return QStringLiteral( "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST" );
    case -15: return QStringLiteral( "CL_COMPILE_PROGRAM_FAILURE" );
    case -16: return QStringLiteral( "CL_LINKER_NOT_AVAILABLE" );
    case -17: return QStringLiteral( "CL_LINK_PROGRAM_FAILURE" );
    case -18: return QStringLiteral( "CL_DEVICE_PARTITION_FAILED" );
    case -19: return QStringLiteral( "CL_KERNEL_ARG_INFO_NOT_AVAILABLE" );
    case -30: return QStringLiteral( "CL_INVALID_VALUE" );
    case -31: return QStringLiteral( "CL_INVALID_DEVICE_TYPE" );
    case -32: return QStringLiteral( "CL_INVALID_PLATFORM" );
    case -33: return QStringLiteral( "CL_INVALID_DEVICE" );
    case -34: return QStringLiteral( "CL_INVALID_CONTEXT" );
    case -35: return QStringLiteral( "CL_INVALID_QUEUE_PROPERTIES" );
    case -36: return QStringLiteral( "CL_INVALID_COMMAND_QUEUE" );
    case -37: return QStringLiteral( "CL_INVALID_HOST_PTR" );
    case -38: return QStringLiteral( "CL_INVALID_MEM_OBJECT" );
    case -39: return QStringLiteral( "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR" );
    case -40: return QStringLiteral( "CL_INVALID_IMAGE_SIZE" );
    case -41: return QStringLiteral( "CL_INVALID_SAMPLER" );
    case -42: return QStringLiteral( "CL_INVALID_BINARY" );
    case -43: return QStringLiteral( "CL_INVALID_BUILD_OPTIONS" );
    case -44: return QStringLiteral( "CL_INVALID_PROGRAM" );
    case -45: return QStringLiteral( "CL_INVALID_PROGRAM_EXECUTABLE" );
    case -46: return QStringLiteral( "CL_INVALID_KERNEL_NAME" );
    case -47: return QStringLiteral( "CL_INVALID_KERNEL_DEFINITION" );
    case -48: return QStringLiteral( "CL_INVALID_KERNEL" );
    case -49: return QStringLiteral( "CL_INVALID_ARG_INDEX" );
    case -50: return QStringLiteral( "CL_INVALID_ARG_VALUE" );
    case -51: return QStringLiteral( "CL_INVALID_ARG_SIZE" );
    case -52: return QStringLiteral( "CL_INVALID_KERNEL_ARGS" );
    case -53: return QStringLiteral( "CL_INVALID_WORK_DIMENSION" );
    case -54: return QStringLiteral( "CL_INVALID_WORK_GROUP_SIZE" );
    case -55: return QStringLiteral( "CL_INVALID_WORK_ITEM_SIZE" );
    case -56: return QStringLiteral( "CL_INVALID_GLOBAL_OFFSET" );
    case -57: return QStringLiteral( "CL_INVALID_EVENT_WAIT_LIST" );
    case -58: return QStringLiteral( "CL_INVALID_EVENT" );
    case -59: return QStringLiteral( "CL_INVALID_OPERATION" );
    case -60: return QStringLiteral( "CL_INVALID_GL_OBJECT" );
    case -61: return QStringLiteral( "CL_INVALID_BUFFER_SIZE" );
    case -62: return QStringLiteral( "CL_INVALID_MIP_LEVEL" );
    case -63: return QStringLiteral( "CL_INVALID_GLOBAL_WORK_SIZE" );
    case -64: return QStringLiteral( "CL_INVALID_PROPERTY" );
    case -65: return QStringLiteral( "CL_INVALID_IMAGE_DESCRIPTOR" );
    case -66: return QStringLiteral( "CL_INVALID_COMPILER_OPTIONS" );
    case -67: return QStringLiteral( "CL_INVALID_LINKER_OPTIONS" );
    case -68: return QStringLiteral( "CL_INVALID_DEVICE_PARTITION_COUNT" );
    case -69: return QStringLiteral( "CL_INVALID_PIPE_SIZE" );
    case -70: return QStringLiteral( "CL_INVALID_DEVICE_QUEUE" );
    case -71: return QStringLiteral( "CL_INVALID_SPEC_ID" );
    case -72: return QStringLiteral( "CL_MAX_SIZE_RESTRICTION_EXCEEDED" );
    case -1002: return QStringLiteral( "CL_INVALID_D3D10_DEVICE_KHR" );
    case -1003: return QStringLiteral( "CL_INVALID_D3D10_RESOURCE_KHR" );
    case -1004: return QStringLiteral( "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR" );
    case -1005: return QStringLiteral( "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR" );
    case -1006: return QStringLiteral( "CL_INVALID_D3D11_DEVICE_KHR" );
    case -1007: return QStringLiteral( "CL_INVALID_D3D11_RESOURCE_KHR" );
    case -1008: return QStringLiteral( "CL_D3D11_RESOURCE_ALREADY_ACQUIRED_KHR" );
    case -1009: return QStringLiteral( "CL_D3D11_RESOURCE_NOT_ACQUIRED_KHR" );
    case -1010: return QStringLiteral( "CL_INVALID_DX9_MEDIA_ADAPTER_KHR" );
    case -1011: return QStringLiteral( "CL_INVALID_DX9_MEDIA_SURFACE_KHR" );
    case -1012: return QStringLiteral( "CL_DX9_MEDIA_SURFACE_ALREADY_ACQUIRED_KHR" );
    case -1013: return QStringLiteral( "CL_DX9_MEDIA_SURFACE_NOT_ACQUIRED_KHR" );
    case -1093: return QStringLiteral( "CL_INVALID_EGL_OBJECT_KHR" );
    case -1092: return QStringLiteral( "CL_EGL_RESOURCE_NOT_ACQUIRED_KHR" );
    case -1001: return QStringLiteral( "CL_PLATFORM_NOT_FOUND_KHR" );
    case -1057: return QStringLiteral( "CL_DEVICE_PARTITION_FAILED_EXT" );
    case -1058: return QStringLiteral( "CL_INVALID_PARTITION_COUNT_EXT" );
    case -1059: return QStringLiteral( "CL_INVALID_PARTITION_NAME_EXT" );
    case -1094: return QStringLiteral( "CL_INVALID_ACCELERATOR_INTEL" );
    case -1095: return QStringLiteral( "CL_INVALID_ACCELERATOR_TYPE_INTEL" );
    case -1096: return QStringLiteral( "CL_INVALID_ACCELERATOR_DESCRIPTOR_INTEL" );
    case -1097: return QStringLiteral( "CL_ACCELERATOR_TYPE_NOT_SUPPORTED_INTEL" );
    case -1000: return QStringLiteral( "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR" );
    case -1098: return QStringLiteral( "CL_INVALID_VA_API_MEDIA_ADAPTER_INTEL" );
    case -1099: return QStringLiteral( "CL_INVALID_VA_API_MEDIA_SURFACE_INTEL" );
    case -1100: return QStringLiteral( "CL_VA_API_MEDIA_SURFACE_ALREADY_ACQUIRED_INTEL" );
    case -1101: return QStringLiteral( "CL_VA_API_MEDIA_SURFACE_NOT_ACQUIRED_INTEL" );
    default: return QStringLiteral( "CL_UNKNOWN_ERROR" );
  }
}

cl::Context QgsOpenClUtils::context()
{
  if ( available() && sPlatform() && sDevice() )
  {
    return cl::Context( sDevice );
  }
  else
  {
    return cl::Context();
  }
}

cl::Program QgsOpenClUtils::buildProgram( const cl::Context &context, const QString &source, ExceptionBehavior exceptionBehavior )
{
  cl::Program program;
  try
  {
    program = cl::Program( context, source.toStdString( ) );
    // OpenCL 1.1 for compatibility with older hardware
    // TODO: make this configurable
    program.build( QStringLiteral( "-cl-std=CL1.1 -I%1" ).arg( sourcePath() ).toStdString().c_str() );
  }
  catch ( cl::BuildError &e )
  {
    QString build_log( buildLog( e ) );
    if ( build_log.isEmpty() )
      build_log = QObject::tr( "Build logs not available!" );
    QString err = QObject::tr( "Error building OpenCL program: %1" )
                  .arg( build_log );
    QgsMessageLog::logMessage( err, LOGMESSAGE_TAG, Qgis::Critical );
    if ( exceptionBehavior == Throw )
      throw e;
  }
  catch ( cl::Error &e )
  {
    QString err = QObject::tr( "Error %1 running OpenCL program in %2" )
                  .arg( errorText( e.err() ), QString::fromStdString( e.what() ) );
    QgsMessageLog::logMessage( err, LOGMESSAGE_TAG, Qgis::Critical );
    throw e;
  }
  return program;
}
