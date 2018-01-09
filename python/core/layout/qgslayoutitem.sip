/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitem.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsLayoutItem : QgsLayoutObject, QGraphicsRectItem, QgsLayoutUndoObjectInterface
{
%Docstring
 Base class for graphical items within a QgsLayout.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutitem.h"
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
%End
%ConvertToSubClassCode
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
      default:
        sipType = 0;
    }
%End
  public:

    enum ReferencePoint
    {
      UpperLeft,
      UpperMiddle,
      UpperRight,
      MiddleLeft,
      Middle,
      MiddleRight,
      LowerLeft,
      LowerMiddle,
      LowerRight,
    };

    enum UndoCommand
    {
      UndoNone,
      UndoIncrementalMove,
      UndoIncrementalResize,
      UndoStrokeColor,
      UndoStrokeWidth,
      UndoBackgroundColor,
      UndoOpacity,
      UndoSetId,
      UndoRotation,
      UndoShapeStyle,
      UndoShapeCornerRadius,
      UndoNodeMove,
      UndoAtlasMargin,
      UndoMapRotation,
      UndoZoomContent,
      UndoOverviewStyle,
      UndoGridFramePenColor,
      UndoMapGridFrameFill1Color,
      UndoMapGridFrameFill2Color,
      UndoMapAnnotationDistance,
      UndoMapGridAnnotationFontColor,
      UndoMapGridLineSymbol,
      UndoMapGridMarkerSymbol,
      UndoPictureRotation,
      UndoPictureFillColor,
      UndoPictureStrokeColor,
      UndoPictureStrokeWidth,
      UndoPictureNorthOffset,
      UndoLabelText,
      UndoLabelFont,
      UndoLabelMargin,
      UndoLabelFontColor,
      UndoLegendText,
      UndoLegendColumnCount,
      UndoLegendSymbolWidth,
      UndoLegendSymbolHeight,
      UndoLegendWmsLegendWidth,
      UndoLegendWmsLegendHeight,
      UndoLegendTitleSpaceBottom,
      UndoLegendGroupSpace,
      UndoLegendLayerSpace,
      UndoLegendSymbolSpace,
      UndoLegendIconSymbolSpace,
      UndoLegendFontColor,
      UndoLegendBoxSpace,
      UndoLegendColumnSpace,
      UndoLegendLineSpacing,
      UndoLegendRasterStrokeWidth,
      UndoLegendRasterStrokeColor,
      UndoLegendTitleFont,
      UndoLegendGroupFont,
      UndoLegendLayerFont,
      UndoLegendItemFont,
      UndoScaleBarLineWidth,
      UndoScaleBarSegmentSize,
      UndoScaleBarSegmentsLeft,
      UndoScaleBarSegments,
      UndoScaleBarHeight,
      UndoScaleBarFontColor,
      UndoScaleBarFillColor,
      UndoScaleBarFillColor2,
      UndoScaleBarStrokeColor,
      UndoScaleBarUnitText,
      UndoScaleBarMapUnitsSegment,
      UndoScaleBarLabelBarSize,
      UndoScaleBarBoxContentSpace,
      UndoArrowStrokeWidth,
      UndoArrowHeadWidth,
      UndoArrowHeadFillColor,
      UndoArrowHeadStrokeColor,

      UndoCustomCommand,
    };

    explicit QgsLayoutItem( QgsLayout *layout, bool manageZValue = true );
%Docstring
Constructor for QgsLayoutItem, with the specified parent ``layout``.

If ``manageZValue`` is true, the z-Value of this item will be managed by the layout.
Generally this is the desired behavior.
%End

    ~QgsLayoutItem();

    virtual void cleanup();
%Docstring
Called just before a batch of items are deleted, allowing them to run cleanup
tasks.
%End

    virtual int type() const;

%Docstring
Returns a unique graphics item type identifier.

Plugin based subclasses should return an identifier greater than QgsLayoutItemRegistry.PluginItem.
%End

    virtual QIcon icon() const;
%Docstring
Returns the item's icon.
%End

    virtual QString uuid() const;
%Docstring
Returns the item identification string. This is a unique random string set for the item
upon creation.

.. note::

   There is no corresponding setter for the uuid - it's created automatically.

.. seealso:: :py:func:`id`

.. seealso:: :py:func:`setId`

