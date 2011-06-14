/***************************************************************************
  simplegraphbuilder.h
  --------------------------------------
  Date                 : 2010-10-25
  Copyright            : (C) 2010 by Yakushev Sergey
  Email                : YakushevS@list.ru
****************************************************************************
*                                                                          *
*   This program is free software; you can redistribute it and/or modify   *
*   it under the terms of the GNU General Public License as published by   *
*   the Free Software Foundation; either version 2 of the License, or      *
*   (at your option) any later version.                                    *
*                                                                          *
***************************************************************************/
#ifndef ROADGRAPH_SIMPLEGRAPHBUILDER
#define ROADGRAPH_SIMPLEGRAPHBUILDER

#include "graphbuilder.h"
#include "utils.h"

//QT4 includes

//QGIS includes
#include <qgsspatialindex.h>

//forward declarations
class QgsDistanceArea;
class QgsCoordinateTransform;

/**
* \class RgSimpleGraphBuilder
* \brief This class making the simple graph
*/

class RgSimpleGraphBuilder : public RgGraphBuilder
{
  public:
    /**
     * default constructor
     */
    RgSimpleGraphBuilder( const QgsCoordinateReferenceSystem& crs, bool ctfEnabled, double topologyTolerance = 0.0 );

    /**
     * MANDATORY BUILDER PROPERTY DECLARATION
     */
    QgsPoint addVertex( const QgsPoint& pt );
    void addArc( const QgsPoint& pt1, const QgsPoint& pt2, double cost, double speed, int featureId );

    /**
     * return Adjacecncy matrix;
     */
    AdjacencyMatrix adjacencyMatrix();
  private:
    AdjacencyMatrix mMatrix;

    QgsSpatialIndex mPointIndex;

    QMap< int, QgsPoint> mPointMap;
};
#endif //SIMPLEGRAPHBUILDER
