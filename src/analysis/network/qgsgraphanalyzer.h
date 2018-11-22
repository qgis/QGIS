/***************************************************************************
  qgsgraphanalyzer.h
  --------------------------------------
  Date                 : 2011-04-14
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

#ifndef QGSGRAPHANALYZER_H
#define QGSGRAPHANALYZER_H

#include <QVector>

#include "qgis.h"
#include "qgis_analysis.h"

class QgsGraph;

/**
 * \ingroup analysis
 *  This class performs graph analysis, e.g. calculates shortest path between two
 * points using different strategies with Dijkstra algorithm
 */

class ANALYSIS_EXPORT QgsGraphAnalyzer
{
  public:

    /**
     * Solve shortest path problem using Dijkstra algorithm
     * \param source source graph
     * \param startVertexIdx index of the start vertex
     * \param criterionNum index of the optimization strategy
     * \param resultTree array that represents shortest path tree. resultTree[ vertexIndex ] == inboundingArcIndex if vertex reachable, otherwise resultTree[ vertexIndex ] == -1.
     * Note that the startVertexIdx will also have a value of -1 and may need special handling by callers.
     * \param resultCost array of the paths costs
     */
    static void SIP_PYALTERNATIVETYPE( SIP_PYLIST ) dijkstra( const QgsGraph *source, int startVertexIdx, int criterionNum, QVector<int> *resultTree = nullptr, QVector<double> *resultCost = nullptr );

#ifdef SIP_RUN
    % MethodCode
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
    % End
#endif

    /**
     * Returns shortest path tree with root-node in startVertexIdx
     * \param source source graph
     * \param startVertexIdx index of the start vertex
     * \param criterionNum index of the optimization strategy
     */
    static QgsGraph *shortestTree( const QgsGraph *source, int startVertexIdx, int criterionNum );
};

#endif // QGSGRAPHANALYZER_H
