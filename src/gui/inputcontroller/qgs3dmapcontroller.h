/***************************************************************************
    qgs3dmapcontroller.h
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

#ifndef QGS3MAPCONTROLLER_H
#define QGS3MAPCONTROLLER_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgis.h"
#include "qgsabstractinputcontroller.h"

/**
 * \ingroup gui
 * \class QgsAbstract3DMapController
 * \brief Abstract base class for all 3D map controllers.
 *
 * \since QGIS 3.34
 */
class GUI_EXPORT QgsAbstract3DMapController : public QgsAbstractInputController
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsAbstract3DMapController, with the specified \a parent object.
     */
    QgsAbstract3DMapController( QObject *parent SIP_TRANSFERTHIS = nullptr );

    Qgis::InputControllerType type() const override;

  signals:
#if 0

    // TODO: add a bunch of signals relating to navigating a 3D map, eg

    /**
     * Emitted when the controller needs to change the 3d camera angle by the specified \a delta.
     */
    void changeCameraAngleByDelta( double delta );

    // etc
#endif
};


#endif // QGS3MAPCONTROLLER_H
