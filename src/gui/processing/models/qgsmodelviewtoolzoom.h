/***************************************************************************
                             qgsmodelviewtoolzoom.h
                             -----------------------
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

#ifndef QGSMODELVIEWTOOLZOOM_H
#define QGSMODELVIEWTOOLZOOM_H

#include "qgis_sip.h"
#include "qgis_gui.h"
#include "qgsmodelviewtool.h"
#include "qgsmodelviewrubberband.h"
#include <memory>

#define SIP_NO_FILE

/**
 * \ingroup gui
 * \brief Model view tool for zooming into and out of the model.
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsModelViewToolZoom : public QgsModelViewTool
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsModelViewToolZoom.
     */
    QgsModelViewToolZoom( QgsModelGraphicsView *view SIP_TRANSFERTHIS );

    void modelPressEvent( QgsModelViewMouseEvent *event ) override;
    void modelMoveEvent( QgsModelViewMouseEvent *event ) override;
    void modelReleaseEvent( QgsModelViewMouseEvent *event ) override;
    void keyPressEvent( QKeyEvent *event ) override;
    void keyReleaseEvent( QKeyEvent *event ) override;
    void deactivate() override;

  protected:
    //! Will be TRUE will marquee zoom operation is in progress
    bool mMarqueeZoom = false;

  private:
    //! Start position for mouse press
    QPoint mMousePressStartPos;

    QPointF mRubberBandStartPos;

    //! Rubber band item
    std::unique_ptr<QgsModelViewRectangularRubberBand> mRubberBand;

    void startMarqueeZoom( QPointF scenePoint );
};

#endif // QGSMODELVIEWTOOLZOOM_H
