/***************************************************************************
    qgsprojectelevationsettingswidget.h
    ---------------------
    begin                : March 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROJECTELEVATIONSETTINGSWIDGET_H
#define QGSPROJECTELEVATIONSETTINGSWIDGET_H

#include "qgsoptionswidgetfactory.h"

#include "ui_qgsprojectelevationsettingswidgetbase.h"

class QgsVectorLayer;

class QgsProjectElevationSettingsWidget : public QgsOptionsPageWidget, private Ui::QgsProjectElevationSettingsWidgetBase
{
    Q_OBJECT
  public:

    QgsProjectElevationSettingsWidget( QWidget *parent = nullptr );

  public slots:
    bool isValid() override;
    void apply() override;

  private slots:

    bool validate();

};


class QgsProjectElevationSettingsWidgetFactory : public QgsOptionsWidgetFactory
{
    Q_OBJECT
  public:
    explicit QgsProjectElevationSettingsWidgetFactory( QObject *parent = nullptr );

    QString title() const override;
    QIcon icon() const override;

    QgsOptionsPageWidget *createWidget( QWidget *parent = nullptr ) const override;
};



#endif // QGSPROJECTELEVATIONSETTINGSWIDGET_H
