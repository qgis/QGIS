/***************************************************************************
                          qgsnmeaconnection.h  -  description
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

#ifndef QGSNMEACONNECTION_H
#define QGSNMEACONNECTION_H

#include "qgsgpsconnection.h"

/**Evaluates NMEA sentences coming from a GPS device*/
class CORE_EXPORT QgsNMEAConnection: public QgsGPSConnection
{
    Q_OBJECT
  public:
    QgsNMEAConnection( QIODevice* dev, int pollInterval = 1000 );
    QgsNMEAConnection( QString port, int pollInterval = 1000 );
    ~QgsNMEAConnection();

    //bool poll( QgsGPSInformation& info, int maxTime );

  protected slots:
    /**Parse available data source content*/
    void parseData();

  private:
    /**Store data from the device before it is processed*/
    QString mStringBuffer;
    /**Splits mStringBuffer into sentences and calls libnmea*/
    void processStringBuffer();
    //handle the different sentence type
    void processGGASentence( const char* data, int len );
    void processRMCSentence( const char* data, int len );
    void processGSVSentence( const char* data, int len );
    void processVTGSentence( const char* data, int len );
    void processGSASentence( const char* data, int len );
};

#endif // QGSNMEACONNECTION_H
