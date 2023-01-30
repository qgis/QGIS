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

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgscolorramp.h"
#include "qgscolorrampshader.h"
#include "qgsrasterrenderer.h"
#include "qgsrectangle.h"

class QDomElement;
class QgsRasterShader;

/**
 * \ingroup core
  * \brief Raster renderer pipe for single band pseudocolor.
  */
class CORE_EXPORT QgsSingleBandPseudoColorRenderer: public QgsRasterRenderer
{

  public:

    //! Note: takes ownership of QgsRasterShader
    QgsSingleBandPseudoColorRenderer( QgsRasterInterface *input, int band = -1, QgsRasterShader *shader SIP_TRANSFER = nullptr );

    //! QgsSingleBandPseudoColorRenderer cannot be copied. Use clone() instead.
    QgsSingleBandPseudoColorRenderer( const QgsSingleBandPseudoColorRenderer & ) = delete;
    //! QgsSingleBandPseudoColorRenderer cannot be copied. Use clone() instead.
    const QgsSingleBandPseudoColorRenderer &operator=( const QgsSingleBandPseudoColorRenderer & ) = delete;

    QgsSingleBandPseudoColorRenderer *clone() const override SIP_FACTORY;
    Qgis::RasterRendererFlags flags() const override;
    static QgsRasterRenderer *create( const QDomElement &elem, QgsRasterInterface *input ) SIP_FACTORY;

    QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback = nullptr ) override SIP_FACTORY;

    //! Takes ownership of the shader
    void setShader( QgsRasterShader *shader SIP_TRANSFER );

    //! Returns the raster shader
    QgsRasterShader *shader() { return mShader.get(); }

    //! \note available in Python as constShader
    const QgsRasterShader *shader() const SIP_PYNAME( constShader ) { return mShader.get(); }

    bool canCreateRasterAttributeTable( ) const override;

    /**
     * Creates a color ramp shader
     * \param colorRamp vector color ramp. Ownership is transferred to the shader.
     * \param colorRampType type of color ramp shader
     * \param classificationMode classification mode
     * \param classes number of classes
     * \param clip clip out of range values
     * \param extent extent used in classification (only used in quantile mode)
     */
    void createShader( QgsColorRamp *colorRamp SIP_TRANSFER = nullptr,
                       QgsColorRampShader::Type colorRampType  = QgsColorRampShader::Interpolated,
                       QgsColorRampShader::ClassificationMode classificationMode = QgsColorRampShader::Continuous,
                       int classes = 0,
                       bool clip = false,
                       const QgsRectangle &extent = QgsRectangle() );

    void writeXml( QDomDocument &doc, QDomElement &parentElem ) const override;
    QList< QPair< QString, QColor > > legendSymbologyItems() const override;
    QList<QgsLayerTreeModelLegendNode *> createLegendNodes( QgsLayerTreeLayer *nodeLayer ) SIP_FACTORY override;
    QList<int> usesBands() const override;
    void toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props = QVariantMap() ) const override;
    bool accept( QgsStyleEntityVisitorInterface *visitor ) const override;

    /**
     * Returns the band used by the renderer
     * \since QGIS 2.7
     */
    int band() const { return mBand; }

    /**
     * Sets the band used by the renderer.
     * \see band
     * \since QGIS 2.10
     */
    void setBand( int bandNo );

    double classificationMin() const { return mClassificationMin; }
    double classificationMax() const { return mClassificationMax; }
    void setClassificationMin( double min );
    void setClassificationMax( double max );

  private:
#ifdef SIP_RUN
    QgsSingleBandPseudoColorRenderer( const QgsSingleBandPseudoColorRenderer & );
    const QgsSingleBandPseudoColorRenderer &operator=( const QgsSingleBandPseudoColorRenderer & );
#endif

    std::unique_ptr< QgsRasterShader > mShader;
    int mBand;

    // Minimum and maximum values used for automatic classification, these
    // values are not used by renderer in rendering process
    double mClassificationMin;
    double mClassificationMax;

};

#endif // QGSSINGLEBANDPSEUDOCOLORRENDERER_H
