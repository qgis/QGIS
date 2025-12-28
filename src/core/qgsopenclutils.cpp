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

#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgssettings.h"

#include <QDebug>
#include <QFile>
#include <QLibrary>
#include <QTextStream>

#include "moc_qgsopenclutils.cpp"

#ifdef Q_OS_WIN
#if defined(UNICODE) && !defined(_UNICODE)
#define _UNICODE
#endif
#include <windows.h>
#include <tchar.h>
#endif

#if defined(_MSC_VER)
#include <windows.h>
#include <excpt.h>
#endif

QLatin1String QgsOpenClUtils::SETTINGS_GLOBAL_ENABLED_KEY = "OpenClEnabled"_L1;
QLatin1String QgsOpenClUtils::SETTINGS_DEFAULT_DEVICE_KEY = "OpenClDefaultDevice"_L1;
QLatin1String QgsOpenClUtils::LOGMESSAGE_TAG = "OpenCL"_L1;
bool QgsOpenClUtils::sAvailable = false;

Q_GLOBAL_STATIC( QString, sSourcePath )


const std::vector<cl::Device> QgsOpenClUtils::devices()
{
  std::vector<cl::Platform> platforms;
  cl::Platform::get( &platforms );
  std::vector<cl::Device> existingDevices;
  for ( const auto &p : platforms )
  {
    const std::string platver = p.getInfo<CL_PLATFORM_VERSION>();
    QgsMessageLog::logMessage( QObject::tr( "Found OpenCL platform %1: %2" )
                               .arg( QString::fromStdString( platver ),
                                     QString::fromStdString( p.getInfo<CL_PLATFORM_NAME>() ) ),
                               LOGMESSAGE_TAG, Qgis::MessageLevel::Info );
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
        QgsMessageLog::logMessage( QObject::tr( "Error %1 on platform %3 searching for OpenCL device: %2" )
                                   .arg( errorText( e.err() ),
                                         QString::fromStdString( e.what() ),
                                         QString::fromStdString( p.getInfo<CL_PLATFORM_NAME>() ) ),
                                   LOGMESSAGE_TAG, Qgis::MessageLevel::Warning );
      }
      if ( _devices.size() > 0 )
      {
        for ( unsigned long i = 0; i < _devices.size(); i++ )
        {
          QgsMessageLog::logMessage( QObject::tr( "Found OpenCL device: %1" )
                                     .arg( deviceId( _devices[i] ) ),
                                     LOGMESSAGE_TAG, Qgis::MessageLevel::Info );
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
  std::call_once( initialized, []( )
  {
#ifdef Q_OS_MAC
    QLibrary openCLLib { u"/System/Library/Frameworks/OpenCL.framework/Versions/Current/OpenCL"_s };
#else
    QLibrary openCLLib { u"OpenCL"_s };
#endif
    openCLLib.setLoadHints( QLibrary::LoadHint::ResolveAllSymbolsHint );
    if ( ! openCLLib.load() )
    {
      QgsMessageLog::logMessage( QObject::tr( "Error loading OpenCL library: %1" )
                                 .arg( openCLLib.errorString() ),
                                 LOGMESSAGE_TAG, Qgis::MessageLevel::Critical );
      return;
    }

#ifdef Q_OS_WIN
#ifdef _UNICODE
#define _T(x) L##x
#else
#define _T(x) x
#endif
    HMODULE hModule = GetModuleHandle( _T( "OpenCL.dll" ) );
    if ( hModule )
    {
      TCHAR pszFileName[1024];
      if ( GetModuleFileName( hModule, pszFileName, 1024 ) < 1024 )
      {
        QgsMessageLog::logMessage( QObject::tr( "Found OpenCL library filename %1" )
                                   .arg( pszFileName ),
                                   LOGMESSAGE_TAG, Qgis::MessageLevel::Info );

        DWORD dwUseless;
        DWORD dwLen = GetFileVersionInfoSize( pszFileName, &dwUseless );
        if ( dwLen )
        {
          LPTSTR lpVI = ( LPTSTR ) malloc( dwLen * sizeof( TCHAR ) );
          if ( lpVI )
          {
            if ( GetFileVersionInfo( pszFileName, 0, dwLen, lpVI ) )
            {
              VS_FIXEDFILEINFO *lpFFI;
              if ( VerQueryValue( lpVI, _T( "\\" ), ( LPVOID * ) &lpFFI, ( UINT * ) &dwUseless ) )
              {
                QgsMessageLog::logMessage( QObject::tr( "OpenCL Product version: %1.%2.%3.%4" )
                                           .arg( lpFFI->dwProductVersionMS >> 16 )
                                           .arg( lpFFI->dwProductVersionMS & 0xffff )
                                           .arg( lpFFI->dwProductVersionLS >> 16 )
                                           .arg( lpFFI->dwProductVersionLS & 0xffff ),
                                           LOGMESSAGE_TAG, Qgis::MessageLevel::Info );
              }

              struct LANGANDCODEPAGE
              {
                WORD wLanguage;
                WORD wCodePage;
              } *lpTranslate;

              DWORD cbTranslate;

              if ( VerQueryValue( lpVI, _T( "\\VarFileInfo\\Translation" ), ( LPVOID * ) &lpTranslate, ( UINT * ) &cbTranslate ) && cbTranslate >= sizeof( struct LANGANDCODEPAGE ) )
              {
                QStringList items = QStringList()
                                    << u"Comments"_s
                                    << u"InternalName"_s
                                    << u"ProductName"_s
                                    << u"CompanyName"_s
                                    << u"LegalCopyright"_s
                                    << u"ProductVersion"_s
                                    << u"FileDescription"_s
                                    << u"LegalTrademarks"_s
                                    << u"PrivateBuild"_s
                                    << u"FileVersion"_s
                                    << u"OriginalFilename"_s
                                    << u"SpecialBuild"_s;
                for ( auto d : items )
                {
                  LPTSTR lpBuffer;
                  QString subBlock = QString( u"\\StringFileInfo\\%1%2\\%3"_s )
                                     .arg( lpTranslate[0].wLanguage, 4, 16, QLatin1Char( '0' ) )
                                     .arg( lpTranslate[0].wCodePage, 4, 16, QLatin1Char( '0' ) )
                                     .arg( d );

                  QgsDebugMsgLevel( QString( "d:%1 subBlock:%2" ).arg( d ).arg( subBlock ), 2 );

                  BOOL r = VerQueryValue( lpVI,
#ifdef UNICODE
                                          subBlock.toStdWString().c_str(),
#else
                                          subBlock.toUtf8(),
#endif
                                          ( LPVOID * )&lpBuffer, ( UINT * )&dwUseless );

                  if ( r && lpBuffer && lpBuffer != INVALID_HANDLE_VALUE && dwUseless < 1023 )
                  {
                    QgsMessageLog::logMessage( QObject::tr( "Found OpenCL version info %1: %2" )
                                               .arg( d )
#ifdef UNICODE
                                               .arg( QString::fromUtf16( ( const ushort * ) lpBuffer ) ),
#else
                                               .arg( QString::fromLocal8Bit( lpBuffer ) ),
#endif
                                               LOGMESSAGE_TAG, Qgis::MessageLevel::Info );
                  }
                }
              }
            }

            free( lpVI );
          }
        }
      }
      else
      {
        QgsMessageLog::logMessage( QObject::tr( "No module handle to OpenCL library" ), LOGMESSAGE_TAG, Qgis::MessageLevel::Warning );
      }
    }
    else
    {
      QgsMessageLog::logMessage( QObject::tr( "No module handle to OpenCL library" ), LOGMESSAGE_TAG, Qgis::MessageLevel::Warning );
    }
#endif

    try
    {
      activate( preferredDevice() );
    }
    catch ( cl::Error &e )
    {
      QgsMessageLog::logMessage( QObject::tr( "Error %1 initializing OpenCL device: %2" )
                                 .arg( errorText( e.err() ), QString::fromStdString( e.what() ) ),
                                 LOGMESSAGE_TAG, Qgis::MessageLevel::Critical );
    }

  } );
}

QString QgsOpenClUtils::sourcePath()
{
  return *sSourcePath();
}

void QgsOpenClUtils::setSourcePath( const QString &value )
{
  *sSourcePath() = value;
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
        return device.getInfo<CL_DEVICE_IMAGE_SUPPORT>() ? u"True"_s : u"False"_s;
      case Info::Image2dMaxHeight:
        return QString::number( device.getInfo<CL_DEVICE_IMAGE2D_MAX_HEIGHT>() );
      case Info::MaxMemAllocSize:
        return QString::number( device.getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>() );
      case Info::Image2dMaxWidth:
        return QString::number( device.getInfo<CL_DEVICE_IMAGE2D_MAX_WIDTH>() );
      case Info::Type:
      {
        const unsigned long type( device.getInfo<CL_DEVICE_TYPE>() );
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
        const QMetaEnum metaEnum = QMetaEnum::fromType<QgsOpenClUtils::HardwareType>();
        return metaEnum.valueToKey( mappedType );
      }
      case Info::Name:
        return QString::fromStdString( device.getInfo<CL_DEVICE_NAME>() );
    }
  }
  catch ( cl::Error &e )
  {
    // This can be a legitimate error when initializing, let's log it quietly
    QgsDebugMsgLevel( u"Error %1 getting info for OpenCL device: %2"_s
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
    const std::string platver = cl::Platform::getDefault().getInfo<CL_PLATFORM_VERSION>();
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
  return u"%1|%2|%3|%4"_s
         .arg( deviceInfo( QgsOpenClUtils::Info::Name, device ) )
         .arg( deviceInfo( QgsOpenClUtils::Info::Vendor, device ) )
         .arg( deviceInfo( QgsOpenClUtils::Info::Version, device ) )
         .arg( deviceInfo( QgsOpenClUtils::Info::Type, device ) );
}

#if defined(_MSC_VER)
static void emitLogMessageForSEHException( int exceptionCode )
{
  QgsMessageLog::logMessage( QObject::tr( "Unexpected exception of code %1 occurred while searching for OpenCL device. Note that the application may become unreliable and may need to be restarted." ).arg( exceptionCode ),
                             QgsOpenClUtils::LOGMESSAGE_TAG, Qgis::MessageLevel::Warning );
}
#endif

bool QgsOpenClUtils::activate( const QString &preferredDeviceId )
{
#if defined(_MSC_VER)
  // Try to capture hard crashes such as https://github.com/qgis/QGIS/issues/59617
  __try
  {
    // We cannot combine together __try and try in the same function.
    return activateInternal( preferredDeviceId );
  }
  __except ( EXCEPTION_EXECUTE_HANDLER )
  {
    emitLogMessageForSEHException( GetExceptionCode() );
    return false;
  }
#else
  return activateInternal( preferredDeviceId );
#endif
}

bool QgsOpenClUtils::activateInternal( const QString &preferredDeviceId )
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
    for ( const auto &p : platforms )
    {
      if ( deviceFound )
        break;
      const std::string platver = p.getInfo<CL_PLATFORM_VERSION>();
      QgsDebugMsgLevel( u"Found OpenCL platform %1: %2"_s.arg( QString::fromStdString( platver ), QString::fromStdString( p.getInfo<CL_PLATFORM_NAME>() ) ), 2 );
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
            QgsMessageLog::logMessage( QObject::tr( "No OpenCL device could be found." ), LOGMESSAGE_TAG, Qgis::MessageLevel::Warning );
          }
        }
        catch ( cl::Error &e )
        {
          QgsDebugError( u"Error %1 on platform %3 searching for OpenCL device: %2"_s
                         .arg( errorText( e.err() ),
                               QString::fromStdString( e.what() ),
                               QString::fromStdString( p.getInfo<CL_PLATFORM_NAME>() ) ) );
        }

      }
    }
    if ( ! plat() )
    {
      QgsMessageLog::logMessage( QObject::tr( "No OpenCL platform found." ), LOGMESSAGE_TAG, Qgis::MessageLevel::Warning );
      sAvailable = false;
    }
    else
    {
      const cl::Platform newP = cl::Platform::setDefault( plat );
      if ( newP != plat )
      {
        QgsMessageLog::logMessage( QObject::tr( "Error setting default platform." ),
                                   LOGMESSAGE_TAG, Qgis::MessageLevel::Warning );
        sAvailable = false;
      }
      else
      {
        cl::Device::setDefault( dev );
        QgsMessageLog::logMessage( QObject::tr( "Active OpenCL device: %1" )
                                   .arg( QString::fromStdString( dev.getInfo<CL_DEVICE_NAME>() ) ),
                                   LOGMESSAGE_TAG, Qgis::MessageLevel::Success );
        sAvailable = true;
      }
    }
  }

  catch ( cl::Error &e )
  {
    QgsMessageLog::logMessage( QObject::tr( "Error %1 searching for OpenCL device: %2" )
                               .arg( errorText( e.err() ), QString::fromStdString( e.what() ) ),
                               LOGMESSAGE_TAG, Qgis::MessageLevel::Warning );
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
    QgsMessageLog::logMessage( QObject::tr( "Could not load OpenCL program from path %1." ).arg( path ), LOGMESSAGE_TAG, Qgis::MessageLevel::Warning );
  }
  return source_str;
}

