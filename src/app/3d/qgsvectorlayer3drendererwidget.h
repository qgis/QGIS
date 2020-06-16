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
#include "qgsmaplayerconfigwidgetfactory.h"
#include "qgsvectorlayer3drenderer.h"

class QComboBox;
class QCheckBox;
class QLabel;
class QStackedWidget;

class QgsVectorLayer;
class QgsMapCanvas;

class QgsRuleBased3DRendererWidget;
class QgsSymbol3DWidget;
class QgsVectorLayer3DPropertiesWidget;


class QgsSingleSymbol3DRendererWidget : public QWidget
{
    Q_OBJECT
  public:
    QgsSingleSymbol3DRendererWidget( QWidget *parent = nullptr );

    //! no transfer of ownership
    void setLayer( QgsVectorLayer *layer );

    //! Returns the cloned symbol or NULLPTR.
    QgsAbstract3DSymbol *symbol();

  signals:
    void widgetChanged();

  private:
    QgsSymbol3DWidget *widgetSymbol = nullptr;

};



//! Widget for configuration of 3D renderer of a vector layer
class QgsVectorLayer3DRendererWidget : public QgsMapLayerConfigWidget
{
    Q_OBJECT
  public:
    explicit QgsVectorLayer3DRendererWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, QWidget *parent = nullptr );

    void syncToLayer( QgsMapLayer *layer ) override;

    void setDockMode( bool dockMode ) override;

    //! Only modifies 3D renderer so we do not want layer repaint (which would trigger unnecessary terrain map update)
    bool shouldTriggerLayerRepaint() const override { return false; }

  public slots:
    void apply() override;

  private slots:
    void onRendererTypeChanged( int index );

  private:
    QComboBox *cboRendererType = nullptr;
    QStackedWidget *widgetRendererStack = nullptr;
    QgsVectorLayer3DPropertiesWidget *widgetBaseProperties = nullptr;

    QLabel *widgetNoRenderer = nullptr;
    QgsSingleSymbol3DRendererWidget *widgetSingleSymbolRenderer = nullptr;
    QgsRuleBased3DRendererWidget *widgetRuleBasedRenderer = nullptr;
};

class QgsVectorLayer3DRendererWidgetFactory : public QObject, public QgsMapLayerConfigWidgetFactory
{
    Q_OBJECT
  public:
    explicit QgsVectorLayer3DRendererWidgetFactory( QObject *parent = nullptr );

    QgsMapLayerConfigWidget *createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool dockWidget, QWidget *parent ) const override;
    bool supportLayerPropertiesDialog() const override;
    bool supportsLayer( QgsMapLayer *layer ) const override;
    QString layerPropertiesPagePositionHint() const override;
};



#endif // QGSVECTORLAYER3DRENDERERWIDGET_H
