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
%Docstring
 Constructor for QgsGraphEdge.
%End

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

    int toVertex() const;
%Docstring
 Returns the index of the vertex at the end of this edge.
.. seealso:: fromVertex()
 :rtype: int
%End

    int fromVertex() const;
%Docstring
 Returns the index of the vertex at the start of this edge.
.. seealso:: toVertex()
 :rtype: int
%End

};


typedef QList< int > QgsGraphEdgeIds;

class QgsGraphVertex
{
%Docstring
 This class implements a graph vertex
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsgraph.h"
%End
  public:

    QgsGraphVertex();
%Docstring
 Default constructor. It is needed for Qt's container, e.g. QVector
%End


    QgsGraphVertex( const QgsPointXY &point );
%Docstring
 This constructor initializes QgsGraphVertex object and associates a vertex with a point
%End

    QgsGraphEdgeIds incomingEdges() const;
%Docstring
 Returns the incoming edge ids, i.e. edges which end at this node.
.. seealso:: outgoingEdges()
 :rtype: QgsGraphEdgeIds
%End

    QgsGraphEdgeIds outgoingEdges() const;
%Docstring
 Returns outgoing edge ids, i.e. edges which start at this node.
.. seealso:: incomingEdges()
 :rtype: QgsGraphEdgeIds
%End

    QgsPointXY point() const;
%Docstring
 Returns point associated with graph vertex.
 :rtype: QgsPointXY
%End

};


class QgsGraph
{
%Docstring
 Mathematical graph representation
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsgraph.h"
%End
  public:

    QgsGraph();
%Docstring
 Constructor for QgsGraph.
%End


    int addVertex( const QgsPointXY &pt );
%Docstring
 Add a vertex to the graph
 :rtype: int
%End

    int addEdge( int fromVertexIdx, int toVertexIdx, const QVector< QVariant > &strategies );
%Docstring
 Add an edge to the graph, going from the ``fromVertexIdx``
 to ``toVertexIdx``.
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

    int findVertex( const QgsPointXY &pt ) const;
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
