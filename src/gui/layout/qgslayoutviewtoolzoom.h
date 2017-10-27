/***************************************************************************
                             qgslayoutviewtoolzoom.h
                             -----------------------
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

#ifndef QGSLAYOUTVIEWTOOLZOOM_H
#define QGSLAYOUTVIEWTOOLZOOM_H

#include "qgis.h"
#include "qgis_gui.h"
#include "qgslayoutviewtool.h"
#include "qgslayoutviewrubberband.h"
#include <memory>

#define SIP_NO_FILE

/**
 * \ingroup gui
 * Layout view tool for zooming into and out of the layout.
 * \since QGIS 3.0
 * \note Not part of stable API, and may be revised for 3.2
 * \note Not available in Python bindings.
 */
class GUI_EXPORT QgsLayoutViewToolZoom : public QgsLayoutViewTool
{

    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutViewToolZoom.
     */
    QgsLayoutViewToolZoom( QgsLayoutView *view SIP_TRANSFERTHIS );

    void layoutPressEvent( QgsLayoutViewMouseEvent *event ) override;
    void layoutMoveEvent( QgsLayoutViewMouseEvent *event ) override;
    void layoutReleaseEvent( QgsLayoutViewMouseEvent *event ) override;
    void keyPressEvent( QKeyEvent *event ) override;
    void keyReleaseEvent( QKeyEvent *event ) override;
    void deactivate() override;

  protected:

    //! Will be true will marquee zoom operation is in progress
    bool mMarqueeZoom = false;

  private:

    //! Start position for mouse press
    QPoint mMousePressStartPos;

    QPointF mRubberBandStartPos;

    //! Rubber band item
    std::unique_ptr< QgsLayoutViewRectangularRubberBand > mRubberBand;

    void startMarqueeZoom( QPointF scenePoint );

};

#endif // QGSLAYOUTVIEWTOOLZOOM_H
