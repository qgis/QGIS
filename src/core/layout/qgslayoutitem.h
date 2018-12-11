/***************************************************************************
                              qgslayoutitem.h
                             -------------------
    begin                : June 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#ifndef QGSLAYOUTITEM_H
#define QGSLAYOUTITEM_H

#include "qgis_core.h"
#include "qgslayoutobject.h"
#include "qgslayoutsize.h"
#include "qgslayoutpoint.h"
#include "qgsrendercontext.h"
#include "qgslayoutundocommand.h"
#include "qgslayoutmeasurement.h"
#include <QGraphicsRectItem>
#include <QIcon>
#include <QPainter>

class QgsLayout;
class QPainter;
class QgsLayoutItemGroup;
class QgsLayoutEffect;


/**
 * \ingroup core
 * \class QgsLayoutItemRenderContext
 * Contains settings and helpers relating to a render of a QgsLayoutItem.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutItemRenderContext
{
  public:

    /**
     * Constructor for QgsLayoutItemRenderContext.
     *
     * The \a renderContext parameter specifies a QgsRenderContext for use within
     * the QgsLayoutItemRenderContext.
     *
     * The \a viewScaleFactor gives the current view zoom (scale factor). It can be
     * used to scale render graphics so that they always appear a constant size,
     * regardless of the current view zoom.
     */
    QgsLayoutItemRenderContext( QgsRenderContext &context, double viewScaleFactor = 1.0 );

    //! QgsLayoutItemRenderContext cannot be copied.
    QgsLayoutItemRenderContext( const QgsLayoutItemRenderContext &other ) = delete;

    //! QgsLayoutItemRenderContext cannot be copied.
    QgsLayoutItemRenderContext &operator=( const QgsLayoutItemRenderContext &other ) = delete;

    /**
     * Returns a reference to the context's render context.
     *
     * Note that the context's painter has been scaled so that painter units are pixels.
     * Use the QgsRenderContext methods to convert from millimeters or other units to the painter's units.
     */
    QgsRenderContext &renderContext() { return mRenderContext; }

    /**
     * Returns a reference to the context's render context.
     *
     * Note that the context's painter has been scaled so that painter units are pixels.
     * Use the QgsRenderContext methods to convert from millimeters or other units to the painter's units.
     *
     * \note Not available in Python bindings.
     */
    const QgsRenderContext &renderContext() const { return mRenderContext; } SIP_SKIP

    /**
     * Returns the current view zoom (scale factor). It can be
     * used to scale render graphics so that they always appear a constant size,
     * regardless of the current view zoom.
     *
     * E.g. a value of 0.5 indicates that the view is zoomed out to 50% size, so rendered
     * items must be scaled by 200% in order to have a constant visible size. A value
     * of 2.0 indicates that the view is zoomed in 200%, so rendered items must be
     * scaled by 50% in order to have a constant visible size.
     */
    double viewScaleFactor() const { return mViewScaleFactor; }

  private:

#ifdef SIP_RUN
    QgsLayoutItemRenderContext( const QgsLayoutItemRenderContext &rh ) SIP_FORCE;
#endif

    QgsRenderContext &mRenderContext;
    double mViewScaleFactor = 1.0;
};

