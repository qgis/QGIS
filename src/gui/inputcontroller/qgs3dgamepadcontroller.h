/***************************************************************************
    qgs3dgamepadcontroller.h
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

#ifndef QGS3DGAMEPADCONTROLLER_H
#define QGS3DGAMEPADCONTROLLER_H

#include "qgis_sip.h"
#include "qgsconfig.h"

SIP_IF_MODULE( HAVE_QTGAMEPAD )

#ifdef HAVE_QTGAMEPAD

#include "qgis_gui.h"
#include "qgis.h"
#include "qgs3dmapcontroller.h"

#include <QPointer>

#ifdef SIP_RUN
// this is needed for the "convert to subclass" code below to compile
//%ModuleHeaderCode
#include "qgs3dgamepadcontroller.h"
//%End
#endif

class QGamepad;


/**
 * \ingroup gui
 * \class QgsGamepad3DMapController
 * \brief Represents a gamepad device used for controlling a 3D map.
 *
 * \since QGIS 3.34
 */
class GUI_EXPORT QgsGamepad3DMapController : public QgsAbstract3DMapController
{
    Q_OBJECT

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( qobject_cast<QgsGamepad3DMapController *>( sipCpp ) )

      sipType = sipType_QgsGamepad3DMapController;
    else
      sipType = nullptr;
    SIP_END
#endif

  public:
    /**
     * Constructor for QgsGamepad3DMapController, with the specified \a gamepadDeviceId and \a parent object.
     */
    QgsGamepad3DMapController( int gamepadDeviceId, QObject *parent SIP_TRANSFERTHIS = nullptr );

    QgsGamepad3DMapController *clone() const override SIP_FACTORY;
    QString deviceId() const override;

    // proxy QGamepad signals and properties here, as QGamepad isn't accessible from Python.
    // Ideally we would share these with QgsGamepad3DMapController, but that would create a diamond
    // inheritance

    /**
     * Returns TRUE if the gamepad is connected.
     */
    bool isConnected() const;

    /**
     * Returns the reported name of the gamepad if one is available.
     */
    QString name() const;

    /**
     * Returns the value of the left thumbstick's X axis. The axis values range from -1.0 to 1.0.
     */
    double axisLeftX() const;

    /**
     * Returns the value of the left thumbstick's Y axis. The axis values range from -1.0 to 1.0.
     */
    double axisLeftY() const;

    /**
     * Returns the value of the right thumbstick's X axis. The axis values range from -1.0 to 1.0.
     */
    double axisRightX() const;

    /**
     * Returns the value of the right thumbstick's Y axis. The axis values range from -1.0 to 1.0.
     */
    double axisRightY() const;

    /**
     * Returns the state of the A button. The value is TRUE when pressed, and FALSE when not pressed.
     */
    bool buttonA() const;

    /**
     * Returns the state of the B button. The value is TRUE when pressed, and FALSE when not pressed.
     */
    bool buttonB() const;

    /**
     * Returns the state of the X button. The value is TRUE when pressed, and FALSE when not pressed.
     */
    bool buttonX() const;

    /**
     * Returns the state of the Y button. The value is TRUE when pressed, and FALSE when not pressed.
     */
    bool buttonY() const;

    /**
     * Returns the state of the left shoulder button. The value is TRUE when pressed, and FALSE when not pressed.
     */
    bool buttonL1() const;

    /**
     * Returns the state of the right shoulder button. The value is TRUE when pressed, and FALSE when not pressed.
     */
    bool buttonR1() const;

    /**
     * Returns the value of the left trigger button. This trigger value ranges from 0.0 when not pressed to 1.0 when pressed completely.
     */
    double buttonL2() const;

    /**
     * Returns the value of the right trigger button. This trigger value ranges from 0.0 when not pressed to 1.0 when pressed completely.
     */
    double buttonR2() const;

    /**
     * Returns the state of the select button. The value is TRUE when pressed, and FALSE when not pressed.
     *
     * This button can sometimes be labeled as the Back button on some gamepads.
     */
    bool buttonSelect() const;

    /**
     * Returns the state of the start button. The value is TRUE when pressed, and FALSE when not pressed.
     *
     * This button can sometimes be labeled as the Forward button on some gamepads.
     */
    bool buttonStart() const;

    /**
     * Returns the state of the left stick button. The value is TRUE when pressed, and FALSE when not pressed.
     *
     * This button is usually triggered by pressing the left joystick itself.
     */
    bool buttonL3() const;

    /**
     * Returns the state of the right stick button. The value is TRUE when pressed, and FALSE when not pressed.
     *
     * This button is usually triggered by pressing the right joystick itself.
     */
    bool buttonR3() const;

