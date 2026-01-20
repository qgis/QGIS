/***************************************************************************
    qgsmaptoolextraitem.h
    ---------------------
    begin                : 2026/01/22
    copyright            : (C) 2026 by Julien Cabieces
    email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLEXTRAITEM_H
#define QGSMAPTOOLEXTRAITEM_H

#include "qgsgraphicsviewmousehandles.h"
#include "qgsmapcanvas.h"
#include "qgsmaptool.h"
#include "qgsrubberband.h"
#include "qobjectuniqueptr.h"

#define SIP_NO_FILE

class QgsTemplatedLineSymbolLayerBase;
class QgsMapToolModifyExtraItems;
class QgsExtraItemRubberBand;

/**
 * \ingroup gui
 * \brief Map tool base class to implement extra item map tool
 *
 * Essentially provides helper methods to deal with extra items
 *
 * \note not available in Python bindings
 *
 * \since QGIS 4.2
 */
class GUI_EXPORT QgsMapToolExtraItemBase : public QgsMapTool
{
    Q_OBJECT

  public:
    /**
     * Constructor
     * \param canvas map canvas
     * \param layer vector layer
     * \param symbolLayer symbol layer (either a marker line or hashed line symbol layer)
     * \param extraItemsFieldIndex index of the field containing the digitized extra items
     */
    QgsMapToolExtraItemBase( QgsMapCanvas *canvas, QgsVectorLayer *layer, QgsTemplatedLineSymbolLayerBase *symbolLayer, int extraItemsFieldIndex );

    /**
     * Map tool state
     */
    enum State
    {
      SelectFeature,  //!< User needs to select a feature
      FeatureSelected //!< User has selected a feature
    };


  protected:
    /**
     * Load current feature existing extra items
     */
    void loadFeatureExtraItems();
    /**
     * Update feature attribute extra item field regarding with the currently digitized
     * extra items
     */
    void updateAttribute();

    /**
     * Process \a event in order to select the current feature at the event mouse position
     */
    void selectFeature( QgsMapMouseEvent *event );

    QPointer<QgsVectorLayer> mLayer;
    QgsTemplatedLineSymbolLayerBase *mSymbolLayer = nullptr;
    int mExtraItemsFieldIndex = -1;
    QgsFeatureId mFeatureId = FID_NULL;
    State mState = SelectFeature;
    std::vector<QObjectUniquePtr<QgsExtraItemRubberBand>> mExtraItems;

    friend class TestQgsMapToolExtraItem;
};

/**
 * \ingroup gui
 * \brief Map tool to add extra items for a giving vector layer and symbol layer.
 *
 * It creates a new extra item on mouse left click and display a rubber band for new and already existing
 * extra items
 *
 * \note not available in Python bindings
 *
 * \since QGIS 4.2
 */
class GUI_EXPORT QgsMapToolAddExtraItem : public QgsMapToolExtraItemBase
{
    Q_OBJECT

  public:
    /**
     * Constructor
     * \param canvas map canvas
     * \param layer vector layer
     * \param symbolLayer symbol layer (either a marker line or hashed line symbol layer)
     * \param extraItemsFieldIndex index of the field containing the digitized extra items
     */
    QgsMapToolAddExtraItem( QgsMapCanvas *canvas, QgsVectorLayer *layer, QgsTemplatedLineSymbolLayerBase *symbolLayer, int extraItemsFieldIndex );

    void canvasPressEvent( QgsMapMouseEvent *e ) override;
    void keyPressEvent( QKeyEvent *event ) override;

  private:
    friend class TestQgsMapToolExtraItem;
};

/**
 * \ingroup gui
 * \brief Map tool to select and modify extra items for a giving vector layer and symbol layer.
 *
 * It allows to multi select extra items, and move, rotate, delete, copy/cut, paste them.
 * It relies on QgsGraphicsViewMouseHandles to deal with those actions.
 *
 * \note not available in Python bindings
 *
 * \since QGIS 4.2
 */
class GUI_EXPORT QgsMapToolModifyExtraItems : public QgsMapToolExtraItemBase
{
    Q_OBJECT

