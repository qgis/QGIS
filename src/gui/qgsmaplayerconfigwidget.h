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

#include "qgsmaplayer.h"
#include "qgspanelwidget.h"

class QgsMapCanvas;

/** \ingroup gui
 * \class QgsMapLayerConfigWidget
 * \brief A panel widget that can be shown in the map style dock
 * \note added in QGIS 2.16
 */
class GUI_EXPORT QgsMapLayerConfigWidget : public QgsPanelWidget
{
    Q_OBJECT
  public:

    /**
       * @brief A panel widget that can be shown in the map style dock
       * @param layer The layer active in the dock.
       * @param canvas The canvas object.
       * @param parent The parent of the widget.
       * @note The widget is created each time the panel is selected in the dock.
       * Keep the loading light as possible for speed in the UI.
       */
    QgsMapLayerConfigWidget( QgsMapLayer* layer, QgsMapCanvas *canvas, QWidget *parent = 0 );

  public slots:
    /**
     * @brief Called when changes to the layer need to be made.
     * Will be called when live update is enabled.
     */
    virtual void apply() = 0;

  protected:
    QgsMapLayer* mLayer;
    QgsMapCanvas* mMapCanvas;
};

#endif // QGSMAPLAYERCONFIGWIDGET_H
