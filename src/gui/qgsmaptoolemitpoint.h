/***************************************************************************
    qgsmaptoolemitpoint.h  -  map tool that emits a signal on click
    ---------------------
    begin                : June 2007
    copyright            : (C) 2007 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLEMITPOINT_H
#define QGSMAPTOOLEMITPOINT_H

#include "qgspoint.h"
#include "qgsmaptool.h"
class QgsMapCanvas;


/** \ingroup gui
 * A map tool that simply emits a point when clicking on the map.
 * Connecting a slot to its canvasClicked() signal will
 * let you implement custom behaviour for the passed in point.
 */
class GUI_EXPORT QgsMapToolEmitPoint : public QgsMapTool
{
    Q_OBJECT

  public:
    //! constructor
    QgsMapToolEmitPoint( QgsMapCanvas* canvas );

    virtual Flags flags() const override { return QgsMapTool::AllowZoomRect; }

    //! Overridden mouse move event
    virtual void canvasMoveEvent( QgsMapMouseEvent* e ) override;

    //! Overridden mouse press event - emits the signal
    virtual void canvasPressEvent( QgsMapMouseEvent* e ) override;

    //! Overridden mouse release event
    virtual void canvasReleaseEvent( QgsMapMouseEvent* e ) override;

  signals:
    //! signal emitted on canvas click
    void canvasClicked( const QgsPoint& point, Qt::MouseButton button );
};

#endif
