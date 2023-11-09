/***************************************************************************
  qgsgraph.h
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
 * This file describes the built-in QGIS classes for modeling a mathematical graph.
 * Vertices are identified by their geographic coordinates and have no additional
 * properties. Number of strategies for calculating edge cost is not limited.
 * Graph may have incidence edges.
 *
 * \file qgsgraph.h
 */

#ifndef QGSGRAPH_H
#define QGSGRAPH_H

#include <QList>
#include <QVector>
#include <QVariant>

#include "qgspointxy.h"
#include "qgis_analysis.h"

class QgsGraphVertex;


/**
 * \ingroup analysis
 * \class QgsGraphEdge
 * \brief This class implements a graph edge
 * \since QGIS 3.0
 */
class ANALYSIS_EXPORT QgsGraphEdge
{
  public:

    /**
     * Constructor for QgsGraphEdge.
     */
    QgsGraphEdge() = default;

    /**
     * Returns edge cost calculated using specified strategy
     * \param strategyIndex strategy index
     */
    QVariant cost( int strategyIndex ) const;

    /**
     * Returns array of available strategies
     */
    QVector< QVariant > strategies() const;

    /**
     * Returns the index of the vertex at the end of this edge.
     * \see fromVertex()
     */
    int toVertex() const;

    /**
     * Returns the index of the vertex at the start of this edge.
     * \see toVertex()
     */
    int fromVertex() const;

  private:

    QVector< QVariant > mStrategies;

    int mToIdx = 0;
    int mFromIdx = 0;

    friend class QgsGraph;
};


typedef QList< int > QgsGraphEdgeIds;

/**
 * \ingroup analysis
 * \class QgsGraphVertex
 * \brief This class implements a graph vertex
 * \since QGIS 3.0
 */
class ANALYSIS_EXPORT QgsGraphVertex
{
  public:

    /**
     * Default constructor. It is needed for Qt's container, e.g. QVector
     */
    QgsGraphVertex() = default;

    /**
     * This constructor initializes QgsGraphVertex object and associates a vertex with a point
     */

    QgsGraphVertex( const QgsPointXY &point );

    /**
     * Returns the incoming edge ids, i.e. edges which end at this node.
     * \see outgoingEdges()
     */
    QgsGraphEdgeIds incomingEdges() const;

    /**
     * Returns outgoing edge ids, i.e. edges which start at this node.
     * \see incomingEdges()
     */
    QgsGraphEdgeIds outgoingEdges() const;

    /**
     * Returns point associated with graph vertex.
     */
    QgsPointXY point() const;

  private:
    QgsPointXY mCoordinate;
    QgsGraphEdgeIds mIncomingEdges;
    QgsGraphEdgeIds mOutgoingEdges;

    friend class QgsGraph;
};

/**
 * \ingroup analysis
 * \class QgsGraph
 * \brief Mathematical graph representation
 * \since QGIS 3.0
 */

class ANALYSIS_EXPORT QgsGraph
{
  public:

    /**
     * Constructor for QgsGraph.
     */
    QgsGraph() = default;

    // Graph constructing methods

    /**
     * Add a vertex to the graph
     */
    int addVertex( const QgsPointXY &pt );

    /**
     * Add an edge to the graph, going from the \a fromVertexIdx
     * to \a toVertexIdx.
     */
    int addEdge( int fromVertexIdx, int toVertexIdx, const QVector< QVariant > &strategies );

    /**
     * Returns number of graph vertices
     */
    int vertexCount() const;

#ifndef SIP_RUN

    /**
     * Returns the vertex at the given index.
     */
    const QgsGraphVertex &vertex( int idx ) const;
#else

    /**
     * Returns the vertex at the given index.
     *
     * \throws IndexError if the vertex is not found.
     */
    QgsGraphVertex vertex( int idx ) const;
    % MethodCode
    if ( sipCpp->hasVertex( a0 ) )
    {
      return sipConvertFromNewType( new QgsGraphVertex( sipCpp->vertex( a0 ) ), sipType_QgsGraphVertex, Py_None );
    }
    else
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    % End
#endif

#ifndef SIP_RUN

