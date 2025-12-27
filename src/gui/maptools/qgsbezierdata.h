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

#include <memory>

#include "qgis_gui.h"
#include "qgspoint.h"

class QgsNurbsCurve;

#define SIP_NO_FILE

///@cond PRIVATE

/**
 * \brief Structure representing an anchor point with its two control handles.
 *
 * Each anchor has:
 *
 * - anchor: the point where the curve passes through
 * - leftHandle: controls the incoming tangent (from previous segment)
 * - rightHandle: controls the outgoing tangent (to next segment)
 *
 * \since QGIS 4.0
 */
struct GUI_EXPORT QgsAnchorWithHandles
{
    QgsPoint anchor;      //!< Anchor point (curve passes through this)
    QgsPoint leftHandle;  //!< Left handle (controls incoming tangent)
    QgsPoint rightHandle; //!< Right handle (controls outgoing tangent)

    //! Constructor with anchor at origin, handles retracted
    QgsAnchorWithHandles() = default;

    //! Constructor with anchor position, handles retracted at anchor
    explicit QgsAnchorWithHandles( const QgsPoint &pt )
      : anchor( pt ), leftHandle( pt ), rightHandle( pt ) {}

    //! Constructor with all points specified
    QgsAnchorWithHandles( const QgsPoint &a, const QgsPoint &left, const QgsPoint &right )
      : anchor( a ), leftHandle( left ), rightHandle( right ) {}
};

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
    int anchorCount() const { return mData.count(); }

    //! Returns the number of handles (always 2 * anchorCount)
    int handleCount() const { return mData.count() * 2; }

    //! Returns the anchor at index \a idx
    QgsPoint anchor( int idx ) const;

    //! Returns the handle at index \a idx
    QgsPoint handle( int idx ) const;

    //! Returns all anchors (extracted from mData)
    QVector<QgsPoint> anchors() const;

    //! Returns all handles (extracted from mData, 2 per anchor)
    QVector<QgsPoint> handles() const;

    /**
     * Returns the anchor with its handles at index \a idx.
     * \param idx anchor index (0-based)
     * \returns QgsAnchorWithHandles structure, or default if index is invalid
     */
    const QgsAnchorWithHandles &anchorWithHandles( int idx ) const;

    /**
     * Returns the interpolated points of the curve for visualization.
     * Uses cubic Bézier interpolation between anchor points.
     */
    QgsPointSequence interpolate() const;

    /**
     * Converts the Poly-Bézier data to a QgsNurbsCurve.
     * The resulting curve is a piecewise cubic Bézier represented as NURBS.
     * \returns new QgsNurbsCurve. Returns nullptr if less than 2 anchors.
     */
    std::unique_ptr<QgsNurbsCurve> asNurbsCurve() const;

    //! Clears all data
    void clear();

    //! Returns TRUE if there are no anchors
    bool isEmpty() const { return mData.isEmpty(); }

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
    QVector<QgsAnchorWithHandles> mData;              //!< Anchor points with their handles (guarantees consistency)
    static const QgsAnchorWithHandles sInvalidAnchor; //!< Invalid anchor for out-of-bounds access
};

///@endcond PRIVATE

#endif // QGSBEZIERDATA_H