.. seealso:: :py:func:`templateUuid`
%End

    QString templateUuid() const;
%Docstring
Returns the item's original identification string. This may differ from the item's uuid()
for items which have been added to an existing layout from a template. In this case
templateUuid() returns the original item UUID at the time the template was created,
while uuid() returns the current instance of the item's unique identifier.

.. seealso:: :py:func:`uuid`
%End

    QString id() const;
%Docstring
Returns the item's ID name. This is not necessarily unique, and duplicate ID names may exist
for a layout.

.. seealso:: :py:func:`setId`

.. seealso:: :py:func:`uuid`
%End

    virtual void setId( const QString &id );
%Docstring
Set the item's ``id`` name. This is not necessarily unique, and duplicate ID names may exist
for a layout.

.. seealso:: :py:func:`id`

.. seealso:: :py:func:`uuid`
%End

    virtual QString displayName() const;
%Docstring
Get item display name. This is the item's id if set, and if
not, a user-friendly string identifying item type.

.. seealso:: :py:func:`id`

.. seealso:: :py:func:`setId`
%End

    virtual void setSelected( bool selected );
%Docstring
Sets whether the item should be selected.
%End

    virtual void setVisibility( const bool visible );
%Docstring
Sets whether the item is ``visible``.

.. note::

   QGraphicsItem.setVisible should not be called directly
on a QgsLayoutItem, as some item types (e.g., groups) need to override
the visibility toggle.
%End

    void setLocked( const bool locked );
%Docstring
Sets whether the item is ``locked``, preventing mouse interactions with the item.

.. seealso:: :py:func:`isLocked`

.. seealso:: :py:func:`lockChanged`
%End

    bool isLocked() const;
%Docstring
Returns true if the item is locked, and cannot be interacted with using the mouse.

.. seealso:: :py:func:`setLocked`

.. seealso:: :py:func:`lockChanged`
%End

    bool isGroupMember() const;
%Docstring
Returns true if the item is part of a QgsLayoutItemGroup group.

.. seealso:: :py:func:`parentGroup`

.. seealso:: :py:func:`setParentGroup`
%End

    QgsLayoutItemGroup *parentGroup() const;
%Docstring
Returns the item's parent group, if the item is part of a QgsLayoutItemGroup group.

.. seealso:: :py:func:`isGroupMember`

.. seealso:: :py:func:`setParentGroup`
%End

    void setParentGroup( QgsLayoutItemGroup *group );
%Docstring
Sets the item's parent ``group``.

.. seealso:: :py:func:`isGroupMember`

.. seealso:: :py:func:`parentGroup`
%End

    virtual int numberExportLayers() const;
%Docstring
Returns the number of layers that this item requires for exporting during layered exports (e.g. SVG).
Returns 0 if this item is to be placed on the same layer as the previous item,
1 if it should be placed on its own layer, and >1 if it requires multiple export layers.

Items which require multiply layers should check QgsLayoutContext.currentExportLayer() during
their rendering to determine which layer should be drawn.
%End

    virtual void paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget );

%Docstring
Handles preparing a paint surface for the layout item and painting the item's
content. Derived classes must not override this method, but instead implement
the pure virtual method QgsLayoutItem.draw.
%End

    void setReferencePoint( const ReferencePoint &point );
%Docstring
Sets the reference ``point`` for positioning of the layout item. This point is also
fixed during resizing of the item, and any size changes will be performed
so that the position of the reference point within the layout remains unchanged.

.. seealso:: :py:func:`referencePoint`
%End

    ReferencePoint referencePoint() const;
%Docstring
Returns the reference point for positioning of the layout item. This point is also
fixed during resizing of the item, and any size changes will be performed
so that the position of the reference point within the layout remains unchanged.

.. seealso:: :py:func:`setReferencePoint`
%End

    virtual QgsLayoutSize fixedSize() const;
%Docstring
Returns the fixed size of the item, if applicable, or an empty size if item can be freely
resized.

.. seealso:: :py:func:`setFixedSize`

.. seealso:: :py:func:`minimumSize`
%End

    virtual QgsLayoutSize minimumSize() const;
%Docstring
Returns the minimum allowed size of the item, if applicable, or an empty size if item can be freely
resized.

.. seealso:: :py:func:`setMinimumSize`

