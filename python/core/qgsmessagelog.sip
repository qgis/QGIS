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
