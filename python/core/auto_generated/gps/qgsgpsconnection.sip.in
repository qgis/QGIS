/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/gps/qgsgpsconnection.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






struct QgsSatelliteInfo
{
  int id;
  bool inUse;
  int elevation;
  int azimuth;
  int signal;
};

struct QgsGpsInformation
{
  double latitude;
  double longitude;
  double elevation;
  double speed; //in km/h
  double direction;
  QList<QgsSatelliteInfo> satellitesInView;
  double pdop;
  double hdop;
  double vdop;
  double hacc; //horizontal accuracy in meters
  double vacc; //vertical accuracy in meters
  QDateTime utcDateTime;
  QChar fixMode;
  int fixType;
  int quality; // from GPGGA
  int satellitesUsed; // from GPGGA
  QChar status; // from GPRMC A,V
  QList<int> satPrn; // list of SVs in use; needed for QgsSatelliteInfo.inUse and other uses
  bool satInfoComplete; // based on GPGSV sentences - to be used to determine when to graph signal and satellite position
};

class QgsGpsConnection : QObject
{
%Docstring
Abstract base class for connection to a GPS device*
%End

%TypeHeaderCode
#include "qgsgpsconnection.h"
#include <qgsgpsdconnection.h>
#include <qgsnmeaconnection.h>
%End
%ConvertToSubClassCode
    if ( sipCpp->inherits( "QgsGpsdConnection" ) )
      sipType = sipType_QgsGpsdConnection;
    else if ( sipCpp->inherits( "QgsNmeaConnection" ) )
      sipType = sipType_QgsNmeaConnection;
    else
      sipType = NULL;
%End
  public:

    enum Status
    {
      NotConnected,
      Connected,
      DataReceived,
      GPSDataReceived
    };

    QgsGpsConnection( QIODevice *dev /Transfer/ );
%Docstring
Constructor

:param dev: input device for the connection (e.g. serial device). The class takes ownership of the object
%End
    ~QgsGpsConnection();
    bool connect();
%Docstring
Opens connection to device
%End
    bool close();
%Docstring
Closes connection to device
%End

    void setSource( QIODevice *source /Transfer/ );
%Docstring
Sets the GPS source. The class takes ownership of the device class
%End

    Status status() const;
%Docstring
Returns the status. Possible state are not connected, connected, data received
%End

    QgsGpsInformation currentGPSInformation() const;
%Docstring
Returns the current gps information (lat, lon, etc.)
%End

  signals:
    void stateChanged( const QgsGpsInformation &info );
    void nmeaSentenceReceived( const QString &substring ); // added to capture 'raw' data

  protected:

  protected slots:
    virtual void parseData() = 0;
%Docstring
Parse available data source content
%End
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/gps/qgsgpsconnection.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
