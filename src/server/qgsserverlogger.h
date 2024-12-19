/***************************************************************************
                              qgsserverlogger.h
                              -----------------
  begin                : May 5, 2014
  copyright            : (C) 2014 by Marco Hugentobler
  email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSERVERLOGGER_H
#define QGSSERVERLOGGER_H


#include "qgsmessagelog.h"

#include <QFile>
#include <QObject>
#include <QString>
#include <QTextStream>
#include "qgis_server.h"

/**
 * \ingroup server
 * \brief Writes message log into server logfile
 */
class SERVER_EXPORT QgsServerLogger : public QgsMessageLogConsole
{
    Q_OBJECT
  public:
    /**
     * Gets the singleton instance
     */
    static QgsServerLogger *instance();

    /**
     * Gets the current log level
     * \returns the log level
     */
    Qgis::MessageLevel logLevel() const { return mLogLevel; }

    /**
      * Set the current log level
      * \param level the log level
      */
    void setLogLevel( Qgis::MessageLevel level );

    /**
      * Set the current log file
      */
    void setLogFile( const QString &filename = QString() );

    /**
     * Activates logging to stderr.
     * \since QGIS 3.4
     */
    void setLogStderr();

  public slots:

    /**
     * Log a message from the server context
     *
     * \param message the message
     * \param tag tag of the message
     * \param level log level of the message
     */
    void logMessage( const QString &message, const QString &tag, Qgis::MessageLevel level ) override;

  protected:
    QgsServerLogger();

  private:
    static QgsServerLogger *sInstance;

    QFile mLogFile;
    bool mLogStderr = false;
    QTextStream mTextStream;
    Qgis::MessageLevel mLogLevel = Qgis::MessageLevel::NoLevel;
};

#endif // QGSSERVERLOGGER_H
