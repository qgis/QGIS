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
#include <qgslayoutitemshape.h>
#include <qgslayoutitempage.h>
%End
%ConvertToSubClassCode
    // the conversions have to be static, because they're using multiple inheritance
    // (seen in PyQt4 .sip files for some QGraphicsItem classes)
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
      UndoCustomCommand,
    };

    explicit QgsLayoutItem( QgsLayout *layout, bool manageZValue = true );
%Docstring
 Constructor for QgsLayoutItem, with the specified parent ``layout``.

 If ``manageZValue`` is true, the z-Value of this item will be managed by the layout.
 Generally this is the desired behavior.
%End

    ~QgsLayoutItem();

    virtual int type() const;

%Docstring
 Return correct graphics item type
.. seealso:: stringType()
 :rtype: int
%End

    virtual QString stringType() const = 0;
%Docstring
 Return the item type as a string.

 This string must be a unique, single word, character only representation of the item type, eg "LayoutScaleBar"
.. seealso:: type()
 :rtype: str
%End

    QString uuid() const;
%Docstring
 Returns the item identification string. This is a unique random string set for the item
 upon creation.
.. note::

   There is no corresponding setter for the uuid - it's created automatically.
.. seealso:: id()
.. seealso:: setId()
 :rtype: str
%End

    QString id() const;
%Docstring
 Returns the item's ID name. This is not necessarily unique, and duplicate ID names may exist
 for a layout.
.. seealso:: setId()
.. seealso:: uuid()
 :rtype: str
%End

    virtual void setId( const QString &id );
%Docstring
 Set the item's ``id`` name. This is not necessarily unique, and duplicate ID names may exist
 for a layout.
.. seealso:: id()
.. seealso:: uuid()
%End

    virtual QString displayName() const;
%Docstring
 Get item display name. This is the item's id if set, and if
 not, a user-friendly string identifying item type.
.. seealso:: id()
.. seealso:: setId()
 :rtype: str
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
.. seealso:: isLocked()
.. seealso:: lockChanged()
%End

    bool isLocked() const;
%Docstring
 Returns true if the item is locked, and cannot be interacted with using the mouse.
.. seealso:: setLocked()
.. seealso:: lockChanged()
 :rtype: bool
%End

    bool isGroupMember() const;
%Docstring
 Returns true if the item is part of a QgsLayoutItemGroup group.
.. seealso:: parentGroup()
.. seealso:: setParentGroup()
 :rtype: bool
%End

    QgsLayoutItemGroup *parentGroup() const;
%Docstring
 Returns the item's parent group, if the item is part of a QgsLayoutItemGroup group.
.. seealso:: isGroupMember()
.. seealso:: setParentGroup()
 :rtype: QgsLayoutItemGroup
%End

    void setParentGroup( QgsLayoutItemGroup *group );
%Docstring
 Sets the item's parent ``group``.
.. seealso:: isGroupMember()
.. seealso:: parentGroup()
%End

    virtual int numberExportLayers() const;
%Docstring
 Returns the number of layers that this item requires for exporting during layered exports (e.g. SVG).
 Returns 0 if this item is to be placed on the same layer as the previous item,
 1 if it should be placed on its own layer, and >1 if it requires multiple export layers.

 Items which require multiply layers should check QgsLayoutContext.currentExportLayer() during
 their rendering to determine which layer should be drawn.
 :rtype: int
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
.. seealso:: referencePoint()
%End

    ReferencePoint referencePoint() const;
%Docstring
 Returns the reference point for positioning of the layout item. This point is also
 fixed during resizing of the item, and any size changes will be performed
 so that the position of the reference point within the layout remains unchanged.
.. seealso:: setReferencePoint()
 :rtype: ReferencePoint
%End

    QgsLayoutSize fixedSize() const;
%Docstring
 Returns the fixed size of the item, if applicable, or an empty size if item can be freely
 resized.
.. seealso:: setFixedSize()
.. seealso:: minimumSize()
 :rtype: QgsLayoutSize
%End

    virtual QgsLayoutSize minimumSize() const;
%Docstring
 Returns the minimum allowed size of the item, if applicable, or an empty size if item can be freely
 resized.
.. seealso:: setMinimumSize()
.. seealso:: fixedSize()
 :rtype: QgsLayoutSize
%End

    virtual void attemptResize( const QgsLayoutSize &size, bool includesFrame = false );
%Docstring
 Attempts to resize the item to a specified target ``size``. Note that the final size of the
 item may not match the specified target size, as items with a fixed or minimum
 size will place restrictions on the allowed item size. Data defined item size overrides
 will also override the specified target size.

 If ``includesFrame`` is true, then the size specified by ``size`` includes the
 item's frame.

