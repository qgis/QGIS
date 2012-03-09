/** \ingroup MapComposer
 * A item that forms part of a map composition.
 */
class QgsComposerItem: QObject, QGraphicsRectItem
{
%TypeHeaderCode
#include <qgscomposeritem.h>
//#include <qgscomposerarrow.h>
//#include <qgscomposeritemgroup.h>
#include <qgscomposerlabel.h>
#include <qgscomposerlegend.h>
#include <qgscomposermap.h>
#include <qgspaperitem.h>
#include <qgscomposerpicture.h>
#include <qgscomposerscalebar.h>
#include <qgscomposershape.h>
#include <qgscomposertable.h>
#include <qgscomposerattributetable.h>
//#include <qgscomposertexttable.h>
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
/*
  case QgsComposerItem::ComposerArrow:
    sipClass = sipClass_QgsComposerArrow;
    *sipCppRet = static_cast<QgsComposerArrow*>(sipCpp);
    break;
  case QgsComposerItem::ComposerItemGroup:
    sipClass = sipClass_QgsComposerItemGroup;
    *sipCppRet = static_cast<QgsComposerItemGroup*>(sipCpp);
    break;
*/
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
/*
  case QgsComposerItem::ComposerTextTable:
    sipClass = sipClass_QgsComposerTextTable;
    *sipCppRet = static_cast<QgsComposerTextTable*>(sipCpp);
    break;
*/
  default:
    sipClass = 0;
  }
