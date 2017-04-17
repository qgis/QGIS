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

#include "qgis_core.h"
#include "qgspoint.h"

class QgsVectorLayer;

/** \ingroup core
 * Represents the result of a snapping operation.
 * */
// ### QGIS 3: remove from API
struct CORE_EXPORT QgsSnappingResult
{
  //! Snap to vertex, to segment or both
  enum SnappingType
  {
    SnapToVertex,
    SnapToSegment,
    //snap to vertex and also to segment if no vertex is within the search tolerance
    SnapToVertexAndSegment
  };

  //! The coordinates of the snapping result
  QgsPoint snappedVertex;

  /** The vertex index of snappedVertex
   or -1 if no such vertex number (e.g. snap to segment)*/
  int snappedVertexNr;
  //! The layer coordinates of the vertex before snappedVertex
  QgsPoint beforeVertex;

  /** The index of the vertex before snappedVertex
   or -1 if no such vertex*/
  int beforeVertexNr;
  //! The layer coordinates of the vertex after snappedVertex
  QgsPoint afterVertex;

  /** The index of the vertex after snappedVertex
   or -1 if no such vertex*/
  int afterVertexNr;
  //! Index of the snapped geometry
  QgsFeatureId snappedAtGeometry;
  //! Layer where the snap occurred
  const QgsVectorLayer *layer = nullptr;
};

#endif
