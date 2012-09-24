class QgsMessageLog
{
%TypeHeaderCode
#include <qgsmessagelog.h>
%End

  public:
    static QgsMessageLog *instance();

    //! add a message to the instance (and create it if necessary)
    static void logMessage( QString message, QString tag = QString::null, int level = 0 );

  signals:
    void messageReceived( QString message, QString tag, int level );

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
    void logMessage( QString message, QString tag, int level );
};
