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

    //! Style for drawing the page/snapping grid
    enum GridStyle
    {
      GridLines, //! Solid lines
      GridDots, //! Dots
      GridCrosses //! Crosses
    };

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
     * Sets the page/snap grid \a resolution.
     * \see gridResolution()
     * \see setGridOffset()
     */
    void setGridResolution( const QgsLayoutMeasurement &resolution ) { mGridResolution = resolution; }

    /**
     * Returns the page/snap grid resolution.
     * \see setGridResolution()
     * \see gridOffset()
     */
    QgsLayoutMeasurement gridResolution() const { return mGridResolution;}

    /**
     * Sets the \a offset of the page/snap grid.
     * \see gridOffset()
     * \see setGridResolution()
     */
    void setGridOffset( const QgsLayoutPoint offset ) { mGridOffset = offset; }

    /**
     * Returns the offset of the page/snap grid.
     * \see setGridOffset()
     * \see gridResolution()
     */
    QgsLayoutPoint gridOffset() const { return mGridOffset; }

    /**
     * Sets the \a pen used for drawing page/snap grids.
     * \see gridPen()
     * \see setGridStyle()
     */
    void setGridPen( const QPen &pen ) { mGridPen = pen; }

    /**
     * Returns the pen used for drawing page/snap grids.
     * \see setGridPen()
     * \see gridStyle()
     */
    QPen gridPen() const { return mGridPen; }

    /**
     * Sets the \a style used for drawing the page/snap grids.
     * \see gridStyle()
     * \see setGridPen()
     */
    void setGridStyle( const GridStyle style ) { mGridStyle = style; }

    /**
     * Returns the style used for drawing the page/snap grids.
     * \see setGridStyle()
     * \see gridPen()
     */
    GridStyle gridStyle() const { return mGridStyle; }

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
     * was snapped, \a snapped will be set to true.
     *
     * The \a scaleFactor argument should be set to the transformation from
     * scalar transform from layout coordinates to pixels, i.e. the
     * graphics view transform().m11() value.
     *
     * If snapToGrid() is disabled, this method will return the point
     * unchanged.
     */
    QPointF snapPointToGrid( QPointF point, double scaleFactor, bool &snapped SIP_OUT ) const;

  private:

    QgsLayout *mLayout = nullptr;

    int mTolerance = 5;

    QgsLayoutMeasurement mGridResolution;
    QgsLayoutPoint mGridOffset;
    QPen mGridPen;
    GridStyle mGridStyle = GridLines;

    bool mSnapToGrid = false;

};

#endif //QGSLAYOUTSNAPPER_H
