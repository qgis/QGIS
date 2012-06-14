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
class QDomElement;

class CORE_EXPORT QgsPalettedRasterRenderer: public QgsRasterRenderer
{
  public:
    /**Renderer owns color array*/
    QgsPalettedRasterRenderer( QgsRasterDataProvider* provider, int bandNumber, QColor* colorArray, int nColors );
    ~QgsPalettedRasterRenderer();
    static QgsRasterRenderer* create( const QDomElement& elem, QgsRasterDataProvider* provider );

    void draw( QPainter* p, QgsRasterViewPort* viewPort, const QgsMapToPixel* theQgsMapToPixel );

    /**Returns number of colors*/
    int nColors() const { return mNColors; }
    /**Returns copy of color array (caller takes ownership)*/
    QColor* colors() const;

    void writeXML( QDomDocument& doc, QDomElement& parentElem ) const;

    void legendSymbologyItems( QList< QPair< QString, QColor > >& symbolItems ) const;

  private:
    int mBandNumber;
    /**Color array*/
    QColor* mColors;
    /**Number of colors*/
    int mNColors;
};

#endif // QGSPALETTEDRASTERRENDERER_H
