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

#include "qgspointxy.h"
#include "qgspointlocator.h"
#include "qgis_gui.h"

class QgsMapCanvas;
class QgsMapToolAdvancedDigitizing;

/**
 * \ingroup gui
 * A QgsMapMouseEvent is the result of a user interaction with the mouse on a QgsMapCanvas.
 * It is sent whenever the user moves, clicks, releases or double clicks the mouse.
 * In addition to the coordinates in pixel space it also knows the coordinates in the mapcanvas' CRS
 * as well as it knows the concept of snapping.
 */
class GUI_EXPORT QgsMapMouseEvent : public QMouseEvent
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( dynamic_cast<QgsMapMouseEvent *>( sipCpp ) )
      sipType = sipType_QgsMapMouseEvent;
    else
      sipType = 0;
    SIP_END
#endif

  public:

    /**
     * Creates a new QgsMapMouseEvent. Should only be required to be called from the QgsMapCanvas.
     *
     * \param mapCanvas The map canvas on which the event occurred
     * \param event     The original mouse event
     */
    QgsMapMouseEvent( QgsMapCanvas *mapCanvas, QMouseEvent *event );

    /**
     * Creates a new QgsMapMouseEvent. Should only be required to be called from the QgsMapCanvas.
     *
     * \param mapCanvas The map canvas on which the event occurred
     * \param type      The type of the event
     * \param pos       The pixel position of the mouse
     * \param button    The pressed button
     * \param buttons   Further buttons that are pressed
     * \param modifiers Keyboard modifiers
     */
    QgsMapMouseEvent( QgsMapCanvas *mapCanvas, QEvent::Type type, QPoint pos, Qt::MouseButton button = Qt::NoButton,
                      Qt::MouseButtons buttons = Qt::NoButton, Qt::KeyboardModifiers modifiers = Qt::NoModifier );

    /**
     * \brief snapPoint will snap the points using the map canvas snapping utils configuration
     * \note if snapping did not succeeded, the map point will be reset to its original position
     */
    QgsPointXY snapPoint();

    /**
     * Returns TRUE if there is a snapped point cached.
     * Will only be useful after snapPoint has previously been called.
     *
     * \returns TRUE if there is a snapped point cached.
     */
    bool isSnapped() const { return mSnapMatch.isValid(); }

    /**
     * \brief mapPoint returns the point in coordinates
     * \returns the point in map coordinates, after snapping if requested in the event.
     */
    inline QgsPointXY mapPoint() const { return mMapPoint; }

    /**
      * Returns the matching data from the most recently snapped point.
      * \returns the snapping data structure
      * \note This method returns the most recent snap match. It must
      * follow a call to snapPoint() in order to have a recent snap result available.
      * \since QGIS 2.14
      */
    QgsPointLocator::Match mapPointMatch() const { return mSnapMatch; }

    /**
     * Set the (snapped) point this event points to in map coordinates.
     * The point in pixel coordinates will be calculated accordingly.
     *
     * \param point The point in map coordinates
     */
    void setMapPoint( const QgsPointXY &point );

    /**
     * Returns the original, unmodified map point of the mouse cursor.
     *
     * \returns The cursor position in map coordinates.
     */
    QgsPointXY originalMapPoint() const { return mMapPoint; }

    /**
     * The snapped mouse cursor in pixel coordinates.
     *
     * \returns The snapped mouse cursor position in pixel coordinates.
     */
    QPoint pixelPoint() const { return mPixelPoint; }

    /**
     * The unsnapped, real mouse cursor position in pixel coordinates.
     * Alias to pos()
     *
     * \returns Mouse position in pixel coordinates
     */
    QPoint originalPixelPoint() const { return pos(); }

    /**
     * Snaps the mapPoint to a grid with the given \a precision.
     * The snapping will be done in the specified \a crs. If this crs is
     * different from the mapCanvas crs, it will be reprojected on the fly.
     *
     * \since QGIS 3.4
     */
    void snapToGrid( double precision, const QgsCoordinateReferenceSystem &crs );

  private:

    QPoint mapToPixelCoordinates( const QgsPointXY &point );

    //! Whether snapPoint() was already called
    bool mHasCachedSnapResult;

    //! Unsnapped point in map coordinates.
    QgsPointXY mOriginalMapPoint;

    //! Location in map coordinates. May be snapped.
    QgsPointXY mMapPoint;

    /**
     * Location in pixel coordinates. May be snapped.
     * Original pixel point available through the parent QMouseEvent.
     */
    QPoint mPixelPoint;

    //! The map canvas on which the event was triggered.
    QgsMapCanvas *mMapCanvas = nullptr;

    QgsPointLocator::Match mSnapMatch;
};

#endif // QGSMAPMOUSEEVENT_H
