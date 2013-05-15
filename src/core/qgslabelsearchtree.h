/***************************************************************************
                          qgslabelsearchtree.h
            Node for raster calculator tree
                          --------------------
    begin                : 2010-11-02
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLABELSEARCHTREE_H
#define QGSLABELSEARCHTREE_H

#include "qgspoint.h"
#include "qgsmaprenderer.h"
#include <QList>
#include <QVector>
#include <pointset.h>
#include <labelposition.h>
#include "qgsrectangle.h"

using namespace pal;

/**A class to query the labeling structure at a given point (small wraper around pal RTree class)*/
class CORE_EXPORT QgsLabelSearchTree
{
  public:
    QgsLabelSearchTree();
    ~QgsLabelSearchTree();

    /**Removes and deletes all the entries*/
    void clear();

    /**Returns label position(s) at a given point. QgsLabelSearchTree keeps ownership, don't delete the LabelPositions
     * @note not available in python bindings
     * TODO: why does this break bindings with QList<QgsLabelPosition>?
     */
    void label( const QgsPoint& p, QList<QgsLabelPosition*>& posList );

    /**Returns label position(s) in given rectangle. QgsLabelSearchTree keeps ownership, don't delete the LabelPositions
     * @note not available in python bindings
     * TODO: why does this break bindings with QList<QgsLabelPosition>?
     */
    void labelsInRect( const QgsRectangle& r, QList<QgsLabelPosition*>& posList );

    /**Inserts label position. Does not take ownership of labelPos
     * @return true in case of success
     * @note not available in python bindings
     */
    bool insertLabel( LabelPosition* labelPos, int featureId, const QString& layerName, const QString& labeltext, const QFont& labelfont, bool diagram = false, bool pinned = false );

  private:
    RTree<QgsLabelPosition*, double, 2, double> mSpatialIndex;
};

#endif // QGSLABELTREE_H