  public:
    /**
     * Constructor
     * \param canvas map canvas
     * \param layer vector layer
     * \param symbolLayer symbol layer (either a marker line or hashed line symbol layer)
     * \param extraItemsFieldIndex index of the field containing the digitized extra items
     */
    QgsMapToolModifyExtraItems( QgsMapCanvas *canvas, QgsVectorLayer *layer, QgsTemplatedLineSymbolLayerBase *symbolLayer, int extraItemsFieldIndex );

    void activate() override;
    void deactivate() override;

    /**
     * Returns currenly selected items
     */
    QList<QGraphicsItem *> selectedItems() const;

    void canvasPressEvent( QgsMapMouseEvent *event ) override;
    void canvasMoveEvent( QgsMapMouseEvent *event ) override;
    void canvasReleaseEvent( QgsMapMouseEvent *event ) override;
    void keyPressEvent( QKeyEvent *event ) override;

    /**
     * Attempt to move \a item by \a deltaX horizontally and \a deltaY vertically.
     * \a deltaX and \a deltaY are expressed in pixels.
     */
    void attemptMoveBy( QGraphicsItem *item, double deltaX, double deltaY );

    /**
     * Attempt to rotate \a item by \a deltaDegree around its center.
     * \a deltaDegree is expressed in degree
     */
    void attemptRotateBy( QGraphicsItem *item, double deltaDegree, double deltaCenterX, double deltaCenterY );

  signals:

    /**
     * Emitted whenever selected items has changed
     */
    void selectedItemsChanged();

  private slots:

    /**
     * Called whenever canvas extent has changed
     */
    void onMapCanvasExtentsChanged();

  private:
    /**
 * \ingroup gui
 * \brief Handles drawing of extra items selection outlines and mouse handles, and
 * allows moving and rotating selected items
 *
 * Mouse events are transferred from QgsMapToolModifyExtraItem map tool to this class.
 * In returns move and rotate actions are transferred back to the maptool
 *
 * \note not available in Python bindings
 *
 * \since QGIS 4.2
 */
    class QgsMapToolModifyExtraItemMouseHandles : public QgsGraphicsViewMouseHandles
    {
      public:
        /**
         * Constructor
         * \param mapTool associated map tool called when implementing move and rotate actions
         * \param canvas map canvas
         */
        QgsMapToolModifyExtraItemMouseHandles( QgsMapToolModifyExtraItems *mapTool, QgsMapCanvas *canvas );

        void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget ) override;
        void setViewportCursor( Qt::CursorShape cursor ) override;
        QList<QGraphicsItem *> sceneItemsAtPoint( QPointF scenePoint ) override;
        QList<QGraphicsItem *> selectedSceneItems( bool includeLockedItems = true ) const override;
        QRectF itemRect( QGraphicsItem *item ) const override;
        void moveItem( QGraphicsItem *item, double deltaX, double deltaY ) override;
        void rotateItem( QGraphicsItem *item, double deltaDegree, double deltaCenterX, double deltaCenterY ) override;
        void setItemRect( QGraphicsItem *item, QRectF rect ) override;

        using QgsGraphicsViewMouseHandles::hoverMoveEvent;
        using QgsGraphicsViewMouseHandles::mouseMoveEvent;
        using QgsGraphicsViewMouseHandles::mousePressEvent;
        using QgsGraphicsViewMouseHandles::mouseReleaseEvent;
        using QgsGraphicsViewMouseHandles::updateHandles;

      private:
        QgsMapToolModifyExtraItems *mMapTool = nullptr;
        QgsMapCanvas *mCanvas = nullptr;

        friend class TestQgsMapToolExtraItem;
    };

    QList<QGraphicsItem *> mSelectedItems;
    QList<std::tuple<double, double, double>> mCopiedItems;
    QObjectUniquePtr<QgsMapToolModifyExtraItemMouseHandles> mMouseHandles;
    bool mHoveringMouseHandles = false;
    bool mCanceled = false;
    bool mDragging = false;
    QRect mSelectionRect;
    QPoint mLastPos;
    QPointF mCopiedItemsTopLeft;
    QObjectUniquePtr<QgsRubberBand> mSelectionRubberBand;

    friend class TestQgsMapToolExtraItem;
};

#endif
