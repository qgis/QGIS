/***************************************************************************
    qgsmaptoolselectannotation.h
    ----------------
    copyright            : (C) 2025 by Mathieu Pellerin
    email                : mathieu at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLSELECTANNOTATION_H
#define QGSMAPTOOLSELECTANNOTATION_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgsannotationitemnode.h"
#include "qgsannotationmaptool.h"
#include "qgsmaptooladvanceddigitizing.h"
#include "qgsmaptoolselectannotationmousehandles.h"
#include "qgspointxy.h"
#include "qgsrectangle.h"
#include "qgsrubberband.h"
#include "qobjectuniqueptr.h"

class QgsRubberBand;
class QgsRenderedAnnotationItemDetails;
class QgsAnnotationItem;
class QgsAnnotationLayer;
class QgsAnnotationItemNodesSpatialIndex;
class QgsMapToolSelectAnnotationMouseHandles;

#define SIP_NO_FILE

/**
 * \ingroup gui
 * \brief An annotation item rubberband used by QgsMapToolSelectAnnotation to represent selected items.
 * \note Not available in Python bindings
 * \since QGIS 4.0
 */
class GUI_EXPORT QgsAnnotationItemRubberBand : public QgsRubberBand
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsAnnotationItemRubberBand
     */
    explicit QgsAnnotationItemRubberBand( const QString &layerId, const QString &itemId, QgsMapCanvas *canvas );
    ~QgsAnnotationItemRubberBand() override = default;

    /**
     * Returns the annotation layer ID.
     */
    QString layerId() const { return mLayerId; }

    /**
     * Returns a pointer to the annotation layer.
     */
    QgsAnnotationLayer *layer() const;

    /**
     * Returns the annotation item ID.
     */
    QString itemId() const { return mItemId; }

    /**
     * Returns a pointer to the annotation item.
     */
    QgsAnnotationItem *item() const;

    /**
     * Update the rubberband using the provided annotation item bounding box.
     */
    void updateBoundingBox( const QgsRectangle &boundingBox );

    /**
     * Returns TRUE if the bounding box requires updating on fresh annotation item rendering.
     */
    bool needsUpdatedBoundingBox() const { return mNeedsUpdatedBoundingBox; }

    /**
     * Attempts to move the annotation item.
     * \param deltaX the X-axis movement in pixel value
     * \param deltaY the Y-axis movement in pixel value
     */
    void attemptMoveBy( double deltaX, double deltaY );

    /**
     * Attempts to rotate the annotation item.
     * \param deltaDegree the rotation value in degree
     */
    void attemptRotateBy( double deltaDegree );

    /**
     * Attempts to move and resize the annotation item.
     * \param rect the rectangular area (in scene units) within which the annotation item will fit in
     */
    void attemptSetSceneRect( const QRectF &rect );

  private:
    QString mLayerId;
    QString mItemId;
    QgsRectangle mBoundingBox;
    bool mNeedsUpdatedBoundingBox = false;
};


/**
 * \ingroup gui
 * \brief A map tool for selecting, moving, and rotating annotations in a QgsAnnotationLayer
 * \note Not available in Python bindings
 * \since QGIS 4.0
 */
class GUI_EXPORT QgsMapToolSelectAnnotation : public QgsAnnotationMapTool
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsMapToolSelectAnnotation
     */
    explicit QgsMapToolSelectAnnotation( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget );
    ~QgsMapToolSelectAnnotation() override;

    void activate() override;
    void deactivate() override;
    void cadCanvasMoveEvent( QgsMapMouseEvent *event ) override;
    void cadCanvasPressEvent( QgsMapMouseEvent *event ) override;
    void cadCanvasReleaseEvent( QgsMapMouseEvent *event ) override;
    void keyPressEvent( QKeyEvent *event ) override;

    /**
     * Returns the current list of selected annotation item rubberband items.
     */
    QList<QgsAnnotationItemRubberBand *> selectedItems() const;

  signals:

    /**
     * Emitted when the selected items list has changed.
     */
    void selectedItemsChanged();

    /**
     * Emitted when the selected items list has changed and a single item is selected.
     */
    void singleItemSelected( QgsAnnotationLayer *layer, const QString &itemId );

    /**
     * Emitted when the selected items list has changed and multiple items are selected.
     */
    void multipleItemsSelected();

    /**
     * Emitted when the selected items list is cleared.
     */
    void selectionCleared();

  private slots:
    void onCanvasRefreshed();

  private:
    qsizetype annotationItemRubberBandIndexFromId( const QString &layerId, const QString &itemId );

    void setSelectedItemFromPoint( const QgsPointXY &mapPoint, bool toggleSelection = false );
    void setSelectedItemsFromRect( const QgsRectangle &mapRect, bool toggleSelection = false );
    void clearSelectedItems();
    void updateSelectedItem();

    std::vector<std::unique_ptr<QgsAnnotationItemRubberBand>> mSelectedItems;
    QList<QPair<QString, QString>> mCopiedItems;
    QgsPointXY mCopiedItemsTopLeft;

    bool mRefreshSelectedItemAfterRedraw = false;

    QObjectUniquePtr<QgsMapToolSelectAnnotationMouseHandles> mMouseHandles;
    bool mHoveringMouseHandles = false;
    QPointF mLastScenePos;

    QObjectUniquePtr<QgsRubberBand> mSelectionRubberBand;
    QRect mSelectionRect;

    bool mDragging = false;
    bool mCanceled = false;
};

#endif // QGSMAPTOOLSELECTANNOTATION_H
