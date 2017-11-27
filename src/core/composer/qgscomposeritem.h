/***************************************************************************
                         qgscomposeritem.h
                             -------------------
    begin                : January 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCOMPOSERITEM_H
#define QGSCOMPOSERITEM_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgscomposeritemcommand.h"
#include "qgscomposerobject.h"
#include <QGraphicsRectItem>
#include <QObject>
#include <QPainter>

class QWidget;
class QDomDocument;
class QDomElement;
class QGraphicsLineItem;
class QgsComposerItemGroup;
class QgsComposition;
class QgsExpressionContext;
class QgsComposerEffect;

/**
 * \ingroup core
 * A item that forms part of a map composition.
 */
class CORE_EXPORT QgsComposerItem: public QgsComposerObject, public QGraphicsRectItem
{
#ifdef SIP_RUN
#include <qgscomposerarrow.h>
#include <qgscomposerframe.h>
#include <qgscomposeritemgroup.h>
#include <qgscomposerlabel.h>
#include <qgscomposerlegend.h>
#include <qgscomposermap.h>
#include <qgspaperitem.h>
#include <qgscomposerpicture.h>
#include <qgscomposerscalebar.h>
#include <qgscomposershape.h>
#include <qgscomposerpolygon.h>
#include <qgscomposerpolyline.h>
#include <qgscomposertexttable.h>
#include <qgslayoutitemshape.h>
#include <qgslayoutitempage.h>
#endif


#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    // the conversions have to be static, because they're using multiple inheritance
    // (seen in PyQt4 .sip files for some QGraphicsItem classes)
    if ( dynamic_cast< QgsComposerItem * >( sipCpp ) )
    {
      switch ( sipCpp->type() )
      {
        case QgsComposerItem::ComposerItem:
          sipType = sipType_QgsComposerItem;
          *sipCppRet = static_cast<QgsComposerItem *>( sipCpp );
          break;
        case QgsComposerItem::ComposerArrow:
          sipType = sipType_QgsComposerArrow;
          *sipCppRet = static_cast<QgsComposerArrow *>( sipCpp );
          break;
        case QgsComposerItem::ComposerItemGroup:
          sipType = sipType_QgsComposerItemGroup;
          *sipCppRet = static_cast<QgsComposerItemGroup *>( sipCpp );
          break;
        case QgsComposerItem::ComposerLabel:
          sipType = sipType_QgsComposerLabel;
          *sipCppRet = static_cast<QgsComposerLabel *>( sipCpp );
          break;
        case QgsComposerItem::ComposerLegend:
          sipType = sipType_QgsComposerLegend;
          *sipCppRet = static_cast<QgsComposerLegend *>( sipCpp );
          break;
        case QgsComposerItem::ComposerMap:
          sipType = sipType_QgsComposerMap;
          *sipCppRet = static_cast<QgsComposerMap *>( sipCpp );
          break;
        case QgsComposerItem::ComposerPaper:
          sipType = sipType_QgsPaperItem;
          *sipCppRet = static_cast<QgsPaperItem *>( sipCpp );
          break;
        case QgsComposerItem::ComposerPicture:
          sipType = sipType_QgsComposerPicture;
          *sipCppRet = static_cast<QgsComposerPicture *>( sipCpp );
          break;
        case QgsComposerItem::ComposerScaleBar:
          sipType = sipType_QgsComposerScaleBar;
          *sipCppRet = static_cast<QgsComposerScaleBar *>( sipCpp );
          break;
        case QgsComposerItem::ComposerShape:
          sipType = sipType_QgsComposerShape;
          *sipCppRet = static_cast<QgsComposerShape *>( sipCpp );
          break;
        case QgsComposerItem::ComposerPolygon:
          sipType = sipType_QgsComposerPolygon;
          *sipCppRet = static_cast<QgsComposerPolygon *>( sipCpp );
          break;
        case QgsComposerItem::ComposerPolyline:
          sipType = sipType_QgsComposerPolyline;
          *sipCppRet = static_cast<QgsComposerPolyline *>( sipCpp );
          break;
        case QgsComposerItem::ComposerFrame:
          sipType = sipType_QgsComposerFrame;
          *sipCppRet = static_cast<QgsComposerFrame *>( sipCpp );
          break;
        default:
          sipType = 0;
      }
    }
    else
    {
      switch ( sipCpp->type() )
      {
        // really, these *should* use the constants from QgsLayoutItemRegistry, but sip doesn't like that!
        case QGraphicsItem::UserType + 101:
          sipType = sipType_QgsLayoutItemPage;
          *sipCppRet = static_cast<QgsLayoutItemPage *>( sipCpp );
          break;
        default:
          sipType = 0;
      }
    }

