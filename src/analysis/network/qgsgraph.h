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
 * 
 * \file qgsgraph.h
 * Этот файл описывает встроенные в QGIS классы описывающие математический граф. Вершина графа идентифицируется своими географическими координатами, никакие дополнительные свойства ей не могут быть присвоены. Количество свойств графа определяется разработчиком и не ограничено, например длина и время движения по дуге. Граф может быть направленным, иметь инцедентные ребра и петли.
 * 
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
 * \ingroup analysis
 * \class QgsGraphEdge
 * \brief This class implement a graph edge
 */
class ANALYSIS_EXPORT QgsGraphEdge
{
  public:
    QgsGraphEdge();

    /**
     * return property value
     * @param propertyIndex property index
     */
    QVariant property(int propertyIndex ) const;
    
    /**
     * get array of proertyes
     */
    QVector< QVariant > properties() const;

    /**
     * return index of outgoing vertex
     */
    int out() const;

    /**
     * return index of incoming vertex
     */
    int in() const;

  private:

    QVector< QVariant > mProperties;

    int mOut;
    int mIn;

  friend class QgsGraph;
};


typedef QList< int > QgsGraphEdgeList;

/**
 * \ingroup analysis
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
    QgsGraphEdgeList outEdges() const;
    
    /**
     * return incoming edges
     */
    QgsGraphEdgeList inEdges() const;
    
    /**
     * return vertex point
     */
    QgsPoint point() const;
  
  private:
    QgsPoint mCoordinate;
    QgsGraphEdgeList mOutEdges;
    QgsGraphEdgeList mInEdges;

  friend class QgsGraph;
};

/**
 * \ingroup analysis
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
    int addEdge( int outVertexIdx, int inVertexIdx, const QVector< QVariant >& properties );
    
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
    int edgeCount() const;
   
    /** 
     * retrun edge at index
     */
    const QgsGraphEdge& edge( int idx ) const;

    /**
     * find vertex by point
     * \return vertex index
     */
    int findVertex( const QgsPoint& pt ) const;

  private: 
    QVector<QgsGraphVertex> mGraphVertexes;
    
    QVector<QgsGraphEdge> mGraphEdges;
};

#endif //QGSGRAPHH
