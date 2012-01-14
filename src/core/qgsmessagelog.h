/***************************************************************************
    qgsmessagelog.h  -  interface for logging messages
    ----------------------
    begin                : October 2011
    copyright            : (C) 2011 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESSAGELOG_H
#define QGSMESSAGELOG_H

#include <QString>
#include <QObject>

/** \ingroup core
 * Interface for logging messages from QGIS in GUI independent way.
 * This class provides abstraction of a tabbed window for showing messages to the user.
 * By default QgsMessageLogOutput will be used if not overridden with another
 * message log creator function.

 * QGIS application uses QgsMessageLog class for logging messages in a dockable
 * window for the user.

 * \note added in 1.9
*/
class CORE_EXPORT QgsMessageLog
{
  public:
    //! add a message to the instance (and create it if necessary)
    static void logMessage( QString message, QString tag = QString::null, int level = 0 );

    //! set log message
    static void setLogger( void ( *logger )( QString message, QString tag, int level ) );

  private:
    //! function
    static void ( *gmLogger )( QString message, QString tag, int level );
};


/**
\brief Default implementation of message logging interface

This class outputs log messages to the standard output. Therefore it might
be the right choice for apps without GUI.
*/
class CORE_EXPORT QgsMessageLogConsole
{
  public:
    static void logger( QString message, QString tag = QString::null, int level = 0 );
};

#endif
