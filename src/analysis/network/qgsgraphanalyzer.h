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
#include "qgis_analysis.h"

class QgsGraph;

/** \ingroup analysis
 *  This class performs graph analysis, e.g. calculates shortest path between two
 * points using different strategies with Dijkstra algorithm
 */

class ANALYSIS_EXPORT QgsGraphAnalyzer
{
  public:

    /**
     * Solve shortest path problem using Dijkstra algorithm
     * @param source source graph
     * @param startVertexIdx index of the start vertex
     * @param criterionNum index of the optimization strategy
     * @param resultTree array that represents shortest path tree. resultTree[ vertexIndex ] == inboundingArcIndex if vertex reachable, otherwise resultTree[ vertexIndex ] == -1
     * @param resultCost array of the paths costs
     */
    static void dijkstra( const QgsGraph* source, int startVertexIdx, int criterionNum, QVector<int>* resultTree = nullptr, QVector<double>* resultCost = nullptr );

    /**
     * Returns shortest path tree with root-node in startVertexIdx
     * @param source source graph
     * @param startVertexIdx index of the start vertex
     * @param criterionNum index of the optimization strategy
     */
    static QgsGraph* shortestTree( const QgsGraph* source, int startVertexIdx, int criterionNum );
};

#endif // QGSGRAPHANALYZER_H
