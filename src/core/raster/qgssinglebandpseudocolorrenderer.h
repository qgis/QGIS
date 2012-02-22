/***************************************************************************
                         qgssinglebandpseudocolorrenderer.h
                         ----------------------------------
    begin                : January 2012
    copyright            : (C) 2012 by Marco Hugentobler
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

#ifndef QGSSINGLEBANDPSEUDOCOLORRENDERER_H
#define QGSSINGLEBANDPSEUDOCOLORRENDERER_H

#include "qgsrasterrenderer.h"

class QDomElement;
class QgsRasterShader;

class QgsSingleBandPseudoColorRenderer: public QgsRasterRenderer
{
  public:
    /**Note: takes ownership of QgsRasterShader*/
    QgsSingleBandPseudoColorRenderer( QgsRasterDataProvider* provider, int band, QgsRasterShader* shader );
    ~QgsSingleBandPseudoColorRenderer();

    static QgsRasterRenderer* create( const QDomElement& elem );

    virtual void draw( QPainter* p, QgsRasterViewPort* viewPort, const QgsMapToPixel* theQgsMapToPixel );

    /**Takes ownership of the shader*/
    void setShader( QgsRasterShader* shader );
    QgsRasterShader* shader() { return mShader; }

  private:
    QgsRasterShader* mShader;
    int mBand;
};

#endif // QGSSINGLEBANDPSEUDOCOLORRENDERER_H
