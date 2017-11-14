/***************************************************************************
                         qgsprocessingfeedback.h
                         -----------------------
    begin                : December 2016
    copyright            : (C) 2016 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGFEEDBACK_H
#define QGSPROCESSINGFEEDBACK_H

#include "qgis_core.h"
#include "qgsfeedback.h"
#include "qgsmessagelog.h"

/**
 * \class QgsProcessingFeedback
 * \ingroup core
 * Base class for providing feedback from a processing algorithm.
 *
 * This base class implementation silently ignores all feedback reported by algorithms.
 * Subclasses of QgsProcessingFeedback can be used to log this feedback or report
 * it to users via the GUI.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingFeedback : public QgsFeedback
{
    Q_OBJECT

  public:

    /**
     * Sets a progress report text string. This can be used in conjunction with
     * setProgress() to provide detailed progress reports, such as "Transformed
     * 4 of 5 layers".
     * \see setProgress()
     */
    virtual void setProgressText( const QString &text ) { Q_UNUSED( text ); }

    /**
     * Reports that the algorithm encountered an error which prevented it
     * from successfully executing.
     */
    virtual void reportError( const QString &error ) { QgsMessageLog::logMessage( error ); }

    /**
     * Pushes a general informational message from the algorithm. This can
     * be used to report feedback which is neither a status report or an
     * error, such as "Found 47 matching features".
     * \see pushCommandInfo()
     * \see pushDebugInfo()
     * \see pushConsoleInfo()
     */
    virtual void pushInfo( const QString &info ) { QgsMessageLog::logMessage( info ); }

    /**
     * Pushes an informational message containing a command from the algorithm.
     * This is usually used to report commands which are executed in an external
     * application or as subprocesses.
     * \see pushInfo()
     * \see pushDebugInfo()
     * \see pushConsoleInfo()
     */
    virtual void pushCommandInfo( const QString &info ) { QgsMessageLog::logMessage( info ); }

    /**
     * Pushes an informational message containing debugging helpers from
     * the algorithm.
     * \see pushInfo()
     * \see pushCommandInfo()
     * \see pushConsoleInfo()
     */
    virtual void pushDebugInfo( const QString &info ) { QgsMessageLog::logMessage( info ); }

    /**
     * Pushes a console feedback message from the algorithm. This is used to
     * report the output from executing an external command or subprocess.
     * \see pushInfo()
     * \see pushDebugInfo()
     * \see pushCommandInfo()
     */
    virtual void pushConsoleInfo( const QString &info ) { QgsMessageLog::logMessage( info ); }

};


#endif // QGSPROCESSINGFEEDBACK_H


