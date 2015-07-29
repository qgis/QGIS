/***************************************************************************
                              qgsmapcanvassnapper.h
                              ---------------------
  begin                : June 21, 2007
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

#ifndef QGSMAPCANVASSNAPPER_H
#define QGSMAPCANVASSNAPPER_H

#include <QList>
#include "qgssnapper.h"

class QgsMapCanvas;
class QPoint;

/** \ingroup gui
 * This class reads the snapping properties from the current project and
 * configures a QgsSnapper to perform the snapping.
 * Snapping can be done to the active layer  (useful for selecting a vertex to
 * manipulate) or to background layers
 */
class GUI_EXPORT QgsMapCanvasSnapper
{
  public:
    /** Constructor
     @param canvas the map canvas to snap to*/
    QgsMapCanvasSnapper( QgsMapCanvas* canvas );

    QgsMapCanvasSnapper();

    ~QgsMapCanvasSnapper();

    /** Does a snap to the current layer. Uses snap mode
       QgsSnapper::SnapWithResultsForSamePosition if topological editing is enabled
       and QgsSnapper::SnapWithOneResult_BY_SEGMENT if not. As this method is usually used to
       find vertices/segments for editing operations, it uses the search radius for vertex
       editing from the qgis options.
       @param p start point of the snap (in pixel coordinates)
       @param results list to which the results are appended
       @param snap_to snap to vertex or to segment
       @param snappingTol snapping tolerance. -1 means that the search radius for vertex edits is taken
       @param excludePoints a list with (map coordinate) points that should be excluded in the snapping result. Useful e.g. for vertex moves where a vertex should not be snapped to its original position*/
    int snapToCurrentLayer( const QPoint& p, QList<QgsSnappingResult>& results, QgsSnapper::SnappingType snap_to, double snappingTol = -1, const QList<QgsPoint>& excludePoints = QList<QgsPoint>() );
    /** Snaps to the background layers. This method is useful to align the features of the
       edited layers to those of other layers (as described in the project properties).
       Uses snap mode QgsSnapper::SnapWithOneResult. Therefore, only the
       closest result is returned.
       @param p start point of the snap (in pixel coordinates)
       @param results snapped points
       @param excludePoints a list with (map coordinate) points that should be excluded in the snapping result. Useful e.g. for vertex moves where a vertex should not be snapped to its original position
       @return 0 in case of success*/
    int snapToBackgroundLayers( const QPoint& p, QList<QgsSnappingResult>& results, const QList<QgsPoint>& excludePoints = QList<QgsPoint>() );
    int snapToBackgroundLayers( const QgsPoint& point, QList<QgsSnappingResult>& results, const QList<QgsPoint>& excludePoints = QList<QgsPoint>() );

    void setMapCanvas( QgsMapCanvas* canvas );

  private:
    /** Pointer to the map canvas*/
    QgsMapCanvas* mMapCanvas;
    /** The object which does the snapping operations*/
    QgsSnapper* mSnapper;
};

#endif
