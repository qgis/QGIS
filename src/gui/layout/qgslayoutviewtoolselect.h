/***************************************************************************
                             qgslayoutviewtoolselect.h
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

#ifndef QGSLAYOUTVIEWTOOLSELECT_H
#define QGSLAYOUTVIEWTOOLSELECT_H

#include "qgis.h"
#include "qgis_gui.h"
#include "qgslayoutviewtool.h"
#include "qgslayoutviewrubberband.h"
#include <memory>

class QgsLayoutMouseHandles;

/**
 * \ingroup gui
 * Layout view tool for selecting items in the layout.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsLayoutViewToolSelect : public QgsLayoutViewTool
{

    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutViewToolSelect.
     */
    QgsLayoutViewToolSelect( QgsLayoutView *view SIP_TRANSFERTHIS );
    ~QgsLayoutViewToolSelect() override;

    void layoutPressEvent( QgsLayoutViewMouseEvent *event ) override;
    void layoutMoveEvent( QgsLayoutViewMouseEvent *event ) override;
    void layoutReleaseEvent( QgsLayoutViewMouseEvent *event ) override;
    void wheelEvent( QWheelEvent *event ) override;
    void keyPressEvent( QKeyEvent *event ) override;
    void deactivate() override;

    ///@cond PRIVATE

    /**
     * Returns the view's mouse handles.
     * \note Not available in Python bindings.
     */
    SIP_SKIP QgsLayoutMouseHandles *mouseHandles();
    ///@endcond

    //! Sets the a \a layout.
    void setLayout( QgsLayout *layout );

  private:

    bool mIsSelecting = false;

    //! Rubber band item
    std::unique_ptr< QgsLayoutViewRubberBand > mRubberBand;

    //! Start position for mouse press
    QPoint mMousePressStartPos;

    //! Start of rubber band creation
    QPointF mRubberBandStartPos;

    QPointer< QgsLayoutMouseHandles > mMouseHandles; //owned by scene
};

#endif // QGSLAYOUTVIEWTOOLSELECT_H
