/***************************************************************************
  qgsvectorlayer3drendererwidget.h
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORLAYER3DRENDERERWIDGET_H
#define QGSVECTORLAYER3DRENDERERWIDGET_H

#include <memory>

#include "qgsmaplayerconfigwidget.h"
#include "qgsvectorlayer3drenderer.h"

class QCheckBox;
class QLabel;
class QStackedWidget;

class QgsLine3DSymbolWidget;
class QgsPoint3DSymbolWidget;
class QgsPolygon3DSymbolWidget;
class QgsVectorLayer;
class QgsMapCanvas;


//! Widget for configuration of 3D renderer of a vector layer
class QgsVectorLayer3DRendererWidget : public QgsMapLayerConfigWidget
{
    Q_OBJECT
  public:
    explicit QgsVectorLayer3DRendererWidget( QgsVectorLayer *layer, QgsMapCanvas *canvas, QWidget *parent = nullptr );

    void setLayer( QgsVectorLayer *layer );

    //! no transfer of ownership
    void setRenderer( const QgsVectorLayer3DRenderer *renderer );
    //! no transfer of ownership
    QgsVectorLayer3DRenderer *renderer();

  public slots:
    void apply() override;

  private slots:
    void onEnabledClicked();

  private:
    QCheckBox *chkEnabled = nullptr;
    QStackedWidget *widgetStack = nullptr;
    QgsLine3DSymbolWidget *widgetLine = nullptr;
    QgsPoint3DSymbolWidget *widgetPoint = nullptr;
    QgsPolygon3DSymbolWidget *widgetPolygon = nullptr;
    QLabel *widgetUnsupported = nullptr;

    std::unique_ptr<QgsVectorLayer3DRenderer> mRenderer;
};

#endif // QGSVECTORLAYER3DRENDERERWIDGET_H
