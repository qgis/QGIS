class QgsHighlight : QgsMapCanvasItem
{
%TypeHeaderCode
#include <qgshighlight.h>
%End
  public:
    QgsHighlight( QgsMapCanvas *mapCanvas, QgsGeometry *geom, QgsVectorLayer *layer );
    ~QgsHighlight();

    void setColor( const QColor & color );

    void setFillColor( const QColor & fillColor );

    /** Set width. Ignored in feature mode. */
    void setWidth( int width );

    void setBuffer( double buffer );

    void setMinWidth( double width );

  protected:
    virtual void paint( QPainter* p );

    //! recalculates needed rectangle
    void updateRect();
};