.. seealso:: minimumSize()
.. seealso:: fixedSize()
.. seealso:: attemptMove()
.. seealso:: sizeWithUnits()
%End

    virtual void attemptMove( const QgsLayoutPoint &point, bool useReferencePoint = true, bool includesFrame = false );
%Docstring
 Attempts to move the item to a specified ``point``.

 If ``useReferencePoint`` is true, this method will respect the item's
 reference point, in that the item will be moved so that its current reference
 point is placed at the specified target point.

 If ``useReferencePoint`` is false, the item will be moved so that ``point``
 falls at the top-left corner of the item.

 If ``includesFrame`` is true, then the position specified by ``point`` represents the
 point at which to place the outside of the item's frame.

 Note that the final position of the item may not match the specified target position,
 as data defined item position may override the specified value.

.. seealso:: attemptResize()
.. seealso:: referencePoint()
.. seealso:: positionWithUnits()
%End


    void attemptSetSceneRect( const QRectF &rect, bool includesFrame = false );
%Docstring
 Attempts to update the item's position and size to match the passed ``rect`` in layout
 coordinates.

 If ``includesFrame`` is true, then the position and size specified by ``rect`` represents the
 position and size at for the outside of the item's frame.

 Note that the final position and size of the item may not match the specified target rect,
 as data defined item position and size may override the specified value.

.. seealso:: attemptResize()
.. seealso:: attemptMove()
.. seealso:: referencePoint()
.. seealso:: positionWithUnits()
%End

    QgsLayoutPoint positionWithUnits() const;
%Docstring
 Returns the item's current position, including units. The position returned
 is the position of the item's reference point, which may not necessarily be the top
 left corner of the item.
.. seealso:: attemptMove()
.. seealso:: referencePoint()
.. seealso:: sizeWithUnits()
 :rtype: QgsLayoutPoint
%End

    QgsLayoutSize sizeWithUnits() const;
%Docstring
 Returns the item's current size, including units.
.. seealso:: attemptResize()
.. seealso:: positionWithUnits()
 :rtype: QgsLayoutSize
%End

    double itemRotation() const;
%Docstring
 Returns the current rotation for the item, in degrees clockwise.

 Note that this method will always return the user-set rotation for the item,
 which may differ from the current item rotation (if data defined rotation
 settings are present). Use QGraphicsItem.rotation() to obtain the current
 item rotation.

.. seealso:: setItemRotation()
 :rtype: float
%End

    bool writeXml( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext &context ) const;
%Docstring
 Stores the item state in a DOM element.
 \param parentElement parent DOM element (e.g. 'Layout' element)
 \param document DOM document
 \param context read write context
.. seealso:: readXml()
 :rtype: bool
%End

    bool readXml( const QDomElement &itemElement, const QDomDocument &document, const QgsReadWriteContext &context );
%Docstring
 Sets the item state from a DOM element.
 \param itemElement is the DOM node corresponding to item (e.g. 'LayoutItem' element)
 \param document DOM document
 \param context read write context
.. seealso:: writeXml()
 :rtype: bool
%End

    virtual QgsAbstractLayoutUndoCommand *createCommand( const QString &text, int id, QUndoCommand *parent = 0 ) /Factory/;


    bool hasFrame() const;
%Docstring
 Returns true if the item includes a frame.
.. seealso:: setFrameEnabled()
.. seealso:: frameStrokeWidth()
.. seealso:: frameJoinStyle()
.. seealso:: frameStrokeColor()
 :rtype: bool
%End

    virtual void setFrameEnabled( bool drawFrame );
%Docstring
 Sets whether this item has a frame drawn around it or not.
.. seealso:: hasFrame()
.. seealso:: setFrameStrokeWidth()
.. seealso:: setFrameJoinStyle()
.. seealso:: setFrameStrokeColor()
%End

    void setFrameStrokeColor( const QColor &color );
%Docstring
 Sets the frame stroke ``color``.
.. seealso:: frameStrokeColor()
.. seealso:: setFrameEnabled()
.. seealso:: setFrameJoinStyle()
.. seealso:: setFrameStrokeWidth()
%End

    QColor frameStrokeColor() const;
%Docstring
 Returns the frame's stroke color. This is only used if hasFrame() returns true.
.. seealso:: hasFrame()
.. seealso:: setFrameStrokeColor()
.. seealso:: frameJoinStyle()
.. seealso:: setFrameStrokeColor()
 :rtype: QColor
%End

    virtual void setFrameStrokeWidth( const QgsLayoutMeasurement &width );
%Docstring
 Sets the frame stroke ``width``.
.. seealso:: frameStrokeWidth()
.. seealso:: setFrameEnabled()
.. seealso:: setFrameJoinStyle()
.. seealso:: setFrameStrokeColor()
%End

    QgsLayoutMeasurement frameStrokeWidth() const;
