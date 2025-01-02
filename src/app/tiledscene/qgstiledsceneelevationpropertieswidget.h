/***************************************************************************
    qgstiledsceneelevationpropertieswidget.h
    ---------------------
    begin                : August 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTILEDSCENEELEVATIONPROPERTIESWIDGET_H
#define QGSTILEDSCENEELEVATIONPROPERTIESWIDGET_H

#include "qgsmaplayerconfigwidget.h"
#include "qgsmaplayerconfigwidgetfactory.h"

#include "ui_qgstiledsceneelevationpropertieswidgetbase.h"

class QgsTiledSceneLayer;

class QgsTiledSceneElevationPropertiesWidget : public QgsMapLayerConfigWidget, private Ui::QgsTiledSceneElevationPropertiesWidgetBase
{
    Q_OBJECT
  public:
    QgsTiledSceneElevationPropertiesWidget( QgsTiledSceneLayer *layer, QgsMapCanvas *canvas, QWidget *parent );

    void syncToLayer( QgsMapLayer *layer ) final;
    bool shouldTriggerLayerRepaint() const override { return false; }

  public slots:
    void apply() override;

  private slots:

    void onChanged();
    void shiftSceneZAxis();

  private:
    QgsTiledSceneLayer *mLayer = nullptr;
    bool mBlockUpdates = false;
};


class QgsTiledSceneElevationPropertiesWidgetFactory : public QObject, public QgsMapLayerConfigWidgetFactory
{
    Q_OBJECT
  public:
    explicit QgsTiledSceneElevationPropertiesWidgetFactory( QObject *parent = nullptr );

    QgsMapLayerConfigWidget *createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool dockWidget, QWidget *parent ) const override;
    bool supportLayerPropertiesDialog() const override;
    bool supportsStyleDock() const override;
    bool supportsLayer( QgsMapLayer *layer ) const override;
    QString layerPropertiesPagePositionHint() const override;
};


#endif // QGSTILEDSCENEELEVATIONPROPERTIESWIDGET_H
