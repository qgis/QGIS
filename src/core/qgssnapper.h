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
#include "qgstolerance.h"
#include "qgsfeature.h"

#include <QList>
#include <QMultiMap>

class QgsMapRenderer;
class QgsMapSettings;
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
  QgsFeatureId snappedAtGeometry;
  /**Layer where the snap occured*/
  const QgsVectorLayer* layer;
};



/**A class that allows advanced snapping operations on a set of vector layers*/
class CORE_EXPORT QgsSnapper
{
  public:
    /**Snap to vertex, to segment or both*/
    enum SnappingType
    {
      SnapToVertex,
      SnapToSegment,
      //snap to vertex and also to segment if no vertex is within the search tolerance
      SnapToVertexAndSegment
    };

    enum SnappingMode
    {
      /**Only one snapping result is returned*/
      SnapWithOneResult,
      /**Several snapping results which have the same position are returned.
         This is useful for topological editing*/
      SnapWithResultsForSamePosition,
      /**All results within the given layer tolerances are returned*/
      SnapWithResultsWithinTolerances
    };

    struct SnapLayer
    {
      /**The layer to which snapping is applied*/
      QgsVectorLayer* mLayer;
      /**The snapping tolerances for the layers, always in source coordinate systems of the layer*/
      double mTolerance;
      /**What snapping type to use (snap to segment or to vertex)*/
      QgsSnapper::SnappingType mSnapTo;
      /**What unit is used for tolerance*/
      QgsTolerance::UnitType mUnitType;
    };

    //!@ deprecated since 2.4 - use constructor with QgsMapSettings
    Q_DECL_DEPRECATED QgsSnapper( QgsMapRenderer* mapRender );

    explicit QgsSnapper( const QgsMapSettings& mapSettings );

    ~QgsSnapper();
    /** Does the snapping operation
     @param startPoint the start point for snapping (in pixel coordinates)
     @param snappingResult the list where the results are inserted (everything in map coordinate system)
     @param excludePoints a list with (map coordinate) points that should be excluded in the snapping result. Useful e.g. for vertex moves where a vertex should not be snapped to its original position
     @return 0 in case of success
     @deprecated
     */
    int snapPoint( const QPoint& startPoint, QList<QgsSnappingResult>& snappingResult, const QList<QgsPoint>& excludePoints = QList<QgsPoint>() );
    /** Does the snapping operation
     @param mapCoordPoint the start point for snapping (in map coordinates)
     @param snappingResult the list where the results are inserted (everything in map coordinate system)
     @param excludePoints a list with (map coordinate) points that should be excluded in the snapping result. Useful e.g. for vertex moves where a vertex should not be snapped to its original position
     @return 0 in case of success
    */
    int snapPoint( const QgsPoint& mapCoordPoint, QList<QgsSnappingResult>& snappingResult, const QList<QgsPoint>& excludePoints = QList<QgsPoint>() );

    //setters
    void setSnapLayers( const QList<QgsSnapper::SnapLayer>& snapLayers );
    void setSnapMode( QgsSnapper::SnappingMode snapMode );

  private:

    /**Removes the snapping results that contains points in exclude list*/
    void cleanResultList( QMultiMap<double, QgsSnappingResult>& list, const QList<QgsPoint>& excludeList ) const;

    /**The map settings object contains information about the output coordinate system
     of the map and about the relationship between pixel space and map space*/
    const QgsMapSettings& mMapSettings;
    /**Snap mode to apply*/
    QgsSnapper::SnappingMode mSnapMode;
    /**List of layers to which snapping is applied*/
    QList<QgsSnapper::SnapLayer> mSnapLayers;
};

#endif