    SIP_END
#endif


    Q_OBJECT
  public:

    enum ItemType
    {
      // base class for the items
      ComposerItem = UserType + 100,

      // derived classes
      ComposerArrow,
      ComposerItemGroup,
      ComposerLabel,
      ComposerLegend,
      ComposerMap,
      ComposerPaper, // QgsPaperItem
      ComposerPicture,
      ComposerScaleBar,
      ComposerShape,
      ComposerPolygon,
      ComposerPolyline,
      ComposerTable,
      ComposerAttributeTable,
      ComposerTextTable,
      ComposerFrame
    };

    //! Describes the action (move or resize in different directon) to be done during mouse move
    enum MouseMoveAction
    {
      MoveItem,
      ResizeUp,
      ResizeDown,
      ResizeLeft,
      ResizeRight,
      ResizeLeftUp,
      ResizeRightUp,
      ResizeLeftDown,
      ResizeRightDown,
      NoAction
    };

    enum ItemPositionMode
    {
      UpperLeft,
      UpperMiddle,
      UpperRight,
      MiddleLeft,
      Middle,
      MiddleRight,
      LowerLeft,
      LowerMiddle,
      LowerRight
    };

    //note - must sync with QgsMapCanvas::WheelAction.
    //TODO - QGIS 3.0 move QgsMapCanvas::WheelAction from GUI->CORE and remove this enum

    /**
     * Modes for zooming item content
     */
    enum ZoomMode
    {
      Zoom = 0, //!< Zoom to center of content
      ZoomRecenter, //!< Zoom and recenter content to point
      ZoomToPoint, //!< Zoom while maintaining relative position of point
      NoZoom //!< No zoom
    };

    /**
     * Constructor
     \param composition parent composition
     \param manageZValue true if the z-Value of this object should be managed by mComposition*/
    QgsComposerItem( QgsComposition *composition SIP_TRANSFERTHIS, bool manageZValue = true );

    /**
     * Constructor with box position and composer object
     \param x x coordinate of item
     \param y y coordinate of item
     \param width width of item
     \param height height of item
     \param composition parent composition
     \param manageZValue true if the z-Value of this object should be managed by mComposition*/
    QgsComposerItem( qreal x, qreal y, qreal width, qreal height, QgsComposition *composition SIP_TRANSFERTHIS, bool manageZValue = true );
    virtual ~QgsComposerItem();

    //! Return correct graphics item type.
    virtual int type() const override { return ComposerItem; }

    /**
     * Returns whether this item has been removed from the composition. Items removed
     * from the composition are not deleted so that they can be restored via an undo
     * command.
     * \returns true if the item has been removed from the composition
     * \since QGIS 2.5
     * \see setIsRemoved
     */
    virtual bool isRemoved() const { return mRemovedFromComposition; }

    /**
     * Sets whether this item has been removed from the composition. Items removed
     * from the composition are not deleted so that they can be restored via an undo
     * command.
     * \param removed set to true if the item has been removed from the composition
     * \since QGIS 2.5
     * \see isRemoved
     */
    void setIsRemoved( const bool removed ) { mRemovedFromComposition = removed; }

    //! \brief Set selected, selected item should be highlighted
    virtual void setSelected( bool s );

    //! \brief Is selected
    virtual bool selected() const { return QGraphicsRectItem::isSelected(); }

    //! Moves item in canvas coordinates
    void move( double dx, double dy );

    /**
     * Move Content of item. Does nothing per default (but implemented in composer map)
       \param dx move in x-direction (canvas coordinates)
       \param dy move in y-direction(canvas coordinates)*/
    virtual void moveContent( double dx, double dy ) { Q_UNUSED( dx ); Q_UNUSED( dy ); }

    /**
     * Zoom content of item. Does nothing per default (but implemented in composer map)
     * \param factor zoom factor, where > 1 results in a zoom in and < 1 results in a zoom out
     * \param point item point for zoom center
     * \param mode zoom mode
     * \since QGIS 2.5
     */
    virtual void zoomContent( const double factor, const QPointF point, const ZoomMode mode = QgsComposerItem::Zoom ) { Q_UNUSED( factor ); Q_UNUSED( point ); Q_UNUSED( mode ); }

