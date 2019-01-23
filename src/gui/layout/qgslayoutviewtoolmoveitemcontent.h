/***************************************************************************
                             qgslayoutviewtoolmoveitemcontent.h
                             -------------------------
    Date                 : October 2017
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

#ifndef QGSLAYOUTVIEWTOOLMOVEITEMCONTENT_H
#define QGSLAYOUTVIEWTOOLMOVEITEMCONTENT_H

#include "qgis_sip.h"
#include "qgis_gui.h"
#include "qgslayoutviewtool.h"

/**
 * \ingroup gui
 * Layout view tool for moving and zooming item content.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsLayoutViewToolMoveItemContent : public QgsLayoutViewTool
{

    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutViewToolMoveItemContent.
     */
    QgsLayoutViewToolMoveItemContent( QgsLayoutView *view SIP_TRANSFERTHIS );

    void layoutPressEvent( QgsLayoutViewMouseEvent *event ) override;
    void layoutMoveEvent( QgsLayoutViewMouseEvent *event ) override;
    void layoutReleaseEvent( QgsLayoutViewMouseEvent *event ) override;
    void wheelEvent( QWheelEvent *event ) override;

  private:

    //! Item to move content
    QgsLayoutItem *mMoveContentItem = nullptr;

    //! Start position of content move
    QPointF mMoveContentStartPos;

    bool mMovingItemContent = false;
};

#endif // QGSLAYOUTVIEWTOOLMOVEITEMCONTENT_H
