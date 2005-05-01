#ifndef DATAPOINTACETATE_H
#define DATAPOINTACETATE_H

#include <qpainter.h>

// silly
#include "qgspoint.h"

#include "qgsacetateobject.h"
#include "qgsmaptopixel.h"


class DataPointAcetate : public QgsAcetateObject {
public:
  
  DataPointAcetate::DataPointAcetate(const QgsPoint& pixelCoords,
				     const QgsPoint& mapCoords)
  // UGLY!
    : QgsAcetateObject(const_cast<QgsPoint&>(pixelCoords)),
      mCoords(mapCoords) {
    
  }

  virtual void draw(QPainter* painter, QgsMapToPixel* cXf = 0) {
    if (cXf) {
      QgsPoint pixelOrigin = cXf->transform(origin());
      int x(pixelOrigin.x());
      int y(pixelOrigin.y());
      QFont font;
      painter->setFont(QFont("helvetica", 9));
      painter->setPen(Qt::black);
      painter->setBrush(Qt::red);
      painter->drawRect(x - 2, y - 2, 5, 5);
      QRect textBounds = painter->boundingRect(x + 4, y + 4, 10, 10, 
					       Qt::AlignLeft,
					       QString("X %1\nY %2").
					       arg(int(mCoords.x())).
					       arg(int(mCoords.y())));
      painter->setBrush(Qt::yellow);
      painter->drawRect(x + 2, y + 2, 
			textBounds.width() + 4, textBounds.height() + 4);
      painter->drawText(textBounds, Qt::AlignLeft, QString("X %1\nY %2").
			arg(int(mCoords.x())).arg(int(mCoords.y())));
    }
  }

private:
  
  QgsPoint mCoords;
};


#endif