    /**
     * Gets the page the item is currently on.
     * \returns page number for item, beginning on page 1
     * \see pagePos
     * \see updatePagePos
     * \since QGIS 2.4
     */
    int page() const;

    /**
     * Returns the item's position relative to its current page.
     * \returns position relative to the page's top left corner.
     * \see page
     * \see updatePagePos
     * \since QGIS 2.4
     */
    QPointF pagePos() const;

    /**
     * Moves the item so that it retains its relative position on the page
     * when the paper size changes.
     * \param newPageWidth new width of the page in mm
     * \param newPageHeight new height of the page in mm
     * \see page
     * \see pagePos
     * \since QGIS 2.4
     */
    void updatePagePos( double newPageWidth, double newPageHeight );

    /**
     * Moves the item to a new position (in canvas coordinates)
      \param x item position x (mm)
      \param y item position y (mm)
      \param itemPoint reference point which coincides with specified position
      \param page if page > 0, y is interpreted as relative to the origin of the specified page, if page <= 0, y is in absolute canvas coordinates.
       a page number of 1 corresponds to the first page.
      */
    void setItemPosition( double x, double y, ItemPositionMode itemPoint = UpperLeft, int page = -1 );

    /**
     * Sets item position and width / height in one go
      \param x item position x (mm)
      \param y item position y (mm)
      \param width item width (mm)
      \param height item height (mm)
      \param itemPoint reference point which coincides with specified position
      \param posIncludesFrame set to true if the position and size arguments include the item's frame stroke
      \param page if page > 0, y is interpreted as relative to the origin of the specified page, if page <= 0, y is in absolute canvas coordinates.
       a page number of 1 corresponds to the first page.
      */
    void setItemPosition( double x, double y, double width, double height, ItemPositionMode itemPoint = UpperLeft, bool posIncludesFrame = false, int page = -1 );

    /**
     * Returns item's last used position mode.
      \note: This property has no effect on actual's item position, which is always the top-left corner. */
    ItemPositionMode lastUsedPositionMode() { return mLastUsedPositionMode; }

    /**
     * Sets this items bound in scene coordinates such that 1 item size units
     corresponds to 1 scene size unit*/
    virtual void setSceneRect( const QRectF &rectangle );

    //! Writes parameter that are not subclass specific in document. Usually called from writeXml methods of subclasses
    bool _writeXml( QDomElement &itemElem, QDomDocument &doc ) const;

    //! Reads parameter that are not subclass specific in document. Usually called from readXml methods of subclasses
    bool _readXml( const QDomElement &itemElem, const QDomDocument &doc );

    /**
     * Whether this item has a frame or not.
     * \returns true if there is a frame around this item, otherwise false.
     * \see setFrameEnabled
     * \see frameStrokeWidth
     * \see frameJoinStyle
     * \see frameStrokeColor
     */
    bool hasFrame() const {return mFrame;}

    /**
     * Set whether this item has a frame drawn around it or not.
     * \param drawFrame draw frame
     * \see hasFrame
     * \see setFrameStrokeWidth
     * \see setFrameJoinStyle
     * \see setFrameStrokeColor
     */
    virtual void setFrameEnabled( const bool drawFrame );

    /**
     * Sets frame stroke color
     * \param color new color for stroke frame
     * \since QGIS 2.6
     * \see frameStrokeColor
     * \see setFrameEnabled
     * \see setFrameJoinStyle
     * \see setFrameStrokeWidth
     */
    virtual void setFrameStrokeColor( const QColor &color );

    /**
     * Returns the frame's stroke color. Only used if hasFrame is true.
     * \returns frame stroke color
     * \since QGIS 2.6
     * \see hasFrame
     * \see setFrameStrokeColor
     * \see frameJoinStyle
     * \see setFrameStrokeColor
     */
    QColor frameStrokeColor() const { return mFrameColor; }

    /**
     * Sets frame stroke width
     * \param strokeWidth new width for stroke frame
     * \since QGIS 2.2
     * \see frameStrokeWidth
     * \see setFrameEnabled
     * \see setFrameJoinStyle
     * \see setFrameStrokeColor
     */
    virtual void setFrameStrokeWidth( const double strokeWidth );