.. seealso:: :py:func:`fixedSize`
%End

    virtual void attemptResize( const QgsLayoutSize &size, bool includesFrame = false );
%Docstring
Attempts to resize the item to a specified target ``size``. Note that the final size of the
item may not match the specified target size, as items with a fixed or minimum
size will place restrictions on the allowed item size. Data defined item size overrides
will also override the specified target size.

If ``includesFrame`` is true, then the size specified by ``size`` includes the
item's frame.

.. seealso:: :py:func:`minimumSize`

.. seealso:: :py:func:`fixedSize`

.. seealso:: :py:func:`attemptMove`

.. seealso:: :py:func:`sizeWithUnits`
%End

    virtual void attemptMove( const QgsLayoutPoint &point, bool useReferencePoint = true, bool includesFrame = false, int page = -1 );
%Docstring
Attempts to move the item to a specified ``point``.

If ``useReferencePoint`` is true, this method will respect the item's
reference point, in that the item will be moved so that its current reference
point is placed at the specified target point.

If ``useReferencePoint`` is false, the item will be moved so that ``point``
falls at the top-left corner of the item.

If ``includesFrame`` is true, then the position specified by ``point`` represents the
point at which to place the outside of the item's frame.

If ``page`` is not left at the default -1 value, then the position specified by ``point``
refers to the relative position on the corresponding layout ``page`` (where a ``page``
of 0 represents the first page).

Note that the final position of the item may not match the specified target position,
as data defined item position may override the specified value.

.. seealso:: :py:func:`attemptMoveBy`

.. seealso:: :py:func:`attemptResize`

.. seealso:: :py:func:`referencePoint`

.. seealso:: :py:func:`positionWithUnits`
%End

    void attemptSetSceneRect( const QRectF &rect, bool includesFrame = false );
%Docstring
Attempts to update the item's position and size to match the passed ``rect`` in layout
coordinates.

If ``includesFrame`` is true, then the position and size specified by ``rect`` represents the
position and size at for the outside of the item's frame.

Note that the final position and size of the item may not match the specified target rect,
as data defined item position and size may override the specified value.

.. seealso:: :py:func:`attemptResize`

.. seealso:: :py:func:`attemptMove`

.. seealso:: :py:func:`referencePoint`

.. seealso:: :py:func:`positionWithUnits`
%End

    void attemptMoveBy( double deltaX, double deltaY );
%Docstring
Attempts to shift the item's position by a specified ``deltaX`` and ``deltaY``, in layout
units.

Note that the final position of the item may not match the specified offsets,
as data defined item position and size may override the specified value.

.. seealso:: :py:func:`attemptResize`

.. seealso:: :py:func:`attemptMove`

.. seealso:: :py:func:`referencePoint`

.. seealso:: :py:func:`positionWithUnits`
%End

    QgsLayoutPoint positionWithUnits() const;
%Docstring
Returns the item's current position, including units. The position returned
is the position of the item's reference point, which may not necessarily be the top
left corner of the item.

.. seealso:: :py:func:`attemptMove`

.. seealso:: :py:func:`referencePoint`

.. seealso:: :py:func:`sizeWithUnits`
%End

    int page() const;
%Docstring
Returns the page the item is currently on, with the first page returning 0.

.. seealso:: :py:func:`pagePos`
%End

    QPointF pagePos() const;
%Docstring
Returns the item's position (in layout units) relative to the top left corner of its current page.

.. seealso:: :py:func:`page`

.. seealso:: :py:func:`pagePositionWithUnits`
%End

    QgsLayoutPoint pagePositionWithUnits() const;
%Docstring
Returns the item's position (in item units) relative to the top left corner of its current page.

.. seealso:: :py:func:`page`

.. seealso:: :py:func:`pagePos`
%End

    QgsLayoutSize sizeWithUnits() const;
%Docstring
Returns the item's current size, including units.

.. seealso:: :py:func:`attemptResize`

.. seealso:: :py:func:`positionWithUnits`
%End

    double itemRotation() const;
%Docstring
Returns the current rotation for the item, in degrees clockwise.

Note that this method will always return the user-set rotation for the item,
which may differ from the current item rotation (if data defined rotation
settings are present). Use QGraphicsItem.rotation() to obtain the current
item rotation.

.. seealso:: :py:func:`setItemRotation`
%End

    bool writeXml( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext &context ) const;
