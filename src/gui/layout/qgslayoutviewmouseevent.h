/***************************************************************************
                             qgslayoutviewmouseevent.h
                             -------------------------
    Date                 : July 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTVIEWMOUSEEVENT_H
#define QGSLAYOUTVIEWMOUSEEVENT_H

#include <QMouseEvent>

#include "qgis_gui.h"

class QgsLayoutView;
class QGraphicsLineItem;
class QgsLayoutItem;

/**
 * \ingroup gui
 * A QgsLayoutViewMouseEvent is the result of a user interaction with the mouse on a QgsLayoutView.
 *
 * It is sent whenever the user moves, clicks, releases or double clicks the mouse.
 * In addition to the coordinates in pixel space it also knows the coordinates the layout space.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsLayoutViewMouseEvent : public QMouseEvent
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( dynamic_cast<QgsLayoutViewMouseEvent *>( sipCpp ) )
      sipType = sipType_QgsLayoutViewMouseEvent;
    else
      sipType = 0;
    SIP_END
#endif

  public:

    /**
     * Constructor for QgsLayoutViewMouseEvent. Should only be required to be called from the QgsLayoutView.
     * \param view The view in which the event occurred.
     * \param event The original mouse event
     * \param snap set to TRUE to snap the point using the layout's snapping settings
     */
    QgsLayoutViewMouseEvent( QgsLayoutView *view, QMouseEvent *event, bool snap = false );

    /**
     * Manually triggers a snap for the mouse event position using the layout's snapper.
     *
     * If the \a horizontalSnapLine and \a verticalSnapLine arguments are specified, then the snapper
     * will automatically display and position these lines to indicate snapping positions to item bounds.
     *
     * The \a ignoreItems argument can be used to specify a list of items to avoid snapping to.
     */
    void snapPoint( QGraphicsLineItem *horizontalSnapLine = nullptr, QGraphicsLineItem *verticalSnapLine = nullptr,
                    const QList<QgsLayoutItem *> &ignoreItems = QList< QgsLayoutItem * >() );

    /**
     * Returns the event point location in layout coordinates.
     */
    QPointF layoutPoint() const;

    /**
     * Returns the snapped event point location in layout coordinates. The snapped point will consider
     * all possible snapping methods, such as snapping to grid or guide lines.
     * \see isSnapped()
     */
    QPointF snappedPoint() const { return mSnappedPoint; }

    /**
     * Returns TRUE if point was snapped, e.g. to grid or guide lines.
     * \see snappedPoint()
     */
    bool isSnapped() const { return mSnapped; }

  private:

    //! The view in which the event was triggered.
    QgsLayoutView *mView = nullptr;

    bool mSnapped = false;
    QPointF mLayoutPoint;
    QPointF mSnappedPoint;

};

#endif // QGSLAYOUTVIEWMOUSEEVENT_H
