/***************************************************************************
  graph.h
  --------------------------------------
  Date                 : 2011-04-01
  Copyright            : (C) 2010 by Yakushev Sergey
  Email                : YakushevS <at> list.ru
****************************************************************************
*                                                                          *
*   This program is free software; you can redistribute it and/or modify   *
*   it under the terms of the GNU General Public License as published by   *
*   the Free Software Foundation; either version 2 of the License, or      *
*   (at your option) any later version.                                    *
*                                                                          *
***************************************************************************/

/*
 * This file describes the built-in QGIS classes modeling a mathematical graph.
 * Vertex is identified by its geographic coordinates (but you can add two vertex
 * with unique coordinate), no additional properties it can not be assigned.
 * Count the number of properties not limited along the arc. Graph may
 * be have incidence arcs.
 *
 * \file qgsgraph.h
 */

#ifndef QGSGRAPHH
#define QGSGRAPHH

// QT4 includes
#include <QList>
#include <QVector>
#include <QVariant>

// QGIS includes
#include "qgspoint.h"

class QgsGraphVertex;

/**
 * \ingroup networkanalysis
 * \class QgsGraphArc
 * \brief This class implement a graph edge
 */
class ANALYSIS_EXPORT QgsGraphArc
{
  public:
    QgsGraphArc();

    /**
     * return property value
     * @param propertyIndex property index
     */
    QVariant property( int propertyIndex ) const;

    /**
     * get array of proertyes
     */
    QVector< QVariant > properties() const;

    /**
     * return index of outgoing vertex
     */
    int outVertex() const;

    /**
     * return index of incoming vertex
     */
    int inVertex() const;

  private:

    QVector< QVariant > mProperties;

    int mOut;
    int mIn;

    friend class QgsGraph;
};


typedef QList< int > QgsGraphArcIdList;

/**
 * \ingroup networkanalysis
 * \class QgsGraphVertex
 * \brief This class implement a graph vertex
 */
class ANALYSIS_EXPORT QgsGraphVertex
{
  public:
    /**
     * default constructor. It need for QT's container, e.g. QVector
     */
    QgsGraphVertex() {}

    /**
     * This constructor initializes QgsGraphVertex object and associates a vertex with a point
     */

    QgsGraphVertex( const QgsPoint& point );

    /**
     * return outgoing edges
     */
    QgsGraphArcIdList outArc() const;

    /**
     * return incoming edges
     */
    QgsGraphArcIdList inArc() const;

    /**
     * return vertex point
     */
    QgsPoint point() const;

  private:
    QgsPoint mCoordinate;
    QgsGraphArcIdList mOutArc;
    QgsGraphArcIdList mInArc;

    friend class QgsGraph;
};

/**
 * \ingroup networkanalysis
 * \class QgsGraph
 * \brief Mathematics graph representation
 */

class ANALYSIS_EXPORT QgsGraph
{
  public:
    QgsGraph();

    ~QgsGraph();

    // begin graph constructing methods
    /**
     * add vertex to a grap
     */
    int addVertex( const QgsPoint& pt );

    /**
     * add edge to a graph
     */
    int addArc( int outVertexIdx, int inVertexIdx, const QVector< QVariant >& properties );

    /**
     * retrun vertex count
     */
    int vertexCount() const;

    /**
     * return vertex at index
     */
    const QgsGraphVertex& vertex( int idx ) const;

    /**
      * retrun edge count
      */
    int arcCount() const;

    /**
     * retrun edge at index
     */
    const QgsGraphArc& arc( int idx ) const;

    /**
     * find vertex by point
     * \return vertex index
     */
    int findVertex( const QgsPoint& pt ) const;

  private:
    QVector<QgsGraphVertex> mGraphVertexes;

    QVector<QgsGraphArc> mGraphArc;
};

#endif //QGSGRAPHH
