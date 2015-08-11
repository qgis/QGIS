/** \ingroup MapComposer
 * A item that forms part of a map composition.
 */
class QgsComposerItem : QgsComposerObject, QGraphicsRectItem
{
%TypeHeaderCode
#include <qgscomposeritem.h>
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
#include <qgscomposertable.h>
#include <qgscomposertexttable.h>
#include <qgscomposerattributetable.h>
%End

%ConvertToSubClassCode
  // the conversions have to be static, because they're using multiple inheritance
  // (seen in PyQt4 .sip files for some QGraphicsItem classes)
  switch (sipCpp->type())
  {
  case QgsComposerItem::ComposerItem:
    sipClass = sipClass_QgsComposerItem;
    *sipCppRet = static_cast<QgsComposerItem*>(sipCpp);
    break;
  case QgsComposerItem::ComposerArrow:
    sipClass = sipClass_QgsComposerArrow;
    *sipCppRet = static_cast<QgsComposerArrow*>(sipCpp);
    break;
  case QgsComposerItem::ComposerItemGroup:
    sipClass = sipClass_QgsComposerItemGroup;
    *sipCppRet = static_cast<QgsComposerItemGroup*>(sipCpp);
    break;
  case QgsComposerItem::ComposerLabel:
    sipClass = sipClass_QgsComposerLabel;
    *sipCppRet = static_cast<QgsComposerLabel*>(sipCpp);
    break;
  case QgsComposerItem::ComposerLegend:
    sipClass = sipClass_QgsComposerLegend;
    *sipCppRet = static_cast<QgsComposerLegend*>(sipCpp);
    break;
  case QgsComposerItem::ComposerMap:
    sipClass = sipClass_QgsComposerMap;
    *sipCppRet = static_cast<QgsComposerMap*>(sipCpp);
    break;
  case QgsComposerItem::ComposerPaper:
    sipClass = sipClass_QgsPaperItem;
    *sipCppRet = static_cast<QgsPaperItem*>(sipCpp);
    break;
  case QgsComposerItem::ComposerPicture:
    sipClass = sipClass_QgsComposerPicture;
    *sipCppRet = static_cast<QgsComposerPicture*>(sipCpp);
    break;
  case QgsComposerItem::ComposerScaleBar:
    sipClass = sipClass_QgsComposerScaleBar;
    *sipCppRet = static_cast<QgsComposerScaleBar*>(sipCpp);
    break;
  case QgsComposerItem::ComposerShape:
    sipClass = sipClass_QgsComposerShape;
    *sipCppRet = static_cast<QgsComposerShape*>(sipCpp);
    break;
  case QgsComposerItem::ComposerTable:
    sipClass = sipClass_QgsComposerTable;
    *sipCppRet = static_cast<QgsComposerTable*>(sipCpp);
    break;
  case QgsComposerItem::ComposerAttributeTable:
    sipClass = sipClass_QgsComposerAttributeTable;
    *sipCppRet = static_cast<QgsComposerAttributeTable*>(sipCpp);
    break;
  case QgsComposerItem::ComposerTextTable:
    sipClass = sipClass_QgsComposerTextTable;
    *sipCppRet = static_cast<QgsComposerTextTable*>(sipCpp);
    break;
  case QgsComposerItem::ComposerFrame:
    sipClass = sipClass_QgsComposerFrame;
    *sipCppRet = static_cast<QgsComposerFrame*>(sipCpp);
    break;
  default:
    sipClass = 0;
  }
%End

  public:

    enum ItemType
    {
      // base class for the items
      ComposerItem,

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
      ComposerTable,
      ComposerAttributeTable,
      ComposerTextTable,
      ComposerFrame
    };

    /** Describes the action (move or resize in different directon) to be done during mouse move*/
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
    /** Modes for zooming item content
     */
    enum ZoomMode
    {
      Zoom = 0, /*!< Zoom to center of content */
      ZoomRecenter, /*!< Zoom and recenter content to point */
      ZoomToPoint, /*!< Zoom while maintaining relative position of point */
      NoZoom /*!< No zoom */
    };

