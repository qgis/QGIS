/***************************************************************************
                             qgsmodelviewmouseevent.h
                             -------------------------
    Date                 : March 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMODELVIEWMOUSEEVENT_H
#define QGSMODELVIEWMOUSEEVENT_H

#include <QMouseEvent>

#include "qgis_gui.h"

#define SIP_NO_FILE

class QgsModelGraphicsView;

/**
 * \ingroup gui
 * A QgsModelViewMouseEvent is the result of a user interaction with the mouse on a QgsModelGraphicsView.
 *
 * It is sent whenever the user moves, clicks, releases or double clicks the mouse.
 * In addition to the coordinates in pixel space it also knows the coordinates the model space.
 *
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsModelViewMouseEvent : public QMouseEvent
{

  public:

    /**
     * Constructor for QgsModelViewMouseEvent. Should only be required to be called from the QgsModelGraphicsView.
     * \param view The view in which the event occurred.
     * \param event The original mouse event
     */
    QgsModelViewMouseEvent( QgsModelGraphicsView *view, QMouseEvent *event, bool snaps );

    /**
     * Returns the event point location in model coordinates.
     */
    QPointF modelPoint() const;

  private:

    //! The view in which the event was triggered.
    QgsModelGraphicsView *mView = nullptr;

    QPointF mModelPoint;

};

#endif // QGSMODELVIEWMOUSEEVENT_H
