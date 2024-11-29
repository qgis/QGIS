/***************************************************************************
    qgsprojectsensorsettingswidget.h
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

#ifndef QGSPROJECTSENSORSETTINGSWIDGET_H
#define QGSPROJECTSENSORSETTINGSWIDGET_H

#include "ui_qgsprojectsensorettingswidgetbase.h"
#include "qgsoptionswidgetfactory.h"

#include <QDomDocument>

class QgsProjectSensorSettingsWidget : public QgsOptionsPageWidget, private Ui::QgsProjectSensorSettingsWidgetBase
{
    Q_OBJECT
  public:
    QgsProjectSensorSettingsWidget( QWidget *parent = nullptr );

  public slots:

    bool isValid() override;
    void apply() override;
    void cancel() override;

  private:
    QDomDocument mPreviousSensors;
    QStringList mConnectedSensors;
};


class QgsProjectSensorSettingsWidgetFactory : public QgsOptionsWidgetFactory
{
    Q_OBJECT
  public:
    explicit QgsProjectSensorSettingsWidgetFactory( QObject *parent = nullptr );

    QgsOptionsPageWidget *createWidget( QWidget *parent = nullptr ) const override;
};


#endif // QGSPROJECTSENSORSETTINGSWIDGET_H