    /** Constructor
     @param composition parent composition
     @param manageZValue true if the z-Value of this object should be managed by mComposition*/
    QgsComposerItem( QgsComposition* composition /TransferThis/, bool manageZValue = true );
    /** Constructor with box position and composer object
     @param x x coordinate of item
     @param y y coordinate of item
     @param width width of item
     @param height height of item
     @param composition parent composition
     @param manageZValue true if the z-Value of this object should be managed by mComposition*/
    QgsComposerItem( qreal x, qreal y, qreal width, qreal height, QgsComposition* composition /TransferThis/, bool manageZValue = true );
    virtual ~QgsComposerItem();

    /** Return correct graphics item type. */
    virtual int type() const;

    /** Returns whether this item has been removed from the composition. Items removed
     * from the composition are not deleted so that they can be restored via an undo
     * command.
     * @returns true if the item has been removed from the composition
     * @note added in QGIS 2.5
     * @see setIsRemoved
     */
    virtual bool isRemoved() const;

    /** Sets whether this item has been removed from the composition. Items removed
     * from the composition are not deleted so that they can be restored via an undo
     * command.
     * @param removed set to true if the item has been removed from the composition
     * @note added in QGIS 2.5
     * @see isRemoved
     */
    void setIsRemoved( const bool removed );

    /** \brief Set selected, selected item should be highlighted */
    virtual void setSelected( bool s );

    /** \brief Is selected */
    virtual bool selected() const;

    /** Moves item in canvas coordinates*/
    void move( double dx, double dy );

    /** Move Content of item. Does nothing per default (but implemented in composer map)
       @param dx move in x-direction (canvas coordinates)
       @param dy move in y-direction(canvas coordinates)*/
    virtual void moveContent( double dx, double dy );

    /** Zoom content of item. Does nothing per default (but implemented in composer map)
     * @param delta value from wheel event that describes direction (positive /negative number)
     * @param x x-position of mouse cursor (in item coordinates)
     * @param y y-position of mouse cursor (in item coordinates)
     * @deprecated use zoomContent( double, QPointF, ZoomMode ) instead
    */
    virtual void zoomContent( int delta, double x, double y ) /Deprecated/;

    /** Zoom content of item. Does nothing per default (but implemented in composer map)
     * @param factor zoom factor, where > 1 results in a zoom in and < 1 results in a zoom out
     * @param point item point for zoom center
     * @param mode zoom mode
     * @note added in QGIS 2.5
    */
    virtual void zoomContent( const double factor, const QPointF point, const ZoomMode mode = QgsComposerItem::Zoom );

    /** Gets the page the item is currently on.
     * @returns page number for item, beginning on page 1
     * @see pagePos
     * @see updatePagePos
     * @note this method was added in version 2.4
    */
    int page() const;

    /** Returns the item's position relative to its current page.
     * @returns position relative to the page's top left corner.
     * @see page
     * @see updatePagePos
     * @note this method was added in version 2.4
    */
    QPointF pagePos() const;

    /** Moves the item so that it retains its relative position on the page
     * when the paper size changes.
     * @param newPageWidth new width of the page in mm
     * @param newPageHeight new height of the page in mm
     * @see page
     * @see pagePos
     * @note this method was added in version 2.4
    */
    void updatePagePos( double newPageWidth, double newPageHeight );

    /** Moves the item to a new position (in canvas coordinates)
      @param x item position x (mm)
      @param y item position y (mm)
      @param itemPoint reference point which coincides with specified position
      @param page if page > 0, y is interpreted as relative to the origin of the specified page, if page <= 0, y is in absolute canvas coordinates.
       a page number of 1 corresponds to the first page.
      */
    void setItemPosition( double x, double y, ItemPositionMode itemPoint = UpperLeft, int page = -1 );