%Docstring
Stores the item state in a DOM element.

:param parentElement: parent DOM element (e.g. 'Layout' element)
:param document: DOM document
:param context: read write context

.. seealso:: :py:func:`readXml`
%End

    bool readXml( const QDomElement &itemElement, const QDomDocument &document, const QgsReadWriteContext &context );
%Docstring
Sets the item state from a DOM element.

:param itemElement: is the DOM node corresponding to item (e.g. 'LayoutItem' element)
:param document: DOM document
:param context: read write context

Note that item subclasses should not rely on all other items being present in the
layout at the time this method is called. Instead, any connections and links to
other items must be made in the finalizeRestoreFromXml() method. E.g. when restoring
a scalebar, the connection to the linked map's signals should be implemented
in finalizeRestoreFromXml(), not readXml().

.. seealso:: :py:func:`writeXml`

.. seealso:: :py:func:`finalizeRestoreFromXml`
%End

    virtual void finalizeRestoreFromXml();
%Docstring
Called after all pending items have been restored from XML. Items can use
this method to run steps which must take place after all items have been restored to the layout,
such as connecting to signals emitted by other items, which may not have existed in the layout
at the time readXml() was called. E.g. a scalebar can use this to connect to its linked
map item after restoration from XML.

.. seealso:: :py:func:`readXml`
%End

    virtual QgsAbstractLayoutUndoCommand *createCommand( const QString &text, int id, QUndoCommand *parent = 0 ) /Factory/;


    bool frameEnabled() const;
%Docstring
Returns true if the item includes a frame.

.. seealso:: :py:func:`setFrameEnabled`

.. seealso:: :py:func:`frameStrokeWidth`

.. seealso:: :py:func:`frameJoinStyle`

.. seealso:: :py:func:`frameStrokeColor`
%End

    virtual void setFrameEnabled( bool drawFrame );
%Docstring
Sets whether this item has a frame drawn around it or not.

.. seealso:: :py:func:`frameEnabled`

.. seealso:: :py:func:`setFrameStrokeWidth`

.. seealso:: :py:func:`setFrameJoinStyle`

.. seealso:: :py:func:`setFrameStrokeColor`
%End

    void setFrameStrokeColor( const QColor &color );
%Docstring
Sets the frame stroke ``color``.

.. seealso:: :py:func:`frameStrokeColor`

.. seealso:: :py:func:`setFrameEnabled`

.. seealso:: :py:func:`setFrameJoinStyle`

.. seealso:: :py:func:`setFrameStrokeWidth`
%End

    QColor frameStrokeColor() const;
%Docstring
Returns the frame's stroke color. This is only used if frameEnabled() returns true.

.. seealso:: :py:func:`frameEnabled`

.. seealso:: :py:func:`setFrameStrokeColor`

.. seealso:: :py:func:`frameJoinStyle`

.. seealso:: :py:func:`setFrameStrokeColor`
%End

    virtual void setFrameStrokeWidth( const QgsLayoutMeasurement &width );
%Docstring
Sets the frame stroke ``width``.

.. seealso:: :py:func:`frameStrokeWidth`

.. seealso:: :py:func:`setFrameEnabled`

.. seealso:: :py:func:`setFrameJoinStyle`

.. seealso:: :py:func:`setFrameStrokeColor`
%End

    QgsLayoutMeasurement frameStrokeWidth() const;
%Docstring
Returns the frame's stroke width. This is only used if frameEnabled() returns true.

.. seealso:: :py:func:`frameEnabled`

.. seealso:: :py:func:`setFrameStrokeWidth`

.. seealso:: :py:func:`frameJoinStyle`

.. seealso:: :py:func:`frameStrokeColor`
%End

    Qt::PenJoinStyle frameJoinStyle() const;
%Docstring
Returns the join style used for drawing the item's frame.

.. seealso:: :py:func:`frameEnabled`

.. seealso:: :py:func:`setFrameJoinStyle`

.. seealso:: :py:func:`frameStrokeWidth`

.. seealso:: :py:func:`frameStrokeColor`
%End

    void setFrameJoinStyle( const Qt::PenJoinStyle style );
%Docstring
Sets the join ``style`` used when drawing the item's frame.

.. seealso:: :py:func:`setFrameEnabled`

