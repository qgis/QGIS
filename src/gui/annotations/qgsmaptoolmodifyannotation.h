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

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgsmaptooladvanceddigitizing.h"
#include "qobjectuniqueptr.h"
#include "qgspointxy.h"
#include "qgsannotationitemnode.h"

class QgsRubberBand;
class QgsRenderedAnnotationItemDetails;
class QgsAnnotationItem;
class QgsAnnotationLayer;
class QgsAnnotationItemNodesSpatialIndex;
class QgsSnapIndicator;

#define SIP_NO_FILE

/**
 * \ingroup gui
 * \brief A map tool for modifying annotations in a QgsAnnotationLayer
 * \note Not available in Python bindings
 * \since QGIS 3.22
 */
class GUI_EXPORT QgsMapToolModifyAnnotation : public QgsMapToolAdvancedDigitizing
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
    const QgsRenderedAnnotationItemDetails *findClosestItemToPoint( const QgsPointXY &mapPoint, const QList<const QgsRenderedAnnotationItemDetails *> &items, QgsRectangle &bounds );
    QgsAnnotationLayer *annotationLayerFromId( const QString &layerId );
    QgsAnnotationItem *annotationItemFromId( const QString &layerId, const QString &itemId );

    void setHoveredItem( const QgsRenderedAnnotationItemDetails *item, const QgsRectangle &itemMapBounds );

    /**
     * Returns the delta (in layer coordinates) by which to move items
     * for the given key \a event.
     */
    QSizeF deltaForKeyEvent( QgsAnnotationLayer *layer, const QgsPointXY &originalCanvasPoint, QKeyEvent *event );

    Action mCurrentAction = Action::NoAction;

    std::unique_ptr<QgsSnapIndicator> mSnapIndicator;

    QObjectUniquePtr<QgsRubberBand> mHoverRubberBand;
    std::vector< QObjectUniquePtr<QgsRubberBand> > mHoveredItemNodeRubberBands;

    // nodes from the current hovered item, reprojected onto the map canvas' CRS
    QList< QgsAnnotationItemNode >  mHoveredItemNodes;

    QObjectUniquePtr<QgsRubberBand> mHoveredNodeRubberBand;

    QObjectUniquePtr<QgsRubberBand> mSelectedRubberBand;
    QObjectUniquePtr<QgsRubberBand> mTemporaryRubberBand;

    QString mHoveredItemId;
    QString mHoveredItemLayerId;

    QString mSelectedItemId;
    QString mSelectedItemLayerId;

    std::unique_ptr< QgsAnnotationItemNodesSpatialIndex > mHoveredItemNodesSpatialIndex;

    QgsPointXY mMoveStartPointCanvasCrs;
    QgsPointXY mMoveStartPointLayerCrs;

    bool mRefreshSelectedItemAfterRedraw = false;

    QgsAnnotationItemNode mTargetNode;

};

#endif // QGSMAPTOOLMODIFYANNOTATION_H
