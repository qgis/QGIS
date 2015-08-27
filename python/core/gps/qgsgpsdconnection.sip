/** Evaluates NMEA sentences coming from gpsd*/
class QgsGpsdConnection: QgsNMEAConnection
{
%TypeHeaderCode
#include "qgsgpsdconnection.h"
%End
  public:
    QgsGpsdConnection( QString host, qint16 port, QString device );
    ~QgsGpsdConnection();
};
