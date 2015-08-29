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

/**
 * A QgsMapMouseEvent is the result of a user interaction with the mouse on a QgsMapCanvas.
 * It is sent whenever the user moves, clicks, releases or double clicks the mouse.
 * In addition to the coordiantes in pixel space it also knows the coordinates in the mapcanvas' CRS
 * as well as it knows the concept of snapping.
 */
class QgsMapMouseEvent : QMouseEvent
{
%TypeHeaderCode
#include "qgsmapmouseevent.h"
%End
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
     * @param mapCanvas The map canvas on which the event occured
     * @param event     The original mouse event
     */
    QgsMapMouseEvent( QgsMapCanvas* mapCanvas, QMouseEvent* event );

    /**
     * @brief snapPoint will snap the points using the map canvas snapping utils configuration
     * @note if snapping did not succeeded, the map point will be reset to its original position
     */
    QgsPoint snapPoint( SnappingMode snappingMode );

    /**
     * returns the first snapped segment. If the cached snapped match is a segment, it will simply return it.
     * Otherwise it will try to snap a segment according to the event's snapping mode. In this case the cache
     * will not be overwritten.
     * @param snapped if given, determines if a segment has been snapped
     * @param allLayers if true, override snapping mode
     */
    QList<QgsPoint> snapSegment( SnappingMode snappingMode, bool* snapped = 0, bool allLayers = false ) const;

    /**
     * Returns true if there is a snapped point cached.
     * Will only be useful after snapPoint has previously been called.
     *
     * @return True if there is a snapped point cached.
     */
    bool isSnapped() const;

    /**
     * @brief mapPoint returns the point in coordinates
     * @return the point in map coordinates, after snapping if requested in the event.
     */
    QgsPoint mapPoint() const;

    QgsPoint originalMapPoint() const;

    QPoint pixelPoint() const;

    QPoint originalPixelPoint() const;
};
