/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/network/qgsgraphanalyzer.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsGraphAnalyzer
{
%Docstring
This class performs graph analysis, e.g. calculates shortest path between two
points using different strategies with Dijkstra algorithm
%End

%TypeHeaderCode
#include "qgsgraphanalyzer.h"
%End
  public:

    static SIP_PYLIST  dijkstra( const QgsGraph *source, int startVertexIdx, int criterionNum, QVector<int> *resultTree = 0, QVector<double> *resultCost = 0 );
%Docstring
Solve shortest path problem using Dijkstra algorithm

:param source: source graph
:param startVertexIdx: index of the start vertex
:param criterionNum: index of the optimization strategy
:param resultTree: array that represents shortest path tree. resultTree[ vertexIndex ] == inboundingArcIndex if vertex reachable, otherwise resultTree[ vertexIndex ] == -1.
                   Note that the startVertexIdx will also have a value of -1 and may need special handling by callers.
:param resultCost: array of the paths costs
%End

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
      PyObject *Int = PyLong_FromLong( treeResult[i] );
      PyList_SET_ITEM( l1, i, Int );
      PyObject *Float = PyFloat_FromDouble( costResult[i] );
      PyList_SET_ITEM( l2, i, Float );
    }

    sipRes = PyTuple_New( 2 );
    PyTuple_SET_ITEM( sipRes, 0, l1 );
    PyTuple_SET_ITEM( sipRes, 1, l2 );
%End

    static QgsGraph *shortestTree( const QgsGraph *source, int startVertexIdx, int criterionNum );
%Docstring
Returns shortest path tree with root-node in startVertexIdx

:param source: source graph
:param startVertexIdx: index of the start vertex
:param criterionNum: index of the optimization strategy
%End
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/network/qgsgraphanalyzer.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
