/***************************************************************************
                         qgsmaplayerrefreshsettingswidget.h
                         ------------------------------
    begin                : August 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPLAYERREFRESHSETTINGSWIDGET_H
#define QGSMAPLAYERREFRESHSETTINGSWIDGET_H

#include "ui_qgslayerrefreshwidgetbase.h"
#include "qgis_gui.h"
#include <QPointer>

#define SIP_NO_FILE

class QgsMapLayer;

/**
 * \ingroup gui
 * \class QgsMapLayerRefreshSettingsWidget
 * \brief A widget for configuring the automatic refresh settings for map layers.
 *
 * \note Not available in Python bindings
 * \since QGIS 3.34
 */
class GUI_EXPORT QgsMapLayerRefreshSettingsWidget : public QWidget, private Ui::QgsLayerRefreshWidgetBase
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsMapLayerRefreshSettingsWidget.
     */
    QgsMapLayerRefreshSettingsWidget( QWidget *parent = nullptr, QgsMapLayer *layer = nullptr );

  public slots:

    /**
     * Sets the \a layer associated with the widget.
     */
    void setLayer( QgsMapLayer *layer );

    /**
     * Saves the settings to the layer.
     */
    void saveToLayer();

    /**
     * Updates the widget state to match the current layer state.
     */
    void syncToLayer();

  private slots:

    void updateHelp();

  private:
    QPointer<QgsMapLayer> mLayer;
};
#endif // QGSMAPLAYERREFRESHSETTINGSWIDGET_H
