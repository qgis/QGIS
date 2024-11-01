/***************************************************************************
    qgsinputcontroller.h
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

#ifndef QGSINPUTCONTROLLER_H
#define QGSINPUTCONTROLLER_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgis.h"

#include <QObject>
#include <QMap>

class QgsAbstract2DMapController;
class QgsAbstract3DMapController;

/**
 * \ingroup gui
 * \class QgsInputControllerManager
 * \brief Manages input control devices.
 *
 * QgsInputControllerManager is not usually directly created, but rather accessed through
 * QgsGui::inputControllerManager().
 *
 * \since QGIS 3.34
 */
class GUI_EXPORT QgsInputControllerManager : public QObject
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsInputControllerManager, with the specified \a parent object.
     *
     * \note QgsInputControllerManager is not usually directly created, but rather accessed through
     * QgsGui::inputControllerManager().
     */
    QgsInputControllerManager( QObject *parent SIP_TRANSFERTHIS = nullptr );
    ~QgsInputControllerManager() override;

    /**
     * Returns a list of the device IDs of available 2D map controllers.
     *
     * \see create2DMapController()
     * \see register2DMapController()
     */
    QStringList available2DMapControllers() const;

    /*
     * IMPORTANT: While it seems like /Factory/ would be the correct annotation here, that's not
     * the case.
     * As per Phil Thomson's advice on https://www.riverbankcomputing.com/pipermail/pyqt/2017-July/039450.html:
     *
     * "
     * /Factory/ is used when the instance returned is guaranteed to be new to Python.
     * In this case it isn't because it has already been seen when being returned by QgsProcessingAlgorithm::createInstance()
     * (However for a different sub-class implemented in C++ then it would be the first time it was seen
     * by Python so the /Factory/ on create() would be correct.)
     *
     * You might try using /TransferBack/ on create() instead - that might be the best compromise.
     * "
     */

    /**
     * Returns a new instance of the 2D map controller with the specified \a deviceId.
     *
     * The caller takes ownership of the returned object.
     *
     * Will return NULLPTR if no matching controller is found.
     *
     * \see available2DMapControllers()
     */
    QgsAbstract2DMapController *create2DMapController( const QString &deviceId ) const SIP_TRANSFERBACK;

    /**
     * Registers a new 2D map \a controller.
     *
     * Ownership of \a controller is transferred to the manager.
     *
     * Returns TRUE if the controller was successfully registered, or FALSE if it could
     * not be registered (e.g. if a controller with matching deviceId has already been registered).
     *
     * \see available2DMapControllers()
     */
    bool register2DMapController( QgsAbstract2DMapController *controller SIP_TRANSFER );

    /**
     * Returns a list of the device IDs of available 3D map controllers.
     *
     * \see create3DMapController()
     * \see register3DMapController()
     */
    QStringList available3DMapControllers() const;

    /*
     * IMPORTANT: While it seems like /Factory/ would be the correct annotation here, that's not
     * the case.
     * As per Phil Thomson's advice on https://www.riverbankcomputing.com/pipermail/pyqt/2017-July/039450.html:
     *
     * "
     * /Factory/ is used when the instance returned is guaranteed to be new to Python.
     * In this case it isn't because it has already been seen when being returned by QgsProcessingAlgorithm::createInstance()
     * (However for a different sub-class implemented in C++ then it would be the first time it was seen
     * by Python so the /Factory/ on create() would be correct.)
     *
     * You might try using /TransferBack/ on create() instead - that might be the best compromise.
     * "
     */

    /**
     * Returns a new instance of the 3D map controller with the specified \a deviceId.
     *
     * The caller takes ownership of the returned object.
     *
     * Will return NULLPTR if no matching controller is found.
     *
     * \see available3DMapControllers()
     */
    QgsAbstract3DMapController *create3DMapController( const QString &deviceId ) const SIP_TRANSFERBACK;

    /**
     * Registers a new 3D map \a controller.
     *
     * Ownership of \a controller is transferred to the manager.
     *
     * Returns TRUE if the controller was successfully registered, or FALSE if it could
     * not be registered (e.g. if a controller with matching deviceId has already been registered).
     *
     * \see available3DMapControllers()
     */
    bool register3DMapController( QgsAbstract3DMapController *controller SIP_TRANSFER );

  private:
    QMap<QString, QgsAbstract2DMapController *> m2DMapControllers;
    QMap<QString, QgsAbstract3DMapController *> m3DMapControllers;
};

#endif // QGSINPUTCONTROLLER_H
