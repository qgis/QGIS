/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layout/qgslayoutviewmouseevent.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsLayoutViewMouseEvent : QMouseEvent
{
%Docstring
 A QgsLayoutViewMouseEvent is the result of a user interaction with the mouse on a QgsLayoutView.

 It is sent whenever the user moves, clicks, releases or double clicks the mouse.
 In addition to the coordinates in pixel space it also knows the coordinates the layout space.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutviewmouseevent.h"
%End

%ConvertToSubClassCode
    if ( dynamic_cast<QgsLayoutViewMouseEvent *>( sipCpp ) )
      sipType = sipType_QgsLayoutViewMouseEvent;
    else
      sipType = 0;
%End
  public:

    QgsLayoutViewMouseEvent( QgsLayoutView *view, QMouseEvent *event, bool snap = false );
%Docstring
 Constructor for QgsLayoutViewMouseEvent. Should only be required to be called from the QgsLayoutView.
 \param view The view in which the event occurred.
 \param event The original mouse event
 \param snap set to true to snap the point using the layout's snapping settings
%End

    QPointF layoutPoint() const;
%Docstring
 Returns the event point location in layout coordinates.
.. seealso:: pos()
 :rtype: QPointF
%End

    QPointF snappedPoint() const;
%Docstring
 Returns the snapped event point location in layout coordinates. The snapped point will consider
 all possible snapping methods, such as snapping to grid or guide lines.
.. seealso:: isSnapped()
.. seealso:: pos()
 :rtype: QPointF
%End

    bool isSnapped() const;
%Docstring
 Returns true if point was snapped, e.g. to grid or guide lines.
.. seealso:: snappedPoint()
 :rtype: bool
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layout/qgslayoutviewmouseevent.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
