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
     * @param criterionNum index of arc property as optimization criterion
     * @param treeResult array represents the shortest path tree. resultTree[ vertexIndex ] == inboundingArcIndex if vertex reacheble and resultTree[ vertexIndex ] == -1 others.
     * @param resultCost array of cost paths
     */
    static SIP_PYLIST dijkstra( const QgsGraph* source, int startVertexIdx, int criterionNum );
%MethodCode
      QVector< int > treeResult;
      QVector< double > costResult;
      QgsGraphAnalyzer::dijkstra( a0, a1, a2, &treeResult, &costResult );

      PyObject *l1 = PyList_New( treeResult.size() );
      if ( l1 == NULL )
      {
        return NULL;
      }
      PyObject *l2 = PyList_New( costResult.size() );
      if ( l2 == NULL )
      {
        return NULL;
      }
      int i;
      for ( i = 0; i < costResult.size(); ++i )
      {
        PyObject *Int = PyInt_FromLong( treeResult[i] );
        PyList_SET_ITEM( l1, i, Int );
        PyObject *Float = PyFloat_FromDouble( costResult[i] );
        PyList_SET_ITEM( l2, i, Float );
      }

      sipRes = PyTuple_New( 2 );
      PyTuple_SET_ITEM( sipRes, 0, l1 );
      PyTuple_SET_ITEM( sipRes, 1, l2 );
%End

    /**
     * return shortest path tree with root-node in startVertexIdx
     * @param source The source graph
     * @param startVertexIdx index of start vertex
     * @param criterionNum index of edge property as optimization criterion
     */
    static QgsGraph* shortestTree( const QgsGraph* source, int startVertexIdx, int criterionNum );
};