%End

  public:

    enum ItemType
    {
      // base class for the items
      ComposerItem = 65636, /* UserType + 100 ... SIP does not do arithmetic */

      // derived classes
      ComposerArrow,
      ComposerItemGroup,
      ComposerLabel,
      ComposerLegend,
      ComposerMap,
      ComposerPaper,  // QgsPaperItem
      ComposerPicture,
      ComposerScaleBar,
      ComposerShape,
      ComposerTable,
      ComposerAttributeTable,
      ComposerTextTable
    };

    /**Describes the action (move or resize in different directon) to be done during mouse move*/
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
      ResizeRightDown
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

    /**Constructor
     @param manageZValue true if the z-Value of this object should be managed by mComposition*/
    QgsComposerItem( QgsComposition* composition /TransferThis/, bool manageZValue = true );
    /**Constructor with box position and composer object
     @param manageZValue true if the z-Value of this object should be managed by mComposition*/
    QgsComposerItem( qreal x, qreal y, qreal width, qreal height, QgsComposition* composition /TransferThis/, bool manageZValue = true );
    virtual ~QgsComposerItem();

    /** return correct graphics item type. Added in v1.7 */
    virtual int type() const;

    /** \brief Set selected, selected item should be highlighted */
    virtual void setSelected( bool s );

    /** \brief Is selected */
    virtual bool selected();

    /** stores state in project */
    virtual bool writeSettings();

    /** read state from project */
    virtual bool readSettings();

    /** delete settings from project file  */
    virtual bool removeSettings();

    /**Moves item in canvas coordinates*/
    void move( double dx, double dy );

    /**Move Content of item. Does nothing per default (but implemented in composer map)
       @param dx move in x-direction (canvas coordinates)
       @param dy move in y-direction(canvas coordinates)*/
    virtual void moveContent( double dx, double dy );

    /**Zoom content of item. Does nothing per default (but implemented in composer map)
     @param delta value from wheel event that describes magnitude and direction (positive /negative number)
    @param x x-position of mouse cursor (in item coordinates)
    @param y y-position of mouse cursor (in item coordinates)*/
    virtual void zoomContent( int delta, double x, double y );

    /**Moves the item to a new position (in canvas coordinates)*/
    void setItemPosition( double x, double y, ItemPositionMode itemPoint = UpperLeft );

    /**Sets item position and width / height in one go
      @note: this method was added in version 1.6*/
    void setItemPosition( double x, double y, double width, double height, ItemPositionMode itemPoint = UpperLeft );

    /**Sets this items bound in scene coordinates such that 1 item size units
     corresponds to 1 scene size unit*/
    virtual void setSceneRect( const QRectF& rectangle );

    /** stores state in Dom node
     * @param node is Dom node corresponding to 'Composer' tag
     * @param temp write template file
     */
    virtual bool writeXML( QDomElement& elem, QDomDocument & doc ) const = 0;

    /**Writes parameter that are not subclass specific in document. Usually called from writeXML methods of subclasses*/
    bool _writeXML( QDomElement& itemElem, QDomDocument& doc ) const;

    /** sets state from Dom document
     * @param itemElem is Dom node corresponding to item tag
     */
    virtual bool readXML( const QDomElement& itemElem, const QDomDocument& doc ) = 0;

    /**Reads parameter that are not subclass specific in document. Usually called from readXML methods of subclasses*/
    bool _readXML( const QDomElement& itemElem, const QDomDocument& doc );



    /** Whether this item has a frame or not.
     * @param none
     * @return boolean - true if there is a frame around this item, otherwise false.
     * @note deprecated since 1.8 don't use!
     * @see hasFrame
     */
    bool frame();
    /** Whether this item has a frame or not.
     * @param none
     * @return boolean - true if there is a frame around this item, otherwise false.
     * @note introduced since 1.8
     * @see hasFrame
     */
    bool hasFrame();
    /** Set whether this item has a frame drawn around it or not.
     * @param none
     * @return void
     * @note deprecated since 1.8 don't use!
     * @see hasFrame
     */
    void setFrame( bool drawFrame );
    /** Set whether this item has a frame drawn around it or not.
     * @param none
     * @return void
     * @note deprecated since 1.8
     * @see hasFrame
     */
    void setFrameEnabled( bool drawFrame );

    /**Composite operations for item groups do nothing per default*/
    virtual void addItem( QgsComposerItem* item );
    virtual void removeItems();

    const QgsComposition* composition() const;

    //functions that encapsulate the workaround for the Qt font bug (that is to scale the font size up and then scale the
    //painter down by the same factor for drawing

    /**Draws Text. Takes care about all the composer specific issues (calculation to pixel, scaling of font and painter
     to work around the Qt font bug)*/
    void drawText( QPainter* p, int x, int y, const QString& text, const QFont& font ) const;

    /**Like the above, but with a rectangle for multiline text*/
    void drawText( QPainter* p, const QRectF& rect, const QString& text, const QFont& font ) const;

    /**Returns the font width in Millimeters (considers upscaling and downscaling with FONT_WORKAROUND_SCALE*/
    double textWidthMillimeters( const QFont& font, const QString& text ) const;

    /**Returns the font height of a character in millimeters
      @note this method was added in version 1.7*/
    double fontHeightCharacterMM( const QFont& font, const QChar& c ) const;

    /**Returns the font ascent in Millimeters (considers upscaling and downscaling with FONT_WORKAROUND_SCALE*/
    double fontAscentMillimeters( const QFont& font ) const;

    /**Calculates font to from point size to pixel size*/
    double pixelFontSize( double pointSize ) const;

    /**Returns a font where size is in pixel and font size is upscaled with FONT_WORKAROUND_SCALE*/
    QFont scaledFontPixelSize( const QFont& font ) const;

    /**Locks / unlocks the item position for mouse drags
    @note this method was added in version 1.2*/
    void setPositionLock( bool lock );

    /**Returns position lock for mouse drags (true means locked)
    @note this method was added in version 1.2*/
    bool positionLock() const;

    /**Update mouse cursor at (item) position
    @note this method was added in version 1.2*/
    void updateCursor( const QPointF& itemPos );

    double rotation() const;

    /**Updates item, with the possibility to do custom update for subclasses*/
    virtual void updateItem();

    /**Get label identification name
      @note this method was added in version 1.7*/
    QString id() const;

    /**Set label identification name
      @note this method was added in version 1.7
                      This method was moved from qgscomposerlabel so that every object can have a
                       id (NathanW)*/
    void setId( const QString& id );

  public slots:
    virtual void setRotation( double r);

  protected:

    //event handlers
    virtual void mouseMoveEvent( QGraphicsSceneMouseEvent * event );
    virtual void mousePressEvent( QGraphicsSceneMouseEvent * event );
    virtual void mouseReleaseEvent( QGraphicsSceneMouseEvent * event );

    virtual void hoverMoveEvent( QGraphicsSceneHoverEvent * event );

    /**Finds out the appropriate cursor for the current mouse position in the widget (e.g. move in the middle, resize at border)*/
    Qt::CursorShape cursorForPosition( const QPointF& itemCoordPos );

    /**Finds out which mouse move action to choose depending on the cursor position inside the widget*/
    QgsComposerItem::MouseMoveAction mouseMoveActionForPosition( const QPointF& itemCoordPos );

    /**Changes the rectangle of an item depending on current mouse action (resize or move)
     @param currentPosition current position of mouse cursor
     @param mouseMoveStartPos cursor position at the start of the current mouse action
     @param originalItem Item position at the start of the mouse action
     @param dx x-Change of mouse cursor
     @param dy y-Change of mouse cursor
     @param changeItem Item to change size (can be the same as originalItem or a differen one)
    */
    void changeItemRectangle( const QPointF& currentPosition, const QPointF& mouseMoveStartPos, const QGraphicsRectItem* originalItem, double dx, double dy, QGraphicsRectItem* changeItem );

    /**Draw selection boxes around item*/
    virtual void drawSelectionBoxes( QPainter* p );

    /**Draw black frame around item*/
    virtual void drawFrame( QPainter* p );

    /**Draw background*/
    virtual void drawBackground( QPainter* p );

    /**Draws arrowhead*/
    void drawArrowHead( QPainter* p, double x, double y, double angle, double arrowHeadWidth ) const;

    /**Returns angle of the line from p1 to p2 (clockwise, starting at N)*/
    double angle( const QPointF& p1, const QPointF& p2 ) const;

    /**Returns the current (zoom level dependent) tolerance to decide if mouse position is close enough to the \
    item border for resizing*/
    double rectHandlerBorderTolerance() const;

    /**Returns the size of the lock symbol depending on the composer zoom level and the item size
    @note: this function was introduced in version 1.2*/
    double lockSymbolSize() const;

    /**Returns the zoom factor of the graphics view.
      @return the factor or -1 in case of error (e.g. graphic view does not exist)
    @note: this function was introduced in version 1.2*/
    double horizontalViewScaleFactor() const;

    /**Calculates width and hight of the picture (in mm) such that it fits into the item frame with the given rotation*/
    bool imageSizeConsideringRotation( double& width, double& height ) const;
    /**Calculates corner point after rotation and scaling*/
    bool cornerPointOnRotatedAndScaledRect( double& x, double& y, double width, double height ) const;

    /**Calculates width / height of the bounding box of a rotated rectangle (mRotation)*/
    void sizeChangedByRotation(double& width, double& height);
    /**Rotates a point / vector
        @param angle rotation angle in degrees, counterclockwise
        @param x in/out: x coordinate before / after the rotation
        @param y in/out: y cooreinate before / after the rotation*/
    void rotate( double angle, double& x, double& y ) const;

    signals:
     /**Is emitted on rotation change to notify north arrow pictures*/
    void rotationChanged( double newRotation );
};