    /** Sets item position and width / height in one go
      @param x item position x (mm)
      @param y item position y (mm)
      @param width item width (mm)
      @param height item height (mm)
      @param itemPoint reference point which coincides with specified position
      @param posIncludesFrame set to true if the position and size arguments include the item's frame border
      @param page if page > 0, y is interpreted as relative to the origin of the specified page, if page <= 0, y is in absolute canvas coordinates.
       a page number of 1 corresponds to the first page.
      */
    void setItemPosition( double x, double y, double width, double height, ItemPositionMode itemPoint = UpperLeft, bool posIncludesFrame = false, int page = -1 );

    /** Returns item's last used position mode.
      @note: This property has no effect on actual's item position, which is always the top-left corner. */
    ItemPositionMode lastUsedPositionMode();

    /** Sets this items bound in scene coordinates such that 1 item size units
     corresponds to 1 scene size unit*/
    virtual void setSceneRect( const QRectF& rectangle );

    /** Writes parameter that are not subclass specific in document. Usually called from writeXML methods of subclasses*/
    bool _writeXML( QDomElement& itemElem, QDomDocument& doc ) const;

    /** Reads parameter that are not subclass specific in document. Usually called from readXML methods of subclasses*/
    bool _readXML( const QDomElement& itemElem, const QDomDocument& doc );

    /** Whether this item has a frame or not.
     * @returns true if there is a frame around this item, otherwise false.
     * @see setFrameEnabled
     * @see frameOutlineWidth
     * @see frameJoinStyle
     * @see frameOutlineColor
     */
    bool hasFrame() const;

    /** Set whether this item has a frame drawn around it or not.
     * @param drawFrame draw frame
     * @see hasFrame
     * @see setFrameOutlineWidth
     * @see setFrameJoinStyle
     * @see setFrameOutlineColor
     */
    virtual void setFrameEnabled( const bool drawFrame );

    /** Sets frame outline color
     * @param color new color for outline frame
     * @note introduced in 2.6
     * @see frameOutlineColor
     * @see setFrameEnabled
     * @see setFrameJoinStyle
     * @see setFrameOutlineWidth
     */
    virtual void setFrameOutlineColor( const QColor& color );

    /** Returns the frame's outline color. Only used if hasFrame is true.
     * @returns frame outline color
     * @note introduced in 2.6
     * @see hasFrame
     * @see setFrameOutlineColor
     * @see frameJoinStyle
     * @see setFrameOutlineColor
     */
    QColor frameOutlineColor() const;

    /** Sets frame outline width
     * @param outlineWidth new width for outline frame
     * @note introduced in 2.2
     * @see frameOutlineWidth
     * @see setFrameEnabled
     * @see setFrameJoinStyle
     * @see setFrameOutlineColor
     */
    virtual void setFrameOutlineWidth( const double outlineWidth );

    /** Returns the frame's outline width. Only used if hasFrame is true.
     * @returns Frame outline width
     * @note introduced in 2.3
     * @see hasFrame
     * @see setFrameOutlineWidth
     * @see frameJoinStyle
     * @see frameOutlineColor
     */
    double frameOutlineWidth() const;

    /** Returns the join style used for drawing the item's frame
     * @returns Join style for outline frame
     * @note introduced in 2.3
     * @see hasFrame
     * @see setFrameJoinStyle
     * @see frameOutlineWidth
     * @see frameOutlineColor
     */
    Qt::PenJoinStyle frameJoinStyle() const;

    /** Sets join style used when drawing the item's frame
     * @param style Join style for outline frame
     * @note introduced in 2.3
     * @see setFrameEnabled
     * @see frameJoinStyle
     * @see setFrameOutlineWidth
     * @see setFrameOutlineColor
     */
    void setFrameJoinStyle( const Qt::PenJoinStyle style );

    /** Returns the estimated amount the item's frame bleeds outside the item's
     * actual rectangle. For instance, if the item has a 2mm frame outline, then
     * 1mm of this frame is drawn outside the item's rect. In this case the
     * return value will be 1.0
     * @note introduced in 2.2
     * @see rectWithFrame
     */
    virtual double estimatedFrameBleed() const;

