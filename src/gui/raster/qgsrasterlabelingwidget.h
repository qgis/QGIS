/***************************************************************************
    qgsrasterlabelingwidget.h
    -------------------------
    begin                : December 2024
    copyright            : (C) 2024 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRASTERLABELINGWIDGET_H
#define QGSRASTERLABELINGWIDGET_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include <QWidget>

#include "ui_qgsrasterlabelingwidgetbase.h"
#include "qgsrasterlabeling.h"
#include "qgis_gui.h"

#include "qgsmaplayerconfigwidget.h"

class QgsLabelingGui;
class QgsMapCanvas;
class QgsRasterLayer;
class QgsMapLayer;
class QgsMessageBar;

/**
 * \ingroup gui
 * \brief Master widget for configuration of labeling of a raster layer
 * \note Not available in Python bindings
 * \since QGIS 3.42
 */
class GUI_EXPORT QgsRasterLabelingWidget : public QgsMapLayerConfigWidget, private Ui::QgsRasterLabelingWidgetBase
{
    Q_OBJECT
  public:
    //! constructor
    QgsRasterLabelingWidget( QgsRasterLayer *layer, QgsMapCanvas *canvas, QWidget *parent = nullptr, QgsMessageBar *messageBar = nullptr );

    void setDockMode( bool dockMode ) override;

  public slots:
    //! Sets the layer to configure
    void setLayer( QgsMapLayer *layer );

    /**
     * Saves the labeling configuration to the destination layer.
     */
    void writeSettingsToLayer();

    //! Saves the labeling configuration and immediately updates the map canvas to reflect the changes
    void apply() override;

    //! Reload the settings shown in the dialog from the current layer
    void adaptToLayer();

  signals:
    //! Emitted when an auxiliary field is created
    void auxiliaryFieldCreated();

  private slots:
    void labelModeChanged( int index );
    void showLabelingEngineRulesPrivate();
    void showEngineConfigDialogPrivate();

  private:
    QgsRasterLayer *mLayer = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QgsMessageBar *mMessageBar = nullptr;

    QWidget *mWidget = nullptr;
};

#endif // QGSRASTERLABELINGWIDGET_H
