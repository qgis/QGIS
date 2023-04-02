/***************************************************************************
    qgssensortablewidget.h
    ---------------------------------
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

#ifndef QGSSENSORTABLEWIDGET_H
#define QGSSENSORTABLEWIDGET_H

#include "ui_qgssensortablewidgetbase.h"
#include "ui_qgssensorsettingswidgetbase.h"

#include "qgis_app.h"
#include "qgsdockwidget.h"
#include "qgsabstractsensor.h"
#include "qgspanelwidget.h"


class QgsSensorModel;
class QgsAbstractSensorWidget;

class APP_EXPORT QgsSensorSettingsWidget : public QgsPanelWidget, private Ui::QgsSensorSettingsWidgetBase
{
    Q_OBJECT

  public:
    QgsSensorSettingsWidget( QgsAbstractSensor *sensor = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    ~QgsSensorSettingsWidget() override = default;

    void apply();

  private:
    void setSensorWidget();

    QgsAbstractSensor *mSensor = nullptr;
    QgsAbstractSensorWidget *mSensorWidget = nullptr;

    bool mDirty = false;
};

class APP_EXPORT QgsSensorTableWidget : public QgsPanelWidget, private Ui::QgsSensorTableWidgetBase
{
    Q_OBJECT

  public:
    QgsSensorTableWidget( QWidget *parent = nullptr );
    ~QgsSensorTableWidget() override = default;

  private:
    QgsSensorModel *mSensorModel = nullptr;
};

#endif // QGSSENSORTABLEWIDGET_H
