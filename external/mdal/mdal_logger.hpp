/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2020 Tomas Mizera (tomas.mizera2 at gmail dot com)
*/

#ifndef MDAL_LOGGER_H
#define MDAL_LOGGER_H

#include <string>

#include "mdal_utils.hpp"

namespace MDAL
{
  /**
   * Namespace including functions responsible for handling logs.
   *
   * Use in code as: MDAL::Log::error/warning( MDAL_Status, logMessage ).
   * By default, output from logger is set to standard stdout, but it is
   * possible to set custom logger output with function setLoggerCallback.
   */
  namespace Log
  {
    void error( MDAL::Error );
    void error( MDAL::Error err, std::string driver );
    void error( MDAL_Status status, std::string mssg );
    void error( MDAL_Status status, std::string driverName, std::string mssg );
    void warning( MDAL_Status status, std::string mssg );
    void warning( MDAL_Status status, std::string driverName, std::string mssg );
    void info( std::string mssg );
    void debug( std::string mssg );

    MDAL_Status getLastStatus();
    void resetLastStatus();

    void setLoggerCallback( MDAL_LoggerCallback callback );
    void setLogVerbosity( MDAL_LogLevel verbosity );
  }
}

#endif // MDAL_LOGGER_H
