/***************************************************************************
  qgsphongtexturedmaterialwidget.h
  --------------------------------------
  Date                 : July 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPHONGTEXTUREDMATERIALWIDGET_H
#define QGSPHONGTEXTUREDMATERIALWIDGET_H

#include "qgsmaterialsettingswidget.h"
#include "qgsabstractmaterialsettings.h"

#include <ui_phongtexturedmaterialwidgetbase.h>

class QgsPhongMaterialSettings;


//! Widget for configuration of textured Phong material settings
class QgsPhongTexturedMaterialWidget : public QgsMaterialSettingsWidget, private Ui::PhongTexturedMaterialWidgetBase
{
    Q_OBJECT
  public:
    explicit QgsPhongTexturedMaterialWidget( QWidget *parent = nullptr );

    static QgsMaterialSettingsWidget *create();

    void setSettings( const QgsAbstractMaterialSettings *settings, QgsVectorLayer *layer ) final;
    QgsAbstractMaterialSettings *settings() final;

  private slots:

    void updateWidgetState();
};

#endif // QGSPHONGTEXTUREDMATERIALWIDGET_H
