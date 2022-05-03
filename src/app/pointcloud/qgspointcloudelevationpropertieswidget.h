/***************************************************************************
    qgspointcloudelevationpropertieswidget.h
    ---------------------
    begin                : December 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTCLOUDELEVATIONPROPERTIESWIDGET_H
#define QGSPOINTCLOUDELEVATIONPROPERTIESWIDGET_H

#include "qgsmaplayerconfigwidget.h"
#include "qgsmaplayerconfigwidgetfactory.h"

#include "ui_qgspointcloudelevationpropertieswidgetbase.h"

class QgsPointCloudLayer;

class QgsPointCloudElevationPropertiesWidget : public QgsMapLayerConfigWidget, private Ui::QgsPointCloudElevationPropertiesWidgetBase
{
    Q_OBJECT
  public:

    QgsPointCloudElevationPropertiesWidget( QgsPointCloudLayer *layer, QgsMapCanvas *canvas, QWidget *parent );

    void syncToLayer( QgsMapLayer *layer ) final;
    bool shouldTriggerLayerRepaint() const override { return false; }

  public slots:
    void apply() override;

  private slots:

    void onChanged();
    void shiftPointCloudZAxis();
  private:

    QgsPointCloudLayer *mLayer = nullptr;
    bool mBlockUpdates = false;

};


class QgsPointCloudElevationPropertiesWidgetFactory : public QObject, public QgsMapLayerConfigWidgetFactory
{
    Q_OBJECT
  public:
    explicit QgsPointCloudElevationPropertiesWidgetFactory( QObject *parent = nullptr );

    QgsMapLayerConfigWidget *createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool dockWidget, QWidget *parent ) const override;
    bool supportLayerPropertiesDialog() const override;
    bool supportsStyleDock() const override;
    bool supportsLayer( QgsMapLayer *layer ) const override;
    QString layerPropertiesPagePositionHint() const override;
};



#endif // QGSPOINTCLOUDELEVATIONPROPERTIESWIDGET_H