    /**
     * Returns the state of the direction pad up button. The value is TRUE when pressed, and FALSE when not pressed.
     */
    bool buttonUp() const;

    /**
     * Returns the state of the direction pad down button. The value is TRUE when pressed, and FALSE when not pressed.
     */
    bool buttonDown() const;

    /**
     * Returns the state of the direction pad left button. The value is TRUE when pressed, and FALSE when not pressed.
     */
    bool buttonLeft() const;

    /**
     * Returns the state of the direction pad right button. The value is TRUE when pressed, and FALSE when not pressed.
     */
    bool buttonRight() const;

    /**
     * Returns the state of the center button. The value is TRUE when pressed, and FALSE when not pressed.
     */
    bool buttonCenter() const;

    /**
     * Returns the state of the center button. The value is TRUE when pressed, and FALSE when not pressed.
     *
     * This button is typically the one in the center of the gamepad with a logo. Not all gamepads have a guide button.
     */
    bool buttonGuide() const;


  signals:


    /**
     * Emitted when the connection state of the gamepad is changed.
     */
    void connectedChanged( bool value );

    /**
     * Emitted when the value of the left thumbstick's X axis is changed.
     *
     * \see axisLeftX()
     */
    void axisLeftXChanged( double value );

    /**
     * Emitted when the value of the left thumbstick's Y axis is changed.
     *
     * \see axisLeftY()
     */
    void axisLeftYChanged( double value );

    /**
     * Emitted when the value of the right thumbstick's X axis is changed.
     *
     * \see axisRightX()
     */
    void axisRightXChanged( double value );

    /**
     * Emitted when the value of the right thumbstick's Y axis is changed.
     *
     * \see axisRightY()
     */
    void axisRightYChanged( double value );

    /**
     * Emitted when the state of the A button is changed.
     *
     * \see buttonA()
     */
    void buttonAChanged( bool value );

    /**
     * Emitted when the state of the B button is changed.
     *
     * \see buttonB()
     */
    void buttonBChanged( bool value );

    /**
     * Emitted when the state of the X button is changed.
     *
     * \see buttonX()
     */
    void buttonXChanged( bool value );

    /**
     * Emitted when the state of the Y button is changed.
     *
     * \see buttonY()
     */
    void buttonYChanged( bool value );

    /**
     * Emitted when the state of the left shoulder button is changed.
     *
     * \see buttonL1()
     */
    void buttonL1Changed( bool value );

    /**
     * Emitted when the state of the right shoulder button is changed.
     *
     * \see buttonR1()
     */
    void buttonR1Changed( bool value );

    /**
     * Emitted when the state of the left trigger button is changed.
     *
     * \see buttonL2()
     */
    void buttonL2Changed( double value );

    /**
     * Emitted when the state of the right trigger button is changed.
     *
     * \see buttonR2()
     */
    void buttonR2Changed( double value );

    /**
     * Emitted when the state of the select button is changed.
     *
     * \see buttonSelect()
     */
    void buttonSelectChanged( bool value );

    /**
     * Emitted when the state of the start button is changed.
     *
     * \see buttonStart()
     */
    void buttonStartChanged( bool value );

    /**
     * Emitted when the state of the left stick button is changed.
     *
     * \see buttonL3()
     */
    void buttonL3Changed( bool value );

    /**
     * Emitted when the state of the right stick button is changed.
     *
     * \see buttonR3()
     */
    void buttonR3Changed( bool value );

    /**
     * Emitted when the state of the direction pad up button is changed.
     *
     * \see buttonUp()
     */
    void buttonUpChanged( bool value );

    /**
     * Emitted when the state of the direction pad down button is changed.
     *
     * \see buttonDown()
     */
    void buttonDownChanged( bool value );

    /**
     * Emitted when the state of the direction pad left button is changed.
     *
     * \see buttonLeft()
     */
    void buttonLeftChanged( bool value );

    /**
     * Emitted when the state of the direction pad right button is changed.
     *
     * \see buttonRight()
     */
    void buttonRightChanged( bool value );

    /**
     * Emitted when the state of the center button is changed.
     *
     * \see buttonCenter()
     */
    void buttonCenterChanged( bool value );

    /**
     * Emitted when the state of the guide button is changed.
     *
     * \see buttonCenter()
     */
    void buttonGuideChanged( bool value );

  private:
    int mGamepadDeviceId = -1;
    QPointer<QGamepad> mGamepad;
};


#endif

#endif // QGS3DGAMEPADCONTROLLER_H
