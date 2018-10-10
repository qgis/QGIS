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

#include "qgis_core.h"

class QgsGpsConnection;
struct QgsGpsInformation;

/**
 * \ingroup core
 * Class to detect the GPS port
 */
class CORE_EXPORT QgsGpsDetector : public QObject
{
    Q_OBJECT
  public:
    QgsGpsDetector( const QString &portName );
    ~QgsGpsDetector() override;

    static QList< QPair<QString, QString> > availablePorts();

  public slots:
    void advance();
    void detected( const QgsGpsInformation & );
    void connDestroyed( QObject * );

  signals:
    void detected( QgsGpsConnection * );
    void detectionFailed();

  private:
    int mPortIndex;
    int mBaudIndex;
    QList< QPair< QString, QString > > mPortList;
    QList<qint32> mBaudList;

    QgsGpsConnection *mConn = nullptr;
};

#endif // QGSGPSDETECTOR_H
