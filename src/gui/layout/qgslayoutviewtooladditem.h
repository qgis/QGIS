/***************************************************************************
                             qgslayoutviewtooladditem.h
                             --------------------------
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

#ifndef QGSLAYOUTVIEWTOOLADDITEM_H
#define QGSLAYOUTVIEWTOOLADDITEM_H

#include "qgis_sip.h"
#include "qgis_gui.h"
#include "qgslayoutviewtool.h"
#include "qgslayoutviewrubberband.h"
#include <memory>

/**
 * \ingroup gui
 * \brief Layout view tool for adding items to a layout.
 */
class GUI_EXPORT QgsLayoutViewToolAddItem : public QgsLayoutViewTool
{
    Q_OBJECT

  public:
    //! Constructs a QgsLayoutViewToolAddItem for the given layout \a view.
    QgsLayoutViewToolAddItem( QgsLayoutView *view SIP_TRANSFERTHIS );

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
    void activate() override;
    void deactivate() override;

    /**
     * Returns any custom properties set for the tool.
     *
     *\see setCustomProperties()
     * \since QGIS 3.18
     */
    QVariantMap customProperties() const;

    /**
     * Sets custom \a properties for the tool.
     *
     * These properties are transient, and are cleared whenever the tool is activated. Callers must ensure
     * that the properties are set only after the tool is activated.
     *
     *\see customProperties()
     * \since QGIS 3.18
     */
    void setCustomProperties( const QVariantMap &properties );

  signals:

    /**
     * Emitted when an item has been created using the tool.
     */
    void createdItem();

  private:
    bool mDrawing = false;

    int mItemMetadataId = -1;

    //! Rubber band item
    std::unique_ptr<QgsLayoutViewRubberBand> mRubberBand;

    //! Start position for mouse press
    QPoint mMousePressStartPos;

    //! Start position for mouse press in layout coordinates
    QPointF mMousePressStartLayoutPos;

    //! Start of rubber band creation
    QPointF mRubberBandStartPos;

    QVariantMap mCustomProperties;
};

#endif // QGSLAYOUTVIEWTOOLADDITEM_H
