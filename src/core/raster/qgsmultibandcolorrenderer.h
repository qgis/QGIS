/***************************************************************************
                         qgsmultibandcolorrenderer.h
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

#ifndef QGSMULTIBANDCOLORRENDERER_H
#define QGSMULTIBANDCOLORRENDERER_H

#include "qgsrasterrenderer.h"

class QgsContrastEnhancement;

/**Renderer for multiband images with the color components*/
class QgsMultiBandColorRenderer: public QgsRasterRenderer
{
  public:
    QgsMultiBandColorRenderer( QgsRasterDataProvider* provider, int redBand, int greenBand, int blueBand,
                               QgsContrastEnhancement* redEnhancement = 0, QgsContrastEnhancement* greenEnhancement = 0,
                               QgsContrastEnhancement* blueEnhancement = 0 );
    ~QgsMultiBandColorRenderer();

    void draw( QPainter* p, QgsRasterViewPort* viewPort, const QgsMapToPixel* theQgsMapToPixel );

    const QgsContrastEnhancement* redContrastEnhancement() const { return mRedContrastEnhancement; }
    void setRedContrastEnhancement( QgsContrastEnhancement* ce ) { mRedContrastEnhancement = ce; }

    const QgsContrastEnhancement* greenContrastEnhancement() const { return mGreenContrastEnhancement; }
    void setGreenContrastEnhancement( QgsContrastEnhancement* ce ) { mGreenContrastEnhancement = ce; }

    const QgsContrastEnhancement* blueContrastEnhancement() const { return mBlueContrastEnhancement; }
    void setBlueContrastEnhancement( QgsContrastEnhancement* ce ) { mBlueContrastEnhancement = ce; }

  private:
    int mRedBand;
    int mGreenBand;
    int mBlueBand;

    QgsContrastEnhancement* mRedContrastEnhancement;
    QgsContrastEnhancement* mGreenContrastEnhancement;
    QgsContrastEnhancement* mBlueContrastEnhancement;
};

#endif // QGSMULTIBANDCOLORRENDERER_H
