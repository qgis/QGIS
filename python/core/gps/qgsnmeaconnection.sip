/** Evaluates NMEA sentences coming from a GPS device*/
class QgsNMEAConnection: QgsGPSConnection {
%TypeHeaderCode
#include "qgsnmeaconnection.h"
%End
  public:
    QgsNMEAConnection( QIODevice *dev );
    ~QgsNMEAConnection();

  protected slots:
    /** Parse available data source content*/
    void parseData();

  protected:
    /** Splits mStringBuffer into sentences and calls libnmea*/
    void processStringBuffer();
    //handle the different sentence type
    void processGGASentence( const char* data, int len );
    void processRMCSentence( const char* data, int len );
    void processGSVSentence( const char* data, int len );
    void processVTGSentence( const char* data, int len );
    void processGSASentence( const char* data, int len );
};