    /**
     * Returns the frame's stroke width. Only used if hasFrame is true.
     * \returns Frame stroke width
     * \since QGIS 2.3
     * \see hasFrame
     * \see setFrameStrokeWidth
     * \see frameJoinStyle
     * \see frameStrokeColor
     */
    double frameStrokeWidth() const { return mFrameWidth; }

    /**
     * Returns the join style used for drawing the item's frame
     * \returns Join style for stroke frame
     * \since QGIS 2.3
     * \see hasFrame
     * \see setFrameJoinStyle
     * \see frameStrokeWidth
     * \see frameStrokeColor
     */
    Qt::PenJoinStyle frameJoinStyle() const { return mFrameJoinStyle; }

    /**
     * Sets join style used when drawing the item's frame
     * \param style Join style for stroke frame
     * \since QGIS 2.3
     * \see setFrameEnabled
     * \see frameJoinStyle
     * \see setFrameStrokeWidth
     * \see setFrameStrokeColor
     */
    void setFrameJoinStyle( const Qt::PenJoinStyle style );

    /**
     * Returns the estimated amount the item's frame bleeds outside the item's
     * actual rectangle. For instance, if the item has a 2mm frame stroke, then
     * 1mm of this frame is drawn outside the item's rect. In this case the
     * return value will be 1.0
     * \since QGIS 2.2
     * \see rectWithFrame
     */
    virtual double estimatedFrameBleed() const;

    /**
     * Returns the item's rectangular bounds, including any bleed caused by the item's frame.
     *  The bounds are returned in the item's coordinate system (see Qt's QGraphicsItem docs for
     *  more details about QGraphicsItem coordinate systems). The results differ from Qt's rect()
     *  function, as rect() makes no allowances for the portion of outlines which are drawn
     *  outside of the item.
     * \since QGIS 2.2
     * \see estimatedFrameBleed
     */
    virtual QRectF rectWithFrame() const;

    /**
     * Whether this item has a Background or not.
     * \returns true if there is a Background around this item, otherwise false.
     * \see setBackgroundEnabled
     * \see backgroundColor
     */
    bool hasBackground() const {return mBackground;}

    /**
     * Set whether this item has a Background drawn around it or not.
     * \param drawBackground draw Background
     * \returns nothing
     * \see hasBackground
     * \see setBackgroundColor
     */
    void setBackgroundEnabled( const bool drawBackground ) { mBackground = drawBackground; }

    /**
     * Gets the background color for this item
     * \returns background color
     * \see setBackgroundColor
     * \see hasBackground
     */
    QColor backgroundColor() const { return mBackgroundColor; }

    /**
     * Sets the background color for this item
     * \param backgroundColor new background color
     * \returns nothing
     * \see backgroundColor
     * \see setBackgroundEnabled
     */
    void setBackgroundColor( const QColor &backgroundColor );

    /**
     * Returns the item's composition blending mode.
     * \returns item blending mode
     * \see setBlendMode
     */
    QPainter::CompositionMode blendMode() const { return mBlendMode; }

    /**
     * Sets the item's composition blending mode
     * \param blendMode blending mode for item
     * \see blendMode
     */
    void setBlendMode( const QPainter::CompositionMode blendMode );

    /**
     * Returns the item's opacity. This method should be used instead of
     * QGraphicsItem::opacity() as any data defined overrides will be
     * respected.
     * \returns opacity as double between 1.0 (opaque) and 0 (transparent).
     * \see setItemOpacity()
     */
    double itemOpacity() const { return mOpacity; }

    /**
     * Sets the item's \a opacity. This method should be used instead of
     * QGraphicsItem::setOpacity() as any data defined overrides will be
     * respected.
     * \param opacity double between 1.0 (opaque) and 0 (transparent).
     * \see itemOpacity()
     */
    void setItemOpacity( const double opacity );

    /**
     * Returns whether effects (e.g., blend modes) are enabled for the item
     * \returns true if effects are enabled
     * \see setEffectsEnabled
     * \see itemOpacity()
     * \see blendMode
     */
    bool effectsEnabled() const { return mEffectsEnabled; }

    /**
     * Sets whether effects (e.g., blend modes) are enabled for the item
     * \param effectsEnabled set to true to enable effects
     * \see effectsEnabled
     * \see setItemOpacity()
     * \see setBlendMode
     */
    void setEffectsEnabled( const bool effectsEnabled );