    /** Returns the item's rectangular bounds, including any bleed caused by the item's frame.
     *  The bounds are returned in the item's coordinate system (see Qt's QGraphicsItem docs for
     *  more details about QGraphicsItem coordinate systems). The results differ from Qt's rect()
     *  function, as rect() makes no allowances for the portion of outlines which are drawn
     *  outside of the item.
     * @note introduced in 2.2
     * @see estimatedFrameBleed
     */
    virtual QRectF rectWithFrame() const;

    /** Whether this item has a Background or not.
     * @returns true if there is a Background around this item, otherwise false.
     * @see setBackgroundEnabled
     * @see backgroundColor
     */
    bool hasBackground() const;

    /** Set whether this item has a Background drawn around it or not.
     * @param drawBackground draw Background
     * @returns nothing
     * @see hasBackground
     * @see setBackgroundColor
     */
    void setBackgroundEnabled( const bool drawBackground );

    /** Gets the background color for this item
     * @returns background color
     * @see setBackgroundColor
     * @see hasBackground
     */
    QColor backgroundColor() const;

    /** Sets the background color for this item
     * @param backgroundColor new background color
     * @returns nothing
     * @see backgroundColor
     * @see setBackgroundEnabled
     */
    void setBackgroundColor( const QColor& backgroundColor );

    /** Returns the item's composition blending mode.
     * @returns item blending mode
     * @see setBlendMode
     */
    QPainter::CompositionMode blendMode() const;

    /** Sets the item's composition blending mode
     * @param blendMode blending mode for item
     * @see blendMode
     */
    void setBlendMode( const QPainter::CompositionMode blendMode );

    /** Returns the item's transparency
     * @returns transparency as integer between 0 (transparent) and 255 (opaque)
     * @see setTransparency
     */
    int transparency() const;

    /** Sets the item's transparency
     * @param transparency integer between 0 (transparent) and 255 (opaque)
     * @see transparency
     */
    void setTransparency( const int transparency );

    /** Returns whether effects (eg blend modes) are enabled for the item
     * @returns true if effects are enabled
     * @see setEffectsEnabled
     * @see transparency
     * @see blendMode
    */
    bool effectsEnabled() const;

    /** Sets whether effects (eg blend modes) are enabled for the item
     * @param effectsEnabled set to true to enable effects
     * @see effectsEnabled
     * @see setTransparency
     * @see setBlendMode
    */
    void setEffectsEnabled( const bool effectsEnabled );

    /** Composite operations for item groups do nothing per default*/
    virtual void addItem( QgsComposerItem* item );
    virtual void removeItems();

    virtual void beginItemCommand( const QString& text );

    /** Starts new composer undo command
      @param commandText command title
      @param c context for mergeable commands (unknown for non-mergeable commands*/
    void beginCommand( const QString& commandText, QgsComposerMergeCommand::Context c = QgsComposerMergeCommand::Unknown );

    virtual void endItemCommand();
    /** Finish current command and push it onto the undo stack */
    void endCommand();
    void cancelCommand();

    //functions that encapsulate the workaround for the Qt font bug (that is to scale the font size up and then scale the
    //painter down by the same factor for drawing

    /** Draws Text. Takes care about all the composer specific issues (calculation to pixel, scaling of font and painter
     * to work around the Qt font bug)
     * @deprecated use QgsComposerUtils::drawText instead
    */
    void drawText( QPainter* p, double x, double y, const QString& text, const QFont& font ) const /Deprecated/;

    /** Like the above, but with a rectangle for multiline text
     * @param p painter to use
     * @param rect rectangle to draw into
     * @param text text to draw
     * @param font font to use
     * @param halignment optional horizontal alignment
     * @param valignment optional vertical alignment
     * @param flags allows for passing Qt::TextFlags to control appearance of rendered text
     * @deprecated use QgsComposerUtils::drawText instead
    */
    void drawText( QPainter* p, const QRectF& rect, const QString& text, const QFont& font, Qt::AlignmentFlag halignment = Qt::AlignLeft, Qt::AlignmentFlag valignment = Qt::AlignTop, int flags = Qt::TextWordWrap ) const /Deprecated/;

