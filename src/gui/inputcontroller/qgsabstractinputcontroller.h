/***************************************************************************
    qgsabstractinputcontroller.h
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

#ifndef QGSABSTRACTINPUTCONTROLLER_H
#define QGSABSTRACTINPUTCONTROLLER_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgis.h"

#include <QObject>

/**
 * \ingroup gui
 * \class QgsAbstractInputController
 * \brief Abstract base class for all input controllers.
 *
 * \since QGIS 3.34
 */
class GUI_EXPORT QgsAbstractInputController : public QObject
{
#ifdef SIP_RUN
#include "qgs2dmapcontroller.h"
#include "qgs3dmapcontroller.h"
#endif

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( qobject_cast<QgsAbstract2DMapController *>( sipCpp ) )
      sipType = sipType_QgsAbstract2DMapController;
    else if ( qobject_cast<QgsAbstract3DMapController *>( sipCpp ) )
      sipType = sipType_QgsAbstract3DMapController;
    else
      sipType = nullptr;
    SIP_END
#endif

    Q_OBJECT

  public:
    /**
     * Constructor for QgsAbstractInputController, with the specified \a parent object.
     */
    QgsAbstractInputController( QObject *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns a new copy of the controller.
     */
    virtual QgsAbstractInputController *clone() const = 0 SIP_FACTORY;

    /**
     * Returns a string uniquely identifying the device.
     */
    virtual QString deviceId() const = 0;

    /**
     * Returns the input controller type.
     */
    virtual Qgis::InputControllerType type() const = 0;
};


#endif // QGSABSTRACTINPUTCONTROLLER_H
