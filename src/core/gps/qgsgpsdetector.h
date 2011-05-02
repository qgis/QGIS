/***************************************************************************
                          qgsgpsdetector.h  -  description
                          -------------------
    begin                : January 13th, 2009
    copyright            : (C) 2009 by Juergen E. Fischer
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

#ifndef QGSGPSDETECTOR_H
#define QGSGPSDETECTOR_H

#include <QObject>
#include <QList>
#include <QPair>

#include "qextserialport.h"

class QgsGPSConnection;
struct QgsGPSInformation;

// Class to detect the GPS port
class CORE_EXPORT QgsGPSDetector : public QObject
{
    Q_OBJECT
  public:
    QgsGPSDetector( QString portName );
    ~QgsGPSDetector();

    static QList< QPair<QString, QString> > availablePorts();

  public slots:
    void advance();
    void detected( const QgsGPSInformation& );
    void connDestroyed( QObject * );

  signals:
    void detected( QgsGPSConnection * );
    void detectionFailed();

  private:
    int mPortIndex;
    int mBaudIndex;
    QList< QPair< QString, QString > > mPortList;
    QList<BaudRateType> mBaudList;

    QgsGPSConnection *mConn;
};

#endif // QGSGPSDETECTOR_H
