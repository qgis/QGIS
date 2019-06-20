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

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsrasterrenderer.h"
#include <memory>

class QgsContrastEnhancement;
class QDomElement;

/**
 * \ingroup core
  * Raster renderer pipe for single band gray.
  */
class CORE_EXPORT QgsSingleBandGrayRenderer: public QgsRasterRenderer
{
  public:
    enum Gradient
    {
      BlackToWhite,
      WhiteToBlack
    };

    QgsSingleBandGrayRenderer( QgsRasterInterface *input, int grayBand );

    //! QgsSingleBandGrayRenderer cannot be copied. Use clone() instead.
    QgsSingleBandGrayRenderer( const QgsSingleBandGrayRenderer & ) = delete;
    //! QgsSingleBandGrayRenderer cannot be copied. Use clone() instead.
    const QgsSingleBandGrayRenderer &operator=( const QgsSingleBandGrayRenderer & ) = delete;

    QgsSingleBandGrayRenderer *clone() const override SIP_FACTORY;

    static QgsRasterRenderer *create( const QDomElement &elem, QgsRasterInterface *input ) SIP_FACTORY;

    QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback = nullptr ) override SIP_FACTORY;

    int grayBand() const { return mGrayBand; }
    void setGrayBand( int band ) { mGrayBand = band; }
    const QgsContrastEnhancement *contrastEnhancement() const { return mContrastEnhancement.get(); }
    //! Takes ownership
    void setContrastEnhancement( QgsContrastEnhancement *ce SIP_TRANSFER );

    void setGradient( Gradient gradient ) { mGradient = gradient; }
    Gradient gradient() const { return mGradient; }

    void writeXml( QDomDocument &doc, QDomElement &parentElem ) const override;

    void legendSymbologyItems( QList< QPair< QString, QColor > > &symbolItems SIP_OUT ) const override;

    QList<int> usesBands() const override;

    void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props = QgsStringMap() ) const override;

  private:
#ifdef SIP_RUN
    QgsSingleBandGrayRenderer( const QgsSingleBandGrayRenderer & );
    const QgsSingleBandGrayRenderer &operator=( const QgsSingleBandGrayRenderer & );
#endif

    int mGrayBand;
    Gradient mGradient;
    std::unique_ptr< QgsContrastEnhancement > mContrastEnhancement;

};

#endif // QGSSINGLEBANDGRAYRENDERER_H
