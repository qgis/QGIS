/***************************************************************************
                          qgsplotmouseevent.h
                          ---------------
    begin                : March 2022
    copyright            : (C) 2022 by Nyall Dawson
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
#ifndef QGSPLOTMOUSEEVENT_H
#define QGSPLOTMOUSEEVENT_H

#include <QMouseEvent>

#include "qgspointxy.h"
#include "qgspointlocator.h"
#include "qgis_gui.h"

class QgsPlotCanvas;

/**
 * \ingroup gui
 * \brief A QgsPlotMouseEvent is the result of a user interaction with the mouse on a QgsPlotCanvas.
 *
 * The event is sent whenever the user moves, clicks, releases or double clicks the mouse.
 *
 * In addition to the coordinates in pixel space the event may have knowledge about
 * geographic coordinates corresponding to the event.
 *
 * \since QGIS 3.26
 */
class GUI_EXPORT QgsPlotMouseEvent : public QMouseEvent
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( dynamic_cast<QgsPlotMouseEvent *>( sipCpp ) )
      sipType = sipType_QgsPlotMouseEvent;
    else
      sipType = 0;
    SIP_END
#endif

  public:

    /**
     * Creates a new QgsPlotMouseEvent.
     *
     * \param canvas The map canvas on which the event occurred
     * \param event The original mouse event
     */
    QgsPlotMouseEvent( QgsPlotCanvas *canvas, QMouseEvent *event );

    /**
     * Creates a new QgsPlotMouseEvent.
     *
     * \param canvas The canvas on which the event occurred
     * \param type The type of the event
     * \param pos The pixel position of the mouse
     * \param button The pressed button
     * \param buttons Further buttons that are pressed
     * \param modifiers Keyboard modifiers
     */
    QgsPlotMouseEvent( QgsPlotCanvas *canvas, QEvent::Type type, QPoint pos, Qt::MouseButton button = Qt::NoButton,
                       Qt::MouseButtons buttons = Qt::NoButton, Qt::KeyboardModifiers modifiers = Qt::NoModifier );

    /**
     * Returns the point in map coordinates corresponding to the event.
     *
     * May return an empty point if the event cannot be converted to a map point.
     */
    QgsPoint mapPoint() const;

    /**
     * Returns the point snapped to the plot, if possible.
     *
     * Returns the original canvas point if snapping was not possible.
     */
    QgsPointXY snappedPoint();

    /**
     * Returns TRUE if the point can be snapped to the plot.
     */
    bool isSnapped();

  private:

    //! The canvas on which the event was triggered.
    QgsPlotCanvas *mCanvas = nullptr;

    //! Location in map coordinates
    QgsPoint mMapPoint;

    bool mHasCachedSnapResult = false;
    QgsPointXY mSnappedPoint;
    bool mIsSnapped = false;

    void snapPoint();

};

#endif // QGSPLOTMOUSEEVENT_H
