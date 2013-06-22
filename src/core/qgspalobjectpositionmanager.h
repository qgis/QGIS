/***************************************************************************
                        qgspalobjectpositionmanager.h  -  description
                        ---------------------------------
   begin                : October 2008
   copyright            : (C) 2008 by Marco Hugentobler
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

#ifndef QGSPALOBJECTPOSITIONMANAGER_H
#define QGSPALOBJECTPOSITIONMANAGER_H

#include "qgsoverlayobjectpositionmanager.h"
#include "pal.h"

class QgsPALGeometry;

/**A class that uses the PAL library for overlay object placement
* \note This class has been added in version 1.1
*/
class CORE_EXPORT QgsPALObjectPositionManager: public QgsOverlayObjectPositionManager
{
  public:
    QgsPALObjectPositionManager();
    ~QgsPALObjectPositionManager();
    void addLayer( QgsVectorLayer* vl, QList<QgsVectorOverlay*>& overlays );
    void findObjectPositions( const QgsRenderContext& renderContext, QGis::UnitType unitType );
    void removeLayers();

    /**Sets algorithm used for object placements. Possibilities are:
      Chain, Popmusic tabu chain, Popmusic tabu, Popmusic chain*/
    void setPlacementAlgorithm( const QString& algorithmName );

  private:
    pal::Pal mPositionEngine;
    int mNumberOfLayers;
    /**Internal storage of the pal geometries between addLayer and findObjectPositions operations*/
    QList<QgsPALGeometry*> mPALGeometries;

    void deletePALGeometries();
};
#endif
