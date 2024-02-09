/***************************************************************************
  qgsmetalroughmaterialwidget.h
  --------------------------------------
  Date                 : December 2023
  Copyright            : (C) 2023 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMETALROUGHMATERIALWIDGET_H
#define QGSMETALROUGHMATERIALWIDGET_H

#include "qgsmaterialsettingswidget.h"
#include "qgsabstractmaterialsettings.h"

#include <ui_metalroughmaterialwidget.h>

class QgsMetalRoughMaterialSettings;


//! Widget for configuration of metal rough material settings
class QgsMetalRoughMaterialWidget : public QgsMaterialSettingsWidget, private Ui::MetalRoughMaterialWidget
{
    Q_OBJECT

  public:
    explicit QgsMetalRoughMaterialWidget( QWidget *parent = nullptr, bool hasOpacity = true );

    static QgsMaterialSettingsWidget *create();

    void setTechnique( QgsMaterialSettingsRenderingTechnique technique ) final;
    void setSettings( const QgsAbstractMaterialSettings *settings, QgsVectorLayer *layer ) final;
    QgsAbstractMaterialSettings *settings() override;

  private slots:

    void updateWidgetState();
};

#endif // QGSMETALROUGHMATERIALWIDGET_H
