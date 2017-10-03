/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsmessagelog.h                                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsMessageLog : QObject
{
%Docstring
 Interface for logging messages from QGIS in GUI independent way.
 This class provides abstraction of a tabbed window for showing messages to the user.
 By default QgsMessageLogOutput will be used if not overridden with another
 message log creator function.

 QGIS application uses QgsMessageLog class for logging messages in a dockable
 window for the user.

 QgsMessageLog is not usually directly created, but rather accessed through
 QgsApplication.messageLog().
%End

%TypeHeaderCode
#include "qgsmessagelog.h"
%End
  public:

    enum MessageLevel
    {
      ALL,
      INFO,
      WARNING,
      CRITICAL,
      NONE
    };

    QgsMessageLog();
%Docstring
 Constructor for QgsMessageLog.
%End

    static void logMessage( const QString &message, const QString &tag = QString(), MessageLevel level = QgsMessageLog::WARNING );
%Docstring
add a message to the instance (and create it if necessary)
%End

  signals:
    void messageReceived( const QString &message, const QString &tag, QgsMessageLog::MessageLevel level );

    void messageReceived( bool received );

};

class QgsMessageLogConsole : QObject
{
%Docstring
 Default implementation of message logging interface

This class outputs log messages to the standard output. Therefore it might
be the right choice for apps without GUI.
%End

%TypeHeaderCode
#include "qgsmessagelog.h"
%End
  public:
    QgsMessageLogConsole();

  public slots:
    void logMessage( const QString &message, const QString &tag, QgsMessageLog::MessageLevel level );
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsmessagelog.h                                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