/**
 * \ingroup core
 * \class QgsLayoutItem
 * \brief Base class for graphical items within a QgsLayout.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutItem : public QgsLayoutObject, public QGraphicsRectItem, public QgsLayoutUndoObjectInterface
{
#ifdef SIP_RUN
#include "qgslayoutitemgroup.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutitempicture.h"
#include "qgslayoutitemlabel.h"
#include "qgslayoutitemlegend.h"
#include "qgslayoutitempolygon.h"
#include "qgslayoutitempolyline.h"
#include "qgslayoutitemscalebar.h"
#include "qgslayoutframe.h"
#include "qgslayoutitemshape.h"
#include "qgslayoutitempage.h"
#endif

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE

    // FREAKKKKIIN IMPORTANT!!!!!!!!!!!
    // IF YOU PUT SOMETHING HERE, PUT IT IN QgsLayoutObject CASTING *****ALSO******
    // (it's not enough for it to be in only one of the places, as sip inconsistently
    // decides which casting code to perform here)

    // the conversions have to be static, because they're using multiple inheritance
    // (seen in PyQt4 .sip files for some QGraphicsItem classes)
    switch ( sipCpp->type() )
    {
      // really, these *should* use the constants from QgsLayoutItemRegistry, but sip doesn't like that!
      case QGraphicsItem::UserType + 101:
        sipType = sipType_QgsLayoutItemGroup;
        *sipCppRet = static_cast<QgsLayoutItemGroup *>( sipCpp );
        break;
      case QGraphicsItem::UserType + 102:
        sipType = sipType_QgsLayoutItemPage;
        *sipCppRet = static_cast<QgsLayoutItemPage *>( sipCpp );
        break;
      case QGraphicsItem::UserType + 103:
        sipType = sipType_QgsLayoutItemMap;
        *sipCppRet = static_cast<QgsLayoutItemMap *>( sipCpp );
        break;
      case QGraphicsItem::UserType + 104:
        sipType = sipType_QgsLayoutItemPicture;
        *sipCppRet = static_cast<QgsLayoutItemPicture *>( sipCpp );
        break;
      case QGraphicsItem::UserType + 105:
        sipType = sipType_QgsLayoutItemLabel;
        *sipCppRet = static_cast<QgsLayoutItemLabel *>( sipCpp );
        break;
      case QGraphicsItem::UserType + 106:
        sipType = sipType_QgsLayoutItemLegend;
        *sipCppRet = static_cast<QgsLayoutItemLegend *>( sipCpp );
        break;
      case QGraphicsItem::UserType + 107:
        sipType = sipType_QgsLayoutItemShape;
        *sipCppRet = static_cast<QgsLayoutItemShape *>( sipCpp );
        break;
      case QGraphicsItem::UserType + 108:
        sipType = sipType_QgsLayoutItemPolygon;
        *sipCppRet = static_cast<QgsLayoutItemPolygon *>( sipCpp );
        break;
      case QGraphicsItem::UserType + 109:
        sipType = sipType_QgsLayoutItemPolyline;
        *sipCppRet = static_cast<QgsLayoutItemPolyline *>( sipCpp );
        break;
      case QGraphicsItem::UserType + 110:
        sipType = sipType_QgsLayoutItemScaleBar;
        *sipCppRet = static_cast<QgsLayoutItemScaleBar *>( sipCpp );
        break;
      case QGraphicsItem::UserType + 111:
        sipType = sipType_QgsLayoutFrame;
        *sipCppRet = static_cast<QgsLayoutFrame *>( sipCpp );
        break;

      // did you read that comment above? NO? Go read it now. You're about to break stuff.

      default:
        sipType = NULL;
    }
    SIP_END
#endif


    Q_OBJECT
    Q_PROPERTY( bool locked READ isLocked WRITE setLocked NOTIFY lockChanged )

  public:

    //! Fixed position reference point
    enum ReferencePoint
    {
      UpperLeft, //!< Upper left corner of item
      UpperMiddle, //!< Upper center of item
      UpperRight, //!< Upper right corner of item
      MiddleLeft, //!< Middle left of item
      Middle, //!< Center of item
      MiddleRight, //!< Middle right of item
      LowerLeft, //!< Lower left corner of item
      LowerMiddle, //!< Lower center of item
      LowerRight, //!< Lower right corner of item
    };

    //! Layout item undo commands, used for collapsing undo commands
    enum UndoCommand
    {
      UndoNone = -1, //!< No command suppression
      UndoIncrementalMove = 1, //!< Layout item incremental movement, e.g. as a result of a keypress
      UndoIncrementalResize, //!< Incremental resize
      UndoStrokeColor, //!< Stroke color adjustment
      UndoStrokeWidth, //!< Stroke width adjustment
      UndoBackgroundColor, //!< Background color adjustment
      UndoOpacity, //!< Opacity adjustment
      UndoSetId, //!< Change item ID
      UndoRotation, //!< Rotation adjustment
      UndoShapeStyle, //!< Shape symbol style
      UndoShapeCornerRadius, //!< Shape corner radius
      UndoNodeMove, //!< Node move
      UndoAtlasMargin, //!< Map atlas margin changed
      UndoMapRotation, //!< Map rotation changed
      UndoZoomContent, //!< Item content zoomed
      UndoOverviewStyle, //!< Map overview style
      UndoGridFramePenColor, //!< Map grid frame pen color
      UndoMapGridFrameFill1Color, //!< Map grid frame fill color 1
      UndoMapGridFrameFill2Color, //!< Map grid frame fill color 2
      UndoMapAnnotationDistance, //!< Map frame annotation distance
      UndoMapGridAnnotationFontColor, //!< Map frame annotation color
      UndoMapGridLineSymbol, //!< Grid line symbol
      UndoMapGridMarkerSymbol, //!< Grid marker symbol
      UndoMapLabelMargin, //!< Margin for labels from edge of map
      UndoPictureRotation, //!< Picture rotation
      UndoPictureFillColor, //!< Picture fill color
      UndoPictureStrokeColor, //!< Picture stroke color
      UndoPictureStrokeWidth, //!< Picture stroke width
      UndoPictureNorthOffset, //!< Picture north offset
      UndoLabelText, //!< Label text
      UndoLabelFont, //!< Label font
      UndoLabelMargin, //!< Label margin
      UndoLabelFontColor, //!< Label color
      UndoLegendText, //!< Legend text
      UndoLegendColumnCount, //!< Legend column count
      UndoLegendSymbolWidth, //!< Legend symbol width
      UndoLegendSymbolHeight, //!< Legend symbol height
      UndoLegendWmsLegendWidth, //!< Legend WMS width
      UndoLegendWmsLegendHeight, //!< Legend WMS height
      UndoLegendTitleSpaceBottom, //!< Legend title space
      UndoLegendGroupSpace, //!< Legend group spacing
      UndoLegendLayerSpace, //!< Legend layer spacing
      UndoLegendSymbolSpace, //!< Legend symbol spacing
      UndoLegendIconSymbolSpace, //!< Legend icon symbol space
      UndoLegendFontColor, //!< Legend font color
      UndoLegendBoxSpace, //!< Legend box space
      UndoLegendColumnSpace, //!< Legend column space
      UndoLegendLineSpacing, //!< Legend line spacing
      UndoLegendRasterStrokeWidth, //!< Legend raster stroke width
      UndoLegendRasterStrokeColor, //!< Legend raster stroke color
      UndoLegendTitleFont, //!< Legend title font
      UndoLegendGroupFont, //!< Legend group font
      UndoLegendLayerFont, //!< Legend layer font
      UndoLegendItemFont, //!< Legend item font
      UndoScaleBarLineWidth, //!< Scalebar line width
      UndoScaleBarSegmentSize, //!< Scalebar segment size
      UndoScaleBarSegmentsLeft, //!< Scalebar segments left
      UndoScaleBarSegments, //!< Scalebar number of segments
      UndoScaleBarHeight, //!< Scalebar height
      UndoScaleBarFontColor, //!< Scalebar font color
      UndoScaleBarFillColor, //!< Scalebar fill color
      UndoScaleBarFillColor2, //!< Scalebar secondary fill color
      UndoScaleBarStrokeColor, //!< Scalebar stroke color
      UndoScaleBarUnitText, //!< Scalebar unit text
      UndoScaleBarMapUnitsSegment, //!< Scalebar map units per segment
      UndoScaleBarLabelBarSize, //!< Scalebar label bar size
      UndoScaleBarBoxContentSpace, //!< Scalebar box context space
      UndoArrowStrokeWidth, //!< Arrow stroke width
      UndoArrowHeadWidth, //!< Arrow head width
      UndoArrowHeadFillColor, //!< Arrow head fill color
      UndoArrowHeadStrokeColor, //!< Arrow head stroke color

      UndoCustomCommand, //!< Base id for plugin based item undo commands
    };

    /**
     * Flags for controlling how an item behaves.
     * \since QGIS 3.4.3
     */
    enum Flag
    {
      FlagOverridesPaint = 1 << 1,  //!< Item overrides the default layout item painting method
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    /**
     * Constructor for QgsLayoutItem, with the specified parent \a layout.
     *
     * If \a manageZValue is true, the z-Value of this item will be managed by the layout.
     * Generally this is the desired behavior.
     */
    explicit QgsLayoutItem( QgsLayout *layout, bool manageZValue = true );

    ~QgsLayoutItem() override;

    /**
     * Called just before a batch of items are deleted, allowing them to run cleanup
     * tasks.
     */
    virtual void cleanup();

    /**
     * Returns a unique graphics item type identifier.
     *
     * Plugin based subclasses should return an identifier greater than QgsLayoutItemRegistry::PluginItem.
     */
    int type() const override;

    /**
     * Returns the item's icon.
     */
    virtual QIcon icon() const { return QgsApplication::getThemeIcon( QStringLiteral( "/mLayoutItem.svg" ) ); }

    /**
     * Returns the item identification string. This is a unique random string set for the item
     * upon creation.
     * \note There is no corresponding setter for the uuid - it's created automatically.
     * \see id()
     * \see setId()
    */
    virtual QString uuid() const { return mUuid; }

    /**
     * Returns the item's flags, which indicate how the item behaves.
     * \since QGIS 3.4.3
     */
    virtual Flags itemFlags() const;

    /**
     * Returns the item's ID name. This is not necessarily unique, and duplicate ID names may exist
     * for a layout.
     * \see setId()
     * \see uuid()
     */
    QString id() const { return mId; }

    /**
     * Set the item's \a id name. This is not necessarily unique, and duplicate ID names may exist
     * for a layout.
     * \see id()
     * \see uuid()
     */
    virtual void setId( const QString &id );

    /**
     * Gets item display name. This is the item's id if set, and if
     * not, a user-friendly string identifying item type.
     * \see id()
     * \see setId()
     */
    virtual QString displayName() const;

    /**
     * Sets whether the item should be selected.
     */
    virtual void setSelected( bool selected );

    /**
     * Sets whether the item is \a visible.
     * \note QGraphicsItem::setVisible should not be called directly
     * on a QgsLayoutItem, as some item types (e.g., groups) need to override
     * the visibility toggle.
     */
    virtual void setVisibility( bool visible );

    /**
     * Sets whether the item is \a locked, preventing mouse interactions with the item.
     * \see isLocked()
     * \see lockChanged()
     */
    void setLocked( bool locked );

    /**
     * Returns true if the item is locked, and cannot be interacted with using the mouse.
     * \see setLocked()
     * \see lockChanged()
     */
    bool isLocked() const { return mIsLocked; }

    /**
     * Returns true if the item is part of a QgsLayoutItemGroup group.
     * \see parentGroup()
     * \see setParentGroup()
     */
    bool isGroupMember() const;

    /**
     * Returns the item's parent group, if the item is part of a QgsLayoutItemGroup group.
     * \see isGroupMember()
     * \see setParentGroup()
     */
    QgsLayoutItemGroup *parentGroup() const;

    /**
     * Sets the item's parent \a group.
     * \see isGroupMember()
     * \see parentGroup()
     */
    void setParentGroup( QgsLayoutItemGroup *group );

    /**
     * Returns the number of layers that this item requires for exporting during layered exports (e.g. SVG).
     * Returns 0 if this item is to be placed on the same layer as the previous item,
     * 1 if it should be placed on its own layer, and >1 if it requires multiple export layers.
     *
     * Items which require multiply layers should check QgsLayoutContext::currentExportLayer() during
     * their rendering to determine which layer should be drawn.
     */
    virtual int numberExportLayers() const { return 0; }

    /**
     * Handles preparing a paint surface for the layout item and painting the item's
     * content. Derived classes must not override this method, but instead implement
     * the pure virtual method QgsLayoutItem::draw.
     */
    void paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget ) override;

    /**
     * Sets the reference \a point for positioning of the layout item. This point is also
     * fixed during resizing of the item, and any size changes will be performed
     * so that the position of the reference point within the layout remains unchanged.
     * \see referencePoint()
     */
    void setReferencePoint( ReferencePoint point );

    /**
     * Returns the reference point for positioning of the layout item. This point is also
     * fixed during resizing of the item, and any size changes will be performed
     * so that the position of the reference point within the layout remains unchanged.
     * \see setReferencePoint()
     */
    ReferencePoint referencePoint() const { return mReferencePoint; }

    /**
     * Returns the fixed size of the item, if applicable, or an empty size if item can be freely
     * resized.
     * \see setFixedSize()
     * \see minimumSize()
    */
    virtual QgsLayoutSize fixedSize() const { return mFixedSize; }

    /**
     * Returns the minimum allowed size of the item, if applicable, or an empty size if item can be freely
     * resized.
     * \see setMinimumSize()
     * \see fixedSize()
    */
    virtual QgsLayoutSize minimumSize() const { return mMinimumSize; }

    /**
     * Attempts to resize the item to a specified target \a size. Note that the final size of the
     * item may not match the specified target size, as items with a fixed or minimum
     * size will place restrictions on the allowed item size. Data defined item size overrides
     * will also override the specified target size.
     *
     * If \a includesFrame is true, then the size specified by \a size includes the
     * item's frame.
     *
     * \see minimumSize()
     * \see fixedSize()
     * \see attemptMove()
     * \see sizeWithUnits()
    */
    virtual void attemptResize( const QgsLayoutSize &size, bool includesFrame = false );

    /**
     * Attempts to move the item to a specified \a point.
     *
     * If \a useReferencePoint is true, this method will respect the item's
     * reference point, in that the item will be moved so that its current reference
     * point is placed at the specified target point.
     *
     * If \a useReferencePoint is false, the item will be moved so that \a point
     * falls at the top-left corner of the item.
     *
     * If \a includesFrame is true, then the position specified by \a point represents the
     * point at which to place the outside of the item's frame.
     *
     * If \a page is not left at the default -1 value, then the position specified by \a point
     * refers to the relative position on the corresponding layout \a page (where a \a page
     * of 0 represents the first page).
     *
     * Note that the final position of the item may not match the specified target position,
     * as data defined item position may override the specified value.
     *
     * \see attemptMoveBy()
     * \see attemptResize()
     * \see referencePoint()
     * \see positionWithUnits()
    */
    virtual void attemptMove( const QgsLayoutPoint &point, bool useReferencePoint = true, bool includesFrame = false, int page = -1 );

    /**
     * Attempts to update the item's position and size to match the passed \a rect in layout
     * coordinates.
     *
     * If \a includesFrame is true, then the position and size specified by \a rect represents the
     * position and size at for the outside of the item's frame.
     *
     * Note that the final position and size of the item may not match the specified target rect,
     * as data defined item position and size may override the specified value.
     *
     * \see attemptResize()
     * \see attemptMove()
     * \see referencePoint()
     * \see positionWithUnits()
     */
    void attemptSetSceneRect( const QRectF &rect, bool includesFrame = false );

    /**
     * Attempts to shift the item's position by a specified \a deltaX and \a deltaY, in layout
     * units.
     *
     * Note that the final position of the item may not match the specified offsets,
     * as data defined item position and size may override the specified value.
     *
     * \see attemptResize()
     * \see attemptMove()
     * \see referencePoint()
     * \see positionWithUnits()
     */
    void attemptMoveBy( double deltaX, double deltaY );

    /**
     * Returns the item's current position, including units. The position returned
     * is the position of the item's reference point, which may not necessarily be the top
     * left corner of the item.
     * \see attemptMove()
     * \see referencePoint()
     * \see sizeWithUnits()
    */
    QgsLayoutPoint positionWithUnits() const { return mItemPosition; }

    /**
     * Returns the page the item is currently on, with the first page returning 0.
     * \see pagePos()
     */
    int page() const;

    /**
     * Returns the item's position (in layout units) relative to the top left corner of its current page.
     * \see page()
     * \see pagePositionWithUnits()
     */
    QPointF pagePos() const;

    /**
     * Returns the item's position (in item units) relative to the top left corner of its current page.
     * \see page()
     * \see pagePos()
     */
    QgsLayoutPoint pagePositionWithUnits() const;

    /**
     * Returns the item's current size, including units.
     * \see attemptResize()
     * \see positionWithUnits()
     */
    QgsLayoutSize sizeWithUnits() const { return mItemSize; }

    /**
     * Returns the current rotation for the item, in degrees clockwise.
     *
     * Note that this method will always return the user-set rotation for the item,
     * which may differ from the current item rotation (if data defined rotation
     * settings are present). Use QGraphicsItem::rotation() to obtain the current
     * item rotation.
     *
     * \see setItemRotation()
     */
    double itemRotation() const;

    /**
     * Stores the item state in a DOM element.
     * \param parentElement parent DOM element (e.g. 'Layout' element)
     * \param document DOM document
     * \param context read write context
     * \see readXml()
     */
    bool writeXml( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext &context ) const;

    /**
     * Sets the item state from a DOM element.
     * \param itemElement is the DOM node corresponding to item (e.g. 'LayoutItem' element)
     * \param document DOM document
     * \param context read write context
     *
     * Note that item subclasses should not rely on all other items being present in the
     * layout at the time this method is called. Instead, any connections and links to
     * other items must be made in the finalizeRestoreFromXml() method. E.g. when restoring
     * a scalebar, the connection to the linked map's signals should be implemented
     * in finalizeRestoreFromXml(), not readXml().
     *
     * \see writeXml()
     * \see finalizeRestoreFromXml()
     */
    bool readXml( const QDomElement &itemElement, const QDomDocument &document, const QgsReadWriteContext &context );

    /**
     * Called after all pending items have been restored from XML. Items can use
     * this method to run steps which must take place after all items have been restored to the layout,
     * such as connecting to signals emitted by other items, which may not have existed in the layout
     * at the time readXml() was called. E.g. a scalebar can use this to connect to its linked
     * map item after restoration from XML.
     * \see readXml()
     */
    virtual void finalizeRestoreFromXml();

    QgsAbstractLayoutUndoCommand *createCommand( const QString &text, int id, QUndoCommand *parent = nullptr ) override SIP_FACTORY;

    /**
     * Returns true if the item includes a frame.
     * \see setFrameEnabled()
     * \see frameStrokeWidth()
     * \see frameJoinStyle()
     * \see frameStrokeColor()
     */
    bool frameEnabled() const { return mFrame; }

    /**
     * Sets whether this item has a frame drawn around it or not.
     * \see frameEnabled()
     * \see setFrameStrokeWidth()
     * \see setFrameJoinStyle()
     * \see setFrameStrokeColor()
     */
    virtual void setFrameEnabled( bool drawFrame );

    /**
     * Sets the frame stroke \a color.
     * \see frameStrokeColor()
     * \see setFrameEnabled()
     * \see setFrameJoinStyle()
     * \see setFrameStrokeWidth()
     */
    void setFrameStrokeColor( const QColor &color );

    /**
     * Returns the frame's stroke color. This is only used if frameEnabled() returns true.
     * \see frameEnabled()
     * \see setFrameStrokeColor()
     * \see frameJoinStyle()
     * \see setFrameStrokeColor()
     */
    QColor frameStrokeColor() const { return mFrameColor; }

    /**
     * Sets the frame stroke \a width.
     * \see frameStrokeWidth()
     * \see setFrameEnabled()
     * \see setFrameJoinStyle()
     * \see setFrameStrokeColor()
     */
    virtual void setFrameStrokeWidth( QgsLayoutMeasurement width );

    /**
     * Returns the frame's stroke width. This is only used if frameEnabled() returns true.
     * \see frameEnabled()
     * \see setFrameStrokeWidth()
     * \see frameJoinStyle()
     * \see frameStrokeColor()
     */
    QgsLayoutMeasurement frameStrokeWidth() const { return mFrameWidth; }

    /**
     * Returns the join style used for drawing the item's frame.
     * \see frameEnabled()
     * \see setFrameJoinStyle()
     * \see frameStrokeWidth()
     * \see frameStrokeColor()
     */
    Qt::PenJoinStyle frameJoinStyle() const { return mFrameJoinStyle; }

    /**
     * Sets the join \a style used when drawing the item's frame.
     * \see setFrameEnabled()
     * \see frameJoinStyle()
     * \see setFrameStrokeWidth()
     * \see setFrameStrokeColor()
     */
    void setFrameJoinStyle( Qt::PenJoinStyle style );

    /**
     * Returns true if the item has a background.
     * \see setBackgroundEnabled()
     * \see backgroundColor()
     */
    bool hasBackground() const { return mBackground; }

    /**
     * Sets whether this item has a background drawn under it or not.
     * \see hasBackground()
     * \see setBackgroundColor()
     */
    void setBackgroundEnabled( bool drawBackground );

    /**
     * Returns the background color for this item. This is only used if hasBackground()
     * returns true.
     * \see setBackgroundColor()
     * \see hasBackground()
     */
    QColor backgroundColor() const { return mBackgroundColor; }

    /**
     * Sets the background \a color for this item.
     * \see backgroundColor()
     * \see setBackgroundEnabled()
     */
    void setBackgroundColor( const QColor &color );

    /**
     * Returns the item's composition blending mode.
     * \see setBlendMode()
     */
    QPainter::CompositionMode blendMode() const { return mBlendMode; }

    /**
     * Sets the item's composition blending \a mode.
     * \see blendMode()
     */
    void setBlendMode( QPainter::CompositionMode mode );

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
    void setItemOpacity( double opacity );

    /**
     * Returns whether the item should be excluded from layout exports and prints.
     * \see setExcludeFromExports()
     */
    bool excludeFromExports() const;

    /**
     * Sets whether the item should be excluded from layout exports and prints.
     * \see excludeFromExports()
     */
    void setExcludeFromExports( bool exclude );

    /**
     * Returns true if the item contains contents with blend modes or transparency
     * effects which can only be reproduced by rastering the item.
     *
     * Subclasses should ensure that implemented overrides of this method
     * also check the base class result.
     *
     * \see requiresRasterization()
     */
    virtual bool containsAdvancedEffects() const;

    /**
     * Returns true if the item is drawn in such a way that forces the whole layout
     * to be rasterized when exporting to vector formats.
     * \see containsAdvancedEffects()
     */
    virtual bool requiresRasterization() const;

    /**
     * Returns the estimated amount the item's frame bleeds outside the item's
     * actual rectangle. For instance, if the item has a 2mm frame stroke, then
     * 1mm of this frame is drawn outside the item's rect. In this case the
     * return value will be 1.0.
     *
     * Returned values are in layout units.

     * \see rectWithFrame()
     */
    virtual double estimatedFrameBleed() const;

    /**
     * Returns the item's rectangular bounds, including any bleed caused by the item's frame.
     * The bounds are returned in the item's coordinate system (see Qt's QGraphicsItem docs for
     * more details about QGraphicsItem coordinate systems). The results differ from Qt's rect()
     * function, as rect() makes no allowances for the portion of outlines which are drawn
     * outside of the item.
     *
     * \see estimatedFrameBleed()
     */
    virtual QRectF rectWithFrame() const;

    /**
     * Moves the content of the item, by a specified \a dx and \a dy in layout units.
     * The default implementation has no effect.
     * \see setMoveContentPreviewOffset()
     * \see zoomContent()
     */
    virtual void moveContent( double dx, double dy );

    /**
     * Sets temporary offset for the item, by a specified \a dx and \a dy in layout units.
     * This is useful for live updates when moving item content in a QgsLayoutView.
     * The default implementation has no effect.
     * \see moveContent()
     */
    virtual void setMoveContentPreviewOffset( double dx, double dy );

    /**
     * Zooms content of item. Does nothing by default.
     * \param factor zoom factor, where > 1 results in a zoom in and < 1 results in a zoom out
     * \param point item point for zoom center
     * \see moveContent()
     */
    virtual void zoomContent( double factor, QPointF point );

    /**
     * Starts new undo command for this item.
     * The \a commandText should be a capitalized, imperative tense description (e.g. "Add Map Item").
     * If specified, multiple consecutive commands for this item with the same \a command will
     * be collapsed into a single undo command in the layout history.
     * \see endCommand()
     * \see cancelCommand()
    */
    void beginCommand( const QString &commandText, UndoCommand command = UndoNone );

    /**
     * Completes the current item command and push it onto the layout's undo stack.
     * \see beginCommand()
     * \see cancelCommand()
     */
    void endCommand();

    /**
     * Cancels the current item command and discards it.
     * \see beginCommand()
     * \see endCommand()
     */
    void cancelCommand();

    /**
     * Returns whether the item should be drawn in the current context.
     */
    bool shouldDrawItem() const;

    QgsExpressionContext createExpressionContext() const override;

  public slots:

    /**
     * Refreshes the item, causing a recalculation of any property overrides and
     * recalculation of its position and size.
     */
    void refresh() override;

    /**
     * Forces a deferred update of any cached image the item uses.
     */
    virtual void invalidateCache();

    /**
     * Triggers a redraw (update) of the item.
     */
    virtual void redraw();

    /**
     * Refreshes a data defined \a property for the item by reevaluating the property's value
     * and redrawing the item with this new value. If \a property is set to
     * QgsLayoutObject::AllProperties then all data defined properties for the item will be
     * refreshed.
    */
    virtual void refreshDataDefinedProperty( QgsLayoutObject::DataDefinedProperty property = QgsLayoutObject::AllProperties );

    /**
     * Sets the layout item's \a rotation, in degrees clockwise.
     *
     * If \a adjustPosition is true, then this rotation occurs around the center of the item.
     * If \a adjustPosition is false, rotation occurs around the item origin.
     *
     * \see itemRotation()
     * \see rotateItem()
    */
    virtual void setItemRotation( double rotation, bool adjustPosition = true );

    /**
     * Rotates the item by a specified \a angle in degrees clockwise around a specified reference point.
     * \see setItemRotation()
     * \see itemRotation()
    */
    virtual void rotateItem( double angle, QPointF transformOrigin );

  signals:

    /**
     * Emitted if the item's frame style changes.
     */
    void frameChanged();

    /**
     * Emitted if the item's lock status changes.
     * \see isLocked()
     * \see setLocked()
     */
    void lockChanged();

    /**
     * Emitted on item rotation change.
     */
    void rotationChanged( double newRotation );

    /**
     * Emitted when the item's size or position changes.
     */
    void sizePositionChanged();

  protected:

    /**
     * Draws a debugging rectangle of the item's current bounds within the specified
     * painter.
     * \param painter destination QPainter
     */
    virtual void drawDebugRect( QPainter *painter );

    /**
     * Draws the item's contents using the specified item render \a context.
     *
     * Note that the context's painter has been scaled so that painter units are pixels.
     * Use the QgsRenderContext methods to convert from millimeters or other units to the painter's units.
     */
    virtual void draw( QgsLayoutItemRenderContext &context ) = 0;

    /**
     * Draws the frame around the item.
     */
    virtual void drawFrame( QgsRenderContext &context );

    /**
     * Draws the background for the item.
     */
    virtual void drawBackground( QgsRenderContext &context );

    /**
     * Sets a fixed \a size for the layout item, which prevents it from being freely
     * resized. Set an empty size if item can be freely resized.
     * \see fixedSize()
     * \see setMinimumSize()
    */
    virtual void setFixedSize( const QgsLayoutSize &size );

    /**
     * Sets the minimum allowed \a size for the layout item. Set an empty size if item can be freely
     * resized.
     * \see minimumSize()
     * \see setFixedSize()
    */
    virtual void setMinimumSize( const QgsLayoutSize &size );

    /**
     * Applies any item-specific size constraint handling to a given \a targetSize in layout units.
     * Subclasses can override this method if they need to apply advanced logic regarding item
     * sizes, which cannot be covered by setFixedSize() or setMinimumSize().
     * Item size constraints are applied after fixed, minimum and data defined size constraints.
     * \see setFixedSize()
     * \see setMinimumSize()
     */
    virtual QSizeF applyItemSizeConstraint( QSizeF targetSize );

    /**
     * Refreshes an item's size by rechecking it against any possible item fixed
     * or minimum sizes.
     * \see setFixedSize()
     * \see setMinimumSize()
     * \see refreshItemPosition()
     */
    void refreshItemSize();

    /**
     * Refreshes an item's position by rechecking it against any possible overrides
     * such as data defined positioning.
     * \see refreshItemSize()
    */
    void refreshItemPosition();

    /**
     * Refreshes an item's rotation by rechecking it against any possible overrides
     * such as data defined rotation.
     *
     * The optional \a origin point specifies the origin (in item coordinates)
     * around which the rotation should be applied.
     *
     * \see refreshItemSize()
     * \see refreshItemPosition()
     */
    void refreshItemRotation( QPointF *origin = nullptr );

    /**
     * Refresh item's opacity, considering data defined opacity.
      * If \a updateItem is set to false the item will not be automatically
      * updated after the opacity is set and a later call to update() must be made.
     */
    void refreshOpacity( bool updateItem = true );

    /**
     * Refresh item's frame, considering data defined colors and frame size.
     * If \a updateItem is set to false, the item will not be automatically updated
     * after the frame is set and a later call to update() must be made.
     */
    void refreshFrame( bool updateItem = true );

    /**
     * Refresh item's background color, considering data defined colors.
     * If \a updateItem is set to false, the item will not be automatically updated
     * after the frame color is set and a later call to update() must be made.
     */
    void refreshBackgroundColor( bool updateItem = true );

    /**
     * Refresh item's blend mode, considering data defined blend mode.
     */
    void refreshBlendMode();

    /**
     * Adjusts the specified \a point at which a \a reference position of the item
     * sits and returns the top left corner of the item, if reference point were placed at the specified position.
     */
    QPointF adjustPointForReferencePosition( QPointF point, QSizeF size, ReferencePoint reference ) const;

    /**
     * Returns the current position (in layout units) of a \a reference point for the item.
    */
    QPointF positionAtReferencePoint( ReferencePoint reference ) const;

    /**
     * Returns the position for the reference point of the item, if the top-left of the item
     * was placed at the specified \a point.
    */
    QgsLayoutPoint topLeftToReferencePoint( const QgsLayoutPoint &point ) const;

    /**
     * Stores item state within an XML DOM element.
     * \param element is the DOM element to store the item's properties in
     * \param document DOM document
     * \param context read write context
     * \see writeXml()
     * \see readPropertiesFromElement()
     */
    virtual bool writePropertiesToElement( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const;

    /**
     * Sets item state from a DOM element.
     * \param element is the DOM element for the item
     * \param document DOM document
     * \param context read write context
     *
     * Note that item subclasses should not rely on all other items being present in the
     * layout at the time this method is called. Instead, any connections and links to
     * other items must be made in the finalizeRestoreFromXml() method. E.g. when restoring
     * a scalebar, the connection to the linked map's signals should be implemented
     * in finalizeRestoreFromXml(), not readPropertiesFromElement().
     *
     * \see writePropertiesToElement()
     * \see readXml()
     */
    virtual bool readPropertiesFromElement( const QDomElement &element, const QDomDocument &document, const QgsReadWriteContext &context );

    /**
     * Applies any present data defined size overrides to the specified layout \a size.
     */
    QgsLayoutSize applyDataDefinedSize( const QgsLayoutSize &size );

  private:

    // true if layout manages the z value for this item
    bool mLayoutManagesZValue = false;

    //! id (not necessarily unique)
    QString mId;

    //! Unique id
    QString mUuid;

    //! Original uuid from template
    QString mTemplateUuid;

    //! Parent group unique id
    QString mParentGroupUuid;

    ReferencePoint mReferencePoint = UpperLeft;
    QgsLayoutSize mFixedSize;
    QgsLayoutSize mMinimumSize;

    QgsLayoutSize mItemSize;
    QgsLayoutPoint mItemPosition;
    double mItemRotation = 0.0;

    //! Whether item should be excluded in exports
    bool mExcludeFromExports = false;

    /**
     * Temporary evaluated item exclusion. Data defined properties may mean
     * this value differs from mExcludeFromExports.
     */
    bool mEvaluatedExcludeFromExports = false;

    //! Composition blend mode for item
    QPainter::CompositionMode mBlendMode = QPainter::CompositionMode_SourceOver;
    std::unique_ptr< QgsLayoutEffect > mEffect;

    //! Item opacity, between 0 and 1
    double mOpacity = 1.0;
    double mEvaluatedOpacity = 1.0;

    QImage mItemCachedImage;
    double mItemCacheDpi = -1;

    bool mIsLocked = false;

    //! True if item has a frame
    bool mFrame = false;
    //! Item frame color
    QColor mFrameColor = QColor( 0, 0, 0 );
    //! Item frame width
    QgsLayoutMeasurement mFrameWidth = QgsLayoutMeasurement( 0.3, QgsUnitTypes::LayoutMillimeters );
    //! Frame join style
    Qt::PenJoinStyle mFrameJoinStyle = Qt::MiterJoin;

    //! True if item has a background
    bool mBackground = true;
    //! Background color
    QColor mBackgroundColor = QColor( 255, 255, 255 );

    bool mBlockUndoCommands = false;

    void initConnectionsToLayout();

    //! Prepares a painter by setting rendering flags
    void preparePainter( QPainter *painter );
    bool shouldDrawAntialiased() const;
    bool shouldDrawDebugRect() const;
    QSizeF applyMinimumSize( QSizeF targetSize );
    QSizeF applyFixedSize( QSizeF targetSize );
    QgsLayoutPoint applyDataDefinedPosition( const QgsLayoutPoint &position );

    double applyDataDefinedRotation( double rotation );
    void updateStoredItemPosition();
    QPointF itemPositionAtReferencePoint( ReferencePoint reference, QSizeF size ) const;
    void setScenePos( QPointF destinationPos );
    bool shouldBlockUndoCommands() const;

    void applyDataDefinedOrientation( double &width, double &height, const QgsExpressionContext &context );

    friend class TestQgsLayoutItem;
    friend class TestQgsLayoutView;
    friend class QgsLayout;
    friend class QgsLayoutItemGroup;
    friend class QgsCompositionConverter;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsLayoutItem::Flags )

#endif //QGSLAYOUTITEM_H



