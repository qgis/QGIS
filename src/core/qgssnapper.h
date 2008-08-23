/***************************************************************************
                              qgssnapper.h
                              ------------
  begin                : June 7, 2007
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

#ifndef QGSSNAPPER_H
#define QGSSNAPPER_H

#include "qgspoint.h"
#include <QList>
#include <QMultiMap>

class QgsMapRenderer;
class QgsVectorLayer;
class QPoint;

/** \ingroup core
 * Represents the result of a snapping operation.
 * */
struct CORE_EXPORT QgsSnappingResult
{
  /**The coordinates of the snapping result*/
  QgsPoint snappedVertex;
  /**The vertex index of snappedVertex
   or -1 if no such vertex number (e.g. snap to segment)*/
  int snappedVertexNr;
  /**The layer coordinates of the vertex before snappedVertex*/
  QgsPoint beforeVertex;
  /**The index of the vertex before snappedVertex
   or -1 if no such vertex*/
  int beforeVertexNr;
  /**The layer coordinates of the vertex after snappedVertex*/
  QgsPoint afterVertex;
  /**The index of the vertex after snappedVertex
   or -1 if no such vertex*/
  int afterVertexNr;
  /**Index of the snapped geometry*/
  int snappedAtGeometry;
  /**Layer where the snap occured*/
  const QgsVectorLayer* layer;
};



/**A class that allows advanced snapping operations on a set of vector layers*/
class CORE_EXPORT QgsSnapper
{
  public:
    /**Snap to vertex, to segment or both*/
    enum SNAP_TO
    {
      SNAP_TO_VERTEX,
      SNAP_TO_SEGMENT,
      //snap to vertex and also to segment if no vertex is within the search tolerance
      SNAP_TO_VERTEX_AND_SEGMENT
    };

    enum SNAP_MODE
    {
      /**Only one snapping result is retured*/
      ONE_RESULT,
      /**Several snapping results which have the same position are returned. This is usefull for topological
       editing*/
      SEVERAL_RESULTS_SAME_POSITION,
      /**All results within the given layer tolerances are returned*/
      ALL_RESULTS_WITHIN_GIVEN_TOLERANCES
    };

    QgsSnapper( QgsMapRenderer* mapRender );
    ~QgsSnapper();
    /**Does the snapping operation
     @param startPoint the start point for snapping (in pixel coordinates)
    @param snappingResult the list where the results are inserted (everything in map coordinate system)
    @param excludePoints a list with (map coordinate) points that should be excluded in the snapping result. Useful e.g. for vertex moves where a vertex should not be snapped to its original position
    @return 0 in case of success*/
    int snapPoint( const QPoint& startPoint, QList<QgsSnappingResult>& snappingResult, const QList<QgsPoint>& excludePoints = QList<QgsPoint>() );

    //setters
    void setLayersToSnap( const QList<QgsVectorLayer*>& layerList );
    void setTolerances( const QList<double>& toleranceList );
    void setSnapToList( const QList<QgsSnapper::SNAP_TO>& snapToList );
    void setSnapMode( QgsSnapper::SNAP_MODE snapMode );

  private:
    /**Don't use the default constructor*/
    QgsSnapper();

    /**Removes the snapping results that contains points in exclude list*/
    void cleanResultList( QMultiMap<double, QgsSnappingResult>& list, const QList<QgsPoint>& excludeList ) const;

    /**The maprender object contains information about the output coordinate system
     of the map and about the relationship between pixel space and map space*/
    QgsMapRenderer* mMapRenderer;
    /**Snap mode to apply*/
    QgsSnapper::SNAP_MODE mSnapMode;
    /**The layers to which snapping is applied*/
    QList<QgsVectorLayer*> mLayersToSnap;
    /**The snapping tolerances for the layers. The order must correspond to the layer list.
     Note that the tolerances are always in source coordinate systems of the layers*/
    QList<double> mSnappingTolerances;
    /**List if snap to segment of to vertex. The order must correspond to the layer list*/
    QList<QgsSnapper::SNAP_TO> mSnapToList;
};

#endif
