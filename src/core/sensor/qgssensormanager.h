/***************************************************************************
    qgssensormanager.h
    ------------------
    Date                 : March 2023
    Copyright            : (C) 2023 Mathieu Pellerin
    Email                : mathieu at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSENSORMANAGER_H
#define QGSSENSORMANAGER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"
#include "qgsabstractsensor.h"

#include <QObject>
#include <QDomElement>

class QgsProject;

/**
 * \ingroup core
 * \class QgsSensorManager
 *
 * \brief Manages sensors.
 *
 * QgsSensorManager handles the storage, serializing and deserializing
 * of sensors. Usually this class is not constructed directly, but
 * rather accessed through a QgsProject via QgsProject::sensorManager().
 *
 * \since QGIS 3.32
 */
class CORE_EXPORT QgsSensorManager : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsSensorManager, with the specified \a parent object.
     */
    explicit QgsSensorManager( QObject *parent SIP_TRANSFERTHIS = nullptr );

    ~QgsSensorManager() override;

    /**
     * Deregisters and removes all sensors from the manager.
     */
    void clear();

    /**
     * Returns a list of pointers to all registered sensors.
     */
    QList<QgsAbstractSensor *> sensors() const;

    /**
     * Returns a registered sensor pointer matching a given \a id. If not
     * matching sensor is found, a NULLPTR will be returned.
     */
    QgsAbstractSensor *sensor( const QString &id ) const;

    /**
     * Registers a new \a sensor.
     * The sensor name does not require uniqueness; sensors will the same
     * name will store their data in the same sensor name key, allowing for
     * registration of sensors to cover multiple devices (e.g. two serial port
     * sensors with alternative port name to cover two machines).
     * \note Takes ownership of the sensor.
     */
    void addSensor( QgsAbstractSensor *sensor SIP_TRANSFER );

    /**
     * Removes a registered sensor matching a given \a id.
     * \returns TRUE if a sensor was removed.
     */
    bool removeSensor( const QString &id );

    /**
     * Returns a list of registered sensor names.
     */
    QStringList sensorNames() const;

    /**
     * Returns the last captured data from a registered sensor matching a given \a name.
     */
    QgsAbstractSensor::SensorData sensorData( const QString &name ) const;

    /**
     * Returns the last captured data of all registered sensors.
     */
    QMap<QString, QgsAbstractSensor::SensorData> sensorsData() const;

    /**
     * Reads the manager's state from a DOM element, restoring all sensors
     * present in the XML document.
     * \see writeXml()
     */
    bool readXml( const QDomElement &element, const QDomDocument &document );

    /**
     * Returns a DOM element representing the state of the manager.
     * \see readXml()
     */
    QDomElement writeXml( QDomDocument &document ) const;

  signals:

    //! Emitted when a sensor has been registered.
    void sensorAdded( const QString &id );

    //! Emitted when a sensor is about to be removed.
    void sensorAboutToBeRemoved( const QString &id );

    //! Emitted when a sensor has been removed.
    void sensorRemoved( const QString &id );

    //! Emitted when a sensor name has changed.
    void sensorNameChanged( const QString &id );

    //! Emitted when a sensor status has changed.
    void sensorStatusChanged( const QString &id );

    //! Emitted when newly captured data from a sensor has occurred.
    void sensorDataCaptured( const QString &id );

    //! Emitted when a sensor error has occurred.
    void sensorErrorOccurred( const QString &id );

  private slots:

    void handleSensorNameChanged();
    void handleSensorStatusChanged();
    void captureSensorData();
    void handleSensorErrorOccurred( const QString &errorMessage );

  private:

    QList<QgsAbstractSensor *> mSensors;
    QMap<QString, QgsAbstractSensor::SensorData> mSensorsData;

};

#endif // QGSSENSORMANAGER_H
