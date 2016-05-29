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

/**
 * @brief Factory class for creating custom map layer property pages
 */
class GUI_EXPORT QgsMapLayerPropertiesFactory
{
  public:
    /** Constructor */
    QgsMapLayerPropertiesFactory();

    /** Destructor */
    virtual ~QgsMapLayerPropertiesFactory();

    /**
     * @brief Create a new properties page
     * @param layer The layer for which to create the page
     * @param parent The parent widget
     * @return The new properties page instance
     */
    virtual QgsVectorLayerPropertiesPage* createVectorLayerPropertiesPage( QgsVectorLayer* layer, QWidget* parent ) = 0;

    /**
     * @brief Creates the QListWidgetItem for the properties page
     * @param layer The layer for which to create the item
     * @param view The parent QListView
     * @return The QListWidgetItem for the properties page
     */
    virtual QListWidgetItem* createVectorLayerPropertiesItem( QgsVectorLayer* layer, QListWidget* view ) = 0;
};

#endif // QGSLAYERPROPERTIESFACTORY_H
