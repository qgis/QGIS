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

#include "qgis_core.h"
#include "qgis.h"

/**
 * \ingroup core
 * Interface for logging messages from QGIS in GUI independent way.
 * This class provides abstraction of a tabbed window for showing messages to the user.
 * By default QgsMessageLogOutput will be used if not overridden with another
 * message log creator function.

 * QGIS application uses QgsMessageLog class for logging messages in a dockable
 * window for the user.
 *
 * QgsMessageLog is not usually directly created, but rather accessed through
 * QgsApplication::messageLog().
*/
class CORE_EXPORT QgsMessageLog : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsMessageLog.
     */
    QgsMessageLog() = default;

    /**
     * Adds a \a message to the log instance (and creates it if necessary).
     *
     * If \a notifyUser is TRUE, then the message should be brought to the user's attention by various UI hints.
     * If it is FALSE, the message should appear in logs silently. Note that log viewer implementations may
     * only respect notification hints for certain message levels.
     */
    static void logMessage( const QString &message, const QString &tag = QString(), Qgis::MessageLevel level = Qgis::Warning, bool notifyUser = true );

  signals:

    /**
     * Emitted whenever the log receives a \a message.
     *
     * This signal is emitted for all messages received by the log, regardless of the \a notifyUser flag's
     * value for the message.
     */
    void messageReceived( const QString &message, const QString &tag, Qgis::MessageLevel level );

    //TODO QGIS 4.0 - remove received argument

    /**
     * Emitted whenever the log receives a message which is not a Qgis::Info level message
     * and which has the \a notifyUser flag as TRUE.
     *
     * If QgsMessageLogNotifyBlocker objects have been created then this signal may be
     * temporarily suppressed.
     * \see QgsMessageLogNotifyBlocker
     */
    void messageReceived( bool received );

  private:

    void emitMessage( const QString &message, const QString &tag, Qgis::MessageLevel level, bool notifyUser = true );

    int mAdviseBlockCount = 0;

    friend class QgsMessageLogNotifyBlocker;

};

/**
 * Temporarily blocks the application QgsMessageLog (see QgsApplication::messageLog()) from emitting the messageReceived( bool )
 * signal for the lifetime of the object.
 *
 * Using this blocker allows messages to be logged without causing user interface hints flagging message log
 * errors to be created.
 *
 * QgsMessageLogNotifyBlocker supports "stacked" blocking, so two QgsMessageLogNotifyBlocker created
 * will both need to be destroyed before the messageReceived( bool ) signal is emitted again.
 *
 * \ingroup core
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsMessageLogNotifyBlocker
{
  public:

    /**
     * Constructor for QgsMessageLogNotifyBlocker.
     *
     * This will block the log from emitting the messageReceived( bool ) signal for the lifetime of this object.
     */
    QgsMessageLogNotifyBlocker();

    //! QgsMessageLogNotifyBlocker cannot be copied
    QgsMessageLogNotifyBlocker( const QgsMessageLogNotifyBlocker &other ) = delete;

    //! QgsMessageLogNotifyBlocker cannot be copied
    QgsMessageLogNotifyBlocker &operator=( const QgsMessageLogNotifyBlocker &other ) = delete;

    ~QgsMessageLogNotifyBlocker();

  private:

#ifdef SIP_RUN
    QgsMessageLogNotifyBlocker( const QgsMessageLogNotifyBlocker &other );
#endif
};

/**
 * \ingroup core
 * \brief Default implementation of message logging interface
 *
 * This class outputs log messages to the standard error. Therefore it might
 * be the right choice for applications without GUI.
 */
class CORE_EXPORT QgsMessageLogConsole : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsMessageLogConsole.
     */
    QgsMessageLogConsole();

  protected:

    /**
     * Formats a log message. Used by child classes.
     *
     * \param message the message to format
     * \param tag the tag of the message
     * \param level the log level of the message
     * \since QGIS 3.4
     */
    QString formatLogMessage( const QString &message, const QString &tag, Qgis::MessageLevel level = Qgis::Info ) const;

  public slots:

    /**
     * Logs a message to stderr.
     *
     * \param message the message to format
     * \param tag the tag of the message
     * \param level the log level of the message
     */
    virtual void logMessage( const QString &message, const QString &tag, Qgis::MessageLevel level );
};

#endif
