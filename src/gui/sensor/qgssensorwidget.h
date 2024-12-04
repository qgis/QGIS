/***************************************************************************
    qgssensorwidget.h
    ---------------------
    begin                : March 2023
    copyright            : (C) 2023 by Mathieu Pellerin
    email                : mathieu at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSSENSORWIDGET_H
#define QGSSENSORWIDGET_H

#include "ui_widget_tcpsocketsensor.h"
#include "ui_widget_udpsocketsensor.h"
#include "ui_widget_serialportsensor.h"

#include "qgsconfig.h"

#include "qgis_sip.h"
#include "qgis_gui.h"
#include "qgsabstractsensor.h"

#include <QWidget>

/**
 * \ingroup gui
 * \class QgsAbstractSensorWidget
 * \brief Base class for widgets which allow control over the properties of sensors.
 * \since QGIS 3.32
 */
class GUI_EXPORT QgsAbstractSensorWidget : public QWidget
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsAbstractSensorWidget.
     * \param parent parent widget
     */
    QgsAbstractSensorWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Creates a new sensor matching the settings defined in the widget.
     */
    virtual QgsAbstractSensor *createSensor() = 0 SIP_FACTORY;

    /**
     * Updates an existing \a sensor to match the settings defined in the widget. If
     * TRUE is returned, the \a sensor was successfully updated.
     *
     * If FALSE is returned, then the widget could not successfully update
     * the \a sensor.
     */
    virtual bool updateSensor( QgsAbstractSensor *sensor ) = 0;

    /**
     * Sets the widget settings to match a given \a sensor. If TRUE is returned, \a sensor
     * was an acceptable type and the widget has been updated to match
     * the \a sensor's properties.
     *
     * If FALSE is returned, then the widget could not be successfully updated
     * to show the properties of \a sensor.
     */
    virtual bool setSensor( QgsAbstractSensor *sensor ) = 0;

  signals:

    /**
     * Emitted whenever configuration changes happened on this sensor configuration.
     */
    void changed();
};

#ifndef SIP_RUN
///@cond PRIVATE

/**
 * \ingroup gui
 * \class QgsTcpSocketSensorWidget
 * \brief A configuration widget which allow control over QgsTcpSocketSensor properties.
 * \since QGIS 3.32
 */
class GUI_EXPORT QgsTcpSocketSensorWidget : public QgsAbstractSensorWidget, private Ui::WidgetTcpSocketSensor
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsTcpSocketSensorWidget.
     * \param parent parent widget
     */
    QgsTcpSocketSensorWidget( QWidget *parent );

    QgsAbstractSensor *createSensor() override;
    bool updateSensor( QgsAbstractSensor *sensor ) override;
    bool setSensor( QgsAbstractSensor *sensor ) override;
};

/**
 * \ingroup gui
 * \class QgsUdpSocketSensorWidget
 * \brief A configuration widget which allow control over QgsUdpSocketSensorWidget properties.
 * \since QGIS 3.32
 */
class GUI_EXPORT QgsUdpSocketSensorWidget : public QgsAbstractSensorWidget, private Ui::WidgetUdpSocketSensor
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsUdpSocketSensorWidget.
     * \param parent parent widget
     */
    QgsUdpSocketSensorWidget( QWidget *parent );

    QgsAbstractSensor *createSensor() override;
    bool updateSensor( QgsAbstractSensor *sensor ) override;
    bool setSensor( QgsAbstractSensor *sensor ) override;
};

#if defined( HAVE_QTSERIALPORT )

/**
 * \ingroup gui
 * \class QgsSerialPortSensorWidget
 * \brief A configuration widget which allow control over QgsSerialPortSensor properties.
 * \since QGIS 3.32
 */
class GUI_EXPORT QgsSerialPortSensorWidget : public QgsAbstractSensorWidget, private Ui::WidgetSerialPortSensor
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsSerialPortSensorWidget.
     * \param parent parent widget
     */
    QgsSerialPortSensorWidget( QWidget *parent );

    QgsAbstractSensor *createSensor() override;
    bool updateSensor( QgsAbstractSensor *sensor ) override;
    bool setSensor( QgsAbstractSensor *sensor ) override;

  private:
    void updateSerialPortDetails();
};
#endif

#endif
///@endcond

#endif // QGSSENSORWIDGET_H