.. seealso:: :py:func:`frameJoinStyle`

.. seealso:: :py:func:`setFrameStrokeWidth`

.. seealso:: :py:func:`setFrameStrokeColor`
%End

    bool hasBackground() const;
%Docstring
Returns true if the item has a background.

.. seealso:: :py:func:`setBackgroundEnabled`

.. seealso:: :py:func:`backgroundColor`
%End

    void setBackgroundEnabled( bool drawBackground );
%Docstring
Sets whether this item has a background drawn under it or not.

.. seealso:: :py:func:`hasBackground`

.. seealso:: :py:func:`setBackgroundColor`
%End

    QColor backgroundColor() const;
%Docstring
Returns the background color for this item. This is only used if hasBackground()
returns true.

.. seealso:: :py:func:`setBackgroundColor`

.. seealso:: :py:func:`hasBackground`
%End

    void setBackgroundColor( const QColor &color );
%Docstring
Sets the background ``color`` for this item.

.. seealso:: :py:func:`backgroundColor`

.. seealso:: :py:func:`setBackgroundEnabled`
%End

    QPainter::CompositionMode blendMode() const;
%Docstring
Returns the item's composition blending mode.

.. seealso:: :py:func:`setBlendMode`
%End

    void setBlendMode( const QPainter::CompositionMode mode );
%Docstring
Sets the item's composition blending ``mode``.

.. seealso:: :py:func:`blendMode`
%End

    double itemOpacity() const;
%Docstring
Returns the item's opacity. This method should be used instead of
QGraphicsItem.opacity() as any data defined overrides will be
respected.

:return: opacity as double between 1.0 (opaque) and 0 (transparent).

.. seealso:: :py:func:`setItemOpacity`
%End

    void setItemOpacity( double opacity );
%Docstring
Sets the item's ``opacity``. This method should be used instead of
QGraphicsItem.setOpacity() as any data defined overrides will be
respected.

:param opacity: double between 1.0 (opaque) and 0 (transparent).

.. seealso:: :py:func:`itemOpacity`
%End

    bool excludeFromExports() const;
%Docstring
Returns whether the item should be excluded from layout exports and prints.

.. seealso:: :py:func:`setExcludeFromExports`
%End

    void setExcludeFromExports( bool exclude );
%Docstring
Sets whether the item should be excluded from layout exports and prints.

.. seealso:: :py:func:`excludeFromExports`
%End

    virtual bool containsAdvancedEffects() const;
%Docstring
Returns true if the item contains contents with blend modes or transparency
effects which can only be reproduced by rastering the item.

Subclasses should ensure that implemented overrides of this method
also check the base class result.

.. seealso:: :py:func:`requiresRasterization`
%End

    virtual bool requiresRasterization() const;
%Docstring
Returns true if the item is drawn in such a way that forces the whole layout
to be rasterized when exporting to vector formats.

.. seealso:: :py:func:`containsAdvancedEffects`
%End

    virtual double estimatedFrameBleed() const;
%Docstring
Returns the estimated amount the item's frame bleeds outside the item's
actual rectangle. For instance, if the item has a 2mm frame stroke, then
1mm of this frame is drawn outside the item's rect. In this case the
return value will be 1.0.

Returned values are in layout units.

.. seealso:: :py:func:`rectWithFrame`
%End

    virtual QRectF rectWithFrame() const;
%Docstring
Returns the item's rectangular bounds, including any bleed caused by the item's frame.
The bounds are returned in the item's coordinate system (see Qt's QGraphicsItem docs for
more details about QGraphicsItem coordinate systems). The results differ from Qt's rect()
function, as rect() makes no allowances for the portion of outlines which are drawn
outside of the item.

.. seealso:: :py:func:`estimatedFrameBleed`
%End

    virtual void moveContent( double dx, double dy );
%Docstring
Moves the content of the item, by a specified ``dx`` and ``dy`` in layout units.
The default implementation has no effect.

.. seealso:: :py:func:`setMoveContentPreviewOffset`

.. seealso:: :py:func:`zoomContent`
%End

    virtual void setMoveContentPreviewOffset( double dx, double dy );
%Docstring
Sets temporary offset for the item, by a specified ``dx`` and ``dy`` in layout units.
This is useful for live updates when moving item content in a :py:class:`QgsLayoutView`.
The default implementation has no effect.

