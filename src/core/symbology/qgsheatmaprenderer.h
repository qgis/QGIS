/***************************************************************************
    qgsheatmaprenderer.h
    ---------------------
    begin                : November 2014
    copyright            : (C) 2014 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSHEATMAPRENDERER_H
#define QGSHEATMAPRENDERER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsrenderer.h"
#include "qgssymbol.h"
#include "qgsexpression.h"
#include "qgsgeometry.h"

class QgsColorRamp;

/**
 * \ingroup core
 * \class QgsHeatmapRenderer
 * \brief A renderer which draws points as a live heatmap
 * \since QGIS 2.7
 */
class CORE_EXPORT QgsHeatmapRenderer : public QgsFeatureRenderer
{
  public:

    QgsHeatmapRenderer();
    ~QgsHeatmapRenderer() override;

    //! Direct copies are forbidden. Use clone() instead.
    QgsHeatmapRenderer( const QgsHeatmapRenderer & ) = delete;
    //! Direct copies are forbidden. Use clone() instead.
    QgsHeatmapRenderer &operator=( const QgsHeatmapRenderer & ) = delete;

    //reimplemented methods
    QgsHeatmapRenderer *clone() const override SIP_FACTORY;
    void startRender( QgsRenderContext &context, const QgsFields &fields ) override;
    bool renderFeature( const QgsFeature &feature, QgsRenderContext &context, int layer = -1, bool selected = false, bool drawVertexMarker = false ) override;
    void stopRender( QgsRenderContext &context ) override;
    //! \note symbolForFeature2 in Python bindings
    QgsSymbol *symbolForFeature( const QgsFeature &feature, QgsRenderContext &context ) const override;
    //! \note symbol2 in Python bindings
    QgsSymbolList symbols( QgsRenderContext &context ) const override;
    QString dump() const override;
    QSet<QString> usedAttributes( const QgsRenderContext &context ) const override;
    //! Creates a new heatmap renderer instance from XML
    static QgsFeatureRenderer *create( QDomElement &element, const QgsReadWriteContext &context ) SIP_FACTORY;
    QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) override;
    static QgsHeatmapRenderer *convertFromRenderer( const QgsFeatureRenderer *renderer ) SIP_FACTORY;

    //reimplemented to extent the request so that points up to heatmap's radius distance outside
    //visible area are included
    void modifyRequestExtent( QgsRectangle &extent, QgsRenderContext &context ) override;

    //heatmap specific methods

    /**
     * Returns the color ramp used for shading the heatmap.
     * \returns color ramp for heatmap
     * \see setColorRamp
     */
    QgsColorRamp *colorRamp() const { return mGradientRamp; }

    /**
     * Sets the color ramp to use for shading the heatmap.
     * \param ramp color ramp for heatmap. Ownership of ramp is transferred to the renderer.
     * \see colorRamp
     */
    void setColorRamp( QgsColorRamp *ramp SIP_TRANSFER );

    /**
     * Returns the radius for the heatmap
     * \returns heatmap radius
     * \see setRadius
     * \see radiusUnit
     * \see radiusMapUnitScale
     */
    double radius() const { return mRadius; }

    /**
     * Sets the radius for the heatmap
     * \param radius heatmap radius
     * \see radius
     * \see setRadiusUnit
     * \see setRadiusMapUnitScale
     */
    void setRadius( const double radius ) { mRadius = radius; }

    /**
     * Returns the units used for the heatmap's radius
     * \returns units for heatmap radius
     * \see radius
     * \see setRadiusUnit
     * \see radiusMapUnitScale
     */
    QgsUnitTypes::RenderUnit radiusUnit() const { return mRadiusUnit; }

    /**
     * Sets the units used for the heatmap's radius
     * \param unit units for heatmap radius
     * \see radiusUnit
     * \see setRadius
     * \see radiusMapUnitScale
     */
    void setRadiusUnit( const QgsUnitTypes::RenderUnit unit ) { mRadiusUnit = unit; }

    /**
     * Returns the map unit scale used for the heatmap's radius
     * \returns map unit scale for heatmap's radius
     * \see radius
     * \see radiusUnit
     * \see setRadiusMapUnitScale
     */
    const QgsMapUnitScale &radiusMapUnitScale() const { return mRadiusMapUnitScale; }

    /**
     * Sets the map unit scale used for the heatmap's radius
     * \param scale map unit scale for heatmap's radius
     * \see setRadius
     * \see setRadiusUnit
     * \see radiusMapUnitScale
     */
    void setRadiusMapUnitScale( const QgsMapUnitScale &scale ) { mRadiusMapUnitScale = scale; }

    /**
     * Returns the maximum value used for shading the heatmap.
     * \returns maximum value for heatmap shading. If 0, then maximum value will be automatically
     * calculated.
     * \see setMaximumValue
     */
    double maximumValue() const { return mExplicitMax; }

    /**
     * Sets the maximum value used for shading the heatmap.
     * \param value maximum value for heatmap shading. Set to 0 for automatic calculation of
     * maximum value.
     * \see maximumValue
     */
    void setMaximumValue( const double value ) { mExplicitMax = value; }

    /**
     * Returns the render quality used for drawing the heatmap.
     * \returns render quality. A value of 1 indicates maximum quality, and increasing the
     * value will result in faster drawing but lower quality rendering.
     * \see setRenderQuality
     */
    double renderQuality() const { return mRenderQuality; }

    /**
     * Sets the render quality used for drawing the heatmap.
     * \param quality render quality. A value of 1 indicates maximum quality, and increasing the
     * value will result in faster drawing but lower quality rendering.
     * \see renderQuality
     */
    void setRenderQuality( const int quality ) { mRenderQuality = quality; }

    /**
     * Returns the expression used for weighting points when generating the heatmap.
     * \returns point weight expression. If empty, all points are equally weighted.
     * \see setWeightExpression
     */
    QString weightExpression() const { return mWeightExpressionString; }

    /**
     * Sets the expression used for weighting points when generating the heatmap.
     * \param expression point weight expression. If set to empty, all points are equally weighted.
     * \see weightExpression
     */
    void setWeightExpression( const QString &expression ) { mWeightExpressionString = expression; }

  private:

    QVector<double> mValues;

    double mCalculatedMaxValue = 0;

    double mRadius = 10;
    int mRadiusPixels = 0;
    double mRadiusSquared = 0;
    QgsUnitTypes::RenderUnit mRadiusUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mRadiusMapUnitScale;

    QString mWeightExpressionString;
    int mWeightAttrNum = -1;
    std::unique_ptr<QgsExpression> mWeightExpression;

    QgsColorRamp *mGradientRamp = nullptr;

    double mExplicitMax = 0.0;
    int mRenderQuality = 3;

    int mFeaturesRendered = 0;

    double uniformKernel( double distance, int bandwidth ) const;
    double quarticKernel( double distance, int bandwidth ) const;
    double triweightKernel( double distance, int bandwidth ) const;
    double epanechnikovKernel( double distance, int bandwidth ) const;
    double triangularKernel( double distance, int bandwidth ) const;

    QgsMultiPointXY convertToMultipoint( const QgsGeometry *geom );
    void initializeValues( QgsRenderContext &context );
    void renderImage( QgsRenderContext &context );
};


#endif // QGSHEATMAPRENDERER_H
