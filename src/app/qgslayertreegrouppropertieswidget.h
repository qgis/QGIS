/***************************************************************************
    qgslayertreegrouppropertieswidget.h
    ---------------------
    begin                : November 2021
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERTREEGROUPPROPERTIESWIDGET_H
#define QGSLAYERTREEGROUPPROPERTIESWIDGET_H

#include "qgsmaplayerconfigwidget.h"
#include "qgsmaplayerconfigwidgetfactory.h"
#include "ui_qgslayertreegrouppropertieswidgetbase.h"
#include <QPointer>

class QgsAnnotationLayer;
class QgsAnnotationItemBaseWidget;
class QStackedWidget;

class QgsLayerTreeGroupPropertiesWidget : public QgsMapLayerConfigWidget, public Ui::QgsLayerTreeGroupPropertiesWidgetBase
{
    Q_OBJECT
  public:

    QgsLayerTreeGroupPropertiesWidget( QgsMapCanvas *canvas, QWidget *parent );
    ~QgsLayerTreeGroupPropertiesWidget() override;

    void syncToLayer( QgsMapLayer *layer ) override;
    void setMapLayerConfigWidgetContext( const QgsMapLayerConfigWidgetContext &context ) override;
    void setDockMode( bool dockMode ) override;

  public slots:
    void apply() override;

  private slots:

    void onLayerPropertyChanged();
  private:

    QPointer< QgsLayerTreeGroup > mLayerTreeGroup;
    bool mBlockLayerUpdates = false;

    std::unique_ptr< QgsPaintEffect > mPaintEffect;

};


class QgsLayerTreeGroupPropertiesWidgetFactory : public QObject, public QgsMapLayerConfigWidgetFactory
{
    Q_OBJECT
  public:
    explicit QgsLayerTreeGroupPropertiesWidgetFactory( QObject *parent = nullptr );

    QgsMapLayerConfigWidget *createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool dockWidget, QWidget *parent ) const override;
    bool supportLayerPropertiesDialog() const override;
    bool supportsStyleDock() const override;
    bool supportsLayer( QgsMapLayer *layer ) const override;
    bool supportsLayerTreeGroup( QgsLayerTreeGroup *group ) const override;
};



#endif // QGSLAYERTREEGROUPPROPERTIESWIDGET_H
