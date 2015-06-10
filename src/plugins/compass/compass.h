/***************************************************************************
                          compass.h
 Functions:               Reads data from a QtMobility QCompass
                             -------------------
    begin                : Jan 28, 2012
    copyright            : (C) 2012 by Marco Bernasocchi
    email                : marco@bernawebdesign.ch

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _COMPASS_H__
#define _COMPASS_H__

#include <QtSensors/QCompass>
#include <QObject>

QTM_USE_NAMESPACE

class Compass : public QObject, public QCompassFilter
{
    Q_OBJECT
  public:
    Compass();
    ~Compass();
    bool filter( QCompassReading *reading );
    bool isActive();
    bool start();
    bool stop();

  private:
    qtimestamp stamp;
    int  mRateVal;
    QCompass mSensor;

  signals:
    void azimuthChanged( const QVariant &azimuth, const QVariant &calibrationLevel );
};

#endif // _COMPASS_H__
