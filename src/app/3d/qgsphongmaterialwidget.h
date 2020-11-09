/***************************************************************************
  qgsphongmaterialwidget.h
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPHONGMATERIALWIDGET_H
#define QGSPHONGMATERIALWIDGET_H

#include "qgsmaterialsettingswidget.h"
#include "qgsabstractmaterialsettings.h"

#include <ui_phongmaterialwidget.h>

class QgsPhongMaterialSettings;


//! Widget for configuration of Phong material settings
class QgsPhongMaterialWidget : public QgsMaterialSettingsWidget, private Ui::PhongMaterialWidget
{
    Q_OBJECT
  public:
    explicit QgsPhongMaterialWidget( QWidget *parent = nullptr );

    static QgsMaterialSettingsWidget *create();

    void setTechnique( QgsMaterialSettingsRenderingTechnique technique ) override;
    void setSettings( const QgsAbstractMaterialSettings *settings, QgsVectorLayer *layer ) override;
    QgsAbstractMaterialSettings *settings() override;

};

#endif // QGSPHONGMATERIALWIDGET_H
