/***************************************************************************
                             qgslayoutviewtooladdnodeitem.h
                             --------------------------
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

#ifndef QGSLAYOUTVIEWTOOLADDNODEITEM_H
#define QGSLAYOUTVIEWTOOLADDNODEITEM_H

#include "qgis.h"
#include "qgis_gui.h"
#include "qgslayoutviewtool.h"
#include <memory>
#include <QAbstractGraphicsShapeItem>

/**
 * \ingroup gui
 * Layout view tool for adding node based items to a layout.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsLayoutViewToolAddNodeItem : public QgsLayoutViewTool
{

    Q_OBJECT

  public:

    //! Constructs a QgsLayoutViewToolAddNodeItem for the given layout \a view.
    QgsLayoutViewToolAddNodeItem( QgsLayoutView *view SIP_TRANSFERTHIS );

    /**
     * Returns the item metadata id for items created by the tool.
     * \see setItemMetadataId()
     */
    int itemMetadataId() const;

    /**
     * Sets the item metadata \a metadataId for items created by the tool.
     *
     * The \a metadataId associates the current tool behavior with a metadata entry
     * from QgsLayoutItemGuiRegistry.
     *
     * \see itemMetadataId()
     */
    void setItemMetadataId( int metadataId );

    void layoutPressEvent( QgsLayoutViewMouseEvent *event ) override;
    void layoutMoveEvent( QgsLayoutViewMouseEvent *event ) override;
    void layoutReleaseEvent( QgsLayoutViewMouseEvent *event ) override;
    void keyPressEvent( QKeyEvent *event ) override;
    void deactivate() override;

  signals:

    /**
     * Emitted when an item has been created using the tool.
     */
    void createdItem();

  private:

    int mItemMetadataId = -1;

    //! Rubber band item
    std::unique_ptr< QAbstractGraphicsShapeItem > mRubberBand;

    QPolygonF mPolygon;

    void addNode( QPointF scenePoint );
    void moveTemporaryNode( QPointF scenePoint, Qt::KeyboardModifiers modifiers );
    void setRubberBandNodes();

};

#endif // QGSLAYOUTVIEWTOOLADDNODEITEM_H
