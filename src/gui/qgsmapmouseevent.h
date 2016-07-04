/***************************************************************************
    qgsmapmouseevent.h  -  mouse event in map coordinates and ability to snap
    ----------------------
    begin                : October 2014
    copyright            : (C) Denis Rouzaud
    email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPMOUSEEVENT_H
#define QGSMAPMOUSEEVENT_H

#include <QMouseEvent>

#include "qgspoint.h"
#include "qgspointlocator.h"
#include "qgssnappingutils.h"

class QgsMapCanvas;
class QgsMapToolAdvancedDigitizing;

/** \ingroup gui
 * A QgsMapMouseEvent is the result of a user interaction with the mouse on a QgsMapCanvas.
 * It is sent whenever the user moves, clicks, releases or double clicks the mouse.
 * In addition to the coordiantes in pixel space it also knows the coordinates in the mapcanvas' CRS
 * as well as it knows the concept of snapping.
 */
class GUI_EXPORT QgsMapMouseEvent : public QMouseEvent
{
  public:

    enum SnappingMode
    {
      NoSnapping,
      SnapProjectConfig,  //!< snap according to the configuration set in the snapping settings
      SnapAllLayers,      //!< snap to all rendered layers (tolerance and type from defaultSettings())
    };

    /**
     * Creates a new QgsMapMouseEvent. Should only be required to be called from the QgsMapCanvas.
     *
     * @param mapCanvas The map canvas on which the event occurred
     * @param event     The original mouse event
     */
    QgsMapMouseEvent( QgsMapCanvas* mapCanvas, QMouseEvent* event );

    /**
     * Creates a new QgsMapMouseEvent. Should only be required to be called from the QgsMapCanvas.
     *
     * @param mapCanvas The map canvas on which the event occurred
     * @param type      The type of the event
     * @param pos       The pixel position of the mouse
     * @param button    The pressed button
     * @param buttons   Further buttons that are pressed
     * @param modifiers Keyboard modifiers
     */
    QgsMapMouseEvent( QgsMapCanvas* mapCanvas, QEvent::Type type, QPoint pos, Qt::MouseButton button = Qt::NoButton,
                      Qt::MouseButtons buttons = Qt::NoButton, Qt::KeyboardModifiers modifiers = Qt::NoModifier );

    /**
     * @brief snapPoint will snap the points using the map canvas snapping utils configuration
     * @note if snapping did not succeeded, the map point will be reset to its original position
     */
    QgsPoint snapPoint( SnappingMode snappingMode );

    /**
     * Returns the first snapped segment. If the cached snapped match is a segment, it will simply return it.
     * Otherwise it will try to snap a segment according to the event's snapping mode. In this case the cache
     * will not be overwritten.
     * @param snappingMode Specify if the default project settings or all layers should be used for snapping
     * @param snapped if given, determines if a segment has been snapped
     * @param allLayers if true, override snapping mode
     */
    QList<QgsPoint> snapSegment( SnappingMode snappingMode, bool* snapped = nullptr, bool allLayers = false ) const;

    /**
     * Returns true if there is a snapped point cached.
     * Will only be useful after snapPoint has previously been called.
     *
     * @return True if there is a snapped point cached.
     */
    bool isSnapped() const { return mSnapMatch.isValid(); }

    /**
     * @brief mapPoint returns the point in coordinates
     * @return the point in map coordinates, after snapping if requested in the event.
     */
    inline QgsPoint mapPoint() const { return mMapPoint; }

    /**
      * Returns the matching data from the most recently snapped point.
      * @return the snapping data structure
      * @note added in 2.14
      */
    QgsPointLocator::Match mapPointMatch() const { return mSnapMatch; }

    /**
     * Set the (snapped) point this event points to in map coordinates.
     * The point in pixel coordinates will be calculated accordingly.
     *
     * @param point The point in map coordinates
     */
    void setMapPoint( const QgsPoint& point );

    /**
     * Returns the original, unmodified map point of the mouse cursor.
     *
     * @return The cursor position in map coordinates.
     */
    QgsPoint originalMapPoint() const { return mMapPoint; }

    /**
     * The snapped mouse cursor in pixel coordinates.
     *
     * @return The snapped mouse cursor position in pixel coordinates.
     */
    QPoint pixelPoint() const { return mPixelPoint; }

    /**
     * The unsnapped, real mouse cursor position in pixel coordinates.
     * Alias to pos()
     *
     * @return Mouse position in pixel coordinates
     */
    QPoint originalPixelPoint() const { return pos(); }

  private:

    QPoint mapToPixelCoordinates( const QgsPoint& point );

    SnappingMode mSnappingMode;

    //! Unsnapped point in map coordinates.
    QgsPoint mOriginalMapPoint;

    //! Location in map coordinates. May be snapped.
    QgsPoint mMapPoint;

    //! Location in pixel coordinates. May be snapped.
    //! Original pixel point available through the parent QMouseEvent.
    QPoint mPixelPoint;

    //! The map canvas on which the event was triggered.
    QgsMapCanvas* mMapCanvas;

    QgsPointLocator::Match mSnapMatch;
};

#endif // QGSMAPMOUSEEVENT_H
