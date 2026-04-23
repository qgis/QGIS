/***************************************************************************
  qgsmetalroughtexturedmaterialwidget.h
  --------------------------------------
  Date                 : April 2026
  Copyright            : (C) 2026 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMETALROUGHTEXTUREDMATERIALWIDGET_H
#define QGSMETALROUGHTEXTUREDMATERIALWIDGET_H

#include "ui_metalroughtexturedmaterialwidgetbase.h"

#include "qgsmaterialsettingswidget.h"

class QgsAbstractMaterialSettings;


//! Widget for configuration of textured metal rough material settings
class QgsMetalRoughTexturedMaterialWidget : public QgsMaterialSettingsWidget, private Ui::MetalRoughTexturedMaterialWidgetBase
{
    Q_OBJECT
  public:
    explicit QgsMetalRoughTexturedMaterialWidget( QWidget *parent = nullptr );

    static QgsMaterialSettingsWidget *create();

    void setSettings( const QgsAbstractMaterialSettings *settings, QgsVectorLayer *layer ) final;
    std::unique_ptr< QgsAbstractMaterialSettings > settings() final;
  public slots:
    void setPreviewVisible( bool visible ) final;
  private slots:

    void updatePreview();
};

#endif // QGSMETALROUGHTEXTUREDMATERIALWIDGET_H
