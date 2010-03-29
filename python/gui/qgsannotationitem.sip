/**An annotation item can be either placed either on screen corrdinates or on map coordinates.
  It may reference a feature and displays that associatiation with a balloon like appearance*/
class QgsAnnotationItem: QgsMapCanvasItem
{
%TypeHeaderCode
#include <qgsannotationitem.h>
%End
  
  public:
    enum MouseMoveAction
    {
      NoAction,
      MoveMapPosition,
      MoveFramePosition,
      ResizeFrameUp,
      ResizeFrameDown,
      ResizeFrameLeft,
      ResizeFrameRight,
      ResizeFrameLeftUp,
      ResizeFrameRightUp,
      ResizeFrameLeftDown,
      ResizeFrameRightDown
    };

    QgsAnnotationItem( QgsMapCanvas* mapCanvas );
    virtual ~QgsAnnotationItem();

    /**Creates an editor widget (caller takes ownership)*/
    virtual QDialog* createEditor() = 0;

    void updatePosition();

    QRectF boundingRect() const;

    virtual QSizeF minimumFrameSize() const;

    /**Returns the mouse move behaviour for a given position
      @param pos the position in scene coordinates*/
    QgsAnnotationItem::MouseMoveAction moveActionForPosition( const QPointF& pos) const;
    /**Returns suitable cursor shape for mouse move action*/
    Qt::CursorShape cursorShapeForAction( MouseMoveAction moveAction ) const;

    //setters and getters
    void setMapPositionFixed( bool fixed );
    bool mapPositionFixed() const;

    virtual void setMapPosition( const QgsPoint& pos );
    QgsPoint mapPosition() const;

    void setFrameSize( const QSizeF& size );
    QSizeF frameSize() const;

    void setOffsetFromReferencePoint( const QPointF& offset );
    QPointF offsetFromReferencePoint() const;

    /**Set symbol that is drawn on map position. Takes ownership*/
    void setMarkerSymbol( QgsMarkerSymbolV2* symbol );
    const QgsMarkerSymbolV2* markerSymbol() const;

    void setFrameBorderWidth( double w );
    double frameBorderWidth() const;

    void setFrameColor( const QColor& c );
    QColor frameColor() const;

    void setFrameBackgroundColor( const QColor& c );
    QColor frameBackgroundColor() const;

    virtual void writeXML( QDomDocument& doc ) const = 0;
    virtual void readXML( const QDomDocument& doc, const QDomElement& itemElem ) = 0;

    void _writeXML( QDomDocument& doc, QDomElement& itemElem ) const;
    void _readXML( const QDomDocument& doc, const QDomElement& annotationElem );
};