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

QLatin1String QgsOpenClUtils::SETTINGS_GLOBAL_ENABLED_KEY = QLatin1Literal( "OpenClEnabled" );
QLatin1String QgsOpenClUtils::SETTINGS_DEFAULT_DEVICE_KEY = QLatin1Literal( "OpenClDefaultDevice" );
QLatin1String QgsOpenClUtils::LOGMESSAGE_TAG = QLatin1Literal( "OpenCL" );
bool QgsOpenClUtils::sAvailable = false;
QString QgsOpenClUtils::sSourcePath = QString();


const std::vector<cl::Device> QgsOpenClUtils::devices()
{
  std::vector<cl::Platform> platforms;
  cl::Platform::get( &platforms );
  std::vector<cl::Device> existingDevices;
  for ( auto &p : platforms )
  {
    std::string platver = p.getInfo<CL_PLATFORM_VERSION>();
    QgsDebugMsg( QStringLiteral( "Found OpenCL platform %1: %2" ).arg( QString::fromStdString( platver ), QString::fromStdString( p.getInfo<CL_PLATFORM_NAME>() ) ) );
    if ( platver.find( "OpenCL " ) != std::string::npos )
    {
      std::vector<cl::Device> _devices;
      // Check for a device
      try
      {
        p.getDevices( CL_DEVICE_TYPE_ALL, &_devices );
      }
      catch ( cl::Error &e )
      {
        QgsDebugMsgLevel( QStringLiteral( "Error %1 on platform %3 searching for OpenCL device: %2" )
                          .arg( errorText( e.err() ),
                                QString::fromStdString( e.what() ),
                                QString::fromStdString( p.getInfo<CL_PLATFORM_NAME>() ) ), 2 );
      }
      if ( _devices.size() > 0 )
      {
        for ( unsigned long i = 0; i < _devices.size(); i++ )
        {
          existingDevices.push_back( _devices[i] );
        }
      }
    }
  }
  return existingDevices;
}

void QgsOpenClUtils::init()
{
  static std::once_flag initialized;
  std::call_once( initialized, [ = ]( )
  {
    try
    {
      activate( preferredDevice() );
    }
    catch ( cl::Error &e )
    {
      QgsMessageLog::logMessage( QObject::tr( "Error %1 initializing OpenCL device: %2" )
                                 .arg( errorText( e.err() ), QString::fromStdString( e.what() ) ),
                                 LOGMESSAGE_TAG, Qgis::Critical );
    }

  } );
}

QString QgsOpenClUtils::sourcePath()
{
  return sSourcePath;
}

void QgsOpenClUtils::setSourcePath( const QString &value )
{
  sSourcePath = value;
}

QString QgsOpenClUtils::activeDeviceInfo( const QgsOpenClUtils::Info infoType )
{
  return deviceInfo( infoType, activeDevice( ) );
}

