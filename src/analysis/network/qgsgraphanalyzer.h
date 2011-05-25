/***************************************************************************
    qgsgraphalyzer.h - QGIS Tools for vector geometry analysis
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

/** \ingroup analysis
 * The QGis class provides graph analysis functions
 */

class QgsGraphAnalyzer
{
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
    static void shortestpath( const QgsGraph* source, int startVertexIdx, int criterionNum, const QVector<int>& destPointCost, QVector<double>& cost, QgsGraph* treeResult );
 
};
#endif //QGSGRAPHANALYZERH