    /** Returns the font width in millimeters (considers upscaling and downscaling with FONT_WORKAROUND_SCALE
     * @deprecated use QgsComposerUtils::textWidthMM instead
    */
    double textWidthMillimeters( const QFont& font, const QString& text ) const /Deprecated/;

    /** Returns the font height of a character in millimeters
     * @deprecated use QgsComposerUtils::fontHeightCharacterMM instead
     */
    double fontHeightCharacterMM( const QFont& font, const QChar& c ) const /Deprecated/;

    /** Returns the font ascent in Millimeters (considers upscaling and downscaling with FONT_WORKAROUND_SCALE
     * @deprecated use QgsComposerUtils::fontAscentMM instead
     */
    double fontAscentMillimeters( const QFont& font ) const /Deprecated/;

    /** Returns the font descent in Millimeters (considers upscaling and downscaling with FONT_WORKAROUND_SCALE
     * @deprecated use QgsComposerUtils::fontDescentMM instead
     */
    double fontDescentMillimeters( const QFont& font ) const /Deprecated/;

    /** Returns the font height in Millimeters (considers upscaling and downscaling with FONT_WORKAROUND_SCALE.
     * Font height equals the font ascent+descent+1 (for baseline).
     * @note Added in version 2.4
     * @deprecated use QgsComposerUtils::fontHeightMM instead
    */
    double fontHeightMillimeters( const QFont& font ) const /Deprecated/;

    /** Calculates font size in mm from a font point size
     * @deprecated use QgsComposerUtils::mmFontSize instead
     */
    double pixelFontSize( double pointSize ) const /Deprecated/;

    /** Returns a font where size is in pixel and font size is upscaled with FONT_WORKAROUND_SCALE
     * @deprecated use QgsComposerUtils::scaledFontPixelSize instead
     */
    QFont scaledFontPixelSize( const QFont& font ) const /Deprecated/;

    /** Locks / unlocks the item position for mouse drags
     * @param lock set to true to prevent item movement and resizing via the mouse
     * @see positionLock
     */
    void setPositionLock( const bool lock );

    /** Returns whether position lock for mouse drags is enabled
     * returns true if item is locked for mouse movement and resizing
     * @see setPositionLock
    */
    bool positionLock() const;

    /** Returns the current rotation for the composer item.
     * @returns rotation for composer item
     * @param valueType controls whether the returned value is the user specified rotation,
     * or the current evaluated rotation (which may be affected by data driven rotation
     * settings).
     * @note this method was added in version 2.1
     * @see setItemRotation
     */
    double itemRotation( const QgsComposerObject::PropertyValueType valueType = QgsComposerObject::EvaluatedValue ) const;

    /** Returns the rotation for the composer item
     * @deprecated Use itemRotation()
     *             instead
     */
    double rotation() const /Deprecated/;

    /** Updates item, with the possibility to do custom update for subclasses*/
    virtual void updateItem();

    /** Get item's id (which is not necessarly unique)
     * @returns item id
     * @see setId
     */
    QString id() const;

    /** Set item's id (which is not necessarly unique)
     * @param id new id for item
     * @see id
     */
    void setId( const QString& id );

    /** Get item identification name
     * @returns unique item identification string
     * @note there is not setter since one can't manually set the id
     * @see id
     * @see setId
    */
    QString uuid() const;

    /** Get item display name. This is the item's id if set, and if
     * not, a user-friendly string identifying item type.
     * @returns display name for item
     * @see id
     * @see setId
     * @note added in version 2.5
    */
    virtual QString displayName() const;

    /** Sets visibility for item.
     * @param visible set to true to show item, false to hide item
     * @note QGraphicsItem::setVisible should not be called directly
     * on a QgsComposerItem, as some item types (eg groups) need to override
     * the visibility toggle.
     * @note added in version 2.5
    */
    virtual void setVisibility( const bool visible );