.. seealso:: :py:func:`moveContent`
%End

    virtual void zoomContent( double factor, QPointF point );
%Docstring
Zooms content of item. Does nothing by default.

:param factor: zoom factor, where > 1 results in a zoom in and < 1 results in a zoom out
:param point: item point for zoom center

.. seealso:: :py:func:`moveContent`
%End

    void beginCommand( const QString &commandText, UndoCommand command = UndoNone );
%Docstring
Starts new undo command for this item.
The ``commandText`` should be a capitalized, imperative tense description (e.g. "Add Map Item").
If specified, multiple consecutive commands for this item with the same ``command`` will
be collapsed into a single undo command in the layout history.

.. seealso:: :py:func:`endCommand`

.. seealso:: :py:func:`cancelCommand`
%End

    void endCommand();
%Docstring
Completes the current item command and push it onto the layout's undo stack.

.. seealso:: :py:func:`beginCommand`

.. seealso:: :py:func:`cancelCommand`
%End

    void cancelCommand();
%Docstring
Cancels the current item command and discards it.

.. seealso:: :py:func:`beginCommand`

.. seealso:: :py:func:`endCommand`
%End

    bool shouldDrawItem() const;
%Docstring
Returns whether the item should be drawn in the current context.
%End

  public slots:

    virtual void refresh();

%Docstring
Refreshes the item, causing a recalculation of any property overrides and
recalculation of its position and size.
%End

    virtual void invalidateCache();
%Docstring
Forces a deferred update of any cached image the item uses.
%End

    virtual void redraw();
%Docstring
Triggers a redraw (update) of the item.
%End

    virtual void refreshDataDefinedProperty( const QgsLayoutObject::DataDefinedProperty property = QgsLayoutObject::AllProperties );
%Docstring
Refreshes a data defined ``property`` for the item by reevaluating the property's value
and redrawing the item with this new value. If ``property`` is set to
QgsLayoutObject.AllProperties then all data defined properties for the item will be
refreshed.
%End

    virtual void setItemRotation( double rotation, bool adjustPosition = true );
%Docstring
Sets the layout item's ``rotation``, in degrees clockwise.

If ``adjustPosition`` is true, then this rotation occurs around the center of the item.
If ``adjustPosition`` is false, rotation occurs around the item origin.

.. seealso:: :py:func:`itemRotation`

.. seealso:: :py:func:`rotateItem`
%End

    virtual void rotateItem( const double angle, const QPointF &transformOrigin );
%Docstring
Rotates the item by a specified ``angle`` in degrees clockwise around a specified reference point.

.. seealso:: :py:func:`setItemRotation`

.. seealso:: :py:func:`itemRotation`
%End

    virtual QgsExpressionContext createExpressionContext() const;


  signals:

    void frameChanged();
%Docstring
Emitted if the item's frame style changes.
%End

    void lockChanged();
%Docstring
Emitted if the item's lock status changes.

.. seealso:: :py:func:`isLocked`

.. seealso:: :py:func:`setLocked`
%End

    void rotationChanged( double newRotation );
%Docstring
Emitted on item rotation change.
%End

    void sizePositionChanged();
%Docstring
Emitted when the item's size or position changes.
%End

  protected:

    virtual void drawDebugRect( QPainter *painter );
%Docstring
Draws a debugging rectangle of the item's current bounds within the specified
painter.

:param painter: destination QPainter
%End

    virtual void draw( QgsRenderContext &context, const QStyleOptionGraphicsItem *itemStyle = 0 ) = 0;
%Docstring
Draws the item's contents using the specified render ``context``.
Note that the context's painter has been scaled so that painter units are pixels.
Use the QgsRenderContext methods to convert from millimeters or other units to the painter's units.
%End

    virtual void drawFrame( QgsRenderContext &context );
%Docstring
Draws the frame around the item.
%End

    virtual void drawBackground( QgsRenderContext &context );
%Docstring
Draws the background for the item.
%End

    virtual void setFixedSize( const QgsLayoutSize &size );
%Docstring
Sets a fixed ``size`` for the layout item, which prevents it from being freely
resized. Set an empty size if item can be freely resized.

.. seealso:: :py:func:`fixedSize`

.. seealso:: :py:func:`setMinimumSize`
%End

    virtual void setMinimumSize( const QgsLayoutSize &size );
