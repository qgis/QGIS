/***************************************************************************
    qgslabelingwidget.h
    ---------------------
    begin                : September 2015
    copyright            : (C) 2015 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLABELINGWIDGET_H
#define QGSLABELINGWIDGET_H

#include <QWidget>

#include "ui_qgslabelingwidget.h"
#include "qgspallabeling.h"
#include "qgsvectorlayerlabeling.h"

#include "qgsmaplayerconfigwidget.h"

class QgsLabelingGui;
class QgsMapCanvas;
class QgsRuleBasedLabelingWidget;
class QgsVectorLayer;
class QgsMapLayer;

/**
 * Master widget for configuration of labeling of a vector layer
 */
class QgsLabelingWidget : public QgsMapLayerConfigWidget, private Ui::QgsLabelingWidget
{
    Q_OBJECT
  public:
    QgsLabelingWidget( QgsVectorLayer *layer, QgsMapCanvas *canvas, QWidget *parent = nullptr );

    /**
     * Returns the labeling gui widget or a nullptr if none.
     *
     * \since QGIS 3.0
     */
    QgsLabelingGui *labelingGui();

  public slots:
    void setLayer( QgsMapLayer *layer );
    //! save config to layer
    void writeSettingsToLayer();

    //! Saves the labeling configuration and immediately updates the map canvas to reflect the changes
    void apply();

    //! reload the settings shown in the dialog from the current layer
    void adaptToLayer();

    void resetSettings();

  signals:

    void auxiliaryFieldCreated();

  protected slots:
    void labelModeChanged( int index );
    void showEngineConfigDialog();

  protected:
    QgsVectorLayer *mLayer = nullptr;
    QgsMapCanvas *mCanvas = nullptr;

    QWidget *mWidget = nullptr;
    std::unique_ptr< QgsPalLayerSettings > mSimpleSettings;
    std::unique_ptr< QgsAbstractVectorLayerLabeling > mOldSettings;
};

#endif // QGSLABELINGWIDGET_H
