/***************************************************************************
    qgsrasterelevationpropertieswidget.h
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

#ifndef QGSRASTERELEVATIONPROPERTIESWIDGET_H
#define QGSRASTERELEVATIONPROPERTIESWIDGET_H

#include "qgsmaplayerconfigwidget.h"
#include "qgsmaplayerconfigwidgetfactory.h"

#include "ui_qgsrasterelevationpropertieswidgetbase.h"

class QgsRasterLayer;

class QgsRasterElevationPropertiesWidget : public QgsMapLayerConfigWidget, private Ui::QgsRasterElevationPropertiesWidgetBase
{
    Q_OBJECT
  public:

    QgsRasterElevationPropertiesWidget( QgsRasterLayer *layer, QgsMapCanvas *canvas, QWidget *parent );

    void syncToLayer( QgsMapLayer *layer ) override;

  public slots:
    void apply() override;

  private slots:

    void onChanged();

  private:

    QgsRasterLayer *mLayer = nullptr;
    bool mBlockUpdates = false;

};


class QgsRasterElevationPropertiesWidgetFactory : public QObject, public QgsMapLayerConfigWidgetFactory
{
    Q_OBJECT
  public:
    explicit QgsRasterElevationPropertiesWidgetFactory( QObject *parent = nullptr );

    QgsMapLayerConfigWidget *createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool dockWidget, QWidget *parent ) const override;
    bool supportLayerPropertiesDialog() const override;
    bool supportsStyleDock() const override;
    bool supportsLayer( QgsMapLayer *layer ) const override;
    QString layerPropertiesPagePositionHint() const override;
};



#endif // QGSRASTERELEVATIONPROPERTIESWIDGET_H
