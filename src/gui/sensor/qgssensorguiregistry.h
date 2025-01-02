/***************************************************************************
                            qgssensorguiregistry.h
                            --------------------------
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
#ifndef QGSSENSORGUIREGISTRY_H
#define QGSSENSORGUIREGISTRY_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgsabstractsensor.h"
#include "qgssensorregistry.h"
#include "qgssensorwidget.h"

#include <QIcon>

/**
 * \ingroup gui
 * \brief Stores GUI metadata about one sensor class.
 *
 * This is a companion to QgsSensorAbstractMetadata, storing only
 * the components related to the GUI behavior of sensor.
 *
 * \note In C++ you can use QgsSensorGuiMetadata convenience class.
 * \since QGIS 3.32
 */
class GUI_EXPORT QgsSensorAbstractGuiMetadata
{
  public:
    /**
     * Constructor for QgsSensorAbstractGuiMetadata with the specified class \a type.
     *
     * \a visibleName should be set to a translated, user visible name identifying the corresponding sensor type.
     */
    QgsSensorAbstractGuiMetadata( const QString &type, const QString &visibleName )
      : mType( type )
      , mVisibleName( visibleName )
    {}

    virtual ~QgsSensorAbstractGuiMetadata() = default;

    /**
     * Returns the unique type code for the sensor class.
     */
    QString type() const { return mType; }

    /**
     * Returns a translated, user visible name identifying the corresponding sensor.
     */
    QString visibleName() const { return mVisibleName; }

    /**
     * Returns an icon representing creation of the sensor type.
     */
    virtual QIcon creationIcon() const { return QgsApplication::getThemeIcon( QStringLiteral( "/mSensor.svg" ) ); }

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
     * Creates a configuration widget for an \a sensor of this type. Can return NULLPTR if no configuration GUI is required.
     */
    virtual QgsAbstractSensorWidget *createSensorWidget( QgsAbstractSensor *sensor ) SIP_TRANSFERBACK
    {
      Q_UNUSED( sensor )
      return nullptr;
    }

    /**
     * Creates an instance of the corresponding sensor type.
     */
    virtual QgsAbstractSensor *createSensor( QObject *parent ) SIP_TRANSFERBACK
    {
      Q_UNUSED( parent )
      return nullptr;
    }

  private:
    QString mType;
    QString mVisibleName;
};

//! Sensor configuration widget creation function
typedef std::function<QgsAbstractSensorWidget *( QgsAbstractSensor *sensor )> QgsSensorWidgetFunc SIP_SKIP;

#ifndef SIP_RUN

/**
 * \ingroup gui
 * \brief Convenience metadata class that uses static functions to handle sensor GUI behavior.
 * \note not available in Python bindings
 */
class GUI_EXPORT QgsSensorGuiMetadata : public QgsSensorAbstractGuiMetadata
{
  public:
    /**
     * Constructor for QgsSensorGuiMetadata with the specified class \a type
     * and \a creationIcon, and function pointers for the configuration widget creation function.
     *
     * \a visibleName should be set to a translated, user visible name identifying the corresponding sensor.
     */
    QgsSensorGuiMetadata( const QString &type, const QString &visibleName, const QIcon &creationIcon, const QgsSensorWidgetFunc &pfWidget = nullptr, const QgsSensorCreateFunc &pfCreateFunc = nullptr )
      : QgsSensorAbstractGuiMetadata( type, visibleName )
      , mIcon( creationIcon )
      , mWidgetFunc( pfWidget )
      , mCreateFunc( pfCreateFunc )
    {}

    /**
     * Returns the classes' configuration widget creation function.
     * \see setWidgetFunction()
     */
    QgsSensorWidgetFunc widgetFunction() const { return mWidgetFunc; }

    /**
     * Sets the classes' sensor configuration widget creation \a function.
     * \see widgetFunction()
     */
    void setWidgetFunction( const QgsSensorWidgetFunc &function ) { mWidgetFunc = function; }