%Docstring
Sets the minimum allowed ``size`` for the layout item. Set an empty size if item can be freely
resized.

.. seealso:: :py:func:`minimumSize`

.. seealso:: :py:func:`setFixedSize`
%End

    virtual QSizeF applyItemSizeConstraint( const QSizeF &targetSize );
%Docstring
Applies any item-specific size constraint handling to a given ``targetSize`` in layout units.
Subclasses can override this method if they need to apply advanced logic regarding item
sizes, which cannot be covered by setFixedSize() or setMinimumSize().
Item size constraints are applied after fixed, minimum and data defined size constraints.

.. seealso:: :py:func:`setFixedSize`

.. seealso:: :py:func:`setMinimumSize`
%End

    void refreshItemSize();
%Docstring
Refreshes an item's size by rechecking it against any possible item fixed
or minimum sizes.

.. seealso:: :py:func:`setFixedSize`

.. seealso:: :py:func:`setMinimumSize`

.. seealso:: :py:func:`refreshItemPosition`
%End

    void refreshItemPosition();
%Docstring
Refreshes an item's position by rechecking it against any possible overrides
such as data defined positioning.

.. seealso:: :py:func:`refreshItemSize`
%End

    void refreshItemRotation( QPointF *origin = 0 );
%Docstring
Refreshes an item's rotation by rechecking it against any possible overrides
such as data defined rotation.

The optional ``origin`` point specifies the origin (in item coordinates)
around which the rotation should be applied.

.. seealso:: :py:func:`refreshItemSize`

.. seealso:: :py:func:`refreshItemPosition`
%End

    void refreshOpacity( bool updateItem = true );
%Docstring
Refresh item's opacity, considering data defined opacity.
If ``updateItem`` is set to false the item will not be automatically
updated after the opacity is set and a later call to update() must be made.
%End

    void refreshFrame( bool updateItem = true );
%Docstring
Refresh item's frame, considering data defined colors and frame size.
If ``updateItem`` is set to false, the item will not be automatically updated
after the frame is set and a later call to update() must be made.
%End

    void refreshBackgroundColor( bool updateItem = true );
%Docstring
Refresh item's background color, considering data defined colors.
If ``updateItem`` is set to false, the item will not be automatically updated
after the frame color is set and a later call to update() must be made.
%End

    void refreshBlendMode();
%Docstring
Refresh item's blend mode, considering data defined blend mode.
%End

    QPointF adjustPointForReferencePosition( const QPointF &point, const QSizeF &size, const ReferencePoint &reference ) const;
%Docstring
Adjusts the specified ``point`` at which a ``reference`` position of the item
sits and returns the top left corner of the item, if reference point were placed at the specified position.
%End

    QPointF positionAtReferencePoint( const ReferencePoint &reference ) const;
%Docstring
Returns the current position (in layout units) of a ``reference`` point for the item.
%End

    QgsLayoutPoint topLeftToReferencePoint( const QgsLayoutPoint &point ) const;
%Docstring
Returns the position for the reference point of the item, if the top-left of the item
was placed at the specified ``point``.
%End

    virtual bool writePropertiesToElement( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const;
%Docstring
Stores item state within an XML DOM element.

:param element: is the DOM element to store the item's properties in
:param document: DOM document
:param context: read write context

.. seealso:: :py:func:`writeXml`

.. seealso:: :py:func:`readPropertiesFromElement`
%End

    virtual bool readPropertiesFromElement( const QDomElement &element, const QDomDocument &document, const QgsReadWriteContext &context );
%Docstring
Sets item state from a DOM element.

:param element: is the DOM element for the item
:param document: DOM document
:param context: read write context

Note that item subclasses should not rely on all other items being present in the
layout at the time this method is called. Instead, any connections and links to
other items must be made in the finalizeRestoreFromXml() method. E.g. when restoring
a scalebar, the connection to the linked map's signals should be implemented
in finalizeRestoreFromXml(), not readPropertiesFromElement().

.. seealso:: :py:func:`writePropertiesToElement`

.. seealso:: :py:func:`readXml`
%End

    QgsLayoutSize applyDataDefinedSize( const QgsLayoutSize &size );
%Docstring
Applies any present data defined size overrides to the specified layout ``size``.
%End

};




/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitem.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
