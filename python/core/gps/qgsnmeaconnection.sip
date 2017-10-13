/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/gps/qgsnmeaconnection.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsNMEAConnection: QgsGPSConnection
{
%Docstring
 Evaluates NMEA sentences coming from a GPS device
%End

%TypeHeaderCode
#include "qgsnmeaconnection.h"
%End
  public:
    QgsNMEAConnection( QIODevice *dev );

  protected slots:
    virtual void parseData();

%Docstring
Parse available data source content
%End

  protected:
    void processStringBuffer();
%Docstring
Splits mStringBuffer into sentences and calls libnmea
%End
    void processGGASentence( const char *data, int len );
    void processRMCSentence( const char *data, int len );
    void processGSVSentence( const char *data, int len );
    void processVTGSentence( const char *data, int len );
    void processGSASentence( const char *data, int len );
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/gps/qgsnmeaconnection.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
