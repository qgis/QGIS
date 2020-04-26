/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2020 Tomas Mizera (tomas.mizera2 at gmail dot com)
*/

#include <iostream>

#include "mdal_logger.hpp"

// Standard output for logger
void _standardStdout( MDAL_LogLevel logLevel, MDAL_Status status, const char *mssg );

static MDAL_Status sLastStatus;
static MDAL_LoggerCallback sLoggerCallback = &_standardStdout;
static MDAL_LogLevel sLogVerbosity = MDAL_LogLevel::Error;

void _log( MDAL_LogLevel logLevel, MDAL_Status status, std::string mssg )
{
  if ( sLoggerCallback && logLevel <= sLogVerbosity )
  {
    sLoggerCallback( logLevel, status, mssg.c_str() );
  }
}

void MDAL::Log::error( MDAL::Error e )
{
  error( e.status, "Driver: " + e.driver + ": " + e.mssg );
}

void MDAL::Log::error( MDAL::Error err, std::string driver )
{
  err.setDriver( driver );
  error( err );
}

void MDAL::Log::error( MDAL_Status status, std::string mssg )
{
  sLastStatus = status;
  _log( MDAL_LogLevel::Error, status, mssg );
}

void MDAL::Log::error( MDAL_Status status, std::string driverName, std::string mssg )
{
  error( status, "Driver: " + driverName + ": " + mssg );
}

void MDAL::Log::warning( MDAL_Status status, std::string mssg )
{
  sLastStatus = status;
  _log( MDAL_LogLevel::Warn, status, mssg );
}

void MDAL::Log::warning( MDAL_Status status, std::string driverName, std::string mssg )
{
  warning( status, "Driver: " + driverName + ": " + mssg );
}

void MDAL::Log::info( std::string mssg )
{
  _log( MDAL_LogLevel::Info, MDAL_Status::None, mssg );
}

void MDAL::Log::debug( std::string mssg )
{
  _log( MDAL_LogLevel::Debug, MDAL_Status::None, mssg );
}

MDAL_Status MDAL::Log::getLastStatus()
{
  return sLastStatus;
}

void MDAL::Log::resetLastStatus()
{
  sLastStatus = MDAL_Status::None;
}

void MDAL::Log::setLoggerCallback( MDAL_LoggerCallback callback )
{
  sLoggerCallback = callback;
}

void MDAL::Log::setLogVerbosity( MDAL_LogLevel verbosity )
{
  sLogVerbosity = verbosity;
}

void _standardStdout( MDAL_LogLevel logLevel, MDAL_Status status, const char *mssg )
{
  switch ( logLevel )
  {
    case Error:
      std::cerr << "ERROR: Status " << status << ": " << mssg << std::endl;
      break;
    case Warn:
      std::cout << "WARN: Status " << status << ": " << mssg << std::endl;
      break;
    case Info:
      std::cout << "INFO: " << mssg << std::endl;
      break;
    case Debug:
      std::cout << "DEBUG: " << mssg << std::endl;
      break;
    default: break;
  }
}
