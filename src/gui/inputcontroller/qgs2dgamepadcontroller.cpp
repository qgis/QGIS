/***************************************************************************
    qgs2dgamepadcontroller.cpp
    ---------------------
    begin                : March 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs2dgamepadcontroller.h"
#include "moc_qgs2dgamepadcontroller.cpp"

#ifdef HAVE_QTGAMEPAD

#include <QtGamepad/QGamepad>

QgsGamepad2DMapController::QgsGamepad2DMapController( int gamepadDeviceId, QObject *parent )
  : QgsAbstract2DMapController( parent )
  , mGamepadDeviceId( gamepadDeviceId )
{
  mGamepad = new QGamepad( gamepadDeviceId, this );

  // proxy raw signals for interested PyQGIS
  connect( mGamepad, &QGamepad::connectedChanged, this, &QgsGamepad2DMapController::connectedChanged );
  connect( mGamepad, &QGamepad::axisLeftXChanged, this, &QgsGamepad2DMapController::axisLeftXChanged );
  connect( mGamepad, &QGamepad::axisLeftYChanged, this, &QgsGamepad2DMapController::axisLeftYChanged );
  connect( mGamepad, &QGamepad::axisRightXChanged, this, &QgsGamepad2DMapController::axisRightXChanged );
  connect( mGamepad, &QGamepad::axisRightYChanged, this, &QgsGamepad2DMapController::axisRightYChanged );
  connect( mGamepad, &QGamepad::buttonAChanged, this, &QgsGamepad2DMapController::buttonAChanged );
  connect( mGamepad, &QGamepad::buttonBChanged, this, &QgsGamepad2DMapController::buttonBChanged );
  connect( mGamepad, &QGamepad::buttonXChanged, this, &QgsGamepad2DMapController::buttonXChanged );
  connect( mGamepad, &QGamepad::buttonYChanged, this, &QgsGamepad2DMapController::buttonYChanged );
  connect( mGamepad, &QGamepad::buttonL1Changed, this, &QgsGamepad2DMapController::buttonL1Changed );
  connect( mGamepad, &QGamepad::buttonR1Changed, this, &QgsGamepad2DMapController::buttonR1Changed );
  connect( mGamepad, &QGamepad::buttonL2Changed, this, &QgsGamepad2DMapController::buttonL2Changed );
  connect( mGamepad, &QGamepad::buttonR2Changed, this, &QgsGamepad2DMapController::buttonR2Changed );
  connect( mGamepad, &QGamepad::buttonSelectChanged, this, &QgsGamepad2DMapController::buttonSelectChanged );
  connect( mGamepad, &QGamepad::buttonStartChanged, this, &QgsGamepad2DMapController::buttonStartChanged );
  connect( mGamepad, &QGamepad::buttonL3Changed, this, &QgsGamepad2DMapController::buttonL3Changed );
  connect( mGamepad, &QGamepad::buttonR3Changed, this, &QgsGamepad2DMapController::buttonR3Changed );
  connect( mGamepad, &QGamepad::buttonUpChanged, this, &QgsGamepad2DMapController::buttonUpChanged );
  connect( mGamepad, &QGamepad::buttonDownChanged, this, &QgsGamepad2DMapController::buttonDownChanged );
  connect( mGamepad, &QGamepad::buttonLeftChanged, this, &QgsGamepad2DMapController::buttonLeftChanged );
  connect( mGamepad, &QGamepad::buttonRightChanged, this, &QgsGamepad2DMapController::buttonRightChanged );
  connect( mGamepad, &QGamepad::buttonCenterChanged, this, &QgsGamepad2DMapController::buttonCenterChanged );
  connect( mGamepad, &QGamepad::buttonGuideChanged, this, &QgsGamepad2DMapController::buttonGuideChanged );

  // also here we would make connections to the 2D map controller signals like zoomMap, depending on some
  // reasonable defaults and user defined QSettings
#if 0
  connect( mGamepad, &QGamepad::axisRightYChanged, this, [ = ]( double value )
  {
    // actually should be on a timer ;)
    emit zoomMap( value + 1 );
  } );
#endif
}

QgsGamepad2DMapController *QgsGamepad2DMapController::clone() const
{
  return new QgsGamepad2DMapController( mGamepadDeviceId );
}

QString QgsGamepad2DMapController::deviceId() const
{
  return QStringLiteral( "gamepad2d:%1" ).arg( mGamepadDeviceId );
}

bool QgsGamepad2DMapController::isConnected() const
{
  return mGamepad->isConnected();
}

QString QgsGamepad2DMapController::name() const
{
  return mGamepad->name();
}

double QgsGamepad2DMapController::axisLeftX() const
{
  return mGamepad->axisLeftX();
}

double QgsGamepad2DMapController::axisLeftY() const
{
  return mGamepad->axisLeftY();
}

double QgsGamepad2DMapController::axisRightX() const
{
  return mGamepad->axisRightX();
}

double QgsGamepad2DMapController::axisRightY() const
{
  return mGamepad->axisRightY();
}

bool QgsGamepad2DMapController::buttonA() const
{
  return mGamepad->buttonA();
}

bool QgsGamepad2DMapController::buttonB() const
{
  return mGamepad->buttonB();
}

bool QgsGamepad2DMapController::buttonX() const
{
  return mGamepad->buttonX();
}

bool QgsGamepad2DMapController::buttonY() const
{
  return mGamepad->buttonY();
}

bool QgsGamepad2DMapController::buttonL1() const
{
  return mGamepad->buttonL1();
}

bool QgsGamepad2DMapController::buttonR1() const
{
  return mGamepad->buttonR1();
}

double QgsGamepad2DMapController::buttonL2() const
{
  return mGamepad->buttonL2();
}

double QgsGamepad2DMapController::buttonR2() const
{
  return mGamepad->buttonR2();
}

bool QgsGamepad2DMapController::buttonSelect() const
{
  return mGamepad->buttonSelect();
}

bool QgsGamepad2DMapController::buttonStart() const
{
  return mGamepad->buttonStart();
}

bool QgsGamepad2DMapController::buttonL3() const
{
  return mGamepad->buttonL3();
}

bool QgsGamepad2DMapController::buttonR3() const
{
  return mGamepad->buttonR3();
}

bool QgsGamepad2DMapController::buttonUp() const
{
  return mGamepad->buttonUp();
}

bool QgsGamepad2DMapController::buttonDown() const
{
  return mGamepad->buttonDown();
}

bool QgsGamepad2DMapController::buttonLeft() const
{
  return mGamepad->buttonLeft();
}

bool QgsGamepad2DMapController::buttonRight() const
{
  return mGamepad->buttonRight();
}

bool QgsGamepad2DMapController::buttonCenter() const
{
  return mGamepad->buttonCenter();
}

bool QgsGamepad2DMapController::buttonGuide() const
{
  return mGamepad->buttonGuide();
}

#endif