%Docstring
 Returns the frame's stroke width. This is only used if hasFrame() returns true.
.. seealso:: hasFrame()
.. seealso:: setFrameStrokeWidth()
.. seealso:: frameJoinStyle()
.. seealso:: frameStrokeColor()
 :rtype: QgsLayoutMeasurement
%End

    Qt::PenJoinStyle frameJoinStyle() const;
%Docstring
 Returns the join style used for drawing the item's frame.
.. seealso:: hasFrame()
.. seealso:: setFrameJoinStyle()
.. seealso:: frameStrokeWidth()
.. seealso:: frameStrokeColor()
 :rtype: Qt.PenJoinStyle
%End

    void setFrameJoinStyle( const Qt::PenJoinStyle style );
%Docstring
 Sets the join ``style`` used when drawing the item's frame.
.. seealso:: setFrameEnabled()
.. seealso:: frameJoinStyle()
.. seealso:: setFrameStrokeWidth()
.. seealso:: setFrameStrokeColor()
%End

    bool hasBackground() const;
%Docstring
 Returns true if the item has a background.
.. seealso:: setBackgroundEnabled()
.. seealso:: backgroundColor()
 :rtype: bool
%End

    void setBackgroundEnabled( bool drawBackground );
%Docstring
 Sets whether this item has a background drawn under it or not.
.. seealso:: hasBackground()
.. seealso:: setBackgroundColor()
%End

    QColor backgroundColor() const;
%Docstring
 Returns the background color for this item. This is only used if hasBackground()
 returns true.
.. seealso:: setBackgroundColor()
.. seealso:: hasBackground()
 :rtype: QColor
%End

    void setBackgroundColor( const QColor &color );
%Docstring
 Sets the background ``color`` for this item.
.. seealso:: backgroundColor()
.. seealso:: setBackgroundEnabled()
%End

    QPainter::CompositionMode blendMode() const;
%Docstring
 Returns the item's composition blending mode.
.. seealso:: setBlendMode()
 :rtype: QPainter.CompositionMode
%End

    void setBlendMode( const QPainter::CompositionMode mode );
%Docstring
 Sets the item's composition blending ``mode``.
.. seealso:: blendMode()
%End

    double itemOpacity() const;
%Docstring
 Returns the item's opacity. This method should be used instead of
 QGraphicsItem.opacity() as any data defined overrides will be
 respected.
 :return: opacity as double between 1.0 (opaque) and 0 (transparent).
.. seealso:: setItemOpacity()
 :rtype: float
%End

    void setItemOpacity( double opacity );
%Docstring
 Sets the item's ``opacity``. This method should be used instead of
 QGraphicsItem.setOpacity() as any data defined overrides will be
 respected.
 \param opacity double between 1.0 (opaque) and 0 (transparent).
.. seealso:: itemOpacity()
%End

    bool excludeFromExports() const;
%Docstring
 Returns whether the item should be excluded from layout exports and prints.
.. seealso:: setExcludeFromExports()
 :rtype: bool
%End

    void setExcludeFromExports( bool exclude );
%Docstring
 Sets whether the item should be excluded from composer exports and prints.
.. seealso:: excludeFromExports()
%End

    virtual double estimatedFrameBleed() const;
%Docstring
 Returns the estimated amount the item's frame bleeds outside the item's
 actual rectangle. For instance, if the item has a 2mm frame stroke, then
 1mm of this frame is drawn outside the item's rect. In this case the
 return value will be 1.0.

 Returned values are in layout units.

.. seealso:: rectWithFrame()
 :rtype: float
%End

    virtual QRectF rectWithFrame() const;
%Docstring
 Returns the item's rectangular bounds, including any bleed caused by the item's frame.
 The bounds are returned in the item's coordinate system (see Qt's QGraphicsItem docs for
 more details about QGraphicsItem coordinate systems). The results differ from Qt's rect()
 function, as rect() makes no allowances for the portion of outlines which are drawn
 outside of the item.

.. seealso:: estimatedFrameBleed()
 :rtype: QRectF
%End

    virtual void moveContent( double dx, double dy );
%Docstring
 Moves the content of the item, by a specified ``dx`` and ``dy`` in layout units.
 The default implementation has no effect.
.. seealso:: setMoveContentPreviewOffset()
.. seealso:: zoomContent()
%End

    virtual void setMoveContentPreviewOffset( double dx, double dy );
%Docstring
 Sets temporary offset for the item, by a specified ``dx`` and ``dy`` in layout units.
 This is useful for live updates when moving item content in a QgsLayoutView.
 The default implementation has no effect.
.. seealso:: moveContent()
%End

    virtual void zoomContent( double factor, QPointF point );
