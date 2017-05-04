/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/network/qgsgraph.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsGraphEdge
{
%Docstring
.. versionadded:: 3.0
 This class implements a graph edge
%End

%TypeHeaderCode
#include "qgsgraph.h"
%End
  public:
    QgsGraphEdge();

    QVariant cost( int strategyIndex ) const;
%Docstring
 Returns edge cost calculated using specified strategy
 \param strategyIndex strategy index
 :rtype: QVariant
%End

    QVector< QVariant > strategies() const;
%Docstring
 Returns array of available strategies
 :rtype: list of QVariant
%End

    int outVertex() const;
%Docstring
 Returns index of the outgoing vertex
 :rtype: int
%End

    int inVertex() const;
%Docstring
 Returns index of the incoming vertex
 :rtype: int
%End

};


typedef QList< int > QgsGraphEdgeIds;

class QgsGraphVertex
{
%Docstring
 This class implements a graph vertex
%End

%TypeHeaderCode
#include "qgsgraph.h"
%End
  public:

    QgsGraphVertex();
%Docstring
 Default constructor. It is needed for Qt's container, e.g. QVector
%End


    QgsGraphVertex( const QgsPoint &point );
%Docstring
 This constructor initializes QgsGraphVertex object and associates a vertex with a point
%End

    QgsGraphEdgeIds outEdges() const;
%Docstring
 Returns outgoing edges ids
 :rtype: QgsGraphEdgeIds
%End

    QgsGraphEdgeIds inEdges() const;
%Docstring
 Return incoming edges ids
 :rtype: QgsGraphEdgeIds
%End

    QgsPoint point() const;
%Docstring
 Returns point associated with graph vertex
 :rtype: QgsPoint
%End

};


class QgsGraph
{
%Docstring
 Mathematical graph representation
%End

%TypeHeaderCode
#include "qgsgraph.h"
%End
  public:
    QgsGraph();


    int addVertex( const QgsPoint &pt );
%Docstring
 Add a vertex to the graph
 :rtype: int
%End

    int addEdge( int outVertexIdx, int inVertexIdx, const QVector< QVariant > &strategies );
%Docstring
 Add an edge to the graph
 :rtype: int
%End

    int vertexCount() const;
%Docstring
 Returns number of graph vertices
 :rtype: int
%End

    const QgsGraphVertex &vertex( int idx ) const;
%Docstring
 Returns vertex at given index
 :rtype: QgsGraphVertex
%End

    int edgeCount() const;
%Docstring
 Returns number of graph edges
 :rtype: int
%End

    const QgsGraphEdge &edge( int idx ) const;
%Docstring
 Returns edge at given index
 :rtype: QgsGraphEdge
%End

    int findVertex( const QgsPoint &pt ) const;
%Docstring
 Find vertex by associated point
 :return: vertex index
 :rtype: int
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/network/qgsgraph.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