QString QgsOpenClUtils::sourceFromBaseName( const QString &baseName )
{
  const QString path = u"%1/%2.cl"_s.arg( sourcePath(), baseName );
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
    case 0: return u"CL_SUCCESS"_s;
    case -1: return u"CL_DEVICE_NOT_FOUND"_s;
    case -2: return u"CL_DEVICE_NOT_AVAILABLE"_s;
    case -3: return u"CL_COMPILER_NOT_AVAILABLE"_s;
    case -4: return u"CL_MEM_OBJECT_ALLOCATION_FAILURE"_s;
    case -5: return u"CL_OUT_OF_RESOURCES"_s;
    case -6: return u"CL_OUT_OF_HOST_MEMORY"_s;
    case -7: return u"CL_PROFILING_INFO_NOT_AVAILABLE"_s;
    case -8: return u"CL_MEM_COPY_OVERLAP"_s;
    case -9: return u"CL_IMAGE_FORMAT_MISMATCH"_s;
    case -10: return u"CL_IMAGE_FORMAT_NOT_SUPPORTED"_s;
    case -12: return u"CL_MAP_FAILURE"_s;
    case -13: return u"CL_MISALIGNED_SUB_BUFFER_OFFSET"_s;
    case -14: return u"CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST"_s;
    case -15: return u"CL_COMPILE_PROGRAM_FAILURE"_s;
    case -16: return u"CL_LINKER_NOT_AVAILABLE"_s;
    case -17: return u"CL_LINK_PROGRAM_FAILURE"_s;
    case -18: return u"CL_DEVICE_PARTITION_FAILED"_s;
    case -19: return u"CL_KERNEL_ARG_INFO_NOT_AVAILABLE"_s;
    case -30: return u"CL_INVALID_VALUE"_s;
    case -31: return u"CL_INVALID_DEVICE_TYPE"_s;
    case -32: return u"CL_INVALID_PLATFORM"_s;
    case -33: return u"CL_INVALID_DEVICE"_s;
    case -34: return u"CL_INVALID_CONTEXT"_s;
    case -35: return u"CL_INVALID_QUEUE_PROPERTIES"_s;
    case -36: return u"CL_INVALID_COMMAND_QUEUE"_s;
    case -37: return u"CL_INVALID_HOST_PTR"_s;
    case -38: return u"CL_INVALID_MEM_OBJECT"_s;
    case -39: return u"CL_INVALID_IMAGE_FORMAT_DESCRIPTOR"_s;
    case -40: return u"CL_INVALID_IMAGE_SIZE"_s;
    case -41: return u"CL_INVALID_SAMPLER"_s;
    case -42: return u"CL_INVALID_BINARY"_s;
    case -43: return u"CL_INVALID_BUILD_OPTIONS"_s;
    case -44: return u"CL_INVALID_PROGRAM"_s;
    case -45: return u"CL_INVALID_PROGRAM_EXECUTABLE"_s;
    case -46: return u"CL_INVALID_KERNEL_NAME"_s;
    case -47: return u"CL_INVALID_KERNEL_DEFINITION"_s;
    case -48: return u"CL_INVALID_KERNEL"_s;
    case -49: return u"CL_INVALID_ARG_INDEX"_s;
    case -50: return u"CL_INVALID_ARG_VALUE"_s;
    case -51: return u"CL_INVALID_ARG_SIZE"_s;
    case -52: return u"CL_INVALID_KERNEL_ARGS"_s;
    case -53: return u"CL_INVALID_WORK_DIMENSION"_s;
    case -54: return u"CL_INVALID_WORK_GROUP_SIZE"_s;
    case -55: return u"CL_INVALID_WORK_ITEM_SIZE"_s;
    case -56: return u"CL_INVALID_GLOBAL_OFFSET"_s;
    case -57: return u"CL_INVALID_EVENT_WAIT_LIST"_s;
    case -58: return u"CL_INVALID_EVENT"_s;
    case -59: return u"CL_INVALID_OPERATION"_s;
    case -60: return u"CL_INVALID_GL_OBJECT"_s;
    case -61: return u"CL_INVALID_BUFFER_SIZE"_s;
    case -62: return u"CL_INVALID_MIP_LEVEL"_s;
    case -63: return u"CL_INVALID_GLOBAL_WORK_SIZE"_s;
    case -64: return u"CL_INVALID_PROPERTY"_s;
    case -65: return u"CL_INVALID_IMAGE_DESCRIPTOR"_s;
    case -66: return u"CL_INVALID_COMPILER_OPTIONS"_s;
    case -67: return u"CL_INVALID_LINKER_OPTIONS"_s;
    case -68: return u"CL_INVALID_DEVICE_PARTITION_COUNT"_s;
    case -69: return u"CL_INVALID_PIPE_SIZE"_s;
    case -70: return u"CL_INVALID_DEVICE_QUEUE"_s;
    case -71: return u"CL_INVALID_SPEC_ID"_s;
    case -72: return u"CL_MAX_SIZE_RESTRICTION_EXCEEDED"_s;
    case -1002: return u"CL_INVALID_D3D10_DEVICE_KHR"_s;
    case -1003: return u"CL_INVALID_D3D10_RESOURCE_KHR"_s;
    case -1004: return u"CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR"_s;
    case -1005: return u"CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR"_s;
    case -1006: return u"CL_INVALID_D3D11_DEVICE_KHR"_s;
    case -1007: return u"CL_INVALID_D3D11_RESOURCE_KHR"_s;
    case -1008: return u"CL_D3D11_RESOURCE_ALREADY_ACQUIRED_KHR"_s;
    case -1009: return u"CL_D3D11_RESOURCE_NOT_ACQUIRED_KHR"_s;
    case -1010: return u"CL_INVALID_DX9_MEDIA_ADAPTER_KHR"_s;
    case -1011: return u"CL_INVALID_DX9_MEDIA_SURFACE_KHR"_s;
    case -1012: return u"CL_DX9_MEDIA_SURFACE_ALREADY_ACQUIRED_KHR"_s;
    case -1013: return u"CL_DX9_MEDIA_SURFACE_NOT_ACQUIRED_KHR"_s;
    case -1093: return u"CL_INVALID_EGL_OBJECT_KHR"_s;
    case -1092: return u"CL_EGL_RESOURCE_NOT_ACQUIRED_KHR"_s;
    case -1001: return u"CL_PLATFORM_NOT_FOUND_KHR"_s;
    case -1057: return u"CL_DEVICE_PARTITION_FAILED_EXT"_s;
    case -1058: return u"CL_INVALID_PARTITION_COUNT_EXT"_s;
    case -1059: return u"CL_INVALID_PARTITION_NAME_EXT"_s;
    case -1094: return u"CL_INVALID_ACCELERATOR_INTEL"_s;
    case -1095: return u"CL_INVALID_ACCELERATOR_TYPE_INTEL"_s;
    case -1096: return u"CL_INVALID_ACCELERATOR_DESCRIPTOR_INTEL"_s;
    case -1097: return u"CL_ACCELERATOR_TYPE_NOT_SUPPORTED_INTEL"_s;
    case -1000: return u"CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR"_s;
    case -1098: return u"CL_INVALID_VA_API_MEDIA_ADAPTER_INTEL"_s;
    case -1099: return u"CL_INVALID_VA_API_MEDIA_SURFACE_INTEL"_s;
    case -1100: return u"CL_VA_API_MEDIA_SURFACE_ALREADY_ACQUIRED_INTEL"_s;
    case -1101: return u"CL_VA_API_MEDIA_SURFACE_NOT_ACQUIRED_INTEL"_s;
    default: return u"CL_UNKNOWN_ERROR"_s;
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
    const cl_command_queue_properties properties = 0;
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
  std::call_once( contextCreated, []()
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
    const float version( QgsOpenClUtils::activePlatformVersion().toFloat( &ok ) );
    if ( ok && version < 2.0f )
    {
      program.build( u"-cl-std=CL%1 -I\"%2\""_s
                     .arg( QgsOpenClUtils::activePlatformVersion( ) )
                     .arg( sourcePath() ).toStdString().c_str() );
    }
    else
    {
      program.build( u"-I\"%1\""_s
                     .arg( sourcePath() ).toStdString().c_str() );
    }
  }
  catch ( cl::BuildError &e )
  {
    QString build_log( buildLog( e ) );
    if ( build_log.isEmpty() )
      build_log = QObject::tr( "Build logs not available!" );
    const QString err = QObject::tr( "Error building OpenCL program: %1" )
                        .arg( build_log );
    QgsMessageLog::logMessage( err, LOGMESSAGE_TAG, Qgis::MessageLevel::Critical );
    if ( exceptionBehavior == Throw )
      throw e;
  }
  catch ( cl::Error &e )
  {
    const QString err = QObject::tr( "Error %1 building OpenCL program in %2" )
                        .arg( errorText( e.err() ), QString::fromStdString( e.what() ) );
    QgsMessageLog::logMessage( err, LOGMESSAGE_TAG, Qgis::MessageLevel::Critical );
    throw e;
  }
  return program;
}