    /**
     * Returns the classes' sensor creation function.
     * \see setSensorCreationFunction()
     */
    QgsSensorCreateFunc sensorCreationFunction() const { return mCreateFunc; }

    /**
     * Sets the classes' sensor creation \a function.
     * \see sensorCreationFunction()
     */
    void setSensorCreationFunction( const QgsSensorCreateFunc &function ) { mCreateFunc = function; }

    QIcon creationIcon() const override { return mIcon.isNull() ? QgsSensorAbstractGuiMetadata::creationIcon() : mIcon; }
    QgsAbstractSensorWidget *createSensorWidget( QgsAbstractSensor *sensor ) override { return mWidgetFunc ? mWidgetFunc( sensor ) : nullptr; }
    QgsAbstractSensor *createSensor( QObject *parent ) override { return mCreateFunc ? mCreateFunc( parent ) : nullptr; }

  protected:
    QIcon mIcon;
    QgsSensorWidgetFunc mWidgetFunc = nullptr;
    QgsSensorCreateFunc mCreateFunc = nullptr;
};

#endif

/**
 * \ingroup gui
 * \class QgsSensorGuiRegistry
 * \brief Registry of available sensor GUI behavior.
 *
 * QgsSensorGuiRegistry is not usually directly created, but rather accessed through
 * QgsGui::sensorGuiRegistry().
 *
 * This acts as a companion to QgsSensorRegistry, handling only
 * the components related to the GUI behavior of sensors.
 *
 * \since QGIS 3.32
 */
class GUI_EXPORT QgsSensorGuiRegistry : public QObject
{
    Q_OBJECT

  public:
    /**
     * Creates a new empty sensor GUI registry.
     *
     * QgsSensorGuiRegistry is not usually directly created, but rather accessed through
     * QgsGui::sensorGuiRegistry().
    */
    QgsSensorGuiRegistry( QObject *parent = nullptr );
    ~QgsSensorGuiRegistry() override;

    QgsSensorGuiRegistry( const QgsSensorGuiRegistry &rh ) = delete;
    QgsSensorGuiRegistry &operator=( const QgsSensorGuiRegistry &rh ) = delete;

    /**
     * Populates the registry with standard sensor types. If called on a non-empty registry
     * then this will have no effect and will return FALSE.
     */
    bool populate();

    /**
     * Returns the metadata for the specified sensor \a type. Returns NULLPTR if
     * a corresponding sensor type was not found in the registry.
     */
    QgsSensorAbstractGuiMetadata *sensorMetadata( const QString &type ) const;

    /**
     * Registers the GUI metadata for a new sensor type. Takes ownership of the metadata instance.
     */
    bool addSensorGuiMetadata( QgsSensorAbstractGuiMetadata *metadata SIP_TRANSFER );

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
     * Creates a new instance of a sensor given the \a type.
     */
    QgsAbstractSensor *createSensor( const QString &type, QObject *parent = nullptr ) const SIP_TRANSFERBACK;

    /**
     * Creates a new instance of a sensor configuration widget for the specified \a sensor. The
     * \a sensor doesn't need to live for the duration of the widget, it is only used when creating
     * the configuration widget to match a sensor type and initiate the widget to match the
     * \a sensor settings.
     */
    QgsAbstractSensorWidget *createSensorWidget( QgsAbstractSensor *sensor ) const SIP_TRANSFERBACK;

    /**
     * Returns a list of sensor types handled by the registry.
     */
    QMap<QString, QString> sensorTypes() const;

  signals:

    /**
     * Emitted whenever a new sensor type is added to the registry, with the specified
     * \a type.
     */
    void sensorAdded( const QString &type, const QString &name );

  private:
#ifdef SIP_RUN
    QgsSensorGuiRegistry( const QgsSensorGuiRegistry &rh );
#endif

    QMap<QString, QgsSensorAbstractGuiMetadata *> mMetadata;
};

#endif //QGSSENSORGUIREGISTRY_H
