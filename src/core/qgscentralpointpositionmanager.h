/***************************************************************************
                         qgscentralpointpositionmanager.h  -  description
                         --------------------------------
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

#ifndef QGSCENTRALPOINTPOSITIONMANAGER
#define QGSCENTRALPOINTPOSITIONMANAGER

#include "qgsoverlayobjectpositionmanager.h"
#include "qgsrendercontext.h"

class QgsPoint;

/**A simple position manager implementation which positions the overlay objects on the center point of
a feature. It does not consider conflicts in case of several overlay layers
* \note This class has been added in version 1.1
*/
class QgsCentralPointPositionManager: public QgsOverlayObjectPositionManager
{
  public:
    QgsCentralPointPositionManager();
    ~QgsCentralPointPositionManager();
    void addLayer( QgsVectorLayer* vl, QList<QgsVectorOverlay*>& overlays );
    /**Removes all the overlays*/
    void removeLayers();
    void findObjectPositions( const QgsRenderContext& context, QGis::UnitType unitType );

  private:
    /**Calculates the central point for points/lines/polygons. Returns 0 in case of success*/
    int findObjectPosition( const unsigned char* wkb, QgsPoint& position ) const;
    /**Calculates the polygon centroid with the algorithm from Graphics gems IV: Centroid of a polygon.
     @return 0 in case of success*/
    int calculatePolygonCentroid( double x[], double y[], int numberOfPoints, double& centroidX, double& centroidY ) const;

    /**Stores all the overlay objects to retrieve all objects when positioning*/
    QList<QgsVectorOverlay*> mOverlays;
};

#endif
