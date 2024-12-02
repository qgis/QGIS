/***************************************************************************
      qgspostgresrastertemporalsettingswidget.h
      ------------------
    begin                : March 2021
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOSTGRESRASTERTEMPORALSETTINGSWIDGET_H
#define QGSPOSTGRESRASTERTEMPORALSETTINGSWIDGET_H

#include "ui_qgspostgresrastertemporalsettingswidgetbase.h"
#include "qgsmaplayerconfigwidget.h"
#include "qgsmaplayerconfigwidgetfactory.h"

class QgsRasterLayer;

class QgsPostgresRasterTemporalSettingsWidget : public QgsMapLayerConfigWidget, private Ui::QgsPostgresRasterTemporalSettingsWidgetBase
{
    Q_OBJECT

  public:
    QgsPostgresRasterTemporalSettingsWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, QWidget *parent = nullptr );

    void syncToLayer( QgsMapLayer *layer ) final;
    void apply() override;
  private slots:

  private:
    QgsRasterLayer *mRasterLayer = nullptr;
};

class QgsPostgresRasterTemporalSettingsConfigWidgetFactory : public QgsMapLayerConfigWidgetFactory
{
  public:
    bool supportLayerPropertiesDialog() const override;
    bool supportsLayer( QgsMapLayer *layer ) const override;
    ParentPage parentPage() const override;
    QgsMapLayerConfigWidget *createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool dockWidget = true, QWidget *parent = nullptr ) const override;
};

#endif // QGSPOSTGRESRASTERTEMPORALSETTINGSWIDGET_H
