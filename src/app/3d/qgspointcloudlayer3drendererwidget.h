/***************************************************************************
  qgspointcloudlayer3drendererwidget.h
  ------------------------------
  Date                 : November 2020
  Copyright            : (C) 2020 by Nedjima Belgacem
  Email                : belgacem dot nedjima at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPOINTCLOUDLAYER3DRENDERERWIDGET_H
#define QGSPOINTCLOUDLAYER3DRENDERERWIDGET_H


#include <memory>

#include "qgsmaplayerconfigwidget.h"
#include "qgspointcloudlayer3drenderer.h"
#include "qgsmaplayerconfigwidgetfactory.h"
#include "qgspointcloud3dsymbol.h"

class QCheckBox;

class QgsPointCloudLayer;
class QgsMapCanvas;
class QgsPointCloud3DSymbolWidget;

//! Widget for configuration of 3D renderer of a point cloud layer
class QgsPointCloudLayer3DRendererWidget : public QgsMapLayerConfigWidget
{
    Q_OBJECT
  public:
    explicit QgsPointCloudLayer3DRendererWidget( QgsPointCloudLayer *layer, QgsMapCanvas *canvas, QWidget *parent = nullptr );

    void syncToLayer( QgsMapLayer *layer ) override;
    void setDockMode( bool dockMode ) override;

    //! no transfer of ownership
    void setRenderer( const QgsPointCloudLayer3DRenderer *renderer );
    //! no transfer of ownership
    QgsPointCloudLayer3DRenderer *renderer();


  public slots:
    void apply() override;

  private:
    QgsPointCloud3DSymbolWidget *mWidgetPointCloudSymbol = nullptr;
};

class QgsPointCloudLayer3DRendererWidgetFactory : public QObject, public QgsMapLayerConfigWidgetFactory
{
    Q_OBJECT
  public:
    explicit QgsPointCloudLayer3DRendererWidgetFactory( QObject *parent = nullptr );

    QgsMapLayerConfigWidget *createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool dockWidget, QWidget *parent ) const override;
    bool supportLayerPropertiesDialog() const override;
    bool supportsLayer( QgsMapLayer *layer ) const override;
    QString layerPropertiesPagePositionHint() const override;
    bool supportsStyleDock() const override;
};

#endif // QGSPOINTCLOUDLAYER3DRENDERERWIDGET_H
