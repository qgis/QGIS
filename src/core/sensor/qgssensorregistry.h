/***************************************************************************
                            qgssensorregistry.h
                            ------------------------
    begin                : March 2023
    copyright            : (C) 2023 by Mathieu Pellerin
    email                : mathieu at opengis dot ch
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSSENSORREGISTRY_H
#define QGSSENSORREGISTRY_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsapplication.h"
#include "qgsabstractsensor.h"

/**
 * \ingroup core
 * \brief Stores metadata about a sensor class.
 *
 * \note In C++ you can use QgsSensorAbstractMetadata convenience class.
 * \since QGIS 3.32
 */
class CORE_EXPORT QgsSensorAbstractMetadata
{
  public:

    /**
     * Constructor for QgsSensorAbstractMetadata with the specified class \a type.
     */
    QgsSensorAbstractMetadata( const QString &type, const QString &visibleName )
      : mType( type )
      , mVisibleName( visibleName )
    {}

    virtual ~QgsSensorAbstractMetadata() = default;

    /**
     * Returns the unique type code for the sensor class.
     */
    QString type() const { return mType; }

    /**
     * Returns a translated, user visible name for the sensor class.
     */
    QString visibleName() const { return mVisibleName; }

    /*
     * IMPORTANT: While it seems like /Factory/ would be the correct annotations here, that's not
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
     * Creates a sensor of this class.
     */
    virtual QgsAbstractSensor *createSensor( QObject *parent ) = 0 SIP_TRANSFERBACK;

  private:

    QString mType;
    QString mVisibleName;
};

//! Sensor creation function
typedef std::function<QgsAbstractSensor *( QObject *parent )> QgsSensorCreateFunc SIP_SKIP;

#ifndef SIP_RUN

/**
 * \ingroup core
 * \brief Convenience metadata class that uses static functions to create sensors and their configuration widgets.
 * \note not available in Python bindings
 */
class CORE_EXPORT QgsSensorMetadata : public QgsSensorAbstractMetadata
{
  public:

    /**
     * Constructor for QgsSensorMetadata with the specified class \a type.
     */
    QgsSensorMetadata( const QString &type, const QString &visibleName,
                       const QgsSensorCreateFunc &pfCreate )
      : QgsSensorAbstractMetadata( type, visibleName )
      , mCreateFunc( pfCreate )
    {}

    /**
     * Returns the classes' sensor creation function.
     */
    QgsSensorCreateFunc createFunction() const { return mCreateFunc; }

    QgsAbstractSensor *createSensor( QObject *parent ) override { return mCreateFunc ? mCreateFunc( parent ) : nullptr; }

  protected:
    QgsSensorCreateFunc mCreateFunc = nullptr;

};

#endif

/**
 * \ingroup core
 * \class QgsSensorRegistry
 * \brief Registry of available sensor types.
 *
 * QgsSensorRegistry is not usually directly created, but rather accessed through
 * QgsApplication::sensorRegistry().
 *
 * A companion class, QgsSensorGuiRegistry, handles the GUI behavior
 * of sensors.
 *
 * \since QGIS 3.32
 */
class CORE_EXPORT QgsSensorRegistry : public QObject
{
    Q_OBJECT

  public:

    /**
     * Creates a new empty item registry.
     *
     * QgsSensorRegistry is not usually directly created, but rather accessed through
     * QgsApplication::sensorRegistry().
     *
     * \see populate()
    */
    QgsSensorRegistry( QObject *parent = nullptr );
    ~QgsSensorRegistry() override;

    /**
     * Populates the registry with standard sensor types. If called on a non-empty registry
     * then this will have no effect and will return FALSE.
     */
    bool populate();

    QgsSensorRegistry( const QgsSensorRegistry &rh ) = delete;
    QgsSensorRegistry &operator=( const QgsSensorRegistry &rh ) = delete;

    /**
     * Returns the metadata for the specified sensor \a type. Returns NULLPTR if
     * a corresponding type was not found in the registry.
     */
    QgsSensorAbstractMetadata *sensorMetadata( const QString &type ) const;

    /*
     * IMPORTANT: While it seems like /Factory/ would be the correct annotations here, that's not
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
     * Registers a new sensor type.
     * \note Takes ownership of the metadata instance.
     */
    bool addSensorType( QgsSensorAbstractMetadata *metadata SIP_TRANSFER );

    /**
     * Removes a new a sensor type from the registry.
     */
    bool removeSensorType( const QString &type );

    /**
     * Creates a new instance of a sensor given the \a type.
     */
    QgsAbstractSensor *createSensor( const QString &type, QObject *parent = nullptr ) const SIP_TRANSFERBACK;

    /**
     * Returns a map of available sensor types to translated name.
     */
    QMap<QString, QString> sensorTypes() const;

  signals:

    /**
     * Emitted whenever a new sensor type is added to the registry, with the specified
     * \a type and visible \a name.
     */
    void sensorAdded( const QString &type, const QString &name );

  private:

#ifdef SIP_RUN
    QgsSensorRegistry( const QgsSensorRegistry &rh );
#endif

    QMap<QString, QgsSensorAbstractMetadata *> mMetadata;

};

#endif //QGSSENSORREGISTRY_H



