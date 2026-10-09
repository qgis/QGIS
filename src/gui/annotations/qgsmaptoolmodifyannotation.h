/***************************************************************************
    qgsmaptoolmodifyannotation.h
    ----------------
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLMODIFYANNOTATION_H
#define QGSMAPTOOLMODIFYANNOTATION_H

#include <optional>

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgsannotationitemnode.h"
#include "qgsannotationmaptool.h"
#include "qgspointxy.h"
#include "qgsrectangle.h"
#include "qobjectuniqueptr.h"

#define SIP_NO_FILE

class QgsRubberBand;
class QgsRenderedAnnotationItemDetails;
class QgsAnnotationItem;
class QgsAnnotationLayer;
class QgsAnnotationRectItem;
class QgsAnnotationItemNodesSpatialIndex;
class QgsSnapIndicator;
class QgsMapToPixel;


/**
 * \ingroup gui
 * \brief A map tool for modifying annotations in a QgsAnnotationLayer
 * \note Not available in Python bindings
 * \since QGIS 3.22
 */
class GUI_EXPORT QgsMapToolModifyAnnotation : public QgsAnnotationMapTool
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsMapToolModifyAnnotation
     */
    explicit QgsMapToolModifyAnnotation( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget );
    ~QgsMapToolModifyAnnotation() override;

    void deactivate() override;
    void cadCanvasMoveEvent( QgsMapMouseEvent *event ) override;
    void cadCanvasPressEvent( QgsMapMouseEvent *event ) override;
    void canvasDoubleClickEvent( QgsMapMouseEvent *event ) override;
    void keyPressEvent( QKeyEvent *event ) override;

  signals:

    /**
     * Emitted when the selected item is changed.
     */
    void itemSelected( QgsAnnotationLayer *layer, const QString &itemId );

    /**
     * Emitted when the selected item is cleared;
     */
    void selectionCleared();

  private slots:
    void onCanvasRefreshed();

  private:
    enum class Action
    {
      NoAction,
      MoveItem,
      MoveNode
    };

    void clearHoveredItem();
    void clearSelectedItem();
    void createHoverBand();
    void createHoveredNodeBand();
    void createSelectedItemBand();

    void setHoveredItemFromPoint( const QgsPointXY &mapPoint );
    void setHoveredItem( const QgsRenderedAnnotationItemDetails *item, const QgsRectangle &itemMapBounds );

    /**
     * Returns the delta (in layer coordinates) by which to move items
     * for the given key \a event.
     */
    QSizeF deltaForKeyEvent( QgsAnnotationLayer *layer, const QgsPointXY &originalCanvasPoint, QKeyEvent *event );

    /**
     * Reconstructs the new (unrotated) bounds, in map coordinates, for a rotated rectangle whose
     * corner is being dragged. The diagonally-opposite corner (at \a fixedMapPoint) stays anchored
     * on screen while the dragged corner follows \a cursorMapPoint. \a angle is the applied rotation
     * in degrees clockwise on screen.
     */
    static QgsRectangle reconstructRotatedResizeBounds( const QgsMapToPixel *mapToPixel, double angle, const QgsPointXY &fixedMapPoint, const QgsPointXY &cursorMapPoint );

    /**
     * Returns the current on-screen (rotated) map position of the vertex diagonally opposite to
     * \a draggedVertex, searching \a nodes. Sets \a found accordingly.
     */
    static QgsPointXY oppositeVertexMapPoint( const QList<QgsAnnotationItemNode> &nodes, int draggedVertex, bool &found );

    /**
     * Returns the new bounds, in \a layer coordinates, for the rotated \a rectItem whose corner
     * is currently being dragged towards \a cursorMapPoint, or nothing if the current node drag is
     * not a rotated resize.
     */
    std::optional<QgsRectangle> rotatedResizeLayerBounds( const QgsAnnotationRectItem *rectItem, QgsAnnotationLayer *layer, const QgsPointXY &cursorMapPoint );

    Action mCurrentAction = Action::NoAction;

    std::unique_ptr<QgsSnapIndicator> mSnapIndicator;

    QObjectUniquePtr<QgsRubberBand> mHoverRubberBand;
    std::vector<QObjectUniquePtr<QgsRubberBand>> mHoveredItemNodeRubberBands;

    // nodes from the current hovered item, reprojected onto the map canvas' CRS
    QList<QgsAnnotationItemNode> mHoveredItemNodes;

    QObjectUniquePtr<QgsRubberBand> mHoveredNodeRubberBand;

    QObjectUniquePtr<QgsRubberBand> mSelectedRubberBand;
    QObjectUniquePtr<QgsRubberBand> mTemporaryRubberBand;

    QPoint mLastHoverPoint;
    QString mHoveredItemId;
    QString mHoveredItemLayerId;
    QgsRectangle mHoveredItemBounds;

    QString mSelectedItemId;
    QString mSelectedItemLayerId;
    QgsRectangle mSelectedItemBounds;

    std::unique_ptr<QgsAnnotationItemNodesSpatialIndex> mHoveredItemNodesSpatialIndex;

    QgsPointXY mMoveStartPointCanvasCrs;
    QgsPointXY mMoveStartPointLayerCrs;
    QgsPointXY mMoveStartPointPixels;

    bool mRefreshSelectedItemAfterRedraw = false;

    QgsAnnotationItemNode mTargetNode;
};

#endif // QGSMAPTOOLMODIFYANNOTATION_H
