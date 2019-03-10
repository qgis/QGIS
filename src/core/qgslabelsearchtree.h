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

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QList>
#include <QVector>
#include "qgspallabeling.h"
#include "rtree.hpp"

class QgsPointXY;

#ifndef SIP_RUN
namespace pal
{
  class LabelPosition;
}
#endif

/**
 * \ingroup core
 * A class to query the labeling structure at a given point (small wrapper around pal RTree class)
 */
class CORE_EXPORT QgsLabelSearchTree
{
  public:

    /**
     * Constructor for QgsLabelSearchTree.
     */
    QgsLabelSearchTree() = default;
    ~QgsLabelSearchTree();

    //! QgsLabelSearchTree cannot be copied.
    QgsLabelSearchTree( const QgsLabelSearchTree &rh ) = delete;
    //! QgsLabelSearchTree cannot be copied.
    QgsLabelSearchTree &operator=( const QgsLabelSearchTree &rh ) = delete;

    //! Removes and deletes all the entries
    void clear();

    /**
     * Returns label position(s) at a given point. QgsLabelSearchTree keeps ownership, don't delete the LabelPositions
     * \note not available in Python bindings
     * TODO: why does this break bindings with QList<QgsLabelPosition>?
     */
    void label( const QgsPointXY &p, QList<QgsLabelPosition *> &posList ) const SIP_SKIP;

    /**
     * Returns label position(s) in given rectangle. QgsLabelSearchTree keeps ownership, don't delete the LabelPositions
     * \note not available in Python bindings
     * TODO: why does this break bindings with QList<QgsLabelPosition>?
     */
    void labelsInRect( const QgsRectangle &r, QList<QgsLabelPosition *> &posList ) const SIP_SKIP;

    /**
     * Inserts label position. Does not take ownership of labelPos
     * \returns TRUE in case of success
     * \note not available in Python bindings
     */
    bool insertLabel( pal::LabelPosition *labelPos, QgsFeatureId featureId, const QString &layerName, const QString &labeltext, const QFont &labelfont, bool diagram = false, bool pinned = false, const QString &providerId = QString() ) SIP_SKIP;

  private:
    // set as mutable because RTree template is not const-correct
    mutable pal::RTree<QgsLabelPosition *, double, 2, double> mSpatialIndex;
    QList< QgsLabelPosition * > mOwnedPositions;

#ifdef SIP_RUN
    //! QgsLabelSearchTree cannot be copied.
    QgsLabelSearchTree( const QgsLabelSearchTree &rh );
    //! QgsLabelSearchTree cannot be copied.
    QgsLabelSearchTree &operator=( const QgsLabelSearchTree & );
#endif
};

#endif // QGSLABELTREE_H
