/***************************************************************************
  qgslightswidget.h
  --------------------------------------
  Date                 : November 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLIGHTSWIDGET_H
#define QGSLIGHTSWIDGET_H

#include <QWidget>

#include "ui_qgslightswidget.h"

#include "qgspointlightsettings.h"


/**
 * Widget for configuration of lights in 3D map scene
 * \since QGIS 3.6
 */
class QgsLightsWidget : public QWidget, private Ui::QgsLightsWidget
{
    Q_OBJECT
  public:
    explicit QgsLightsWidget( QWidget *parent = nullptr );

    void setPointLights( const QList<QgsPointLightSettings> &pointLights );

    QList<QgsPointLightSettings> pointLights();

  signals:

  private slots:
    void onCurrentLightChanged( int index );
    void updateCurrentLightParameters();
    void onAddLight();
    void onRemoveLight();

  private:
    void updateLightsList();

  private:
    QList<QgsPointLightSettings> mPointLights;
};

#endif // QGSLIGHTSWIDGET_H
