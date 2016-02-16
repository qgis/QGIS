struct QgsSatelliteInfo
{
%TypeHeaderCode
#include <qgsgpsconnection.h>
%End

  int id;
  bool inUse;
  int elevation;
  int azimuth;
  int signal;
};

struct QgsGPSInformation
{
%TypeHeaderCode
#include <qgsgpsconnection.h>
%End

  double latitude;
  double longitude;
  double elevation;
  double speed; //in km/h
  double direction;
  QList<QgsSatelliteInfo> satellitesInView;
  double pdop;
  double hdop;
  double vdop;
  double hacc; //horizontal accurancy in meters
  double vacc; //vertical accurancy in meters
  QDateTime utcDateTime;
  QChar fixMode;
  int fixType;
  int quality; // from GPGGA
  int satellitesUsed; // from GPGGA
  QChar status; // from GPRMC A,V
  QList<int> satPrn; // list of SVs in use; needed for QgsSatelliteInfo.inUse and other uses
  bool satInfoComplete; // based on GPGSV sentences - to be used to determine when to graph signal and satellite position
};

/** Abstract base class for connection to a GPS device*/
class QgsGPSConnection : QObject
{
%TypeHeaderCode
#include <qgsgpsconnection.h>
#include <qgsgpsdconnection.h>
#include <qgsnmeaconnection.h>
%End

%ConvertToSubClassCode
  if (sipCpp->inherits("QgsGpsdConnection"))
    sipType = sipType_QgsGpsdConnection;
  else if (sipCpp->inherits("QgsNMEAConnection"))
    sipType = sipType_QgsNMEAConnection;
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

    /** Constructor
        @param dev input device for the connection (e.g. serial device). The class takes ownership of the object
      */
    QgsGPSConnection( QIODevice* dev /Transfer/ );
    virtual ~QgsGPSConnection();
    /** Opens connection to device*/
    bool connect();
    /** Closes connection to device*/
    bool close();

    /** Sets the GPS source. The class takes ownership of the device class*/
    void setSource( QIODevice* source /Transfer/ );

    /** Returns the status. Possible state are not connected, connected, data received*/
    Status status() const;

    /** Returns the current gps information (lat, lon, etc.)*/
    QgsGPSInformation currentGPSInformation() const;

  signals:
    void stateChanged( const QgsGPSInformation& info );
    void nmeaSentenceReceived( const QString& substring ); // added to capture 'raw' data

  protected slots:
    /** Parse available data source content*/
    virtual void parseData() = 0;
};
