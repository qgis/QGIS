/***************************************************************************
  qgstiledscenelayer3drendererwidget.h
  --------------------------------------
  Date                 : August 2023
  Copyright            : (C) 2023 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTILEDSCENELAYER3DRENDERERWIDGET_H
#define QGSTILEDSCENELAYER3DRENDERERWIDGET_H

#include "qgsmaplayerconfigwidget.h"
#include "qgsmaplayerconfigwidgetfactory.h"

#include "ui_qgstiledscenelayer3dpropertieswidget.h"

class QgsTiledSceneLayer;


class QgsTiledSceneLayer3DPropertiesWidget : public QWidget, private Ui::QgsTiledSceneLayer3DPropertiesWidget
{
    Q_OBJECT
  public:
    QgsTiledSceneLayer3DPropertiesWidget( QWidget *parent = nullptr );

    void syncToLayer( QgsTiledSceneLayer *layer );
    void apply();

  signals:
    void widgetChanged();

  private:
    QgsTiledSceneLayer *mLayer = nullptr;
};


class QgsTiledSceneLayer3DRendererWidget : public QgsMapLayerConfigWidget
{
    Q_OBJECT
  public:
    QgsTiledSceneLayer3DRendererWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, QWidget *parent = nullptr );

    void syncToLayer( QgsMapLayer *layer ) final;

  public slots:
    void apply() override;

  private:
    QgsTiledSceneLayer3DPropertiesWidget *mWidget = nullptr;
};


class QgsTiledSceneLayer3DRendererWidgetFactory : public QObject, public QgsMapLayerConfigWidgetFactory
{
    Q_OBJECT
  public:
    explicit QgsTiledSceneLayer3DRendererWidgetFactory( QObject *parent = nullptr );

    QgsMapLayerConfigWidget *createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool dockWidget, QWidget *parent ) const override;
    bool supportLayerPropertiesDialog() const override;
    bool supportsStyleDock() const override;
    bool supportsLayer( QgsMapLayer *layer ) const override;
    QString layerPropertiesPagePositionHint() const override;
};


#endif // QGSTILEDSCENELAYER3DRENDERERWIDGET_H
