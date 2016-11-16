/***************************************************************************
    qgsgraphalyzer.h - QGIS Tools for graph analysis
                             -------------------
    begin                : 14 april 2010
    copyright            : (C) Sergey Yakushev
    email                : yakushevs@list.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGRAPHANALYZER_H
#define QGSGRAPHANALYZER_H

#include <QVector>

class QgsGraph;

/** \ingroup analysis
 * QGIS class with graph analysis functions
 */

class ANALYSIS_EXPORT QgsGraphAnalyzer
{
  public:

    /**
     * solve shortest path problem using dijkstra algorithm
     * @param source source graph
     * @param startVertexIdx index of the start vertex
     * @param criterionNum index of the arc property as optimization criterion
     * @param resultTree array represents the shortest path tree. resultTree[ vertexIndex ] == inboundingArcIndex if vertex reachable, otherwise resultTree[ vertexIndex ] == -1
     * @param resultCost array of cost paths
     */
    static void dijkstra( const QgsGraph* source, int startVertexIdx, int criterionNum, QVector<int>* resultTree = nullptr, QVector<double>* resultCost = nullptr );

    /**
     * return shortest path tree with root-node in startVertexIdx
     * @param source source graph
     * @param startVertexIdx index of the start vertex
     * @param criterionNum index of the edge property as optimization criterion
     */
    static QgsGraph* shortestTree( const QgsGraph* source, int startVertexIdx, int criterionNum );
};

#endif // QGSGRAPHANALYZER_H
