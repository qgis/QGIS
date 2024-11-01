/***************************************************************************
    qgspointcloudrendererpropertieswidget.h
    ---------------------
    begin                : November 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTCLOUDRENDERERPROPERTIESWIDGET_H
#define QGSPOINTCLOUDRENDERERPROPERTIESWIDGET_H

#include "qgis_sip.h"
#include "qgis_gui.h"

#include "ui_qgspointcloudrendererpropsdialogbase.h"
#include "qgsmaplayerconfigwidget.h"

class QgsPointCloudLayer;
class QgsStyle;
class QgsPointCloudRendererWidget;
class QgsMapCanvas;
class QgsSymbolWidgetContext;
class QgsMessageBar;

/**
 * \ingroup gui
 * \brief A generic widget for setting the 2D renderer for a point cloud layer.
 *
 * \since QGIS 3.18
 */
class GUI_EXPORT QgsPointCloudRendererPropertiesWidget : public QgsMapLayerConfigWidget, private Ui::QgsPointCloudRendererPropsDialogBase
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsPointCloudRendererPropertiesWidget, associated with the specified \a layer and \a style database.
     */
    QgsPointCloudRendererPropertiesWidget( QgsPointCloudLayer *layer, QgsStyle *style, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets the \a context in which the widget is shown, e.g., the associated map canvas and expression contexts.
     */
    void setContext( const QgsSymbolWidgetContext &context );

    void syncToLayer( QgsMapLayer *layer ) final;
    void setDockMode( bool dockMode ) final;

  public slots:

    void apply() override;

  private slots:

    void rendererChanged();

    void emitWidgetChanged();

  private:
    static void initRendererWidgetFunctions();

    QgsPointCloudLayer *mLayer = nullptr;
    QgsStyle *mStyle = nullptr;

    QgsPointCloudRendererWidget *mActiveWidget = nullptr;

    QgsMapCanvas *mMapCanvas = nullptr;
    QgsMessageBar *mMessageBar = nullptr;

    bool mBlockChangedSignal = false;
};


#endif // QGSPOINTCLOUDRENDERERPROPERTIESWIDGET_H
