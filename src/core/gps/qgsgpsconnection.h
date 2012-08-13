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
#include <QObject>
#include <QString>

class QIODevice;

struct CORE_EXPORT QgsSatelliteInfo
{
  int id;
  bool inUse;
  int elevation;
  int azimuth;
  int signal;
};

struct CORE_EXPORT QgsGPSInformation
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
  double hacc; //horizontal accurancy in meters
  double vacc; //vertical accurancy in meters
  QDateTime utcDateTime;
  QChar fixMode;
  int fixType;
  int quality;      // from GPGGA
  int satellitesUsed; // from GPGGA
  QChar status;     // from GPRMC A,V
  QList<int>satPrn; // list of SVs in use; needed for QgsSatelliteInfo.inUse and other uses
  bool satInfoComplete;  // based on GPGSV sentences - to be used to determine when to graph signal and satellite position
};

/**Abstract base class for connection to a GPS device*/
class CORE_EXPORT QgsGPSConnection : public QObject
{
    Q_OBJECT
  public:

    enum Status
    {
      NotConnected,
      Connected,
      DataReceived,
      GPSDataReceived
    };

    /**Constructor
        @param dev input device for the connection (e.g. serial device). The class takes ownership of the object
      */
    QgsGPSConnection( QIODevice* dev );
    virtual ~QgsGPSConnection();
    /**Opens connection to device*/
    bool connect();
    /**Closes connection to device*/
    bool close();

    /**Sets the GPS source. The class takes ownership of the device class*/
    void setSource( QIODevice* source );

    /**Returns the status. Possible state are not connected, connected, data received*/
    Status status() const { return mStatus; }

    /**Returns the current gps information (lat, lon, etc.)*/
    QgsGPSInformation currentGPSInformation() const { return mLastGPSInformation; }

  signals:
    void stateChanged( const QgsGPSInformation& info );
    void nmeaSentenceReceived( const QString& substring );  // added to capture 'raw' data

  protected:
    /**Data source (e.g. serial device, socket, file,...)*/
    QIODevice* mSource;
    /**Last state of the gps related variables (e.g. position, time, ...)*/
    QgsGPSInformation mLastGPSInformation;
    /**Connection status*/
    Status mStatus;

  private:
    /**Closes and deletes mSource*/
    void cleanupSource();
    void clearLastGPSInformation();

  protected slots:
    /**Parse available data source content*/
    virtual void parseData() = 0;
};

#endif // QGSGPSCONNECTION_H
