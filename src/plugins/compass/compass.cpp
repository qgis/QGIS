/***************************************************************************
                          compass.cpp
 Functions:
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

#include "compass.h"

Compass::Compass()
{
//  mRateVal = 0;
//  if (mRateVal > 0)
//  {
//    qDebug() << "Compasssensor setdatarate " << endl;
//    mSensor.setDataRate(mRateVal);
//  }
//  qDebug() << "Data rate 2: " << mSensor.dataRate();
  mSensor.addFilter( this );
  start();
}

Compass::~Compass()
{
}

bool Compass::filter( QCompassReading *reading )
{
//    int diff = ( reading->timestamp() - stamp );
//    stamp = reading->timestamp();

//    QString str;
//    str = QString("%1 deg (%2 CalibLevel)")
//        .arg(reading->azimuth(), 3, 'f', 0)
//        .arg(reading->calibrationLevel(), 3, 'f', 0);
//    qDebug() << str << endl;

  emit azimuthChanged( reading->azimuth(), reading->calibrationLevel() );
  return false; // don't store the reading in the sensor
}

bool Compass::isActive()
{
  return mSensor.isActive();
}

bool Compass::start()
{
  mSensor.start();
  if ( !mSensor.isActive() )
  {
    qDebug() << "Compasssensor didn't start!" << endl;
    return false;
  }
  return true;
}

bool Compass::stop()
{
  mSensor.stop();
  if ( mSensor.isActive() )
  {
    qDebug() << "Compasssensor didn't stop!" << endl;
    return false;
  }
  return true;
}
