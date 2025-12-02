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
#include "qgsmaptoolselectannotationmousehandles.h"
#include "qgsmaptooladvanceddigitizing.h"
#include "qobjectuniqueptr.h"
#include "qgspointxy.h"
#include "qgsannotationitemnode.h"
#include "qgsrectangle.h"
#include "qgsrubberband.h"

class QgsRubberBand;
class QgsRenderedAnnotationItemDetails;
class QgsAnnotationItem;
class QgsAnnotationLayer;
class QgsAnnotationItemNodesSpatialIndex;
class QgsMapToolSelectAnnotationMouseHandles;
class QgsSnapIndicator;

#define SIP_NO_FILE

class GUI_EXPORT QgsAnnotationItemRubberBand : public QgsRubberBand
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsAnnotationItemRubberBand
     */
    explicit QgsAnnotationItemRubberBand( const QString &layerId, const QString &itemId, QgsMapCanvas *canvas );
    ~QgsAnnotationItemRubberBand() override;

    QString layerId() const { return mLayerId; }
    QgsAnnotationLayer *layer() const;

    QString itemId() const { return mItemId; }
    QgsAnnotationItem *item() const;

    void updateBoundingBox( const QgsRectangle &boundingBox );
    bool needsUpdatedBoundingBox() const { return mNeedsUpdatedBoundingBox; }

    void attemptMoveBy( double deltaX, double deltaY );
    void attemptRotateBy( double deltaDegree );
    void attemptSetSceneRect( const QRectF &rect );

    bool operator==( const QgsAnnotationItemRubberBand &other ) const;

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
class GUI_EXPORT QgsMapToolSelectAnnotation : public QgsMapToolAdvancedDigitizing
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
    const QgsRenderedAnnotationItemDetails *findClosestItemToPoint( const QgsPointXY &mapPoint, const QList<const QgsRenderedAnnotationItemDetails *> &items, QgsRectangle &bounds );
    QgsAnnotationLayer *annotationLayerFromId( const QString &layerId );
    QgsAnnotationItem *annotationItemFromId( const QString &layerId, const QString &itemId );
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
