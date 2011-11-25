class QgsMessageLog
{
%TypeHeaderCode
#include <qgsmessagelog.h>
%End

  public:
    //! add a message to the instance (and create it if necessary)
    static void logMessage( QString message, QString tag = QString::null, int level = 0 );
};
