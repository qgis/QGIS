/***************************************************************************
                         qgsoverlayobjectpositionmanager.h  -  description
                         ---------------------------------
    begin                : January 2007
    copyright            : (C) 2007 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOVERLAYOBJECTPOSITIONMANAGER_H
#define QGSOVERLAYOBJECTPOSITIONMANAGER_H

#include <QList>
#include "qgis.h"

class QgsRenderContext;
class QgsVectorLayer;
class QgsVectorOverlay;

/**Interface for classes that arrange overlay objects of different layers on the map
* \note This class has been added in version 1.1
*/
class QgsOverlayObjectPositionManager
{
  public:
    //virtual destructor needed for proper memory management
    virtual ~QgsOverlayObjectPositionManager() {}
    /**Adds a layer that may contain * overlays to the position manager. The overlay objects contained in the
    overlays will then be considered in label placement*/
    virtual void addLayer( QgsVectorLayer* vl, QList<QgsVectorOverlay*>& overlays ) = 0;
    /**Removes all the layers*/
    virtual void removeLayers() = 0;
    /**Calculate positions for the overlay objects
      @param context Context of rendering operation (Painter, scale factor)
      @param unitType meters, feet, degrees*/
    virtual void findObjectPositions( const QgsRenderContext& context, QGis::UnitType unitType ) = 0;
};

#endif
