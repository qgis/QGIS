/***************************************************************************
    qgs2dmapcontroller.h
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

#ifndef QGS2DMAPCONTROLLER_H
#define QGS2DMAPCONTROLLER_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgis.h"
#include "qgsabstractinputcontroller.h"

/**
 * \ingroup gui
 * \class QgsAbstract2DMapController
 * \brief Abstract base class for all 2D map controllers.
 *
 * \since QGIS 3.34
 */
class GUI_EXPORT QgsAbstract2DMapController : public QgsAbstractInputController
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsAbstract2DMapController, with the specified \a parent object.
     */
    QgsAbstract2DMapController( QObject *parent SIP_TRANSFERTHIS = nullptr );

    Qgis::InputControllerType type() const override;

  signals:

#if 0
    // TODO: add a bunch of signals relating to navigating a 2D map, eg

    /**
     * Emitted when the controller should change the scale of a map by the given \a factor.
     */
    void zoomMap( double factor );

    // etc
#endif
};


#endif // QGS2DMAPCONTROLLER_H
