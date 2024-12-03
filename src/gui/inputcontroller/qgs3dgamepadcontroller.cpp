/***************************************************************************
    qgs3dgamepadcontroller.cpp
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

#include "qgs3dgamepadcontroller.h"
#include "moc_qgs3dgamepadcontroller.cpp"

#ifdef HAVE_QTGAMEPAD

#include <QtGamepad/QGamepad>

QgsGamepad3DMapController::QgsGamepad3DMapController( int gamepadDeviceId, QObject *parent )
  : QgsAbstract3DMapController( parent )
  , mGamepadDeviceId( gamepadDeviceId )
{
  mGamepad = new QGamepad( gamepadDeviceId, this );

  // proxy raw signals for interested PyQGIS
  connect( mGamepad, &QGamepad::connectedChanged, this, &QgsGamepad3DMapController::connectedChanged );
  connect( mGamepad, &QGamepad::axisLeftXChanged, this, &QgsGamepad3DMapController::axisLeftXChanged );
  connect( mGamepad, &QGamepad::axisLeftYChanged, this, &QgsGamepad3DMapController::axisLeftYChanged );
  connect( mGamepad, &QGamepad::axisRightXChanged, this, &QgsGamepad3DMapController::axisRightXChanged );
  connect( mGamepad, &QGamepad::axisRightYChanged, this, &QgsGamepad3DMapController::axisRightYChanged );
  connect( mGamepad, &QGamepad::buttonAChanged, this, &QgsGamepad3DMapController::buttonAChanged );
  connect( mGamepad, &QGamepad::buttonBChanged, this, &QgsGamepad3DMapController::buttonBChanged );
  connect( mGamepad, &QGamepad::buttonXChanged, this, &QgsGamepad3DMapController::buttonXChanged );
  connect( mGamepad, &QGamepad::buttonYChanged, this, &QgsGamepad3DMapController::buttonYChanged );
  connect( mGamepad, &QGamepad::buttonL1Changed, this, &QgsGamepad3DMapController::buttonL1Changed );
  connect( mGamepad, &QGamepad::buttonR1Changed, this, &QgsGamepad3DMapController::buttonR1Changed );
  connect( mGamepad, &QGamepad::buttonL2Changed, this, &QgsGamepad3DMapController::buttonL2Changed );
  connect( mGamepad, &QGamepad::buttonR2Changed, this, &QgsGamepad3DMapController::buttonR2Changed );
  connect( mGamepad, &QGamepad::buttonSelectChanged, this, &QgsGamepad3DMapController::buttonSelectChanged );
  connect( mGamepad, &QGamepad::buttonStartChanged, this, &QgsGamepad3DMapController::buttonStartChanged );
  connect( mGamepad, &QGamepad::buttonL3Changed, this, &QgsGamepad3DMapController::buttonL3Changed );
  connect( mGamepad, &QGamepad::buttonR3Changed, this, &QgsGamepad3DMapController::buttonR3Changed );
  connect( mGamepad, &QGamepad::buttonUpChanged, this, &QgsGamepad3DMapController::buttonUpChanged );
  connect( mGamepad, &QGamepad::buttonDownChanged, this, &QgsGamepad3DMapController::buttonDownChanged );
  connect( mGamepad, &QGamepad::buttonLeftChanged, this, &QgsGamepad3DMapController::buttonLeftChanged );
  connect( mGamepad, &QGamepad::buttonRightChanged, this, &QgsGamepad3DMapController::buttonRightChanged );
  connect( mGamepad, &QGamepad::buttonCenterChanged, this, &QgsGamepad3DMapController::buttonCenterChanged );
  connect( mGamepad, &QGamepad::buttonGuideChanged, this, &QgsGamepad3DMapController::buttonGuideChanged );

  // also here we would make connections to the 2D map controller signals like zoomMap, depending on some
  // reasonable defaults and user defined QSettings
}

QgsGamepad3DMapController *QgsGamepad3DMapController::clone() const
{
  return new QgsGamepad3DMapController( mGamepadDeviceId );
}

QString QgsGamepad3DMapController::deviceId() const
{
  return QStringLiteral( "gamepad3d:%1" ).arg( mGamepadDeviceId );
}

bool QgsGamepad3DMapController::isConnected() const
{
  return mGamepad->isConnected();
}

QString QgsGamepad3DMapController::name() const
{
  return mGamepad->name();
}

double QgsGamepad3DMapController::axisLeftX() const
{
  return mGamepad->axisLeftX();
}

double QgsGamepad3DMapController::axisLeftY() const
{
  return mGamepad->axisLeftY();
}

double QgsGamepad3DMapController::axisRightX() const
{
  return mGamepad->axisRightX();
}

double QgsGamepad3DMapController::axisRightY() const
{
  return mGamepad->axisRightY();
}

bool QgsGamepad3DMapController::buttonA() const
{
  return mGamepad->buttonA();
}

bool QgsGamepad3DMapController::buttonB() const
{
  return mGamepad->buttonB();
}

bool QgsGamepad3DMapController::buttonX() const
{
  return mGamepad->buttonX();
}

bool QgsGamepad3DMapController::buttonY() const
{
  return mGamepad->buttonY();
}

bool QgsGamepad3DMapController::buttonL1() const
{
  return mGamepad->buttonL1();
}

bool QgsGamepad3DMapController::buttonR1() const
{
  return mGamepad->buttonR1();
}

double QgsGamepad3DMapController::buttonL2() const
{
  return mGamepad->buttonL2();
}

double QgsGamepad3DMapController::buttonR2() const
{
  return mGamepad->buttonR2();
}

bool QgsGamepad3DMapController::buttonSelect() const
{
  return mGamepad->buttonSelect();
}

bool QgsGamepad3DMapController::buttonStart() const
{
  return mGamepad->buttonStart();
}

bool QgsGamepad3DMapController::buttonL3() const
{
  return mGamepad->buttonL3();
}

bool QgsGamepad3DMapController::buttonR3() const
{
  return mGamepad->buttonR3();
}

bool QgsGamepad3DMapController::buttonUp() const
{
  return mGamepad->buttonUp();
}

bool QgsGamepad3DMapController::buttonDown() const
{
  return mGamepad->buttonDown();
}

bool QgsGamepad3DMapController::buttonLeft() const
{
  return mGamepad->buttonLeft();
}

bool QgsGamepad3DMapController::buttonRight() const
{
  return mGamepad->buttonRight();
}

bool QgsGamepad3DMapController::buttonCenter() const
{
  return mGamepad->buttonCenter();
}

bool QgsGamepad3DMapController::buttonGuide() const
{
  return mGamepad->buttonGuide();
}

#endif
