/***************************************************************************
                         qgspalettedrasterrenderer.h
                         ---------------------------
    begin                : December 2011
    copyright            : (C) 2011 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPALETTEDRASTERRENDERER_H
#define QGSPALETTEDRASTERRENDERER_H

#include "qgsrasterrenderer.h"

class QColor;

class QgsPalettedRasterRenderer: public QgsRasterRenderer
{
  public:
    /**Renderer owns color array*/
    QgsPalettedRasterRenderer( QgsRasterDataProvider* provider, int bandNumber, QColor* colorArray, int nColors, QgsRasterResampler* resampler = 0 );
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
