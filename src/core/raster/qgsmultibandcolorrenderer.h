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

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsrasterrenderer.h"

class QgsContrastEnhancement;
class QDomElement;

/**
 * \ingroup core
  * \brief Renderer for multiband images with the color components
*/
class CORE_EXPORT QgsMultiBandColorRenderer: public QgsRasterRenderer
{
  public:
    QgsMultiBandColorRenderer( QgsRasterInterface *input, int redBand, int greenBand, int blueBand,
                               QgsContrastEnhancement *redEnhancement = nullptr, QgsContrastEnhancement *greenEnhancement = nullptr,
                               QgsContrastEnhancement *blueEnhancement = nullptr );
    ~QgsMultiBandColorRenderer() override;

    //! QgsMultiBandColorRenderer cannot be copied. Use clone() instead.
    QgsMultiBandColorRenderer( const QgsMultiBandColorRenderer & ) = delete;
    //! QgsMultiBandColorRenderer cannot be copied. Use clone() instead.
    const QgsMultiBandColorRenderer &operator=( const QgsMultiBandColorRenderer & ) = delete;

    QgsMultiBandColorRenderer *clone() const override SIP_FACTORY;
    Qgis::RasterRendererFlags flags() const override;

    static QgsRasterRenderer *create( const QDomElement &elem, QgsRasterInterface *input ) SIP_FACTORY;

    QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback = nullptr ) override SIP_FACTORY;

    int redBand() const { return mRedBand; }
    void setRedBand( int band ) { mRedBand = band; }
    int greenBand() const { return mGreenBand; }
    void setGreenBand( int band ) { mGreenBand = band; }
    int blueBand() const { return mBlueBand; }
    void setBlueBand( int band ) { mBlueBand = band; }

    /**
     * Returns the contrast enhancement to use for the red channel.
     *
     * \see setRedContrastEnhancement()
     * \see greenContrastEnhancement()
     * \see blueContrastEnhancement()
     */
    const QgsContrastEnhancement *redContrastEnhancement() const { return mRedContrastEnhancement; }

    /**
     * Sets the contrast enhancement to use for the red channel.
     *
     * Ownership of the enhancement is transferred.
     *
     * \see redContrastEnhancement()
     * \see setGreenContrastEnhancement()
     * \see setBlueContrastEnhancement()
     */
    void setRedContrastEnhancement( QgsContrastEnhancement *ce SIP_TRANSFER );

    /**
     * Returns the contrast enhancement to use for the green channel.
     *
     * \see setGreenContrastEnhancement()
     * \see redContrastEnhancement()
     * \see blueContrastEnhancement()
     */
    const QgsContrastEnhancement *greenContrastEnhancement() const { return mGreenContrastEnhancement; }

    /**
     * Sets the contrast enhancement to use for the green channel.
     *
     * Ownership of the enhancement is transferred.
     *
     * \see greenContrastEnhancement()
     * \see setRedContrastEnhancement()
     * \see setBlueContrastEnhancement()
     */
    void setGreenContrastEnhancement( QgsContrastEnhancement *ce SIP_TRANSFER );

    /**
     * Returns the contrast enhancement to use for the blue channel.
     *
     * \see setBlueContrastEnhancement()
     * \see redContrastEnhancement()
     * \see greenContrastEnhancement()
     */
    const QgsContrastEnhancement *blueContrastEnhancement() const { return mBlueContrastEnhancement; }

    /**
     * Sets the contrast enhancement to use for the blue channel.
     *
     * Ownership of the enhancement is transferred.
     *
     * \see blueContrastEnhancement()
     * \see setRedContrastEnhancement()
     * \see setGreenContrastEnhancement()
     */
    void setBlueContrastEnhancement( QgsContrastEnhancement *ce SIP_TRANSFER );

    void writeXml( QDomDocument &doc, QDomElement &parentElem ) const override;

    QList<int> usesBands() const override;
    QList<QgsLayerTreeModelLegendNode *> createLegendNodes( QgsLayerTreeLayer *nodeLayer ) SIP_FACTORY override;

    void toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props = QVariantMap() ) const override;

  private:
#ifdef SIP_RUN
    QgsMultiBandColorRenderer( const QgsMultiBandColorRenderer & );
    const QgsMultiBandColorRenderer &operator=( const QgsMultiBandColorRenderer & );
#endif

    int mRedBand;
    int mGreenBand;
    int mBlueBand;

    QgsContrastEnhancement *mRedContrastEnhancement = nullptr;
    QgsContrastEnhancement *mGreenContrastEnhancement = nullptr;
    QgsContrastEnhancement *mBlueContrastEnhancement = nullptr;

};

#endif // QGSMULTIBANDCOLORRENDERER_H
