/***************************************************************************
    qgsmapstylepanel.h
    ---------------------
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
#ifndef QGSMAPSTYLEPANEL_H
#define QGSMAPSTYLEPANEL_H

#include <QWidget>
#include <QIcon>

#include "qgsmaplayer.h"
#include "qgspanelwidget.h"

class QgsMapCanvas;

/** \ingroup gui
 * \class QgsMapStylePanel
 * \brief A panel widget that can be shown in the map style dock
 * \note added in QGIS 2.16
 */
class GUI_EXPORT QgsMapStylingPanel : public QgsPanelWidget
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
    QgsMapStylingPanel( QgsMapLayer* layer, QgsMapCanvas *canvas, QWidget *parent = 0 );

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


/** \ingroup gui
 * \class QgsMapStylePanelFactory
 * \note added in QGIS 2.16
 */
class GUI_EXPORT QgsMapStylingPanelFactory
{
  public:
    Q_DECLARE_FLAGS( LayerTypesFlags, QgsMapLayer::LayerType )

    /** Constructor */
    QgsMapStylingPanelFactory();

    /** Destructor */
    virtual ~QgsMapStylingPanelFactory();

    /**
     * @brief The icon that will be shown in the UI for the panel.
     * @return A QIcon for the panel icon.
     */
    virtual QIcon icon() = 0;

    /**
     * @brief The title of the panel.
     * @note This may or may not be shown to the user.
     * @return Title of the panel
     */
    virtual QString title() = 0;

    /**
     * @brief Check if the layer is supported for this widget.
     * @return True if this layer is supported for this widget
     */
    virtual bool supportsLayer( QgsMapLayer *layer ) = 0;

    /**
     * @brief Factory fucntion to create the widget on demand as needed by the dock.
     * @note This function is called each time the panel is selected. Keep it light for better UX.
     * @param layer The active layer in the dock.
     * @param canvas The map canvas.
     * @param parent The parent of the widget.
     * @return A new QgsMapStylePanel which is shown in the map style dock.
     */
    virtual QgsMapStylingPanel* createPanel( QgsMapLayer* layer, QgsMapCanvas *canvas, QWidget* parent ) = 0;
};


#endif // QGSMAPSTYLEPANEL_H
