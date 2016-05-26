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

class GUI_EXPORT QgsMapLayerPropertiesFactory
{
  public:
    QgsMapLayerPropertiesFactory();

    virtual QgsVectorLayerPropertiesPage* createVectorLayerPropertiesPage( QgsVectorLayer* layer, QWidget* parent ) = 0;
    virtual QListWidgetItem* createVectorLayerPropertiesItem( QgsVectorLayer* layer, QListWidget* view ) = 0;
};

#endif // QGSLAYERPROPERTIESFACTORY_H
