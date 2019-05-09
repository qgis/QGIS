/***************************************************************************
    qgsmaplayerconfigwidget.h
    -------------------------
    begin                : June 2016
    copyright            : (C) 2016 by Nathan Woodrow
    email                : woodrow dot nathan at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMAPLAYERCONFIGWIDGET_H
#define QGSMAPLAYERCONFIGWIDGET_H

#include <QWidget>
#include <QIcon>

#include "qgspanelwidget.h"
#include "qgis_gui.h"

class QgsMapCanvas;
class QgsMapLayer;

/**
 * \ingroup gui
 * \class QgsMapLayerConfigWidget
 * \brief A panel widget that can be shown in the map style dock
 * \since QGIS 2.16
 */
class GUI_EXPORT QgsMapLayerConfigWidget : public QgsPanelWidget
{
    Q_OBJECT
  public:

    /**
       * \brief A panel widget that can be shown in the map style dock
       * \param layer The layer active in the dock.
       * \param canvas The canvas object.
       * \param parent The parent of the widget.
       * \note The widget is created each time the panel is selected in the dock.
       * Keep the loading light as possible for speed in the UI.
       */
    QgsMapLayerConfigWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, QWidget *parent = nullptr );

    /**
     * Whether this config widget changes map layer properties in a way that triggerRepaint() should
     * be called for the layer after applying changes. This is true by default, but some config widgets
     * (for example 3D rendering config) do not need layer repaint as they do not modify 2D map rendering.
     * \since QGIS 3.8
     */
    virtual bool shouldTriggerLayerRepaint() const { return true; }

  public slots:

    /**
     * \brief Called when changes to the layer need to be made.
     * Will be called when live update is enabled.
     */
    virtual void apply() = 0;

  protected:
    QgsMapLayer *mLayer = nullptr;
    QgsMapCanvas *mMapCanvas = nullptr;
};

#endif // QGSMAPLAYERCONFIGWIDGET_H
