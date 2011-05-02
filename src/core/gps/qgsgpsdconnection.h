/***************************************************************************
                          qgsgpsdconnection.h  -  description
                          -------------------
    begin                : October 4th, 2010
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

#ifndef QGSGPSDCONNECTION_H
#define QGSGPSDCONNECTION_H

#include "qgsnmeaconnection.h"

#include <QAbstractSocket>

/**Evaluates NMEA sentences coming from gpsd*/
class CORE_EXPORT QgsGpsdConnection: public QgsNMEAConnection
{
    Q_OBJECT
  public:
    QgsGpsdConnection( QString host, qint16 port, QString device );
    ~QgsGpsdConnection();

  private slots:
    void connected();
    void error( QAbstractSocket::SocketError );

  private:
    QString mDevice;
};

#endif // QGSGPSDCONNECTION_H
