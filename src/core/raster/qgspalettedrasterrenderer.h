#ifndef QGSPALETTEDRASTERRENDERER_H
#define QGSPALETTEDRASTERRENDERER_H

#include "qgsrasterrenderer.h"

class QColor;

class QgsPalettedRasterRenderer: public QgsRasterRenderer
{
  public:
    /**Renderer owns color array*/
    QgsPalettedRasterRenderer( QgsRasterDataProvider* provider, int bandNumber, QColor* colorArray, int nColors );
    ~QgsPalettedRasterRenderer();
    void draw( QPainter* p, QgsRasterViewPort* viewPort, const QgsMapToPixel* theQgsMapToPixel );

  private:
    int mBandNumber;
    /**Color array*/
    QColor* mColors;
    /**Number of colors*/
    int mNColors;
};

#endif // QGSPALETTEDRASTERRENDERER_H
