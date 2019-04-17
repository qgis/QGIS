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

class QComboBox;
class QCheckBox;
class QLabel;
class QStackedWidget;

class QgsVectorLayer;
class QgsMapCanvas;

class QgsRuleBased3DRendererWidget;
class QgsSymbol3DWidget;


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
    explicit QgsVectorLayer3DRendererWidget( QgsVectorLayer *layer, QgsMapCanvas *canvas, QWidget *parent = nullptr );

    void setLayer( QgsVectorLayer *layer );

    void setDockMode( bool dockMode ) override;

  public slots:
    void apply() override;

  private slots:
    void onRendererTypeChanged( int index );

  private:
    QComboBox *cboRendererType = nullptr;
    QStackedWidget *widgetRendererStack = nullptr;

    QLabel *widgetNoRenderer = nullptr;
    QgsSingleSymbol3DRendererWidget *widgetSingleSymbolRenderer = nullptr;
    QgsRuleBased3DRendererWidget *widgetRuleBasedRenderer = nullptr;
};


#endif // QGSVECTORLAYER3DRENDERERWIDGET_H
