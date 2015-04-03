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

class QgsMapCanvas;
class QgsMapToolAdvancedDigitizing;

class APP_EXPORT QgsMapMouseEvent : public QMouseEvent
{
  public:

    enum SnappingMode
    {
      NoSnapping,
      SnapProjectConfig,  //!< snap according to the configuration set in the snapping settings
      SnapAllLayers,      //!< snap to all rendered layers (tolerance and type from defaultSettings())
    };

    explicit QgsMapMouseEvent( QgsMapToolAdvancedDigitizing* mapTool, QMouseEvent* event, SnappingMode mode = NoSnapping );

    explicit QgsMapMouseEvent( QgsMapToolAdvancedDigitizing* mapTool, QgsPoint point,
                               Qt::MouseButton button, Qt::KeyboardModifiers modifiers,
                               QEvent::Type eventType = QEvent::MouseButtonRelease, SnappingMode mode = NoSnapping );

    //! returns the corresponding map tool
    QgsMapToolAdvancedDigitizing* mapTool() {return mMapTool;}

    //! modify the point in map coordinates without changing values in pixel coordinates
    void setPoint( const QgsPoint& point );

    //! returns the first snapped segment. If the snapped match is a segment, it will simply return it.
    //! Otherwise it will try to snap a segment according to the event's snapping mode
    //! @param snapped if given, determines if a segment has been snapped
    //! @param allLayers if true, override snapping mode
    QList<QgsPoint> snapSegment( bool* snapped = 0, bool allLayers = false ) const;

    /**
     * @brief mapPoint returns the point in coordinates
     * @return the point in map coordinates, after snapping if requested in the event.
     */
    QgsPoint mapPoint() const { return mMapPoint; }

    //! determines if the returned mapPoint() is snapped (to a vertex or to a segment)
    bool isSnapped() const { return mSnapMatch.isValid(); }

    //! determines if the returned mapPoint() is snapped to a vertex. If snapped to a segment (or not snapped at all), will be set to false.
    bool isSnappedToVertex() const { return mSnapMatch.hasVertex(); }

  private:
    /**
     * @brief snapPoint will snap the points using the map canvas snapping utils configuration
     * @note if snapping did not succeeded, the map point will be reset to its original position
     */
    void snapPoint();

    static QPoint mapToPixelCoordinates( QgsMapCanvas* canvas, const QgsPoint& point );

    QgsPoint mMapPoint;

    QgsPoint mOriginalPoint;

    QgsMapToolAdvancedDigitizing* mMapTool;
    QgsPointLocator::Match mSnapMatch;
    SnappingMode mSnappingMode;
};

#endif // QGSMAPMOUSEEVENT_H
