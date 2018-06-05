/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/gps/qgsnmeaconnection.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsNmeaConnection: QgsGpsConnection
{
%Docstring
Evaluates NMEA sentences coming from a GPS device

.. versionadded:: 1.4
%End

%TypeHeaderCode
#include "qgsnmeaconnection.h"
%End
  public:

    QgsNmeaConnection( QIODevice *device );
%Docstring
Constructs a QgsNmeaConnection with given ``device``.
%End

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
    void processGgaSentence( const char *data, int len );
%Docstring
process GGA sentence
%End
    void processRmcSentence( const char *data, int len );
%Docstring
process RMC sentence
%End
    void processGsvSentence( const char *data, int len );
%Docstring
process GSV sentence
%End
    void processVtgSentence( const char *data, int len );
%Docstring
process VTG sentence
%End
    void processGsaSentence( const char *data, int len );
%Docstring
process GSA sentence
%End
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/gps/qgsnmeaconnection.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