    /** Returns whether the item should be excluded from composer exports and prints
     * @param valueType controls whether the returned value is the user specified vaule,
     * or the current evaluated value (which may be affected by data driven settings).
     * @returns true if item should be excluded
     * @note added in version 2.5
     * @see setExcludeFromExports
     */
    bool excludeFromExports( const QgsComposerObject::PropertyValueType valueType = QgsComposerObject::EvaluatedValue );

    /** Sets whether the item should be excluded from composer exports and prints
     * @param exclude set to true to exclude the item from exports
     * @note added in version 2.5
     * @see excludeFromExports
     */
    virtual void setExcludeFromExports( const bool exclude );

    /** Returns whether this item is part of a group
     * @returns true if item is in a group
     * @note added in version 2.5
     * @see setIsGroupMember
    */
    bool isGroupMember() const;

    /** Sets whether this item is part of a group
     * @param isGroupMember set to true if item is in a group
     * @note added in version 2.5
     * @see isGroupMember
    */
    void setIsGroupMember( const bool isGroupMember );

    /** Get the number of layers that this item requires for exporting as layers
     * @returns 0 if this item is to be placed on the same layer as the previous item,
     * 1 if it should be placed on its own layer, and >1 if it requires multiple export layers
     * @note this method was added in version 2.4
     * @see setCurrentExportLayer
    */
    int numberExportLayers() const;

    /** Sets the current layer to draw for exporting
     * @param layerIdx can be set to -1 to draw all item layers, and must be less than numberExportLayers()
     * @note this method was added in version 2.4
     * @see numberExportLayers
    */
    void setCurrentExportLayer( const int layerIdx = -1 );

  public slots:
    /** Sets the item rotation
     * @deprecated Use setItemRotation( double rotation ) instead
     */
    virtual void setRotation( double r );

    /** Sets the item rotation
     * @param r item rotation in degrees
     * @param adjustPosition set to true if item should be shifted so that rotation occurs
     * around item center. If false, rotation occurs around item origin
     * @note this method was added in version 2.1
     * @see itemRotation
    */
    virtual void setItemRotation( const double r, const bool adjustPosition = false );

    void repaint();

    /** Refreshes a data defined property for the item by reevaluating the property's value
     * and redrawing the item with this new value.
     * @param property data defined property to refresh. If property is set to
     * QgsComposerItem::AllProperties then all data defined properties for the item will be
     * refreshed.
     * @note this method was added in version 2.5
    */
    virtual void refreshDataDefinedProperty( const QgsComposerObject::DataDefinedProperty property = QgsComposerObject::AllProperties );

  protected:

    //event handlers
    virtual void mouseMoveEvent( QGraphicsSceneMouseEvent * event );
    virtual void mousePressEvent( QGraphicsSceneMouseEvent * event );
    virtual void mouseReleaseEvent( QGraphicsSceneMouseEvent * event );

    virtual void hoverMoveEvent( QGraphicsSceneHoverEvent * event );

    /** Draws additional graphics on selected items. The base implementation has
     * no effect.
    */
    virtual void drawSelectionBoxes( QPainter* p );

    /** Draw black frame around item*/
    virtual void drawFrame( QPainter* p );

    /** Draw background*/
    virtual void drawBackground( QPainter* p );

    /** Draws arrowhead
     * @deprecated use QgsComposerUtils::drawArrowHead instead
     */
    void drawArrowHead( QPainter* p, double x, double y, double angle, double arrowHeadWidth ) const /Deprecated/;

    /** Returns angle of the line from p1 to p2 (clockwise, starting at N)*/
    double angle( const QPointF& p1, const QPointF& p2 ) const /Deprecated/;

    /** Returns the current (zoom level dependent) tolerance to decide if mouse position is close enough to the
    item border for resizing*/
    double rectHandlerBorderTolerance() const;

    /** Returns the size of the lock symbol depending on the composer zoom level and the item size
    * @deprecated will be removed in QGIS 3.0
    */
    double lockSymbolSize() const /Deprecated/;

