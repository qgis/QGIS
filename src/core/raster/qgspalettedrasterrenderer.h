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

#include <QVector>

#include "qgsrasterrenderer.h"

class QColor;
class QDomElement;

/** \ingroup core
  * Renderer for paletted raster images.
*/
class CORE_EXPORT QgsPalettedRasterRenderer: public QgsRasterRenderer
{
  public:
    /** Renderer owns color array*/
    QgsPalettedRasterRenderer( QgsRasterInterface* input, int bandNumber, QColor* colorArray, int nColors, const QVector<QString>& labels = QVector<QString>() );
    QgsPalettedRasterRenderer( QgsRasterInterface* input, int bandNumber, QRgb* colorArray, int nColors, const QVector<QString>& labels = QVector<QString>() );
    ~QgsPalettedRasterRenderer();
    QgsPalettedRasterRenderer * clone() const override;
    static QgsRasterRenderer* create( const QDomElement& elem, QgsRasterInterface* input );

    QgsRasterBlock *block( int bandNo, const QgsRectangle & extent, int width, int height ) override;

    /** Returns number of colors*/
    int nColors() const { return mNColors; }
    /** Returns copy of color array (caller takes ownership)*/
    QColor* colors() const;

    /** Returns copy of rgb array (caller takes ownership)
     @note not available in python bindings
     */
    QRgb* rgbArray() const;

    /** Return optional category label
     *  @note added in 2.1 */
    QString label( int idx ) const { return mLabels.value( idx ); }

    /** Set category label
     *  @note added in 2.1 */
    void setLabel( int idx, const QString& label );

    void writeXML( QDomDocument& doc, QDomElement& parentElem ) const override;

    void legendSymbologyItems( QList< QPair< QString, QColor > >& symbolItems ) const override;

    QList<int> usesBands() const override;

  private:
    int mBand;
    /** Color array*/
    QRgb* mColors;
    /** Number of colors*/
    int mNColors;
    /** Optional category labels, size of vector may be < mNColors */
    QVector<QString> mLabels;

    QgsPalettedRasterRenderer( const QgsPalettedRasterRenderer& );
    const QgsPalettedRasterRenderer& operator=( const QgsPalettedRasterRenderer& );
};

#endif // QGSPALETTEDRASTERRENDERER_H
