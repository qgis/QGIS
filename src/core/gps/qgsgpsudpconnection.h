/***************************************************************************
                          qgsgpsudpconnection.h  -  description
                          -------------------
    begin                : January 22nd, 2014
    copyright            : (C) 2010 by Jürgen E. Fischer, norBIT GmbH
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGPSUDPCONNECTION_H
#define QGSGPSUDPCONNECTION_H

#include "qgsnmeaconnection.h"

#include <QAbstractSocket>
#include <QTimer>

#include <QIODevice>
#include <QApplication>
#include <QStringList>


//from libnmea
#include "parse.h"
#include "gmath.h"
#include "info.h"

#define KNOTS_TO_KMH 1.852

/**Evaluates NMEA sentences coming from UDP*/
class CORE_EXPORT QgsGpsUdpConnection: public QgsGPSConnection
{
    Q_OBJECT
  public:
//     QgsGpsUdpConnection( QString host, qint16 port, QString device);
      QgsGpsUdpConnection(qint16 port);
    ~QgsGpsUdpConnection();
    QTimer timer;
    
    protected slots:
    void parseData();
    private slots:
    void error( QAbstractSocket::SocketError );
    protected:
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
//JT

  private:
    QString mDevice;
};

#endif // QGSGPSUDPCONNECTION_H
