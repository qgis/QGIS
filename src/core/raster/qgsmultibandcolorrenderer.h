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
  * \brief Renderer for multiband images with the color components.
*/
class CORE_EXPORT QgsMultiBandColorRenderer: public QgsRasterRenderer
{
  public:

    /**
     * Constructor for QgsMultiBandColorRenderer.
     * \param input input raster interface
     * \param redBand band number for red channel
     * \param greenBand band number for green channel
     * \param blueBand band number for blue channel
     * \param redEnhancement optional contrast enhancement for red channel. Ownership is transferred to the renderer.
     * \param greenEnhancement optional contrast enhancement for green channel. Ownership is transferred to the renderer.
     * \param blueEnhancement optional contrast enhancement for blue channel. Ownership is transferred to the renderer.
     */
    QgsMultiBandColorRenderer( QgsRasterInterface *input, int redBand, int greenBand, int blueBand,
                               QgsContrastEnhancement *redEnhancement SIP_TRANSFER = nullptr,
                               QgsContrastEnhancement *greenEnhancement SIP_TRANSFER = nullptr,
                               QgsContrastEnhancement *blueEnhancement SIP_TRANSFER = nullptr );
    ~QgsMultiBandColorRenderer() override;

    //! QgsMultiBandColorRenderer cannot be copied. Use clone() instead.
    QgsMultiBandColorRenderer( const QgsMultiBandColorRenderer & ) = delete;
    //! QgsMultiBandColorRenderer cannot be copied. Use clone() instead.
    const QgsMultiBandColorRenderer &operator=( const QgsMultiBandColorRenderer & ) = delete;

    QgsMultiBandColorRenderer *clone() const override SIP_FACTORY;
    Qgis::RasterRendererFlags flags() const override;

    static QgsRasterRenderer *create( const QDomElement &elem, QgsRasterInterface *input ) SIP_FACTORY;

    QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback = nullptr ) override SIP_FACTORY;

    /**
     * Returns the band number for the red channel.
     *
     * \see setRedBand()
     */
    int redBand() const { return mRedBand; }

    /**
     * Sets the \a band number for the red channel.
     *
     * \see redBand()
     */
    void setRedBand( int band ) { mRedBand = band; }

    /**
     * Returns the band number for the green channel.
     *
     * \see setRedBand()
     */
    int greenBand() const { return mGreenBand; }

    /**
     * Sets the \a band number for the green channel.
     *
     * \see greenBand()
     */
    void setGreenBand( int band ) { mGreenBand = band; }

    /**
     * Returns the band number for the blue channel.
     *
     * \see setRedBand()
     */
    int blueBand() const { return mBlueBand; }

    /**
     * Sets the \a band number for the blue channel.
     *
     * \see blueBand()
     */
    void setBlueBand( int band ) { mBlueBand = band; }

    /**
     * Returns the contrast enhancement to use for the red channel.
     *
     * \see setRedContrastEnhancement()
     * \see greenContrastEnhancement()
     * \see blueContrastEnhancement()
     */
    const QgsContrastEnhancement *redContrastEnhancement() const;

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
    const QgsContrastEnhancement *greenContrastEnhancement() const;

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
    const QgsContrastEnhancement *blueContrastEnhancement() const;

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

    Q_DECL_DEPRECATED void toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props = QVariantMap() ) const override SIP_DEPRECATED;
    bool toSld( QDomDocument &doc, QDomElement &element, QgsSldExportContext &context ) const override;

    /**
     * \brief Refreshes the renderer according to the \a min and \a max values associated with the \a extent.
     * \a min and \a max size need to be at least 3.
     * If \a min or \a max size is greater than 3, the last values are ignored.
     * The first value is associated with the red contrast.
     * The second value is associated with the green contrast.
     * The third value is associated with the blue contrast.
     * NaN values are ignored.
     * If \a forceRefresh is TRUE, this will force the refresh even if needsRefresh() returns FALSE.
     * \returns TRUE if the renderer has been refreshed
     * \note not available in Python bindings
     *
     * \since QGIS 3.42
     */
    bool refresh( const QgsRectangle &extent, const QList<double> &min, const QList<double> &max, bool forceRefresh = false ) override SIP_SKIP;

  private:
#ifdef SIP_RUN
    QgsMultiBandColorRenderer( const QgsMultiBandColorRenderer & );
    const QgsMultiBandColorRenderer &operator=( const QgsMultiBandColorRenderer & );
#endif

    int mRedBand = 1;
    int mGreenBand = 1;
    int mBlueBand = 1;

    std::unique_ptr< QgsContrastEnhancement > mRedContrastEnhancement;
    std::unique_ptr< QgsContrastEnhancement > mGreenContrastEnhancement;
    std::unique_ptr< QgsContrastEnhancement > mBlueContrastEnhancement;

};

#endif // QGSMULTIBANDCOLORRENDERER_H
