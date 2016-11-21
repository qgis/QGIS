/**
 * \ingroup analysis
 * \class QgsGraphEdge
 * \brief This class implements a graph edge
 */
class QgsGraphEdge
{
%TypeHeaderCode
#include <qgsgraph.h>
%End

  public:
    QgsGraphEdge();

    /**
     * Returns edge cost calculated using specified strategy
     * @param strategyIndex strategy index
     */
    QVariant cost( int strategyIndex ) const;

    /**
     * Returns array of available strategies
     */
    QVector< QVariant > strategies() const;

    /**
     * Returns index of the outgoing vertex
     */
    int outVertex() const;

    /**
     * Returns index of the incoming vertex
     */
    int inVertex() const;
};


typedef QList< int > QgsGraphEdgeIds;

/**
 * \ingroup analysis
 * \class QgsGraphVertex
 * \brief This class implements a graph vertex
 */
class QgsGraphVertex
{
%TypeHeaderCode
#include <qgsgraph.h>
%End

  public:
    /**
     * Default constructor. It is needed for Qt's container, e.g. QVector
     */
    QgsGraphVertex();

    /**
     * This constructor initializes QgsGraphVertex object and associates a vertex with a point
     */

    QgsGraphVertex( const QgsPoint& point );

    /**
     * Returns outgoing edges ids
     */
    QgsGraphEdgeIds outEdges() const;

    /**
     * Return incoming edges ids
     */
    QgsGraphEdgeIds inEdges() const;

    /**
     * Returns point associated with graph vertex
     */
    QgsPoint point() const;
};

/**
 * \ingroup analysis
 * \class QgsGraph
 * \brief Mathematical graph representation
 */

class QgsGraph
{
%TypeHeaderCode
#include <qgsgraph.h>
%End

  public:
    QgsGraph();

    // Graph constructing methods

    /**
     * Add a vertex to the graph
     */
    int addVertex( const QgsPoint& pt );

    /**
     * Add an edge to the graph
     */
    int addEdge( int outVertexIdx, int inVertexIdx, const QVector< QVariant >& strategies );

    /**
     * Returns number of graph vertices
     */
    int vertexCount() const;

    /**
     * Returns vertex at given index
     */
    const QgsGraphVertex& vertex( int idx ) const;

    /**
      * Returns number of graph edges
      */
    int edgeCount() const;

    /**
     * Returns edge at given index
     */
    const QgsGraphEdge& edge( int idx ) const;

    /**
     * Find vertex by associated point
     * \return vertex index
     */
    int findVertex( const QgsPoint& pt ) const;
};