    //! Composite operations for item groups do nothing per default
    virtual void addItem( QgsComposerItem *item ) { Q_UNUSED( item ); }
    virtual void removeItems() {}

    virtual void beginItemCommand( const QString &text ) { beginCommand( text ); }

    /**
     * Starts new composer undo command
      \param commandText command title
      \param c context for mergeable commands (unknown for non-mergeable commands*/
    void beginCommand( const QString &commandText, QgsComposerMergeCommand::Context c = QgsComposerMergeCommand::Unknown );

    virtual void endItemCommand() { endCommand(); }
    //! Finish current command and push it onto the undo stack
    void endCommand();
    void cancelCommand();

    //functions that encapsulate the workaround for the Qt font bug (that is to scale the font size up and then scale the
    //painter down by the same factor for drawing

    /**
     * Locks / unlocks the item position for mouse drags
     * \param lock set to true to prevent item movement and resizing via the mouse
     * \see positionLock
     */
    void setPositionLock( const bool lock );

    /**
     * Returns whether position lock for mouse drags is enabled
     * returns true if item is locked for mouse movement and resizing
     * \see setPositionLock
     */
    bool positionLock() const { return mItemPositionLocked; }

    /**
     * Returns the current rotation for the composer item, in degrees clockwise.
     * \param valueType controls whether the returned value is the user specified rotation,
     * or the current evaluated rotation (which may be affected by data driven rotation
     * settings).
     * \since QGIS 2.1
     * \see setItemRotation()
     */
    double itemRotation( const QgsComposerObject::PropertyValueType valueType = QgsComposerObject::EvaluatedValue ) const;

    /**
     * Updates (redraws) the item, with the possibility to do custom update for subclasses.
     * Subclasses should check updatesEnabled() to determine whether updates are
     * currently permitted for the item.
     */
    virtual void updateItem();

    /**
     * Get item's id (which is not necessarly unique)
     * \returns item id
     * \see setId
     */
    QString id() const { return mId; }

    /**
     * Set item's id (which is not necessarly unique)
     * \param id new id for item
     * \see id
     */
    virtual void setId( const QString &id );

    /**
     * Get item identification name
     * \returns unique item identification string
     * \note there is not setter since one can't manually set the id
     * \see id
     * \see setId
     */
    QString uuid() const { return mUuid; }

    /**
     * Get item display name. This is the item's id if set, and if
     * not, a user-friendly string identifying item type.
     * \returns display name for item
     * \see id
     * \see setId
     * \since QGIS 2.5
     */
    virtual QString displayName() const;

    /**
     * Sets visibility for item.
     * \param visible set to true to show item, false to hide item
     * \note QGraphicsItem::setVisible should not be called directly
     * on a QgsComposerItem, as some item types (e.g., groups) need to override
     * the visibility toggle.
     * \since QGIS 2.5
     */
    virtual void setVisibility( const bool visible );

    /**
     * Returns whether the item should be excluded from composer exports and prints
     * \param valueType controls whether the returned value is the user specified value,
     * or the current evaluated value (which may be affected by data driven settings).
     * \returns true if item should be excluded
     * \since QGIS 2.5
     * \see setExcludeFromExports
     */
    bool excludeFromExports( const QgsComposerObject::PropertyValueType valueType = QgsComposerObject::EvaluatedValue );

    /**
     * Sets whether the item should be excluded from composer exports and prints
     * \param exclude set to true to exclude the item from exports
     * \since QGIS 2.5
     * \see excludeFromExports
     */
    virtual void setExcludeFromExports( const bool exclude );

    /**
     * Returns whether this item is part of a group
     * \returns true if item is in a group
     * \since QGIS 2.5
     * \see setIsGroupMember
     */
    bool isGroupMember() const { return mIsGroupMember; }

    /**
     * Sets whether this item is part of a group
     * \param isGroupMember set to true if item is in a group
     * \since QGIS 2.5
     * \see isGroupMember
     */
    void setIsGroupMember( const bool isGroupMember );

    /**
     * Get the number of layers that this item requires for exporting as layers
     * \returns 0 if this item is to be placed on the same layer as the previous item,
     * 1 if it should be placed on its own layer, and >1 if it requires multiple export layers
     * \since QGIS 2.4
     * \see setCurrentExportLayer
     */
    virtual int numberExportLayers() const { return 0; }

