/**
 * \ingroup networkanalysis
 * \class QgsGraphArc
 * \brief This class implement a graph edge
 */
class QgsGraphArc
{
%TypeHeaderCode
#include <qgsgraph.h>
%End

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
};


typedef QList< int > QgsGraphArcIdList;

/**
 * \ingroup networkanalysis
 * \class QgsGraphVertex
 * \brief This class implement a graph vertex
 */
class QgsGraphVertex
{
%TypeHeaderCode
#include <qgsgraph.h>
%End

  public:
    /**
     * default constructor. It need for QT's container, e.g. QVector
     */
    QgsGraphVertex();

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
};

/**
 * \ingroup networkanalysis
 * \class QgsGraph
 * \brief Mathematics graph representation
 */

class QgsGraph
{
%TypeHeaderCode
#include <qgsgraph.h>
%End

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
     * return vertex count
     */
    int vertexCount() const;

    /**
     * return vertex at index
     */
    const QgsGraphVertex& vertex( int idx ) const;

    /**
      * return edge count
      */
    int arcCount() const;

    /**
     * return edge at index
     */
    const QgsGraphArc& arc( int idx ) const;

    /**
     * find vertex by point
     * \return vertex index
     */
    int findVertex( const QgsPoint& pt ) const;
};
