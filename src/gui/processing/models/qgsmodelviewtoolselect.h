/***************************************************************************
                             qgsmodelviewtoolselect.h
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

#ifndef QGSMODELVIEWTOOLSELECT_H
#define QGSMODELVIEWTOOLSELECT_H

#include "qgis_sip.h"
#include "qgis_gui.h"
#include "qgsmodelviewtool.h"
#include "qgsmodelviewrubberband.h"
#include <memory>

class QgsModelViewMouseHandles;
class QGraphicsItem;

#define SIP_NO_FILE

/**
 * \ingroup gui
 * \brief Model designer view tool for selecting items in the model.
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsModelViewToolSelect : public QgsModelViewTool
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsModelViewToolSelect.
     */
    QgsModelViewToolSelect( QgsModelGraphicsView *view SIP_TRANSFERTHIS );
    ~QgsModelViewToolSelect() override;

    void modelPressEvent( QgsModelViewMouseEvent *event ) override;
    void modelMoveEvent( QgsModelViewMouseEvent *event ) override;
    void modelDoubleClickEvent( QgsModelViewMouseEvent *event ) override;
    void modelReleaseEvent( QgsModelViewMouseEvent *event ) override;
    void wheelEvent( QWheelEvent *event ) override;
    void keyPressEvent( QKeyEvent *event ) override;
    void deactivate() override;
    bool allowItemInteraction() override;

    /**
     * Returns the view's mouse handles.
     */
    QgsModelViewMouseHandles *mouseHandles();

    //! Sets the a \a scene.
    void setScene( QgsModelGraphicsScene *scene );

    /**
     * Resets the internal cache following a scene change.
     */
    void resetCache();

  private:
    bool mIsSelecting = false;

    //! Rubber band item
    std::unique_ptr<QgsModelViewRubberBand> mRubberBand;

    //! Start position for mouse press
    QPoint mMousePressStartPos;

    //! Start of rubber band creation
    QPointF mRubberBandStartPos;

    QPointer<QgsModelViewMouseHandles> mMouseHandles; //owned by scene
    QList<QGraphicsItem *> mHoverEnteredItems;
};

#endif // QGSMODELVIEWTOOLSELECT_H
