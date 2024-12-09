/***************************************************************************
    qgsmeshlabelingwidget.h
    ---------------------
    begin                : November 2023
    copyright            : (C) 2023 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMESHLABELINGWIDGET_H
#define QGSMESHLABELINGWIDGET_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include <QWidget>

#include "ui_qgsmeshlabelingwidget.h"
#include "qgspallabeling.h"
#include "qgsmeshlayerlabeling.h"
#include "qgis_gui.h"

#include "qgsmaplayerconfigwidget.h"

class QgsLabelingGui;
class QgsMapCanvas;
class QgsMeshLayer;
class QgsMapLayer;
class QgsMessageBar;

/**
 * \ingroup gui
 * \brief Master widget for configuration of labeling of a mesh layer
 *
 * \note This class is not a part of public API
 * \since QGIS 3.36
 */
class GUI_EXPORT QgsMeshLabelingWidget : public QgsMapLayerConfigWidget, private Ui::QgsMeshLabelingWidget
{
    Q_OBJECT
  public:
    //! constructor
    QgsMeshLabelingWidget( QgsMeshLayer *layer, QgsMapCanvas *canvas, QWidget *parent = nullptr, QgsMessageBar *messageBar = nullptr );

    void setDockMode( bool dockMode ) override;

    /**
     * Returns the labeling gui widget or NULLPTR if none.
     */
    QgsLabelingGui *labelingGui();

  public slots:
    //! Sets the layer to configure
    void setLayer( QgsMapLayer *layer );
    //! save config to layer
    void writeSettingsToLayer();

    //! Saves the labeling configuration and immediately updates the map canvas to reflect the changes
    void apply() override;

    //! reload the settings shown in the dialog from the current layer
    void adaptToLayer();

    //! Reset the settings
    void resetSettings();

  signals:
    //! Emitted when an auxiliary field is created
    void auxiliaryFieldCreated();

  private slots:
    void labelModeChanged( int index );

  private:
    enum Mode
    {
      ModeNone,
      ModeVertices,
      ModeFaces
    };

    QgsMeshLayer *mLayer = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QgsMessageBar *mMessageBar = nullptr;

    QWidget *mWidget = nullptr;
    std::unique_ptr<QgsPalLayerSettings> mSettings;
    std::unique_ptr<QgsAbstractMeshLayerLabeling> mOldSettings;
    bool mOldLabelsEnabled = false;
};

#endif // QGSMESHLABELINGWIDGET_H
