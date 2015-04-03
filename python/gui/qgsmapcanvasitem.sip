
class QgsMapCanvasItem : QGraphicsItem
{
%TypeHeaderCode
#include <qgsmapcanvasitem.h>
%End

  protected:

    //! protected constructor: cannot be constructed directly
    QgsMapCanvasItem( QgsMapCanvas* mapCanvas /TransferThis/ );

    virtual ~QgsMapCanvasItem();

    //! function to be implemented by derived classes
    virtual void paint( QPainter * painter ) = 0;

    //! paint function called by map canvas
    virtual void paint( QPainter * painter,
                        const QStyleOptionGraphicsItem * option,
                        QWidget * widget = 0 );

    //! schedules map canvas for repaint
    void updateCanvas();

    /**Sets render context parameters
    @param p painter for rendering
    @param context out: configured context
    @return true in case of success */
    bool setRenderContextVariables( QPainter* p, QgsRenderContext& context ) const;

  public:

    //! called on changed extent or resize event to update position of the item
    virtual void updatePosition();

    //! default implementation for canvas items
    virtual QRectF boundingRect() const;

    //! sets current offset, to be called from QgsMapCanvas
    //! @deprecated since v2.4 - not called by QgsMapCanvas anymore
    void setPanningOffset( const QPoint& point ) /Deprecated/;

    //! returns canvas item rectangle in map units
    QgsRectangle rect() const;

    //! sets canvas item rectangle in map units
    void setRect( const QgsRectangle& r, bool resetRotation = true );

    //! transformation from screen coordinates to map coordinates
    QgsPoint toMapCoordinates( const QPoint& point ) const;

    //! transformation from map coordinates to screen coordinates
    QPointF toCanvasCoordinates( const QgsPoint& point ) const;
};


