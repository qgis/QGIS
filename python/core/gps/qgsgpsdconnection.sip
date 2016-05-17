/** Evaluates NMEA sentences coming from gpsd*/
class QgsGpsdConnection: QgsNMEAConnection
{
%TypeHeaderCode
#include "qgsgpsdconnection.h"
%End
  public:
    QgsGpsdConnection( const QString& host, qint16 port, const QString& device );
    ~QgsGpsdConnection();
};
