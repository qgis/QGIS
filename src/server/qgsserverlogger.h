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

#define SIP_NO_FILE


#include "qgsmessagelog.h"

#include <QFile>
#include <QObject>
#include <QString>
#include <QTextStream>

//! Writes message log into server logfile
class QgsServerLogger: public QObject
{
    Q_OBJECT
  public:

    /**
     * Get the singleton instance
     */
    static QgsServerLogger *instance();

    /**
     * Get the current log level
     * \returns the log level
     * \since QGIS 3.0
     */
    QgsMessageLog::MessageLevel logLevel() const { return mLogLevel; }

    /**
      * Set the current log level
      * \param level the log level
      * \since QGIS 3.0
      */
    void setLogLevel( QgsMessageLog::MessageLevel level );

    /**
      * Set the current log file
      */
    void setLogFile( const QString &f );

  public slots:

    /**
     * Log a message from the server context
     *
     * \param message the message
     * \param tag tag of the message
     * \param level log level of the message
     */
    void logMessage( const QString &message, const QString &tag, QgsMessageLog::MessageLevel level );

  protected:
    QgsServerLogger();

  private:
    static QgsServerLogger *sInstance;

    QFile mLogFile;
    QTextStream mTextStream;
    QgsMessageLog::MessageLevel mLogLevel = QgsMessageLog::NONE;
};

#endif // QGSSERVERLOGGER_H
