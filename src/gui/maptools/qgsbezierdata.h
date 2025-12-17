/***************************************************************************
    qgsbezierdata.h  -  Data structure for Poly-Bézier curve digitizing
    ---------------------
    begin                : December 2025
    copyright            : (C) 2025 by Loïc Bartoletti
    email                : loic dot bartoletti at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSBEZIERDATA_H
#define QGSBEZIERDATA_H

#include "qgis_gui.h"
#include "qgspoint.h"

class QgsNurbsCurve;

#define SIP_NO_FILE

///@cond PRIVATE

/**
 * \brief Data structure for managing Poly-Bézier curve during digitizing.
 *
 * This class stores anchor points (where the curve passes through) and
 * handle points (tangent control points, 2 per anchor).
 *
 * Handle indexing:
 *
 * - Handle index 2*i is the "left" handle of anchor i (controls incoming tangent)
 * - Handle index 2*i+1 is the "right" handle of anchor i (controls outgoing tangent)
 *
 * \since QGIS 4.0
 */
class GUI_EXPORT QgsBezierData
{
  public:
    //! Default constructor
    QgsBezierData() = default;

    //! Number of interpolated points per Bézier segment for visualization
    static constexpr int INTERPOLATION_POINTS = 32;

    /**
     * Adds an anchor point at the given position.
     * Both handles are initially placed at the anchor position.
     * \param pt anchor point position
     */
    void addAnchor( const QgsPoint &pt );

    /**
     * Moves the anchor at index \a idx to the new position \a pt.
     * Handles are moved relative to the anchor.
     * \param idx anchor index (0-based)
     * \param pt new position
     */
    void moveAnchor( int idx, const QgsPoint &pt );

    /**
     * Moves the handle at index \a idx to the new position \a pt.
     * \param idx handle index (0-based, 2 handles per anchor)
     * \param pt new position
     */
    void moveHandle( int idx, const QgsPoint &pt );

    /**
     * Inserts a new anchor point at the given segment.
     * \param segmentIdx segment index where to insert (0 = before first segment)
     * \param pt anchor position
     */
    void insertAnchor( int segmentIdx, const QgsPoint &pt );

    /**
     * Deletes the anchor at index \a idx and its associated handles.
     * \param idx anchor index to delete
     */
    void deleteAnchor( int idx );

    /**
     * Retracts (collapses) the handle at index \a idx to its anchor position.
     * \param idx handle index
     */
    void retractHandle( int idx );

    /**
     * Extends (expands) the handle at index \a idx from its anchor.
     * Used when a handle is initially at the anchor position.
     * \param idx handle index
     * \param pt new handle position
     */
    void extendHandle( int idx, const QgsPoint &pt );

    //! Returns the number of anchor points
    int anchorCount() const { return mAnchors.count(); }

    //! Returns the number of handles (always 2 * anchorCount)
    int handleCount() const { return mHandles.count(); }

    //! Returns the anchor at index \a idx
    QgsPoint anchor( int idx ) const;

    //! Returns the handle at index \a idx
    QgsPoint handle( int idx ) const;

    //! Returns all anchors
    const QVector<QgsPoint> &anchors() const { return mAnchors; }

    //! Returns all handles
    const QVector<QgsPoint> &handles() const { return mHandles; }

    /**
     * Returns the interpolated points of the curve for visualization.
     * Uses cubic Bézier interpolation between anchor points.
     */
    QgsPointSequence interpolate() const;

    /**
     * Converts the Poly-Bézier data to a QgsNurbsCurve.
     * The resulting curve is a piecewise cubic Bézier represented as NURBS.
     * \returns new QgsNurbsCurve, caller takes ownership. Returns nullptr if less than 2 anchors.
     */
    QgsNurbsCurve *asNurbsCurve() const;

    //! Clears all data
    void clear();

    //! Returns TRUE if there are no anchors
    bool isEmpty() const { return mAnchors.isEmpty(); }

    /**
     * Finds the closest anchor to the given point within tolerance.
     * \param pt point to search from
     * \param tolerance search tolerance
     * \returns anchor index or -1 if not found
     */
    int findClosestAnchor( const QgsPoint &pt, double tolerance ) const;

    /**
     * Finds the closest handle to the given point within tolerance.
     * \param pt point to search from
     * \param tolerance search tolerance
     * \returns handle index or -1 if not found
     */
    int findClosestHandle( const QgsPoint &pt, double tolerance ) const;

    /**
     * Finds the closest segment to the given point within tolerance.
     * \param pt point to search from
     * \param tolerance search tolerance
     * \returns segment index or -1 if not found
     */
    int findClosestSegment( const QgsPoint &pt, double tolerance ) const;

  private:
    QVector<QgsPoint> mAnchors; //!< Anchor points (curve passes through these)
    QVector<QgsPoint> mHandles; //!< Handle points (2 per anchor: left and right)

    /**
     * Evaluates a cubic Bézier segment at parameter t.
     * \param p0 start point (anchor)
     * \param p1 control point 1 (right handle of start)
     * \param p2 control point 2 (left handle of end)
     * \param p3 end point (anchor)
     * \param t parameter [0, 1]
     * \returns point on the curve
     */
    static QgsPoint evaluateBezier( const QgsPoint &p0, const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &p3, double t );
};

///@endcond PRIVATE

#endif // QGSBEZIERDATA_H
