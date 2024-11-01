/***************************************************************************
    qgstiledscenerendererpropertieswidget.h
    ---------------------
    begin                : August 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTILEDSCENERENDERERPROPERTIESWIDGET_H
#define QGSTILEDSCENERENDERERPROPERTIESWIDGET_H

#include "qgis_sip.h"
#include "qgis_gui.h"

#include "ui_qgstiledscenerendererpropsdialogbase.h"
#include "qgsmaplayerconfigwidget.h"

class QgsTiledSceneLayer;
class QgsStyle;
class QgsTiledSceneRendererWidget;
class QgsMapCanvas;
class QgsSymbolWidgetContext;
class QgsMessageBar;

/**
 * \ingroup gui
 * \brief A generic widget for setting the 2D renderer for a tiled scene layer.
 *
 * \since QGIS 3.34
 */
class GUI_EXPORT QgsTiledSceneRendererPropertiesWidget : public QgsMapLayerConfigWidget, private Ui::QgsTiledSceneRendererPropsDialogBase
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsTiledSceneRendererPropertiesWidget, associated with the specified \a layer and \a style database.
     */
    QgsTiledSceneRendererPropertiesWidget( QgsTiledSceneLayer *layer, QgsStyle *style, QWidget *parent SIP_TRANSFERTHIS = nullptr );

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

    QgsTiledSceneLayer *mLayer = nullptr;
    QgsStyle *mStyle = nullptr;

    QgsTiledSceneRendererWidget *mActiveWidget = nullptr;

    QgsMapCanvas *mMapCanvas = nullptr;
    QgsMessageBar *mMessageBar = nullptr;

    bool mBlockChangedSignal = false;
};


#endif // QGSTILEDSCENERENDERERPROPERTIESWIDGET_H
