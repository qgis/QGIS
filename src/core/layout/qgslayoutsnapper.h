/***************************************************************************
                             qgslayoutsnapper.h
                             -------------------
    begin                : July 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLAYOUTSNAPPER_H
#define QGSLAYOUTSNAPPER_H

#include "qgis_core.h"
#include "qgslayoutmeasurement.h"
#include "qgslayoutpoint.h"
#include "qgslayoutguidecollection.h"
#include <QPen>

class QgsLayout;

/**
 * \ingroup core
 * \class QgsLayoutSnapper
 * \brief Manages snapping grids and preset snap lines in a layout, and handles
 * snapping points to the nearest grid coordinate/snap line when possible.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutSnapper
{

  public:

    /**
     * Constructor for QgsLayoutSnapper, attached to the specified \a layout.
     */
    QgsLayoutSnapper( QgsLayout *layout );

    /**
     * Sets the snap \a tolerance (in pixels) to use when snapping.
     * \see snapTolerance()
     */
    void setSnapTolerance( const int snapTolerance ) { mTolerance = snapTolerance; }

    /**
     * Returns the snap tolerance (in pixels) to use when snapping.
     * \see setSnapTolerance()
     */
    int snapTolerance() const { return mTolerance; }

    /**
     * Returns true if snapping to grid is enabled.
     * \see setSnapToGrid()
     */
    bool snapToGrid() const { return mSnapToGrid; }

    /**
     * Sets whether snapping to grid is \a enabled.
     * \see snapToGrid()
     */
    void setSnapToGrid( bool enabled ) { mSnapToGrid = enabled; }

    /**
     * Returns true if snapping to guides is enabled.
     * \see setSnapToGuides()
     */
    bool snapToGuides() const { return mSnapToGuides; }

    /**
     * Sets whether snapping to guides is \a enabled.
     * \see snapToGuides()
     */
    void setSnapToGuides( bool enabled ) { mSnapToGuides = enabled; }

    /**
     * Snaps a layout coordinate \a point. If \a point was snapped, \a snapped will be set to true.
     *
     * The \a scaleFactor argument should be set to the transformation from
     * scalar transform from layout coordinates to pixels, i.e. the
     * graphics view transform().m11() value.
     *
     * This method considers snapping to the grid, snap lines, etc.
     */
    QPointF snapPoint( QPointF point, double scaleFactor, bool &snapped SIP_OUT ) const;

    /**
     * Snaps a layout coordinate \a point to the grid. If \a point
     * was snapped horizontally, \a snappedX will be set to true. If \a point
     * was snapped vertically, \a snappedY will be set to true.
     *
     * The \a scaleFactor argument should be set to the transformation from
     * scalar transform from layout coordinates to pixels, i.e. the
     * graphics view transform().m11() value.
     *
     * If snapToGrid() is disabled, this method will return the point
     * unchanged.
     */
    QPointF snapPointToGrid( QPointF point, double scaleFactor, bool &snappedX SIP_OUT, bool &snappedY SIP_OUT ) const;

    /**
     * Snaps a layout coordinate \a point to the grid. If \a point
     * was snapped, \a snapped will be set to true.
     *
     * The \a scaleFactor argument should be set to the transformation from
     * scalar transform from layout coordinates to pixels, i.e. the
     * graphics view transform().m11() value.
     *
     * If snapToGrid() is disabled, this method will return the point
     * unchanged.
     */
    double snapPointToGuides( double original, QgsLayoutGuide::Orientation orientation, double scaleFactor, bool &snapped SIP_OUT ) const;

  private:

    QgsLayout *mLayout = nullptr;

    int mTolerance = 5;
    bool mSnapToGrid = false;
    bool mSnapToGuides = true;

};

#endif //QGSLAYOUTSNAPPER_H
