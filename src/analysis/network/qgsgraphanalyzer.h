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

#ifndef QGSGRAPHANALYZERH
#define QGSGRAPHANALYZERH

//QT-includes
#include <QVector>

// forward-declaration
class QgsGraph;

/** \ingroup networkanalysis
 * The QGis class provides graph analysis functions
 */

class ANALYSIS_EXPORT QgsGraphAnalyzer
{
  public:
    /**
     * solve shortest path problem using dijkstra algorithm
     * @param source The source graph
     * @param startVertexIdx index of start vertex
     * @param criterionNum index of arc property as optimization criterion
     * @param resultTree array represents the shortest path tree. resultTree[ vertexIndex ] == inboundingArcIndex if vertex reacheble and resultTree[ vertexIndex ] == -1 others.
     * @param resultCost array of cost paths
     */
    static void dijkstra( const QgsGraph* source, int startVertexIdx, int criterionNum, QVector<int>* resultTree = NULL, QVector<double>* resultCost = NULL );

    /**
     * return shortest path tree with root-node in startVertexIdx
     * @param source The source graph
     * @param startVertexIdx index of start vertex
     * @param criterionNum index of edge property as optimization criterion
     */
    static QgsGraph* shortestTree( const QgsGraph* source, int startVertexIdx, int criterionNum );
};
#endif //QGSGRAPHANALYZERH
