class QgsGraphAnalyzer
{
%TypeHeaderCode
#include <qgsgraphanalyzer.h>
%End

  public:
    /**
     * solve shortest path problem using dijkstra algorithm
     * @param source The source graph
     * @param startVertexIdx index of start vertex
     * @param criterionNum index of edge property as optimization criterion
     * @param destPointCost array of vertex indexes. Function calculating shortest path costs for vertices with these indexes
     * @param cost array of cost paths
     * @param treeResult return shortest path tree
     */
//    static void shortestpath( const QgsGraph* source, int startVertexIdx, int criterionNum, const QVector<int>& destPointCost, QVector<double>& cost, QgsGraph* treeResult );

    /**
     * return shortest path tree with root-node in startVertexIdx
     * @param source The source graph
     * @param startVertexIdx index of start vertex
     * @param criterionNum index of edge property as optimization criterion
     */
    static QgsGraph* shortestTree( const QgsGraph* source, int startVertexIdx, int criterionNum );
};

