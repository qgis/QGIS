/***************************************************************************
  qgsshadingrenderersettingswidget.h - QgsShadingRendererSettingsWidget

 ---------------------
 begin                : 12.12.2022
 copyright            : (C) 2022 by Vincent Cloarec
 email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSSHADINGRENDERERSETTINGSWIDGET_H
#define QGSSHADINGRENDERERSETTINGSWIDGET_H

#include "qgsmaplayerconfigwidget.h"
#include "ui_qgselevationshadingrenderersettingswidget.h"
#include "qgsmaplayerconfigwidgetfactory.h"

class QgsElevationShadingRendererSettingsWidget : public QgsMapLayerConfigWidget, private Ui::QgsElevationShadingRendererSettingsWidget
{
    Q_OBJECT

  public:
    explicit QgsElevationShadingRendererSettingsWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, QWidget *parent = nullptr );

    void apply() override;

  private slots:
    void syncToProject();
    void onChanged();

  private:
    bool mBlockUpdates = false;
};


class QgsElevationShadingRendererSettingsWidgetFactory : public QObject, public QgsMapLayerConfigWidgetFactory
{
    Q_OBJECT
  public:
    explicit QgsElevationShadingRendererSettingsWidgetFactory( QObject *parent = nullptr );

    QgsMapLayerConfigWidget *createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool dockWidget, QWidget *parent ) const override;
    bool supportsStyleDock() const override { return true; }
};

#endif // QGSSHADINGRENDERERSETTINGSWIDGET_H
