
#include <QPainter>
#include "qgsgeorefdatapoint.h"
#include "qgsmapcanvas.h"

QgsGeorefDataPoint::QgsGeorefDataPoint(QgsMapCanvas* mapCanvas, int id,
          const QgsPoint& pixelCoords, const QgsPoint& mapCoords)
 : QgsMapCanvasItem(mapCanvas), mId(id),
   mPixelCoords(pixelCoords), mMapCoords(mapCoords)
{
  updatePosition();
}


void QgsGeorefDataPoint::paint(QPainter* p)
{
  QString msg = QString("X %1\nY %2").arg(QString::number(mMapCoords.x(),'f')).arg(QString::number(mMapCoords.y(),'f'));
  QFont font;
  p->setFont(QFont("helvetica", 9));
  p->setPen(Qt::black);
  p->setBrush(Qt::red);
  p->drawRect(-2, -2, 5, 5);
  QRect textBounds = p->boundingRect(4, 4, 10, 10, Qt::AlignLeft, msg);
  p->setBrush(Qt::yellow);
  p->drawRect(2, 2, textBounds.width() + 4, textBounds.height() + 4);
  p->drawText(textBounds, Qt::AlignLeft, msg);
  
  mTextBounds = QSizeF(textBounds.width(), textBounds.height());
}

QRectF QgsGeorefDataPoint::boundingRect() const
{
  return QRectF(-2,-2, mTextBounds.width() + 6, mTextBounds.height() + 6);
}

void QgsGeorefDataPoint::updatePosition()
{
  setPos(toCanvasCoords(mPixelCoords));
}
