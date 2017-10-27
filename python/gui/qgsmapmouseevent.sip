/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsmapmouseevent.h                                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsMapMouseEvent : QMouseEvent
{
%Docstring
 A QgsMapMouseEvent is the result of a user interaction with the mouse on a QgsMapCanvas.
 It is sent whenever the user moves, clicks, releases or double clicks the mouse.
 In addition to the coordinates in pixel space it also knows the coordinates in the mapcanvas' CRS
 as well as it knows the concept of snapping.
%End

%TypeHeaderCode
#include "qgsmapmouseevent.h"
%End
%ConvertToSubClassCode
    if ( dynamic_cast<QgsMapMouseEvent *>( sipCpp ) )
      sipType = sipType_QgsMapMouseEvent;
    else
      sipType = 0;
%End
  public:

    QgsMapMouseEvent( QgsMapCanvas *mapCanvas, QMouseEvent *event );
%Docstring
 Creates a new QgsMapMouseEvent. Should only be required to be called from the QgsMapCanvas.

 \param mapCanvas The map canvas on which the event occurred
 \param event     The original mouse event
%End

    QgsMapMouseEvent( QgsMapCanvas *mapCanvas, QEvent::Type type, QPoint pos, Qt::MouseButton button = Qt::NoButton,
                      Qt::MouseButtons buttons = Qt::NoButton, Qt::KeyboardModifiers modifiers = Qt::NoModifier );
%Docstring
 Creates a new QgsMapMouseEvent. Should only be required to be called from the QgsMapCanvas.

 \param mapCanvas The map canvas on which the event occurred
 \param type      The type of the event
 \param pos       The pixel position of the mouse
 \param button    The pressed button
 \param buttons   Further buttons that are pressed
 \param modifiers Keyboard modifiers
%End

    QgsPointXY snapPoint();
%Docstring
 snapPoint will snap the points using the map canvas snapping utils configuration
.. note::

   if snapping did not succeeded, the map point will be reset to its original position
 :rtype: QgsPointXY
%End

    bool isSnapped() const;
%Docstring
 Returns true if there is a snapped point cached.
 Will only be useful after snapPoint has previously been called.

 :return: True if there is a snapped point cached.
 :rtype: bool
%End

    QgsPointXY mapPoint() const;
%Docstring
 mapPoint returns the point in coordinates
 :return: the point in map coordinates, after snapping if requested in the event.
 :rtype: QgsPointXY
%End

    QgsPointLocator::Match mapPointMatch() const;
%Docstring
 Returns the matching data from the most recently snapped point.
 :return: the snapping data structure
.. versionadded:: 2.14
 :rtype: QgsPointLocator.Match
%End

    void setMapPoint( const QgsPointXY &point );
%Docstring
 Set the (snapped) point this event points to in map coordinates.
 The point in pixel coordinates will be calculated accordingly.

 \param point The point in map coordinates
%End

    QgsPointXY originalMapPoint() const;
%Docstring
 Returns the original, unmodified map point of the mouse cursor.

 :return: The cursor position in map coordinates.
 :rtype: QgsPointXY
%End

    QPoint pixelPoint() const;
%Docstring
 The snapped mouse cursor in pixel coordinates.

 :return: The snapped mouse cursor position in pixel coordinates.
 :rtype: QPoint
%End

    QPoint originalPixelPoint() const;
%Docstring
 The unsnapped, real mouse cursor position in pixel coordinates.
 Alias to pos()

 :return: Mouse position in pixel coordinates
 :rtype: QPoint
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsmapmouseevent.h                                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
