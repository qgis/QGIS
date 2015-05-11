class QgsHighlight : QgsMapCanvasItem
{
%TypeHeaderCode
#include <qgshighlight.h>
%End
  public:
    QgsHighlight( QgsMapCanvas *mapCanvas, const QgsGeometry *geom, QgsVectorLayer *layer );
    ~QgsHighlight();

    /** Set line/outline to color, polygon fill to color with alpha = 63.
     *  This is legacy function, use setFillColor() after setColor() if different fill color is required. */
    void setColor( const QColor & color );

    /** Set polygons fill color.
     * @note: added in version 2.3 */
    void setFillColor( const QColor & fillColor );

    /** Set width. Ignored in feature mode. */
    void setWidth( int width );

    /** Set line / outline buffer in millimeters.
     *  @note: added in version 2.3 */
    void setBuffer( double buffer );

    /** Set minimum line / outline width in millimeters.
     *  @note: added in version 2.3 */
    void setMinWidth( double width );

    const QgsMapLayer *layer() const;

    virtual void updatePosition();

  protected:
    virtual void paint( QPainter* p );

    //! recalculates needed rectangle
    void updateRect();
};
