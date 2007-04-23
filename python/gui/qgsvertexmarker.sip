
class QgsVertexMarker : QgsMapCanvasItem
{
%TypeHeaderCode
#include <qgsvertexmarker.h>
%End

  public:
    
    //! Icons
    enum IconType
    {
      ICON_NONE,
      ICON_CROSS,
      ICON_X,
      ICON_BOX
    };

    QgsVertexMarker(QgsMapCanvas* mapCanvas /TransferThis/);
    
    void setCenter(const QgsPoint& point);
    
    void setIconType(int iconType);
    
    void setIconSize(int iconSize);
    
    void setColor(const QColor& color);
    
    void setPenWidth(int width);
    
    void paint(QPainter* p);
    
    QRectF boundingRect() const;

    virtual void updatePosition();

};