QString QgsOpenClUtils::deviceInfo( const Info infoType, cl::Device device )
{
  try
  {
    switch ( infoType )
    {
      case Info::Vendor:
        return QString::fromStdString( device.getInfo<CL_DEVICE_VENDOR>() );
      case Info::Profile:
        return QString::fromStdString( device.getInfo<CL_DEVICE_PROFILE>() );
      case Info::Version:
        return QString::fromStdString( device.getInfo<CL_DEVICE_VERSION>() );
      case Info::ImageSupport:
        return device.getInfo<CL_DEVICE_IMAGE_SUPPORT>() ? QStringLiteral( "True" ) : QStringLiteral( "False" );
      case Info::Image2dMaxHeight:
        return QString::number( device.getInfo<CL_DEVICE_IMAGE2D_MAX_HEIGHT>() );
      case Info::MaxMemAllocSize:
        return QString::number( device.getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>() );
      case Info::Image2dMaxWidth:
        return QString::number( device.getInfo<CL_DEVICE_IMAGE2D_MAX_WIDTH>() );
      case Info::Type:
      {
        unsigned long type( device.getInfo<CL_DEVICE_TYPE>() );
        int mappedType;
        switch ( type )
        {
          case CL_DEVICE_TYPE_CPU:
            mappedType = QgsOpenClUtils::HardwareType::CPU;
            break;
          case CL_DEVICE_TYPE_GPU:
            mappedType = QgsOpenClUtils::HardwareType::GPU;
            break;
          default:
            mappedType = QgsOpenClUtils::HardwareType::Other;
        }
        QMetaEnum metaEnum = QMetaEnum::fromType<QgsOpenClUtils::HardwareType>();
        return metaEnum.valueToKey( mappedType );
      }
      case Info::Name:
        return QString::fromStdString( device.getInfo<CL_DEVICE_NAME>() );
    }
  }
  catch ( cl::Error &e )
  {
    // This can be a legitimate error when initializing, let's log it quietly
    QgsDebugMsgLevel( QStringLiteral( "Error %1 getting info for OpenCL device: %2" )
                      .arg( errorText( e.err() ), QString::fromStdString( e.what() ) ),
                      4 );
    return QString();
  }
  return QString();
}


bool QgsOpenClUtils::enabled()
{
  return QgsSettings().value( SETTINGS_GLOBAL_ENABLED_KEY, false, QgsSettings::Section::Core ).toBool();
}

cl::Device QgsOpenClUtils::activeDevice()
{
  return cl::Device::getDefault();
}

QString QgsOpenClUtils::activePlatformVersion()
{
  QString version;
  if ( cl::Platform::getDefault()() )
  {
    std::string platver = cl::Platform::getDefault().getInfo<CL_PLATFORM_VERSION>();
    if ( platver.find( "OpenCL " ) != std::string::npos )
    {
      version = QString::fromStdString( platver.substr( 7 ) ).split( ' ' ).first();
    }
  }
  return version;
}

void QgsOpenClUtils::storePreferredDevice( const QString deviceId )
{
  QgsSettings().setValue( SETTINGS_DEFAULT_DEVICE_KEY, deviceId, QgsSettings::Section::Core );
}

QString QgsOpenClUtils::preferredDevice()
{
  return QgsSettings().value( SETTINGS_DEFAULT_DEVICE_KEY, QString( ), QgsSettings::Section::Core ).toString();
}

QString QgsOpenClUtils::deviceId( const cl::Device device )
{
  return QStringLiteral( "%1|%2|%3|%4" )
         .arg( deviceInfo( QgsOpenClUtils::Info::Name, device ) )
         .arg( deviceInfo( QgsOpenClUtils::Info::Vendor, device ) )
         .arg( deviceInfo( QgsOpenClUtils::Info::Version, device ) )
         .arg( deviceInfo( QgsOpenClUtils::Info::Type, device ) );
}

