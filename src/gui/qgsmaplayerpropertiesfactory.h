/***************************************************************************
    qgslayeroptionsfactory.h
     --------------------------------------
    Date                 : 9.7.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERPROPERTIESFACTORY_H
#define QGSLAYERPROPERTIESFACTORY_H

#include <QListWidgetItem>

#include "qgsvectorlayerpropertiespage.h"
#include "qgsmapstylepanel.h"

/** \ingroup gui
 * \class QgsMapLayerPropertiesFactory
 * \note added in QGIS 2.16
 * Factory class for creating custom map layer property pages
 */
class GUI_EXPORT QgsMapLayerPanelFactory
{
  public:
    /** Constructor */
    QgsMapLayerPanelFactory();

    /** Destructor */
    virtual ~QgsMapLayerPanelFactory();

    /**
     * @brief The icon that will be shown in the UI for the panel.
     * @return A QIcon for the panel icon.
     */
    virtual QIcon icon() { return QIcon(); }

    /**
     * @brief The title of the panel.
     * @note This may or may not be shown to the user.
     * @return Title of the panel
     */
    virtual QString title() { return QString(); }

    /**
     * @brief Check if the layer is supported for this widget.
     * @return True if this layer is supported for this widget
     */
    virtual bool supportsLayer( QgsMapLayer *layer );

    /**
     * @brief Create a new properties page
     * @param layer The layer for which to create the page
     * @param parent The parent widget
     * @return The new properties page instance
     */
    virtual QgsMapLayerPanel* createPropertiesPage( QgsVectorLayer* layer, QWidget* parent );

    /**
     * @brief Factory fucntion to create the widget on demand as needed by the dock.
     * @note This function is called each time the panel is selected. Keep it light for better UX.
     * @param layer The active layer in the dock.
     * @param canvas The map canvas.
     * @param parent The parent of the widget.
     * @return A new QgsMapStylePanel which is shown in the map style dock.
     */
    virtual QgsMapLayerPanel* createPanel( QgsMapLayer* layer, QgsMapCanvas *canvas, QWidget* parent );
};

#endif // QGSLAYERPROPERTIESFACTORY_H