    /**
     * Sets the current layer to draw for exporting
     * \param layerIdx can be set to -1 to draw all item layers, and must be less than numberExportLayers()
     * \since QGIS 2.4
     * \see numberExportLayers
     */
    virtual void setCurrentExportLayer( const int layerIdx = -1 ) { mCurrentExportLayer = layerIdx; }

    /**
     * Creates an expression context relating to the item's current state. The context includes
     * scopes for global, project, composition, atlas and item properties.
     * \since QGIS 2.12
     */
    virtual QgsExpressionContext createExpressionContext() const override;

    /**
     * Sets whether updates to the item are enabled. If false,
     * the item will not be redrawn. This can be used to prevent
     * multiple item updates when many settings for an item are
     * changed sequentially.
     * \since QGIS 3.0
     * \see updatesEnabled()
     */
    void setUpdatesEnabled( bool enabled ) { mUpdatesEnabled = enabled; }

    /**
     * Returns whether updates to the item are enabled. If false,
     * the item will not be redrawn. This can be used to prevent
     * multiple item updates when many settings for an item are
     * changed sequentially.
     * \since QGIS 3.0
     * \see setUpdatesEnabled()
     */
    bool updatesEnabled() const { return mUpdatesEnabled; }

  public slots:

    /**
     * Sets the item \a rotation, in degrees clockwise.
     * \param rotation item rotation in degrees
     * \param adjustPosition set to true if item should be shifted so that rotation occurs
     * around item center. If false, rotation occurs around item origin
     * \since QGIS 2.1
     * \see itemRotation
     */
    virtual void setItemRotation( const double rotation, const bool adjustPosition = false );

    void repaint() override;

    /**
     * Refreshes a data defined property for the item by reevaluating the property's value
     * and redrawing the item with this new value.
     * \param property data defined property to refresh. If property is set to
     * QgsComposerItem::AllProperties then all data defined properties for the item will be
     * refreshed.
     * \param context expression context for evaluating data defined expressions
     * \since QGIS 2.5
     */
    virtual void refreshDataDefinedProperty( const QgsComposerObject::DataDefinedProperty property = QgsComposerObject::AllProperties, const QgsExpressionContext *context = nullptr ) override;

  protected:
    //! True if item has been removed from the composition
    bool mRemovedFromComposition;

    QgsComposerItem::MouseMoveAction mCurrentMouseMoveAction;
    //! Start point of the last mouse move action (in scene coordinates)
    QPointF mMouseMoveStartPos;
    //! Position of the last mouse move event (in scene coordinates)
    QPointF mLastMouseEventPos;

    //! Rectangle used during move and resize actions
    QGraphicsRectItem *mBoundingResizeRectangle = nullptr;
    QGraphicsLineItem *mHAlignSnapItem = nullptr;
    QGraphicsLineItem *mVAlignSnapItem = nullptr;

    //! True if item fram needs to be painted
    bool mFrame;
    //! Item frame color
    QColor mFrameColor;
    //! Item frame width
    double mFrameWidth = 0.3;
    //! Frame join style
    Qt::PenJoinStyle mFrameJoinStyle = Qt::MiterJoin;

    //! True if item background needs to be painted
    bool mBackground;
    //! Background color
    QColor mBackgroundColor;

    /**
     * True if item position  and size cannot be changed with mouse move
     */
    bool mItemPositionLocked;

    //! Backup to restore item appearance if no view scale factor is available
    mutable double mLastValidViewScaleFactor;

    //! Item rotation in degrees, clockwise
    double mItemRotation;

    /**
     * Temporary evaluated item rotation in degrees, clockwise. Data defined rotation may mean
     * this value differs from mItemRotation.
     */
    double mEvaluatedItemRotation;

    //! Composition blend mode for item
    QPainter::CompositionMode mBlendMode;
    bool mEffectsEnabled;
    QgsComposerEffect *mEffect = nullptr;

    //! Item opacity, between 0 and 1
    double mOpacity = 1.0;

    //! Whether item should be excluded in exports
    bool mExcludeFromExports;

    /**
     * Temporary evaluated item exclusion. Data defined properties may mean
     * this value differs from mExcludeFromExports.
     */
    bool mEvaluatedExcludeFromExports;

    //! The item's position mode
    ItemPositionMode mLastUsedPositionMode;

    //! Whether or not this item is part of a group
    bool mIsGroupMember;

    /**
     * The layer that needs to be exported
     * \note: if -1, all layers are to be exported
     * \since QGIS 2.4
     */
    int mCurrentExportLayer;