bool QgsOpenClUtils::activate( const QString &preferredDeviceId )
{
  if ( deviceId( activeDevice() ) == preferredDeviceId )
  {
    sAvailable = true;
    return false;
  }
  try
  {
    std::vector<cl::Platform> platforms;
    cl::Platform::get( &platforms );
    cl::Platform plat;
    cl::Device dev;
    bool deviceFound = false;
    for ( auto &p : platforms )
    {
      if ( deviceFound )
        break;
      std::string platver = p.getInfo<CL_PLATFORM_VERSION>();
      QgsDebugMsg( QStringLiteral( "Found OpenCL platform %1: %2" ).arg( QString::fromStdString( platver ), QString::fromStdString( p.getInfo<CL_PLATFORM_NAME>() ) ) );
      if ( platver.find( "OpenCL " ) != std::string::npos )
      {
        std::vector<cl::Device> devices;
        // Search for a device
        try
        {
          p.getDevices( CL_DEVICE_TYPE_ALL, &devices );
          // First search for the preferred device
          if ( ! preferredDeviceId.isEmpty() )
          {
            for ( const auto &_dev : devices )
            {
              if ( preferredDeviceId == deviceId( _dev ) )
              {
                // Got one!
                plat = p;
                dev = _dev;
                deviceFound = true;
                break;
              }
            }
          }
          // Not found or preferred device id not set: get the first GPU
          if ( ! deviceFound )
          {
            for ( const auto &_dev : devices )
            {
              if ( _dev.getInfo<CL_DEVICE_TYPE>() == CL_DEVICE_TYPE_GPU )
              {
                // Got one!
                plat = p;
                dev = _dev;
                deviceFound = true;
                break;
              }
            }
          }
          // Still nothing? Get the first device
          if ( ! deviceFound )
          {
            for ( const auto &_dev : devices )
            {
              if ( _dev.getInfo<CL_DEVICE_TYPE>() == CL_DEVICE_TYPE_CPU )
              {
                // Got one!
                plat = p;
                dev = _dev;
                deviceFound = true;
                break;
              }
            }
          }
          if ( ! deviceFound )
          {
            QgsMessageLog::logMessage( QObject::tr( "No OpenCL device could be found." ), LOGMESSAGE_TAG, Qgis::Warning );
          }
        }
        catch ( cl::Error &e )
        {
          QgsDebugMsg( QStringLiteral( "Error %1 on platform %3 searching for OpenCL device: %2" )
                       .arg( errorText( e.err() ),
                             QString::fromStdString( e.what() ),
                             QString::fromStdString( p.getInfo<CL_PLATFORM_NAME>() ) ) );
        }

      }
    }
    if ( ! plat() )
    {
      QgsMessageLog::logMessage( QObject::tr( "No OpenCL platform found." ), LOGMESSAGE_TAG, Qgis::Warning );
      sAvailable = false;
    }
    else
    {
      cl::Platform newP = cl::Platform::setDefault( plat );
      if ( newP != plat )
      {
        QgsMessageLog::logMessage( QObject::tr( "Error setting default platform." ),
                                   LOGMESSAGE_TAG, Qgis::Warning );
        sAvailable = false;
      }
      else
      {
        cl::Device::setDefault( dev );
        QgsMessageLog::logMessage( QObject::tr( "Active OpenCL device: %1" )
                                   .arg( QString::fromStdString( dev.getInfo<CL_DEVICE_NAME>() ) ),
                                   LOGMESSAGE_TAG, Qgis::Success );
        sAvailable = true;
      }
    }
  }

  catch ( cl::Error &e )
  {
    QgsMessageLog::logMessage( QObject::tr( "Error %1 searching for OpenCL device: %2" )
                               .arg( errorText( e.err() ), QString::fromStdString( e.what() ) ),
                               LOGMESSAGE_TAG, Qgis::Warning );
    sAvailable = false;
  }
  return sAvailable;
}

QString QgsOpenClUtils::deviceDescription( const cl::Device device )
{
  return QStringLiteral(
           "Type: <b>%9</b><br>"
           "Name: <b>%1</b><br>"
           "Vendor: <b>%2</b><br>"
           "Profile: <b>%3</b><br>"
           "Version: <b>%4</b><br>"
           "Image support: <b>%5</b><br>"
           "Max image2d width: <b>%6</b><br>"
           "Max image2d height: <b>%7</b><br>"
           "Max mem alloc size: <b>%8</b><br>"
         ).arg( QgsOpenClUtils::deviceInfo( QgsOpenClUtils::Info::Name, device ),
                QgsOpenClUtils::deviceInfo( QgsOpenClUtils::Info::Vendor, device ),
                QgsOpenClUtils::deviceInfo( QgsOpenClUtils::Info::Profile, device ),
                QgsOpenClUtils::deviceInfo( QgsOpenClUtils::Info::Version, device ),
                QgsOpenClUtils::deviceInfo( QgsOpenClUtils::Info::ImageSupport, device ),
                QgsOpenClUtils::deviceInfo( QgsOpenClUtils::Info::Image2dMaxWidth, device ),
                QgsOpenClUtils::deviceInfo( QgsOpenClUtils::Info::Image2dMaxHeight, device ),
                QgsOpenClUtils::deviceInfo( QgsOpenClUtils::Info::MaxMemAllocSize, device ),
                QgsOpenClUtils::deviceInfo( QgsOpenClUtils::Info::Type, device ) );
}