%Docstring
 Zooms content of item. Does nothing by default.
 \param factor zoom factor, where > 1 results in a zoom in and < 1 results in a zoom out
 \param point item point for zoom center
.. seealso:: moveContent()
%End

    void beginCommand( const QString &commandText, UndoCommand command = UndoNone );
%Docstring
 Starts new undo command for this item.
 The ``commandText`` should be a capitalized, imperative tense description (e.g. "Add Map Item").
 If specified, multiple consecutive commands for this item with the same ``command`` will
 be collapsed into a single undo command in the layout history.
.. seealso:: endCommand()
.. seealso:: cancelCommand()
%End

    void endCommand();
%Docstring
 Completes the current item command and push it onto the layout's undo stack.
.. seealso:: beginCommand()
.. seealso:: cancelCommand()
%End

    void cancelCommand();
%Docstring
 Cancels the current item command and discards it.
.. seealso:: beginCommand()
.. seealso:: endCommand()
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

.. seealso:: itemRotation()
.. seealso:: rotateItem()
%End

    virtual void rotateItem( const double angle, const QPointF &transformOrigin );
%Docstring
 Rotates the item by a specified ``angle`` in degrees clockwise around a specified reference point.
.. seealso:: setItemRotation()
.. seealso:: itemRotation()
%End

  signals:

    void frameChanged();
%Docstring
 Emitted if the item's frame style changes.
%End

    void lockChanged();
%Docstring
 Emitted if the item's lock status changes.
.. seealso:: isLocked()
.. seealso:: setLocked()
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
 @param painter destination QPainter
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
.. seealso:: fixedSize()
.. seealso:: setMinimumSize()
%End

    virtual void setMinimumSize( const QgsLayoutSize &size );
%Docstring
 Sets the minimum allowed ``size`` for the layout item. Set an empty size if item can be freely
 resized.
.. seealso:: minimumSize()
.. seealso:: setFixedSize()
%End

    virtual QSizeF applyItemSizeConstraint( const QSizeF &targetSize );
%Docstring
 Applies any item-specific size constraint handling to a given ``targetSize`` in layout units.
 Subclasses can override this method if they need to apply advanced logic regarding item
 sizes, which cannot be covered by setFixedSize() or setMinimumSize().
 Item size constraints are applied after fixed, minimum and data defined size constraints.
.. seealso:: setFixedSize()
.. seealso:: setMinimumSize()
 :rtype: QSizeF
%End

    void refreshItemSize();
%Docstring
 Refreshes an item's size by rechecking it against any possible item fixed
 or minimum sizes.
.. seealso:: setFixedSize()
.. seealso:: setMinimumSize()
.. seealso:: refreshItemPosition()
%End

    void refreshItemPosition();
%Docstring
 Refreshes an item's position by rechecking it against any possible overrides
 such as data defined positioning.
.. seealso:: refreshItemSize()
%End

    void refreshItemRotation( QPointF *origin = 0 );
%Docstring
 Refreshes an item's rotation by rechecking it against any possible overrides
 such as data defined rotation.

 The optional ``origin`` point specifies the origin (in item coordinates)
 around which the rotation should be applied.

.. seealso:: refreshItemSize()
.. seealso:: refreshItemPosition()
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
 :rtype: QPointF
%End

    QPointF positionAtReferencePoint( const ReferencePoint &reference ) const;
%Docstring
 Returns the current position (in layout units) of a ``reference`` point for the item.
 :rtype: QPointF
%End

    QgsLayoutPoint topLeftToReferencePoint( const QgsLayoutPoint &point ) const;
%Docstring
 Returns the position for the reference point of the item, if the top-left of the item
 was placed at the specified ``point``.
 :rtype: QgsLayoutPoint
%End

    virtual bool writePropertiesToElement( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const;
%Docstring
 Stores item state within an XML DOM element.
 \param element is the DOM element to store the item's properties in
 \param document DOM document
 \param context read write context
.. seealso:: writeXml()
.. seealso:: readPropertiesFromElement()
 :rtype: bool
%End

    virtual bool readPropertiesFromElement( const QDomElement &element, const QDomDocument &document, const QgsReadWriteContext &context );
%Docstring
 Sets item state from a DOM element.
 \param element is the DOM element for the item
 \param document DOM document
 \param context read write context
.. seealso:: writePropertiesToElement()
.. seealso:: readXml()
 :rtype: bool
%End

    bool shouldDrawItem() const;
%Docstring
 Returns whether the item should be drawn in the current context.
 :rtype: bool
%End

    QgsLayoutSize applyDataDefinedSize( const QgsLayoutSize &size );
%Docstring
 Applies any present data defined size overrides to the specified layout ``size``.
 :rtype: QgsLayoutSize
%End

};




/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitem.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