    /**
     * Draws additional graphics on selected items. The base implementation has
     * no effect.
     */
    virtual void drawSelectionBoxes( QPainter *p );

    //! Draw black frame around item
    virtual void drawFrame( QPainter *p );

    //! Draw background
    virtual void drawBackground( QPainter *p );

    /**
     * Returns the current (zoom level dependent) tolerance to decide if mouse position is close enough to the
    item border for resizing*/
    double rectHandlerBorderTolerance() const;

    /**
     * Returns the zoom factor of the graphics view.
     * \returns the factor or -1 in case of error (e.g. graphic view does not exist)
     */
    double horizontalViewScaleFactor() const;

    //some utility functions

    //! Return horizontal align snap item. Creates a new graphics line if 0
    QGraphicsLineItem *hAlignSnapItem();
    void deleteHAlignSnapItem();
    //! Return vertical align snap item. Creates a new graphics line if 0
    QGraphicsLineItem *vAlignSnapItem();
    void deleteVAlignSnapItem();
    void deleteAlignItems();

    /**
     * Evaluates an item's bounding rect to consider data defined position and size of item
     * and reference point
     * \param newRect target bounding rect for item
     * \param resizeOnly set to true if the item is only being resized. If true then
     * the position of the returned rect will be adjusted to account for the item's
     * position mode
     * \param context expression context for evaluating data defined expressions
     * \returns bounding box rectangle for item after data defined size and position have been
     * set and position mode has been accounted for
     * \since QGIS 2.5
     */
    QRectF evalItemRect( const QRectF &newRect, const bool resizeOnly = false, const QgsExpressionContext *context = nullptr );

    /**
     * Returns whether the item should be drawn in the current context
     * \returns true if item should be drawn
     * \since QGIS 2.5
     */
    bool shouldDrawItem() const;

  signals:
    //! Is emitted on item rotation change
    void itemRotationChanged( double newRotation );
    //! Emitted if the rectangle changes
    void sizeChanged();

    /**
     * Emitted if the item's frame style changes
     * \since QGIS 2.2
     */
    void frameChanged();

    /**
     * Emitted if the item's lock status changes
     * \since QGIS 2.5
     */
    void lockChanged();

  private:
    // id (not unique)
    QString mId;
    // name (unique)
    QString mUuid;
    // name (temporary when loaded from template)
    QString mTemplateUuid;
    // true if composition manages the z value for this item
    bool mCompositionManagesZValue;

    /**
     * Whether updates to the item are enabled. If false,
     * the item should not be redrawn.
     */
    bool mUpdatesEnabled = true;

    /**
     * Refresh item's rotation, considering data defined rotation setting
      *\param updateItem set to false to prevent the item being automatically updated
      *\param rotateAroundCenter set to true to rotate the item around its center rather
      * than its origin
      * \param context expression context for evaulating data defined rotation
      * \since QGIS 2.5
     */
    void refreshRotation( const bool updateItem = true, const bool rotateAroundCenter = false, const QgsExpressionContext &context = QgsExpressionContext() );

    /**
     * Refresh item's opacity, considering data defined opacity
      * \param updateItem set to false to prevent the item being automatically updated
      * after the opacity is set
      * \param context expression context for evaulating data defined opacity
      * \since QGIS 2.5
     */
    void refreshOpacity( const bool updateItem = true, const QgsExpressionContext &context = QgsExpressionContext() );

    /**
     * Refresh item's frame color, considering data defined colors
      * \param updateItem set to false to prevent the item being automatically updated
      * after the frame color is set
      * \param context expression context for evaulating data defined color
     */
    void refreshFrameColor( const bool updateItem = true, const QgsExpressionContext &context = QgsExpressionContext() );

    /**
     * Refresh item's background color, considering data defined colors
      * \param updateItem set to false to prevent the item being automatically updated
      * after the background color is set
      * \param context expression context for evaulating data defined color
     */
    void refreshBackgroundColor( const bool updateItem = true, const QgsExpressionContext &context = QgsExpressionContext() );

    /**
     * Refresh item's blend mode, considering data defined blend mode
     * \since QGIS 2.5
     */
    void refreshBlendMode( const QgsExpressionContext &context );

    void init( const bool manageZValue );

    friend class QgsComposerItemGroup; // to access mTemplateUuid
};

#endif
