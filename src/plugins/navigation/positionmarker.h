

#ifndef POSITIONMARKER_H
#define POSITIONMARKER_H

#include "qgsmapcanvasitem.h"
#include "qgspoint.h"

class QPainter;

class PositionMarker : public QgsMapCanvasItem
{
  public:
    
    PositionMarker(QgsMapCanvas* mapCanvas);
    
    void setPosition(const QgsPoint& point);
    
    void setIcon(bool hasPosition);
    
    void setAngle(double angle);
    
    void paint(QPainter* p);

    QRectF boundingRect() const;
    
  protected:
    
    bool mHasPosition;
    
    //! angle in degrees (north = 0, south = 180)
    double mAngle;
    
    //! size
    int mIconSize;

    //! coordinates of the point in the center
    QgsPoint mPosition;
};

#endif
