/***************************************************************************
                          qgsgpsdconnection.h  -  description
                          -------------------
    begin                : October 4th, 2010
    copyright            : (C) 2010 by Jürgen E. Fischer, norBIT GmbH
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGPSDCONNECTION_H
#define QGSGPSDCONNECTION_H

#include "qgis_core.h"
#include "qgsnmeaconnection.h"

#include <QAbstractSocket>

/**
 * \ingroup core
 * Evaluates NMEA sentences coming from gpsd
 */
class CORE_EXPORT QgsGpsdConnection: public QgsNmeaConnection
{
    Q_OBJECT
  public:
    QgsGpsdConnection( const QString &host, qint16 port, const QString &device );

  private slots:
    void connected();
    void error( QAbstractSocket::SocketError );

  private:
    QString mDevice;
};

#endif // QGSGPSDCONNECTION_H
