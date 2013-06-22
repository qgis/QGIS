class QgsHighlight : public QgsMapCanvasItem
{
%TypeHeaderCode
#include <qgshighlight.h>
%End
  public:
    QgsHighlight( QgsMapCanvas *mapCanvas, QgsGeometry *geom, QgsVectorLayer *layer );
    ~QgsHighlight();

    void setColor( const QColor & color );
    void setWidth( int width );

  protected:
    virtual void paint( QPainter* p );

    //! recalculates needed rectangle
    void updateRect();
};