QString QgsOpenClUtils::deviceDescription( const QString deviceId )
{
  for ( const auto &dev : devices( ) )
  {
    if ( QgsOpenClUtils::deviceId( dev ) == deviceId )
      return deviceDescription( dev );
  }
  return QString();
}

bool QgsOpenClUtils::available()
{
  init();
  return sAvailable;
}

void QgsOpenClUtils::setEnabled( bool enabled )
{
  QgsSettings().setValue( SETTINGS_GLOBAL_ENABLED_KEY, enabled, QgsSettings::Section::Core );
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

QString QgsOpenClUtils::buildLog( cl::BuildError &error )
{
  cl::BuildLogType build_logs = error.getBuildLog();
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

cl::CommandQueue QgsOpenClUtils::commandQueue()
{
  // Depending on the platform version, to avoid a crash
  // we need to use the legacy calls to C API instead of the 2.0
  // compatible C++ API.
  cl::Context context( QgsOpenClUtils::context() );
  if ( QgsOpenClUtils::activePlatformVersion().toFloat() >= 200 )
  {
    return cl::CommandQueue( context );
  }
  else  // legacy
  {
    cl::Device device( QgsOpenClUtils::activeDevice() );
    cl_command_queue_properties properties = 0;
    Q_NOWARN_DEPRECATED_PUSH
    cl_command_queue queue = clCreateCommandQueue( context(), device(),  properties, nullptr );
    Q_NOWARN_DEPRECATED_POP
    return cl::CommandQueue( queue, true );
  }
}

cl::Context QgsOpenClUtils::context()
{
  static cl::Context context;
  static std::once_flag contextCreated;
  std::call_once( contextCreated, [ = ]()
  {
    if ( available() && cl::Platform::getDefault()() && cl::Device::getDefault()() )
    {
      context = cl::Context( cl::Device::getDefault() );
    }
  } );
  return context;
}

cl::Program QgsOpenClUtils::buildProgram( const cl::Context &, const QString &source, ExceptionBehavior exceptionBehavior )
{
  // Deprecated: ignore context and use default
  return buildProgram( source, exceptionBehavior );
}

cl::Program QgsOpenClUtils::buildProgram( const QString &source, QgsOpenClUtils::ExceptionBehavior exceptionBehavior )
{
  cl::Program program;
  try
  {
    program = cl::Program( QgsOpenClUtils::context(), source.toStdString( ) );
    // OpenCL version for compatibility with older hardware, but it's up to
    // llvm to support latest CL versions
    bool ok;
    float version( QgsOpenClUtils::activePlatformVersion().toFloat( &ok ) );
    if ( ok && version < 2.0f )
    {
      program.build( QStringLiteral( "-cl-std=CL%1 -I%2" )
                     .arg( QgsOpenClUtils::activePlatformVersion( ) )
                     .arg( sourcePath() ).toStdString().c_str() );
    }
    else
    {
      program.build( QStringLiteral( "-I%1" )
                     .arg( sourcePath() ).toStdString().c_str() );
    }
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
    QString err = QObject::tr( "Error %1 building OpenCL program in %2" )
                  .arg( errorText( e.err() ), QString::fromStdString( e.what() ) );
    QgsMessageLog::logMessage( err, LOGMESSAGE_TAG, Qgis::Critical );
    throw e;
  }
  return program;
}
