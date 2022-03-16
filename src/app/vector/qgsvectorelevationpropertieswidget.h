/***************************************************************************
    qgsvectorrelevationpropertieswidget.h
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

#ifndef QGSVECTORELEVATIONPROPERTIESWIDGET_H
#define QGSVECTORELEVATIONPROPERTIESWIDGET_H

#include "qgsmaplayerconfigwidget.h"
#include "qgsmaplayerconfigwidgetfactory.h"

#include "ui_qgsvectorelevationpropertieswidgetbase.h"

class QgsVectorLayer;

class QgsVectorElevationPropertiesWidget : public QgsMapLayerConfigWidget, private Ui::QgsVectorElevationPropertiesWidgetBase
{
    Q_OBJECT
  public:

    QgsVectorElevationPropertiesWidget( QgsVectorLayer *layer, QgsMapCanvas *canvas, QWidget *parent );

    void syncToLayer( QgsMapLayer *layer ) override;

  public slots:
    void apply() override;

  private slots:

    void onChanged();
    void clampingChanged();
    void bindingChanged();

  private:

    QgsVectorLayer *mLayer = nullptr;
    bool mBlockUpdates = false;

};


class QgsVectorElevationPropertiesWidgetFactory : public QObject, public QgsMapLayerConfigWidgetFactory
{
    Q_OBJECT
  public:
    explicit QgsVectorElevationPropertiesWidgetFactory( QObject *parent = nullptr );

    QgsMapLayerConfigWidget *createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool dockWidget, QWidget *parent ) const override;
    bool supportLayerPropertiesDialog() const override;
    bool supportsStyleDock() const override;
    bool supportsLayer( QgsMapLayer *layer ) const override;
    QString layerPropertiesPagePositionHint() const override;
};



#endif // QGSVECTORELEVATIONPROPERTIESWIDGET_H