    /** Returns the zoom factor of the graphics view.
      @return the factor or -1 in case of error (e.g. graphic view does not exist)
    */
    double horizontalViewScaleFactor() const;

    //some utility functions

    /** Calculates width and hight of the picture (in mm) such that it fits into the item frame with the given rotation.
     * @deprecated will be removed in QGIS 3.0
    */
    bool imageSizeConsideringRotation( double& width, double& height, double rotation ) const /Deprecated/;

    /** Calculates width and hight of the picture (in mm) such that it fits into the item frame with the given rotation
     * @deprecated will be removed in QGIS 3.0
     */
    bool imageSizeConsideringRotation( double& width, double& height ) const /Deprecated/;

    /** Calculates the largest scaled version of originalRect which fits within boundsRect, when it is rotated by
      * a specified amount
      * @param originalRect QRectF to be rotated and scaled
      * @param boundsRect QRectF specifying the bounds which the rotated and scaled rectangle must fit within
      * @param rotation the rotation in degrees to be applied to the rectangle
      * @deprecated use QgsComposerUtils::largestRotatedRectWithinBounds instead
      */
    QRectF largestRotatedRectWithinBounds( QRectF originalRect, QRectF boundsRect, double rotation ) const /Deprecated/;

    /** Calculates corner point after rotation and scaling
     * @deprecated will be removed in QGIS 3.0
    */
    bool cornerPointOnRotatedAndScaledRect( double& x, double& y, double width, double height, double rotation ) const /Deprecated/;

    /** Calculates corner point after rotation and scaling
     * @deprecated will be removed in QGIS 3.0
    */
    bool cornerPointOnRotatedAndScaledRect( double& x, double& y, double width, double height ) const /Deprecated/;

    /** Calculates width / height of the bounding box of a rotated rectangle
     * @deprecated will be removed in QGIS 3.0
    */
    void sizeChangedByRotation( double& width, double& height, double rotation ) /Deprecated/;

    /** Calculates width / height of the bounding box of a rotated rectangle
    * @deprecated will be removed in QGIS 3.0
    */
    void sizeChangedByRotation( double& width, double& height ) /Deprecated/;

    /** Rotates a point / vector
     * @param angle rotation angle in degrees, counterclockwise
     * @param x in/out: x coordinate before / after the rotation
     * @param y in/out: y cooreinate before / after the rotation
     * @deprecated use QgsComposerUtils:rotate instead
    */
    void rotate( double angle, double& x, double& y ) const /Deprecated/;

    /** Return horizontal align snap item. Creates a new graphics line if 0*/
    QGraphicsLineItem* hAlignSnapItem();
    void deleteHAlignSnapItem();
    /** Return vertical align snap item. Creates a new graphics line if 0*/
    QGraphicsLineItem* vAlignSnapItem();
    void deleteVAlignSnapItem();
    void deleteAlignItems();

    /** Evaluates an item's bounding rect to consider data defined position and size of item
     * and reference point
     * @param newRect target bouding rect for item
     * @param resizeOnly set to true if the item is only being resized. If true then
     * the position of the returned rect will be adjusted to account for the item's
     * position mode
     * @returns bounding box rectangle for item after data defined size and position have been
     * set and position mode has been accounted for
     * @note added in QGIS 2.5
    */
    QRectF evalItemRect( const QRectF &newRect, const bool resizeOnly = false );

    /** Returns whether the item should be drawn in the current context
     * @returns true if item should be drawn
     * @note added in QGIS 2.5
    */
    bool shouldDrawItem() const;

  signals:
    /** Is emitted on item rotation change*/
    void itemRotationChanged( double newRotation );
    /** Emitted if the rectangle changes*/
    void sizeChanged();
    /** Emitted if the item's frame style changes
     * @note: this function was introduced in version 2.2
    */
    void frameChanged();
    /** Emitted if the item's lock status changes
     * @note: this function was introduced in version 2.5
    */
    void lockChanged();

};
