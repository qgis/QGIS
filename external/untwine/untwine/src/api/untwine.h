/*****************************************************************************
 *   Copyright (c) 2020, Hobu, Inc. (info@hobu.co)                           *
 *                                                                           *
 *   All rights reserved.                                                    *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation; either version 3 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 ****************************************************************************/

#ifndef UNTWINE_H
#define UNTWINE_H

#ifdef UNTWINE_STATIC
#  define UNTWINE_EXPORT
#else
#  if defined _WIN32 || defined __CYGWIN__
#    ifdef UNTWINE_EXPORTS
#      ifdef __GNUC__
#        define UNTWINE_EXPORT __attribute__ ((dllexport))
#      else
#        define UNTWINE_EXPORT __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
#      endif
#    else
#      ifdef __GNUC__
#        define UNTWINE_EXPORT __attribute__ ((dllimport))
#      else
#        define UNTWINE_EXPORT __declspec(dllimport) // Note: actually gcc seems to also supports this syntax.
#      endif
#    endif
#  else
#    if __GNUC__ >= 4
#      define UNTWINE_EXPORT __attribute__ ((visibility ("default")))
#    else
#      define UNTWINE_EXPORT
#    endif
#  endif
#endif

#include <functional>
#include <string>

/**
 * Log levels
 */
namespace Untwine {

     /**
     * Returns untwine version (x.y.z)
     */
    UNTWINE_EXPORT std::string version();

    enum LogLevel
    {
      Error,
      Warn,
      Info,
      Debug
    };

    typedef std::function<void(LogLevel logLevel, const std::string& message)> LoggerCallbackFunction;

    /**
     * Sets custom callback for logging output
     */
    UNTWINE_EXPORT void SetLoggerCallback( LoggerCallbackFunction callback );

    /**
     * Sets maximum log level (verbosity)
     *
     * By default logger outputs errors only.
     * Log levels (low to high): Error, Warn, Info, Debug
     * For example, if LogLevel is set to Warn, logger outputs errors and warnings.
     */
    UNTWINE_EXPORT void SetLogVerbosity( LogLevel verbosity );

    UNTWINE_EXPORT struct Feedback
    {
        enum Status {
            Ready = 0, // before start
            Canceled, // user cancelled, not running
            Running, // running (cancellation may be already requested)
            Finished, //Success (finished)
            Failed // Failed to succeed (all reasons BUT cancellation by user)
        };

        bool cancellationRequested = false; //!< QGIS sets this, UNTWINE reads it and try to cancel the job when first possible
        Status status = Ready; //! UNTWINE sets this when the status of job is changed, QGIS reads it
        float progress = 0.0f; //!< 0-100, UNTWINE sets this periodically as the job progress goes. QGIS reads it.
    }

    /**
     * Starts a new preflight step from the folder of files or a single point cloud file
     * \param uri single point cloud file readable by PDAL
     * \param outputDir folder to write point cloud buckets
     * \param options string map defining options/flags, empty for all defaults
     * \param feedback feedback for reporting progress, result of task and pass user request for cancellation
     */
    UNTWINE_EXPORT void PreFlightClustering(
        const std::string& uri,
        const std::string& outputDir,
        const std::map<std::string, std::string>& options,
        Feedback& feedback
    );

    /**
     * Starts a new bottom-up indexing
     * \param inputDir input directory from UNTWINE_PreFlight
     * \param outputDir folder to write EPT files
     * \param options string map defining options/flags, empty for all defaults
     * \param feedback feedback for reporting progress, result of task and pass user request for cancellation
     */
    UNTWINE_EXPORT void BottomUpIndexing(
        const std::string& inputDir,
        const std::string& outputDir,
        const std::map<std::string, std::string>& options,
        Feedback& feedback
    );

} // namespace Untwine

#endif // UNTWINE_H