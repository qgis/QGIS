
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


void QgsGeorefDataPoint::drawShape(QPainter & p)
{
  QString msg = QString("X %1\nY %2").arg(QString::number(mMapCoords.x(),'f')).arg(QString::number(mMapCoords.y(),'f'));
  QPoint pnt = toCanvasCoords(mPixelCoords);
  int x = pnt.x(), y = pnt.y();
  QFont font;
  p.setFont(QFont("helvetica", 9));
  p.setPen(Qt::black);
  p.setBrush(Qt::red);
  p.drawRect(x - 2, y - 2, 5, 5);
  QRect textBounds = p.boundingRect(x + 4, y + 4, 10, 10, Qt::AlignLeft, msg);
  p.setBrush(Qt::yellow);
  p.drawRect(x + 2, y + 2, textBounds.width() + 4, textBounds.height() + 4);
  p.drawText(textBounds, Qt::AlignLeft, msg);
  
#ifdef QGISDEBUG
  std::cout << "data point at :: " << x << "," << y << std::endl;
#endif
  
  setSize(textBounds.width() + 6, textBounds.height() + 6);
}

void QgsGeorefDataPoint::updatePosition()
{
  QPoint pt = toCanvasCoords(mPixelCoords);
  move(pt.x() - 2, pt.y() - 2);
  show();

#ifdef QGISDEBUG
  std::cout << "georefDataPoint::updatePosition: " << pt.x() << "," << pt.y() << std::endl;
#endif
}
