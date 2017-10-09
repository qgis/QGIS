/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposeritem.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsComposerItem: QgsComposerObject, QGraphicsRectItem
{
%Docstring
 A item that forms part of a map composition.
%End

%TypeHeaderCode
#include "qgscomposeritem.h"
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
%End
%ConvertToSubClassCode
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

%End
  public:

    enum ItemType
    {
      // base
      ComposerItem,

      // derived
      ComposerArrow,
      ComposerItemGroup,
      ComposerLabel,
      ComposerLegend,
      ComposerMap,
      ComposerPaper,
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


    enum ZoomMode
    {
      Zoom,
      ZoomRecenter,
      ZoomToPoint,
      NoZoom
    };

    QgsComposerItem( QgsComposition *composition /TransferThis/, bool manageZValue = true );
%Docstring
 Constructor
\param composition parent composition
\param manageZValue true if the z-Value of this object should be managed by mComposition*
%End

    QgsComposerItem( qreal x, qreal y, qreal width, qreal height, QgsComposition *composition /TransferThis/, bool manageZValue = true );
%Docstring
 Constructor with box position and composer object
\param x x coordinate of item
\param y y coordinate of item
\param width width of item
\param height height of item
\param composition parent composition
\param manageZValue true if the z-Value of this object should be managed by mComposition*
%End
    virtual ~QgsComposerItem();

    virtual int type() const;
%Docstring
Return correct graphics item type.
 :rtype: int
%End

    virtual bool isRemoved() const;
%Docstring
 Returns whether this item has been removed from the composition. Items removed
 from the composition are not deleted so that they can be restored via an undo
 command.
 :return: true if the item has been removed from the composition
.. versionadded:: 2.5
.. seealso:: setIsRemoved
 :rtype: bool
%End

    void setIsRemoved( const bool removed );
%Docstring
 Sets whether this item has been removed from the composition. Items removed
 from the composition are not deleted so that they can be restored via an undo
 command.
 \param removed set to true if the item has been removed from the composition
.. versionadded:: 2.5
.. seealso:: isRemoved
%End

    virtual void setSelected( bool s );
%Docstring
 Set selected, selected item should be highlighted
%End

    virtual bool selected() const;
%Docstring
 Is selected
 :rtype: bool
%End

    void move( double dx, double dy );
%Docstring
Moves item in canvas coordinates
%End

    virtual void moveContent( double dx, double dy );
%Docstring
 Move Content of item. Does nothing per default (but implemented in composer map)
\param dx move in x-direction (canvas coordinates)
\param dy move in y-direction(canvas coordinates)*
%End

    virtual void zoomContent( const double factor, const QPointF point, const ZoomMode mode = QgsComposerItem::Zoom );
%Docstring
 Zoom content of item. Does nothing per default (but implemented in composer map)
 \param factor zoom factor, where > 1 results in a zoom in and < 1 results in a zoom out
 \param point item point for zoom center
 \param mode zoom mode
.. versionadded:: 2.5
%End

    int page() const;
%Docstring
 Gets the page the item is currently on.
 :return: page number for item, beginning on page 1
.. seealso:: pagePos
.. seealso:: updatePagePos
.. versionadded:: 2.4
 :rtype: int
%End

    QPointF pagePos() const;
%Docstring
 Returns the item's position relative to its current page.
 :return: position relative to the page's top left corner.
.. seealso:: page
.. seealso:: updatePagePos
.. versionadded:: 2.4
 :rtype: QPointF
%End

    void updatePagePos( double newPageWidth, double newPageHeight );
%Docstring
 Moves the item so that it retains its relative position on the page
 when the paper size changes.
 \param newPageWidth new width of the page in mm
 \param newPageHeight new height of the page in mm
.. seealso:: page
.. seealso:: pagePos
.. versionadded:: 2.4
%End

    void setItemPosition( double x, double y, ItemPositionMode itemPoint = UpperLeft, int page = -1 );
%Docstring
 Moves the item to a new position (in canvas coordinates)
\param x item position x (mm)
\param y item position y (mm)
\param itemPoint reference point which coincides with specified position
\param page if page > 0, y is interpreted as relative to the origin of the specified page, if page <= 0, y is in absolute canvas coordinates.
a page number of 1 corresponds to the first page.
%End

    void setItemPosition( double x, double y, double width, double height, ItemPositionMode itemPoint = UpperLeft, bool posIncludesFrame = false, int page = -1 );
%Docstring
 Sets item position and width / height in one go
\param x item position x (mm)
\param y item position y (mm)
\param width item width (mm)
\param height item height (mm)
\param itemPoint reference point which coincides with specified position
\param posIncludesFrame set to true if the position and size arguments include the item's frame stroke
\param page if page > 0, y is interpreted as relative to the origin of the specified page, if page <= 0, y is in absolute canvas coordinates.
a page number of 1 corresponds to the first page.
%End

    ItemPositionMode lastUsedPositionMode();
%Docstring
 Returns item's last used position mode.
\note: This property has no effect on actual's item position, which is always the top-left corner. *
 :rtype: ItemPositionMode
%End

    virtual void setSceneRect( const QRectF &rectangle );
%Docstring
 Sets this items bound in scene coordinates such that 1 item size units
corresponds to 1 scene size unit*
%End

    bool _writeXml( QDomElement &itemElem, QDomDocument &doc ) const;
%Docstring
Writes parameter that are not subclass specific in document. Usually called from writeXml methods of subclasses
 :rtype: bool
%End

    bool _readXml( const QDomElement &itemElem, const QDomDocument &doc );
%Docstring
Reads parameter that are not subclass specific in document. Usually called from readXml methods of subclasses
 :rtype: bool
%End

    bool hasFrame() const;
%Docstring
 Whether this item has a frame or not.
 :return: true if there is a frame around this item, otherwise false.
.. seealso:: setFrameEnabled
.. seealso:: frameStrokeWidth
.. seealso:: frameJoinStyle
.. seealso:: frameStrokeColor
 :rtype: bool
%End

    virtual void setFrameEnabled( const bool drawFrame );
%Docstring
 Set whether this item has a frame drawn around it or not.
 \param drawFrame draw frame
.. seealso:: hasFrame
.. seealso:: setFrameStrokeWidth
.. seealso:: setFrameJoinStyle
.. seealso:: setFrameStrokeColor
%End

    virtual void setFrameStrokeColor( const QColor &color );
%Docstring
 Sets frame stroke color
 \param color new color for stroke frame
.. versionadded:: 2.6
.. seealso:: frameStrokeColor
.. seealso:: setFrameEnabled
.. seealso:: setFrameJoinStyle
.. seealso:: setFrameStrokeWidth
%End

    QColor frameStrokeColor() const;
%Docstring
 Returns the frame's stroke color. Only used if hasFrame is true.
 :return: frame stroke color
.. versionadded:: 2.6
.. seealso:: hasFrame
.. seealso:: setFrameStrokeColor
.. seealso:: frameJoinStyle
.. seealso:: setFrameStrokeColor
 :rtype: QColor
%End

    virtual void setFrameStrokeWidth( const double strokeWidth );
%Docstring
 Sets frame stroke width
 \param strokeWidth new width for stroke frame
.. versionadded:: 2.2
.. seealso:: frameStrokeWidth
.. seealso:: setFrameEnabled
.. seealso:: setFrameJoinStyle
.. seealso:: setFrameStrokeColor
%End

    double frameStrokeWidth() const;
%Docstring
 Returns the frame's stroke width. Only used if hasFrame is true.
 :return: Frame stroke width
.. versionadded:: 2.3
.. seealso:: hasFrame
.. seealso:: setFrameStrokeWidth
.. seealso:: frameJoinStyle
.. seealso:: frameStrokeColor
 :rtype: float
%End

    Qt::PenJoinStyle frameJoinStyle() const;
%Docstring
 Returns the join style used for drawing the item's frame
 :return: Join style for stroke frame
.. versionadded:: 2.3
.. seealso:: hasFrame
.. seealso:: setFrameJoinStyle
.. seealso:: frameStrokeWidth
.. seealso:: frameStrokeColor
 :rtype: Qt.PenJoinStyle
%End

    void setFrameJoinStyle( const Qt::PenJoinStyle style );
%Docstring
 Sets join style used when drawing the item's frame
 \param style Join style for stroke frame
.. versionadded:: 2.3
.. seealso:: setFrameEnabled
.. seealso:: frameJoinStyle
.. seealso:: setFrameStrokeWidth
.. seealso:: setFrameStrokeColor
%End

    virtual double estimatedFrameBleed() const;
%Docstring
 Returns the estimated amount the item's frame bleeds outside the item's
 actual rectangle. For instance, if the item has a 2mm frame stroke, then
 1mm of this frame is drawn outside the item's rect. In this case the
 return value will be 1.0
.. versionadded:: 2.2
.. seealso:: rectWithFrame
 :rtype: float
%End

    virtual QRectF rectWithFrame() const;
%Docstring
 Returns the item's rectangular bounds, including any bleed caused by the item's frame.
  The bounds are returned in the item's coordinate system (see Qt's QGraphicsItem docs for
  more details about QGraphicsItem coordinate systems). The results differ from Qt's rect()
  function, as rect() makes no allowances for the portion of outlines which are drawn
  outside of the item.
.. versionadded:: 2.2
.. seealso:: estimatedFrameBleed
 :rtype: QRectF
%End

    bool hasBackground() const;
%Docstring
 Whether this item has a Background or not.
 :return: true if there is a Background around this item, otherwise false.
.. seealso:: setBackgroundEnabled
.. seealso:: backgroundColor
 :rtype: bool
%End

    void setBackgroundEnabled( const bool drawBackground );
%Docstring
 Set whether this item has a Background drawn around it or not.
 \param drawBackground draw Background
 :return: nothing
.. seealso:: hasBackground
.. seealso:: setBackgroundColor
%End

    QColor backgroundColor() const;
%Docstring
 Gets the background color for this item
 :return: background color
.. seealso:: setBackgroundColor
.. seealso:: hasBackground
 :rtype: QColor
%End

    void setBackgroundColor( const QColor &backgroundColor );
%Docstring
 Sets the background color for this item
 \param backgroundColor new background color
 :return: nothing
.. seealso:: backgroundColor
.. seealso:: setBackgroundEnabled
%End

    QPainter::CompositionMode blendMode() const;
%Docstring
 Returns the item's composition blending mode.
 :return: item blending mode
.. seealso:: setBlendMode
 :rtype: QPainter.CompositionMode
%End

    void setBlendMode( const QPainter::CompositionMode blendMode );
%Docstring
 Sets the item's composition blending mode
 \param blendMode blending mode for item
.. seealso:: blendMode
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

    void setItemOpacity( const double opacity );
%Docstring
 Sets the item's ``opacity``. This method should be used instead of
 QGraphicsItem.setOpacity() as any data defined overrides will be
 respected.
 \param opacity double between 1.0 (opaque) and 0 (transparent).
.. seealso:: itemOpacity()
%End

    bool effectsEnabled() const;
%Docstring
 Returns whether effects (e.g., blend modes) are enabled for the item
 :return: true if effects are enabled
.. seealso:: setEffectsEnabled
.. seealso:: itemOpacity()
.. seealso:: blendMode
 :rtype: bool
%End

    void setEffectsEnabled( const bool effectsEnabled );
%Docstring
 Sets whether effects (e.g., blend modes) are enabled for the item
 \param effectsEnabled set to true to enable effects
.. seealso:: effectsEnabled
.. seealso:: setItemOpacity()
.. seealso:: setBlendMode
%End

    virtual void addItem( QgsComposerItem *item );
%Docstring
Composite operations for item groups do nothing per default
%End
    virtual void removeItems();

    virtual void beginItemCommand( const QString &text );

    void beginCommand( const QString &commandText, QgsComposerMergeCommand::Context c = QgsComposerMergeCommand::Unknown );
%Docstring
 Starts new composer undo command
\param commandText command title
\param c context for mergeable commands (unknown for non-mergeable commands*
%End

    virtual void endItemCommand();
    void endCommand();
%Docstring
Finish current command and push it onto the undo stack
%End
    void cancelCommand();


    void setPositionLock( const bool lock );
%Docstring
 Locks / unlocks the item position for mouse drags
 \param lock set to true to prevent item movement and resizing via the mouse
.. seealso:: positionLock
%End

    bool positionLock() const;
%Docstring
 Returns whether position lock for mouse drags is enabled
 returns true if item is locked for mouse movement and resizing
.. seealso:: setPositionLock
 :rtype: bool
%End

    double itemRotation( const QgsComposerObject::PropertyValueType valueType = QgsComposerObject::EvaluatedValue ) const;
%Docstring
 Returns the current rotation for the composer item, in degrees clockwise.
 \param valueType controls whether the returned value is the user specified rotation,
 or the current evaluated rotation (which may be affected by data driven rotation
 settings).
.. versionadded:: 2.1
.. seealso:: setItemRotation()
 :rtype: float
%End

    virtual void updateItem();
%Docstring
 Updates (redraws) the item, with the possibility to do custom update for subclasses.
 Subclasses should check updatesEnabled() to determine whether updates are
 currently permitted for the item.
%End

    QString id() const;
%Docstring
 Get item's id (which is not necessarly unique)
 :return: item id
.. seealso:: setId
 :rtype: str
%End

    virtual void setId( const QString &id );
%Docstring
 Set item's id (which is not necessarly unique)
 \param id new id for item
.. seealso:: id
%End

    QString uuid() const;
%Docstring
 Get item identification name
 :return: unique item identification string
.. note::

   there is not setter since one can't manually set the id
.. seealso:: id
.. seealso:: setId
 :rtype: str
%End

    virtual QString displayName() const;
%Docstring
 Get item display name. This is the item's id if set, and if
 not, a user-friendly string identifying item type.
 :return: display name for item
.. seealso:: id
.. seealso:: setId
.. versionadded:: 2.5
 :rtype: str
%End

    virtual void setVisibility( const bool visible );
%Docstring
 Sets visibility for item.
 \param visible set to true to show item, false to hide item
.. note::

   QGraphicsItem.setVisible should not be called directly
 on a QgsComposerItem, as some item types (e.g., groups) need to override
 the visibility toggle.
.. versionadded:: 2.5
%End

    bool excludeFromExports( const QgsComposerObject::PropertyValueType valueType = QgsComposerObject::EvaluatedValue );
%Docstring
 Returns whether the item should be excluded from composer exports and prints
 \param valueType controls whether the returned value is the user specified value,
 or the current evaluated value (which may be affected by data driven settings).
 :return: true if item should be excluded
.. versionadded:: 2.5
.. seealso:: setExcludeFromExports
 :rtype: bool
%End

    virtual void setExcludeFromExports( const bool exclude );
%Docstring
 Sets whether the item should be excluded from composer exports and prints
 \param exclude set to true to exclude the item from exports
.. versionadded:: 2.5
.. seealso:: excludeFromExports
%End

    bool isGroupMember() const;
%Docstring
 Returns whether this item is part of a group
 :return: true if item is in a group
.. versionadded:: 2.5
.. seealso:: setIsGroupMember
 :rtype: bool
%End

    void setIsGroupMember( const bool isGroupMember );
%Docstring
 Sets whether this item is part of a group
 \param isGroupMember set to true if item is in a group
.. versionadded:: 2.5
.. seealso:: isGroupMember
%End

    virtual int numberExportLayers() const;
%Docstring
 Get the number of layers that this item requires for exporting as layers
 :return: 0 if this item is to be placed on the same layer as the previous item,
 1 if it should be placed on its own layer, and >1 if it requires multiple export layers
.. versionadded:: 2.4
.. seealso:: setCurrentExportLayer
 :rtype: int
%End

    virtual void setCurrentExportLayer( const int layerIdx = -1 );
%Docstring
 Sets the current layer to draw for exporting
 \param layerIdx can be set to -1 to draw all item layers, and must be less than numberExportLayers()
.. versionadded:: 2.4
.. seealso:: numberExportLayers
%End

    virtual QgsExpressionContext createExpressionContext() const;
%Docstring
 Creates an expression context relating to the item's current state. The context includes
 scopes for global, project, composition, atlas and item properties.
.. versionadded:: 2.12
 :rtype: QgsExpressionContext
%End

    void setUpdatesEnabled( bool enabled );
%Docstring
 Sets whether updates to the item are enabled. If false,
 the item will not be redrawn. This can be used to prevent
 multiple item updates when many settings for an item are
 changed sequentially.
.. versionadded:: 3.0
.. seealso:: updatesEnabled()
%End

    bool updatesEnabled() const;
%Docstring
 Returns whether updates to the item are enabled. If false,
 the item will not be redrawn. This can be used to prevent
 multiple item updates when many settings for an item are
 changed sequentially.
.. versionadded:: 3.0
.. seealso:: setUpdatesEnabled()
 :rtype: bool
%End

  public slots:

    virtual void setItemRotation( const double rotation, const bool adjustPosition = false );
%Docstring
 Sets the item ``rotation``, in degrees clockwise.
 \param rotation item rotation in degrees
 \param adjustPosition set to true if item should be shifted so that rotation occurs
 around item center. If false, rotation occurs around item origin
.. versionadded:: 2.1
.. seealso:: itemRotation
%End

    virtual void repaint();


    virtual void refreshDataDefinedProperty( const QgsComposerObject::DataDefinedProperty property = QgsComposerObject::AllProperties, const QgsExpressionContext *context = 0 );
%Docstring
 Refreshes a data defined property for the item by reevaluating the property's value
 and redrawing the item with this new value.
 \param property data defined property to refresh. If property is set to
 QgsComposerItem.AllProperties then all data defined properties for the item will be
 refreshed.
 \param context expression context for evaluating data defined expressions
.. versionadded:: 2.5
%End

  protected:
















    virtual void drawSelectionBoxes( QPainter *p );
%Docstring
 Draws additional graphics on selected items. The base implementation has
 no effect.
%End

    virtual void drawFrame( QPainter *p );
%Docstring
Draw black frame around item
%End

    virtual void drawBackground( QPainter *p );
%Docstring
Draw background
%End

    double rectHandlerBorderTolerance() const;
%Docstring
 Returns the current (zoom level dependent) tolerance to decide if mouse position is close enough to the
item border for resizing*
 :rtype: float
%End

    double horizontalViewScaleFactor() const;
%Docstring
 Returns the zoom factor of the graphics view.
 :return: the factor or -1 in case of error (e.g. graphic view does not exist)
 :rtype: float
%End


    QGraphicsLineItem *hAlignSnapItem();
%Docstring
Return horizontal align snap item. Creates a new graphics line if 0
 :rtype: QGraphicsLineItem
%End
    void deleteHAlignSnapItem();
    QGraphicsLineItem *vAlignSnapItem();
%Docstring
Return vertical align snap item. Creates a new graphics line if 0
 :rtype: QGraphicsLineItem
%End
    void deleteVAlignSnapItem();
    void deleteAlignItems();

    QRectF evalItemRect( const QRectF &newRect, const bool resizeOnly = false, const QgsExpressionContext *context = 0 );
%Docstring
 Evaluates an item's bounding rect to consider data defined position and size of item
 and reference point
 \param newRect target bounding rect for item
 \param resizeOnly set to true if the item is only being resized. If true then
 the position of the returned rect will be adjusted to account for the item's
 position mode
 \param context expression context for evaluating data defined expressions
 :return: bounding box rectangle for item after data defined size and position have been
 set and position mode has been accounted for
.. versionadded:: 2.5
 :rtype: QRectF
%End

    bool shouldDrawItem() const;
%Docstring
 Returns whether the item should be drawn in the current context
 :return: true if item should be drawn
.. versionadded:: 2.5
 :rtype: bool
%End

  signals:
    void itemRotationChanged( double newRotation );
%Docstring
Is emitted on item rotation change
%End
    void sizeChanged();
%Docstring
Emitted if the rectangle changes
%End

    void frameChanged();
%Docstring
 Emitted if the item's frame style changes
.. versionadded:: 2.2
%End

    void lockChanged();
%Docstring
 Emitted if the item's lock status changes
.. versionadded:: 2.5
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposeritem.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
