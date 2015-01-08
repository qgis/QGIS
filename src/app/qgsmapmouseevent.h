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

#include "qgsmapcanvassnapper.h"
#include "qgspoint.h"

class QgsMapToolAdvancedDigitizing;

class APP_EXPORT QgsMapMouseEvent : public QMouseEvent
{
  public:
    explicit QgsMapMouseEvent( QgsMapToolAdvancedDigitizing* mapTool, QMouseEvent* event, bool doSnap = false );

    explicit QgsMapMouseEvent( QgsMapToolAdvancedDigitizing* mapTool, QgsPoint point,
                               Qt::MouseButton button, Qt::KeyboardModifiers modifiers,
                               QEvent::Type eventType = QEvent::MouseButtonRelease, bool doSnap = false );

    //! returns the corresponding map tool
    QgsMapToolAdvancedDigitizing* mapTool() {return mMapTool;}

    //! modify the point in map coordinates without changing values in pixel coordinates
    void setPoint( const QgsPoint& point );

    //! returns the first snapped segment
    QList<QgsPoint> snappedSegment( bool* snapped = 0 ) const;

    /**
     * @brief mapPoint returns the point in coordinates
     * @param snappedPoint determines if the result is a snapped point or not. If snapped to a segment, will be set to false.
     * @return the point in map coordinates, after snapping if requested in the event.
     */
    QgsPoint mapPoint( bool* snappedPoint = 0 ) const;

    /**
     * @brief snapPoint will snap the points using the map canvas snapper
     * @note if snapping did not succeeded, the map point will be reset to its original position
     */
    bool snapPoint();


  private:
    static QPoint mapToPixelCoordinates( QgsMapCanvas* canvas, const QgsPoint& point );

    QgsPoint mMapPoint;

    QgsPoint mOriginalPoint;
    bool mSnapped;

    QgsMapToolAdvancedDigitizing* mMapTool;
    QList<QgsSnappingResult> mSnapResults;
};

#endif // QGSMAPMOUSEEVENT_H
