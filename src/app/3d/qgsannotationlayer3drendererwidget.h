/***************************************************************************
  qgsannotationlayer3drendererwidget.h
  ------------------------------
  Date                 : September 2025
  Copyright            : (C) 2025 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSANNOTATIONLAYER3DRENDERERWIDGET_H
#define QGSANNOTATIONLAYER3DRENDERERWIDGET_H

#include "ui_qgsannotationlayer3drendererwidget.h"

#include <memory>

#include "qgsmaplayerconfigwidget.h"
#include "qgsmaplayerconfigwidgetfactory.h"

class QgsAnnotationLayer;
class QgsAnnotationLayer3DRenderer;
class QgsMapCanvas;


//! Widget for configuration of 3D renderer of an annotation layer
class QgsAnnotationLayer3DRendererWidget : public QgsMapLayerConfigWidget, private Ui::QgsAnnotationLayer3dRendererWidgetBase
{
    Q_OBJECT
  public:
    enum class RendererType
    {
      None,
      Billboards,
    };
    Q_ENUM( RendererType )

    explicit QgsAnnotationLayer3DRendererWidget( QgsAnnotationLayer *layer, QgsMapCanvas *canvas, QWidget *parent = nullptr );

    void syncToLayer( QgsMapLayer *layer ) final;

    //! no transfer of ownership
    void setRenderer( const QgsAnnotationLayer3DRenderer *renderer );
    std::unique_ptr< QgsAnnotationLayer3DRenderer > renderer();

  public slots:
    void apply() override;

  private slots:
    void rendererTypeChanged();
    void clampingChanged();

  private:
    int mBlockChanges = 0;
    std::unique_ptr<QgsAnnotationLayer3DRenderer> mRenderer;
};

class QgsAnnotationLayer3DRendererWidgetFactory : public QObject, public QgsMapLayerConfigWidgetFactory
{
    Q_OBJECT
  public:
    explicit QgsAnnotationLayer3DRendererWidgetFactory( QObject *parent = nullptr );

    QgsMapLayerConfigWidget *createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool dockWidget, QWidget *parent ) const override;
    bool supportLayerPropertiesDialog() const override;
    bool supportsStyleDock() const override;
    bool supportsLayer( QgsMapLayer *layer ) const override;
    QString layerPropertiesPagePositionHint() const override;
};

#endif // QGSANNOTATIONLAYER3DRENDERERWIDGET_H
