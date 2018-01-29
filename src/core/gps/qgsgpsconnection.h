/***************************************************************************
                          qgsgpsconnection.h  -  description
                          -------------------
    begin                : November 30th, 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGPSCONNECTION_H
#define QGSGPSCONNECTION_H

#include <QDateTime>
#include "qgis.h"
#include <QObject>
#include <QString>

#include "qgis_core.h"

class QIODevice;

struct CORE_EXPORT QgsSatelliteInfo
{
  int id;
  bool inUse;
  int elevation;
  int azimuth;
  int signal;
};

struct CORE_EXPORT QgsGpsInformation
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

/**
 * \ingroup core
 * Abstract base class for connection to a GPS device*/
class CORE_EXPORT QgsGpsConnection : public QObject
{
#ifdef SIP_RUN
#include <qgsgpsdconnection.h>
#include <qgsnmeaconnection.h>
#endif


#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( sipCpp->inherits( "QgsGpsdConnection" ) )
      sipType = sipType_QgsGpsdConnection;
    else if ( sipCpp->inherits( "QgsNmeaConnection" ) )
      sipType = sipType_QgsNmeaConnection;
    else
      sipType = NULL;
    SIP_END
#endif

    Q_OBJECT
  public:

    enum Status
    {
      NotConnected,
      Connected,
      DataReceived,
      GPSDataReceived
    };

    /**
     * Constructor
        \param dev input device for the connection (e.g. serial device). The class takes ownership of the object
      */
    QgsGpsConnection( QIODevice *dev SIP_TRANSFER );
    ~QgsGpsConnection() override;
    //! Opens connection to device
    bool connect();
    //! Closes connection to device
    bool close();

    //! Sets the GPS source. The class takes ownership of the device class
    void setSource( QIODevice *source SIP_TRANSFER );

    //! Returns the status. Possible state are not connected, connected, data received
    Status status() const { return mStatus; }

    //! Returns the current gps information (lat, lon, etc.)
    QgsGpsInformation currentGPSInformation() const { return mLastGPSInformation; }

  signals:
    void stateChanged( const QgsGpsInformation &info );
    void nmeaSentenceReceived( const QString &substring ); // added to capture 'raw' data

  protected:
    //! Data source (e.g. serial device, socket, file,...)
    QIODevice *mSource = nullptr;
    //! Last state of the gps related variables (e.g. position, time, ...)
    QgsGpsInformation mLastGPSInformation;
    //! Connection status
    Status mStatus;

  private:
    //! Closes and deletes mSource
    void cleanupSource();
    void clearLastGPSInformation();

  protected slots:
    //! Parse available data source content
    virtual void parseData() = 0;
};

#endif // QGSGPSCONNECTION_H
