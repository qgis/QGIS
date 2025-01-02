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

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include <QWidget>

#include "ui_qgslabelingwidget.h"
#include "qgspallabeling.h"
#include "qgsvectorlayerlabeling.h"
#include "qgis_gui.h"

#include "qgsmaplayerconfigwidget.h"

class QgsLabelingGui;
class QgsMapCanvas;
class QgsRuleBasedLabelingWidget;
class QgsVectorLayer;
class QgsMapLayer;
class QgsMessageBar;

/**
 * \ingroup gui
 * \brief Master widget for configuration of labeling of a vector layer
 *
 * \note This class is not a part of public API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsLabelingWidget : public QgsMapLayerConfigWidget, private Ui::QgsLabelingWidget
{
    Q_OBJECT
  public:
    //! constructor
    QgsLabelingWidget( QgsVectorLayer *layer, QgsMapCanvas *canvas, QWidget *parent = nullptr, QgsMessageBar *messageBar = nullptr );

    /**
     * Returns the labeling gui widget or NULLPTR if none.
     *
     */
    QgsLabelingGui *labelingGui();

    /**
     * Shows the labeling engine rules.
     *
     * The rules widget will either be shown as a dialog or an inline panel, depending on the \a parent widget.
     *
     * \since QGIS 3.42
     */
    static void showLabelingEngineRules( QWidget *parent, QgsMapCanvas *canvas );

    /**
     * Shows the labeling engine configuration.
     *
     * The config widget will either be shown as a dialog or an inline panel, depending on the \a parent widget.
     *
     * \since QGIS 3.42
     */
    static void showEngineConfiguration( QWidget *parent, QgsMapCanvas *canvas );

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
    void showLabelingEngineRulesPrivate();
    void showEngineConfigDialogPrivate();

  private:
    enum Mode
    {
      ModeNone,
      ModeSingle,
      ModeRuleBased,
      ModeBlocking
    };

    QgsVectorLayer *mLayer = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QgsMessageBar *mMessageBar = nullptr;

    QWidget *mWidget = nullptr;
    std::unique_ptr<QgsPalLayerSettings> mSimpleSettings;
    std::unique_ptr<QgsAbstractVectorLayerLabeling> mOldSettings;
    bool mOldLabelsEnabled = false;
};

#endif // QGSLABELINGWIDGET_H
