/***************************************************************************
  qgstracer.h
  --------------------------------------
  Date                 : January 2016
  Copyright            : (C) 2016 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTRACER_H
#define QGSTRACER_H

class QgsVectorLayer;

#include <QSet>
#include <QVector>

#include "qgspoint.h"
#include "qgscoordinatereferencesystem.h"

struct QgsTracerGraph;

/**
 * Utility class that construct a planar graph from the input vector
 * layers and provides shortest path search for tracing of existing
 * features.
 *
 * @note added in QGIS 2.14
 */
class CORE_EXPORT QgsTracer : public QObject
{
    Q_OBJECT
  public:
    QgsTracer();
    ~QgsTracer();

    //! Get layers used for tracing
    QList<QgsVectorLayer*> layers() const { return mLayers; }
    //! Set layers used for tracing
    void setLayers( const QList<QgsVectorLayer*>& layers );

    //! Get CRS used for tracing
    QgsCoordinateReferenceSystem destinationCrs() const { return mCRS; }
    //! Set CRS used for tracing
    void setDestinationCrs( const QgsCoordinateReferenceSystem& crs );

    //! Build the internal data structures. This may take some time
    //! depending on how big the input layers are. It is not necessary
    //! to call this method explicitly - it will be called by findShortestPath()
    //! if necessary.
    void init();

    //! Given two points, find the shortest path and return points on the way.
    //! If the points are not located on existing vertices or edges,
    //! search will fail and return empty array. The search will also fail
    //! if the two points are not connected.
    QVector<QgsPoint> findShortestPath( const QgsPoint& p1, const QgsPoint& p2 );

  private:
    void initGraph();
    void invalidateGraph();

  private:
    //! Graph data structure for path searching
    QgsTracerGraph* mGraph;
    //! Input layers for the graph building
    QList<QgsVectorLayer*> mLayers;
    //! Destination CRS in which graph is built and tracing done
    QgsCoordinateReferenceSystem mCRS;
};


#endif // QGSTRACER_H