    /**
     * Removes the vertex at specified \a index.
     *
     * All edges which are incoming or outgoing edges for the vertex will also be removed.
     *
     * \since QGIS 3.24
     */
    void removeVertex( int index );
#else

    /**
     * Removes the vertex at specified \a index.
     *
     * All edges which are incoming or outgoing edges for the vertex will also be removed.
     *
     * \throws IndexError if the vertex is not found.
     * \since QGIS 3.24
     */
    void removeVertex( int index );
    % MethodCode
    if ( sipCpp->hasVertex( a0 ) )
    {
      sipCpp->removeVertex( a0 );
    }
    else
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    % End
#endif

    /**
      * Returns number of graph edges
      */
    int edgeCount() const;

#ifndef SIP_RUN

    /**
     * Returns the edge at the given index.
     */
    const QgsGraphEdge &edge( int idx ) const;
#else

    /**
     * Returns the edge at the given index.
     *
     * \throws IndexError if the edge is not found.
     */
    QgsGraphEdge edge( int idx ) const;
    % MethodCode
    if ( sipCpp->hasEdge( a0 ) )
    {
      return sipConvertFromNewType( new QgsGraphEdge( sipCpp->edge( a0 ) ), sipType_QgsGraphEdge, Py_None );
    }
    else
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    % End
#endif


#ifndef SIP_RUN

    /**
     * Removes the edge at specified \a index.
     *
     * The incoming and outgoing edges for all graph vertices will be updated accordingly. Vertices which
     * no longer have any incoming or outgoing edges as a result will be removed from the graph automatically.
     *
     * \since QGIS 3.24
     */
    void removeEdge( int index );
#else

    /**
     * Removes the edge at specified \a index.
     *
     * The incoming and outgoing edges for all graph vertices will be updated accordingly. Vertices which
     * no longer have any incoming or outgoing edges as a result will be removed from the graph automatically.
     *
     * \throws IndexError if the vertex is not found.
     * \since QGIS 3.24
     */
    void removeEdge( int index );
    % MethodCode
    if ( sipCpp->hasEdge( a0 ) )
    {
      sipCpp->removeEdge( a0 );
    }
    else
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    % End
#endif

    /**
     * Find vertex by associated point
     * \returns vertex index
     */
    int findVertex( const QgsPointXY &pt ) const;

#ifndef SIP_RUN

    /**
     * Finds the first edge which is the opposite of the edge with the specified index.
     *
     * This represents the edge which has the same vertices as the specified edge, but
     * the opposite direction in the graph.(I.e. the edge which starts at the "from" vertex
     * of the specified edge and ends at the "to" vertex.)
     *
     * Returns -1 if no opposite edge exists.
     *
     * \since QGIS 3.24
    */
    int findOppositeEdge( int index ) const;
#else

    /**
     * Finds the first edge which is the opposite of the edge with the specified index.
     *
     * This represents the edge which has the same vertices as the specified edge, but
     * the opposite direction in the graph.(I.e. the edge which starts at the "from" vertex
     * of the specified edge and ends at the "to" vertex.)
     *
     * Returns -1 if no opposite edge exists.
     *
     * \throws IndexError if the edge with the specified \a index is not found.
     *
     * \since QGIS 3.24
    */
    int findOppositeEdge( int index ) const;
    % MethodCode
    if ( sipCpp->hasEdge( a0 ) )
    {
      sipRes = sipCpp->findOppositeEdge( a0 );
    }
    else
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    % End
#endif

    /**
     * Returns whether the edge of the given index exists.
     *
     * \since QGIS 3.24
     */
    bool hasEdge( int index ) const;

    /**
     * Returns whether the vertex of the given index exists.
     *
     * \since QGIS 3.24
     */
    bool hasVertex( int index ) const;

  protected:

#ifndef SIP_RUN
    //! Graph vertices
    QHash<int, QgsGraphVertex> mGraphVertices;

    //! Graph edges
    QHash<int, QgsGraphEdge> mGraphEdges;
#endif


  private:

    int mNextVertexId = 0;
    int mNextEdgeId = 0;
};

#endif // QGSGRAPH_H
