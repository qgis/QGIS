/***************************************************************************
    qgsbezierdata.h  -  Data structure for Poly-Bézier curve digitizing
    ---------------------
    begin                : December 2025
    copyright            : (C) 2025 by Loïc Bartoletti
                           Adapted from BezierEditing plugin work by Takayuki Mizutani
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

#define SIP_NO_FILE

class QgsNurbsCurve;

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
    explicit QgsAnchorWithHandles( const QgsPoint &point )
      : anchor( point ), leftHandle( point ), rightHandle( point ) {}

    //! Constructor with all points specified
    QgsAnchorWithHandles( const QgsPoint &point, const QgsPoint &left, const QgsPoint &right )
      : anchor( point ), leftHandle( left ), rightHandle( right ) {}
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
     * \param point anchor point position
     */
    void addAnchor( const QgsPoint &point );

    /**
     * Moves the anchor at index \a index to the new position \a point.
     * Handles are moved relative to the anchor.
     * \param index anchor index (0-based)
     * \param point new position
     */
    void moveAnchor( int index, const QgsPoint &point );

    /**
     * Moves the handle at index \a index to the new position \a point.
     * \param index handle index (0-based, 2 handles per anchor)
     * \param point new position
     */
    void moveHandle( int index, const QgsPoint &point );

    /**
     * Inserts a new anchor point at the given segment.
     * \param segmentIndex segment index where to insert (0 = before first segment)
     * \param point anchor position
     */
    void insertAnchor( int segmentIndex, const QgsPoint &point );

    /**
     * Deletes the anchor at index \a index and its associated handles.
     * \param index anchor index to delete
     */
    void deleteAnchor( int index );

    /**
     * Retracts (collapses) the handle at index \a index to its anchor position.
     * \param index handle index
     */
    void retractHandle( int index );

    /**
     * Extends (expands) the handle at index \a index from its anchor.
     * Used when a handle is initially at the anchor position.
     * \param index handle index
     * \param point new handle position
     */
    void extendHandle( int index, const QgsPoint &point );

    //! Returns the number of anchor points
    int anchorCount() const { return mData.count(); }

    //! Returns the number of handles (always 2 * anchorCount)
    int handleCount() const { return mData.count() * 2; }

    //! Returns the anchor at index \a index
    QgsPoint anchor( int index ) const;

    //! Returns the handle at index \a index
    QgsPoint handle( int index ) const;

    //! Returns all anchors (extracted from mData)
    QVector<QgsPoint> anchors() const;

    //! Returns all handles (extracted from mData, 2 per anchor)
    QVector<QgsPoint> handles() const;

    /**
     * Returns the anchor with its handles at index \a index.
     * \param index anchor index (0-based)
     * \returns QgsAnchorWithHandles structure, or default if index is invalid
     */
    const QgsAnchorWithHandles &anchorWithHandles( int index ) const;

    /**
     * Returns the interpolated points of the curve for visualization.
     * Uses cubic Bézier interpolation between anchor points.
     */
    QgsPointSequence interpolateLine() const;

    /**
     * Converts the Poly-Bézier data to a QgsNurbsCurve.
     * The resulting curve is a piecewise Bézier of \a degree represented as NURBS.
     * \param degree The degree of the Bézier segments (defaults to 3 for cubic).
     * \returns new QgsNurbsCurve. Returns nullptr if less than 2 anchors or degree < 1.
     */
    std::unique_ptr<QgsNurbsCurve> asNurbsCurve( int degree = 3 ) const;

    /**
     * Creates QgsBezierData from a poly-Bézier NURBS curve control points.
     *
     * Converts NURBS control point layout (anchor, handle, ..., handle, anchor, ...)
     * to QgsBezierData structure (anchors with left/right handles).
     *
     * \param controlPoints Control points from a poly-Bézier NURBS curve
     *                      (must have d*k+1 points where k is the number of segments and d is the degree)
     * \param degree The degree of the Bézier segments (defaults to 3 for cubic).
     * \returns QgsBezierData with anchors and handles extracted
     * \since QGIS 4.0
     */
    static QgsBezierData fromPolyBezierControlPoints( const QVector<QgsPoint> &controlPoints, int degree = 3 );

    /**
     * Creates QgsBezierData from a poly-Bézier NURBS curve control points.
     *
     * Overload that accepts 2D points.
     *
     * \param controlPoints Control points from a poly-Bézier NURBS curve
     *                      (must have d*k+1 points where k is the number of segments and d is the degree)
     * \param degree The degree of the Bézier segments (defaults to 3 for cubic).
     * \returns QgsBezierData with anchors and handles extracted
     * \since QGIS 4.0
     */
    static QgsBezierData fromPolyBezierControlPoints( const QVector<QgsPointXY> &controlPoints, int degree = 3 );

    /**
     * Calculates and updates the symmetric handle positions when dragging a Poly-Bézier anchor.
     *
     * \param controlPoints The list of control points to update.
     * \param anchorIndex The index of the anchor being dragged.
     * \param mousePosition The position of the mouse cursor (defining the direction/length of handles relative to anchor).
     * \since QGIS 4.0
     */
    static void calculateSymmetricHandles( QVector<QgsPoint> &controlPoints, int anchorIndex, const QgsPoint &mousePosition );

    /**
     * Calculates and updates the symmetric handle positions for a single anchor.
     *
     * \param anchor The anchor point (center).
     * \param mousePosition The new position for the handle that follows the mouse.
     * \param handleFollow The handle that will be moved to \a mousePosition.
     * \param handleOpposite The handle that will be moved to the symmetric opposite position.
     * \since QGIS 4.0
     */
    static void calculateSymmetricHandles( const QgsPoint &anchor, const QgsPoint &mousePosition, QgsPoint *handleFollow, QgsPoint *handleOpposite );

    /**
     * Updates the handles of the anchor at \a anchorIndex symmetrically.
     * The right handle will follow \a mousePosition, and the left handle will
     * be placed in the opposite direction.
     *
     * \param anchorIndex The index of the anchor to update.
     * \param mousePosition The new position for the right handle.
     * \since QGIS 4.0
     */
    void calculateSymmetricHandles( int anchorIndex, const QgsPoint &mousePosition );

    //! Clears all data
    void clear();

    //! Returns TRUE if there are no anchors
    bool isEmpty() const { return mData.isEmpty(); }

    /**
     * Finds the closest anchor to the given point within tolerance.
     * \param point point to search from
     * \param tolerance search tolerance
     * \returns anchor index or -1 if not found
     */
    int findClosestAnchor( const QgsPoint &point, double tolerance ) const;

    /**
     * Finds the closest handle to the given point within tolerance.
     * \param point point to search from
     * \param tolerance search tolerance
     * \returns handle index or -1 if not found
     */
    int findClosestHandle( const QgsPoint &point, double tolerance ) const;

    /**
     * Finds the closest segment to the given point within tolerance.
     * \param point point to search from
     * \param tolerance search tolerance
     * \returns segment index or -1 if not found
     */
    int findClosestSegment( const QgsPoint &point, double tolerance ) const;

  private:
    QVector<QgsAnchorWithHandles> mData;              //!< Anchor points with their handles (guarantees consistency)
    static const QgsAnchorWithHandles sInvalidAnchor; //!< Invalid anchor for out-of-bounds access
};

///@endcond PRIVATE

#endif // QGSBEZIERDATA_H
