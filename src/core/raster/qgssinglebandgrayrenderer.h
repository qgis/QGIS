/***************************************************************************
                         qgssinglebandgrayrenderer.h
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

#ifndef QGSSINGLEBANDGRAYRENDERER_H
#define QGSSINGLEBANDGRAYRENDERER_H

#include "qgsrasterrenderer.h"

class QgsContrastEnhancement;
class QDomElement;

class CORE_EXPORT QgsSingleBandGrayRenderer: public QgsRasterRenderer
{
  public:
    QgsSingleBandGrayRenderer( QgsRasterDataProvider* provider, int grayBand );
    ~QgsSingleBandGrayRenderer();

    static QgsRasterRenderer* create( const QDomElement& elem, QgsRasterDataProvider* provider );

    virtual void draw( QPainter* p, QgsRasterViewPort* viewPort, const QgsMapToPixel* theQgsMapToPixel );

    int grayBand() const { return mGrayBand; }
    void setGrayBand( int band ) { mGrayBand = band; }
    const QgsContrastEnhancement* contrastEnhancement() const { return mContrastEnhancement; }
    /**Takes ownership*/
    void setContrastEnhancement( QgsContrastEnhancement* ce );

    void writeXML( QDomDocument& doc, QDomElement& parentElem ) const;

    void legendSymbologyItems( QList< QPair< QString, QColor > >& symbolItems ) const;

    QList<int> usesBands() const;

  private:
    int mGrayBand;
    QgsContrastEnhancement* mContrastEnhancement;
};

#endif // QGSSINGLEBANDGRAYRENDERER_H
