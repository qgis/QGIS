/***************************************************************************
                             qgslayoutviewtooleditnodes.h
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

#ifndef QGSLAYOUTVIEWTOOLEDITNODES_H
#define QGSLAYOUTVIEWTOOLEDITNODES_H

#include "qgis.h"
#include "qgis_gui.h"
#include "qgslayoutviewtool.h"

class QgsLayoutNodesItem;

/**
 * \ingroup gui
 * Layout view tool for edit node based items in the layout.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsLayoutViewToolEditNodes : public QgsLayoutViewTool
{

    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutViewToolEditNodes.
     */
    QgsLayoutViewToolEditNodes( QgsLayoutView *view SIP_TRANSFERTHIS );

    void activate() override;

    void layoutPressEvent( QgsLayoutViewMouseEvent *event ) override;
    void layoutMoveEvent( QgsLayoutViewMouseEvent *event ) override;
    void layoutReleaseEvent( QgsLayoutViewMouseEvent *event ) override;
    void deactivate() override;

  private:

    const double mMoveContentSearchRadius = 25;

    QgsLayoutNodesItem *mNodesItem = nullptr;
    int mNodesItemIndex = -1;

    //! Start position of content move
    QPointF mMoveContentStartPos;

    bool isMoving = false;

    void displayNodes( bool display = true );
    void deselectNodes();
    void setSelectedNode( QgsLayoutNodesItem *shape, int index );


};

#endif // QGSLAYOUTVIEWTOOLEDITNODES_H
