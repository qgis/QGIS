/***************************************************************************
                             qgsmodelsnapper.h
                             -------------------
    begin                : March 2020
    copyright            : (C) 2020 by Nyall Dawson
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
#ifndef QGSMODELSNAPPER_H
#define QGSMODELSNAPPER_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include <QPen>


#define SIP_NO_FILE

/**
 * \ingroup gui
 * \class QgsModelSnapper
 * \brief Manages snapping grids and preset snap lines in a layout, and handles
 * snapping points to the nearest grid coordinate/snap line when possible.
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsModelSnapper
{
  public:
    /**
     * Constructor for QgsModelSnapper, attached to the specified \a layout.
     */
    QgsModelSnapper();

    /**
     * Sets the snap \a tolerance (in pixels) to use when snapping.
     * \see snapTolerance()
     */
    void setSnapTolerance( int snapTolerance );

    /**
     * Returns the snap tolerance (in pixels) to use when snapping.
     * \see setSnapTolerance()
     */
    int snapTolerance() const { return mTolerance; }

    /**
     * Returns TRUE if snapping to grid is enabled.
     * \see setSnapToGrid()
     */
    bool snapToGrid() const { return mSnapToGrid; }

    /**
     * Sets whether snapping to grid is \a enabled.
     * \see snapToGrid()
     */
    void setSnapToGrid( bool enabled );

    /**
     * Snaps a layout coordinate \a point. If \a point was snapped, \a snapped will be set to TRUE.
     *
     * The \a scaleFactor argument should be set to the transformation from
     * scalar transform from layout coordinates to pixels, i.e. the
     * graphics view transform().m11() value.
     *
     * This method considers snapping to the grid, snap lines, etc.
     *
     * If the \a horizontalSnapLine and \a verticalSnapLine arguments are specified, then the snapper
     * will automatically display and position these lines to indicate snapping positions to item bounds.
     *
     * A list of items to ignore during the snapping can be specified via the \a ignoreItems list.

     * \see snapRect()
     */
    QPointF snapPoint( QPointF point, double scaleFactor, bool &snapped SIP_OUT, bool snapHorizontal = true, bool snapVertical = true ) const;

    /**
     * Snaps a layout coordinate \a rect. If \a rect was snapped, \a snapped will be set to TRUE.
     *
     * Snapping occurs by moving the rectangle alone. The rectangle will not be resized
     * as a result of the snap operation.
     *
     * The \a scaleFactor argument should be set to the transformation from
     * scalar transform from layout coordinates to pixels, i.e. the
     * graphics view transform().m11() value.
     *
     * This method considers snapping to the grid, snap lines, etc.
     *
     * If the \a horizontalSnapLine and \a verticalSnapLine arguments are specified, then the snapper
     * will automatically display and position these lines to indicate snapping positions to item bounds.
     *
     * A list of items to ignore during the snapping can be specified via the \a ignoreItems list.
     *
     * \see snapPoint()
     */
    QRectF snapRect( const QRectF &rect, double scaleFactor, bool &snapped SIP_OUT, bool snapHorizontal = true, bool snapVertical = true ) const;

    /**
     * Snaps a layout coordinate \a rect. If \a rect was snapped, \a snapped will be set to TRUE.
     *
     * The \a scaleFactor argument should be set to the transformation from
     * scalar transform from layout coordinates to pixels, i.e. the
     * graphics view transform().m11() value.
     *
     * This method considers snapping to the grid, snap lines, etc.
     *
     * If the \a horizontalSnapLine and \a verticalSnapLine arguments are specified, then the snapper
     * will automatically display and position these lines to indicate snapping positions to item bounds.
     *
     * A list of items to ignore during the snapping can be specified via the \a ignoreItems list.
     *
     * \see snapPoint()
     */
    QRectF snapRectWithResize( const QRectF &rect, double scaleFactor, bool &snapped SIP_OUT, bool snapHorizontal = true, bool snapVertical = true ) const;

    /**
     * Snaps a layout coordinate \a point to the grid. If \a point
     * was snapped horizontally, \a snappedX will be set to TRUE. If \a point
     * was snapped vertically, \a snappedY will be set to TRUE.
     *
     * The \a scaleFactor argument should be set to the transformation from
     * scalar transform from layout coordinates to pixels, i.e. the
     * graphics view transform().m11() value.
     *
     * If snapToGrid() is disabled, this method will return the point
     * unchanged.
     *
     * \see snapPointsToGrid()
     */
    QPointF snapPointToGrid( QPointF point, double scaleFactor, bool &snappedX SIP_OUT, bool &snappedY SIP_OUT ) const;

    /**
     * Snaps a set of \a points to the grid. If the points
     * were snapped, \a snapped will be set to TRUE.
     *
     * The \a scaleFactor argument should be set to the transformation from
     * scalar transform from layout coordinates to pixels, i.e. the
     * graphics view transform().m11() value.
     *
     * If snapToGrid() is disabled, this method will not attempt to snap the points.
     *
     * The returned value is the smallest delta which the points need to be shifted by in order to align
     * one of the points to the grid.
     *
     * \see snapPointToGrid()
     */
    QPointF snapPointsToGrid( const QList<QPointF> &points, double scaleFactor, bool &snappedX SIP_OUT, bool &snappedY SIP_OUT ) const;

  private:
    int mTolerance = 5;
    bool mSnapToGrid = false;
};

#endif //QGSMODELSNAPPER_H
