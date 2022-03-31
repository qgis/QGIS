/***************************************************************************
    qgsmeshelevationpropertieswidget.h
    ---------------------
    begin                : February 2022
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

#ifndef QGSMESHELEVATIONPROPERTIESWIDGET_H
#define QGSMESHELEVATIONPROPERTIESWIDGET_H

#include "qgsmaplayerconfigwidget.h"
#include "qgsmaplayerconfigwidgetfactory.h"

#include "ui_qgsmeshelevationpropertieswidgetbase.h"

class QgsMeshLayer;

class QgsMeshElevationPropertiesWidget : public QgsMapLayerConfigWidget, private Ui::QgsMeshElevationPropertiesWidgetBase
{
    Q_OBJECT
  public:

    QgsMeshElevationPropertiesWidget( QgsMeshLayer *layer, QgsMapCanvas *canvas, QWidget *parent );

    void syncToLayer( QgsMapLayer *layer ) override;

  public slots:
    void apply() override;

  private slots:

    void onChanged();

  private:

    QgsMeshLayer *mLayer = nullptr;
    bool mBlockUpdates = false;

};


class QgsMeshElevationPropertiesWidgetFactory : public QObject, public QgsMapLayerConfigWidgetFactory
{
    Q_OBJECT
  public:
    explicit QgsMeshElevationPropertiesWidgetFactory( QObject *parent = nullptr );

    QgsMapLayerConfigWidget *createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool dockWidget, QWidget *parent ) const override;
    bool supportLayerPropertiesDialog() const override;
    bool supportsStyleDock() const override;
    bool supportsLayer( QgsMapLayer *layer ) const override;
    QString layerPropertiesPagePositionHint() const override;
};



#endif // QGSMESHELEVATIONPROPERTIESWIDGET_H
