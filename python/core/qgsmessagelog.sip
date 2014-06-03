class QgsMessageLog : QObject
{
%TypeHeaderCode
#include <qgsmessagelog.h>
%End

  public:
    static QgsMessageLog *instance();

    enum MessageLevel
    {
      INFO,
      WARNING,
      CRITICAL,
    };

    //! add a message to the instance (and create it if necessary)
    static void logMessage( QString message, QString tag = QString::null, MessageLevel level = WARNING );

  signals:
    void messageReceived( QString message, QString tag, MessageLevel level );

  private:
    QgsMessageLog();
};


/**
\brief Default implementation of message logging interface

This class outputs log messages to the standard output. Therefore it might
be the right choice for apps without GUI.
*/
class QgsMessageLogConsole : QObject
{
%TypeHeaderCode
#include <qgsmessagelog.h>
%End

  public:
    QgsMessageLogConsole();

  public slots:
    void logMessage( QString message, QString tag, QgsMessageLog::MessageLevel level );
};
