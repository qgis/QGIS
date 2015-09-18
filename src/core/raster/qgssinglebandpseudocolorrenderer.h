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

/** \ingroup core
  * Raster renderer pipe for single band pseudocolor.
  */
class CORE_EXPORT QgsSingleBandPseudoColorRenderer: public QgsRasterRenderer
{
  public:
    /** Note: takes ownership of QgsRasterShader*/
    QgsSingleBandPseudoColorRenderer( QgsRasterInterface* input, int band, QgsRasterShader* shader );
    ~QgsSingleBandPseudoColorRenderer();
    QgsRasterInterface * clone() const override;

    static QgsRasterRenderer* create( const QDomElement& elem, QgsRasterInterface* input );

    QgsRasterBlock* block( int bandNo, const QgsRectangle & extent, int width, int height ) override;

    /** Takes ownership of the shader*/
    void setShader( QgsRasterShader* shader );
    QgsRasterShader* shader() { return mShader; }
    const QgsRasterShader* shader() const { return mShader; }

    void writeXML( QDomDocument& doc, QDomElement& parentElem ) const override;

    void legendSymbologyItems( QList< QPair< QString, QColor > >& symbolItems ) const override;

    QList<int> usesBands() const override;

    /** Returns the band used by the renderer
     * @note added in QGIS 2.7
     */
    int band() const { return mBand; }

    /** Sets the band used by the renderer.
    * @see band
    * @note added in QGIS 2.10
    */
    void setBand( int bandNo );

    double classificationMin() const { return mClassificationMin; }
    double classificationMax() const { return mClassificationMax; }
    void setClassificationMin( double min ) { mClassificationMin = min; }
    void setClassificationMax( double max ) { mClassificationMax = max; }
    int classificationMinMaxOrigin() const { return mClassificationMinMaxOrigin; }
    void setClassificationMinMaxOrigin( int origin ) { mClassificationMinMaxOrigin = origin; }

  private:
    QgsRasterShader* mShader;
    int mBand;

    // Minimum and maximum values used for automatic classification, these
    // values are not used by renderer in rendering process
    double mClassificationMin;
    double mClassificationMax;

    int mClassificationMinMaxOrigin;
};

#endif // QGSSINGLEBANDPSEUDOCOLORRENDERER_H
