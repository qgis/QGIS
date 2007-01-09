
#include <QPainter>
#include <QPainterPath>

#include "positionmarker.h"


PositionMarker::PositionMarker(QgsMapCanvas* mapCanvas)
  : QgsMapCanvasItem(mapCanvas)
{
  mHasPosition = false;
  mAngle = 0;
  setZValue(100); // be on top
}

void PositionMarker::setIcon(bool hasPosition)
{
  mHasPosition = hasPosition;
}

void PositionMarker::setAngle(double angle)
{    
  mAngle = angle;
}

void PositionMarker::setPosition(const QgsPoint& point)
{
  if (mPosition != point)
  {
    mPosition = point;
    QPointF pt = toCanvasCoords(mPosition);
    setPos(pt);
    updateCanvas();
  }
}

QRectF PositionMarker::boundingRect() const
{
  return QRectF(-12,-12,12,12);
}

void PositionMarker::paint(QPainter* p)
{
  //if (mHasPosition)
  {
    QPainterPath path;
    path.moveTo(0,-10);
    path.lineTo(10,10);
    path.lineTo(0,5);
    path.lineTo(-10,10);
    path.lineTo(0,-10);
    
    // render position with angle
    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    if (mHasPosition)
      p->setBrush(QBrush(QColor(0,0,0)));
    else
      p->setBrush(QBrush(QColor(200,200,200)));
    p->setPen(QColor(255,255,255));
    p->rotate(mAngle);
    p->drawPath(path);
    p->restore();
  }
/*  else
  {
    // TODO: draw '?' to show that we don't have position
}*/
}
