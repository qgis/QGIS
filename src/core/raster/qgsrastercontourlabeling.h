/***************************************************************************
                         qgsrastercontourlabeling.h
                         ---------------
    begin                : February 2026
    copyright            : (C) 2026 by the QGIS project
    email                : info at qgis dot org
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERCONTOURLABELING_H
#define QGSRASTERCONTOURLABELING_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsrasterlabeling.h"

class QgsLineString;
class QgsNumericFormat;

#ifndef SIP_RUN

/**
 * \ingroup core
 * \brief Implements labeling for raster contour lines.
 *
 * Generates contour line geometries from a raster band using GDAL and registers
 * them with the labeling engine for placement along the contour lines.
 *
 * \note Not available in Python bindings
 *
 * \since QGIS 3.44
 */
class CORE_EXPORT QgsRasterContourLabelProvider : public QgsRasterLayerLabelProvider
{
  public:
    explicit QgsRasterContourLabelProvider( QgsRasterLayer *layer );
    ~QgsRasterContourLabelProvider() override;

    void generateLabels( QgsRenderContext &context, QgsRasterPipe *pipe, QgsRasterViewPort *rasterViewPort, QgsRasterLayerRendererFeedback *feedback ) override;

    void setContourInterval( double interval ) { mContourInterval = interval; }
    void setContourIndexInterval( double interval ) { mContourIndexInterval = interval; }
    void setInputBand( int band ) { mInputBand = band; }
    void setDownscale( double downscale ) { mDownscale = downscale; }
    void setLabelIndexOnly( bool indexOnly ) { mLabelIndexOnly = indexOnly; }

    void addContourLabel( const QgsLineString &line, const QString &text, QgsRenderContext &context );

  private:
    double mContourInterval = 100.0;
    double mContourIndexInterval = 0.0;
    int mInputBand = 1;
    double mDownscale = 4.0;
    bool mLabelIndexOnly = false;
};

#endif

/**
 * \ingroup core
 * \brief Labeling configuration for raster contour lines.
 *
 * Produces labels placed along contour lines generated on-the-fly from raster data.
 *
 * \since QGIS 3.44
 */
class CORE_EXPORT QgsRasterLayerContourLabeling : public QgsAbstractRasterLayerLabeling
{
  public:

    QgsRasterLayerContourLabeling();
    ~QgsRasterLayerContourLabeling() override;

    QString type() const override;
    QgsRasterLayerContourLabeling *clone() const override SIP_FACTORY;
    std::unique_ptr< QgsRasterLayerLabelProvider > provider( QgsRasterLayer *layer ) const override SIP_SKIP;
    QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) const override;
    bool accept( QgsStyleEntityVisitorInterface *visitor ) const override;
    bool requiresAdvancedEffects() const override;
    bool hasNonDefaultCompositionMode() const override;
    void multiplyOpacity( double opacityFactor ) override;
    bool isInScaleRange( double scale ) const override;

    //! Creates a QgsRasterLayerContourLabeling from a DOM element with saved configuration
    static QgsRasterLayerContourLabeling *create( const QDomElement &element, const QgsReadWriteContext &context ) SIP_FACTORY;

    QgsTextFormat textFormat() const;
    void setTextFormat( const QgsTextFormat &format );

    const QgsNumericFormat *numericFormat() const;
    void setNumericFormat( QgsNumericFormat *format SIP_TRANSFER );

    bool labelIndexOnly() const { return mLabelIndexOnly; }
    void setLabelIndexOnly( bool indexOnly ) { mLabelIndexOnly = indexOnly; }

    double priority() const { return mPriority; }
    void setPriority( double priority ) { mPriority = priority; }

    const QgsLabelPlacementSettings &placementSettings() const { return mPlacementSettings; } SIP_SKIP
    QgsLabelPlacementSettings &placementSettings() { return mPlacementSettings; }
    void setPlacementSettings( const QgsLabelPlacementSettings &settings ) { mPlacementSettings = settings; }

    const QgsLabelThinningSettings &thinningSettings() const { return mThinningSettings; } SIP_SKIP
    QgsLabelThinningSettings &thinningSettings() { return mThinningSettings; }
    void setThinningSettings( const QgsLabelThinningSettings &settings ) { mThinningSettings = settings; }

    double zIndex() const;
    void setZIndex( double index );

    double maximumScale() const;
    void setMaximumScale( double scale );

    double minimumScale() const;
    void setMinimumScale( double scale );

    void setScaleBasedVisibility( bool enabled );
    bool hasScaleBasedVisibility() const;

  private:
    bool mLabelIndexOnly = false;

    QgsTextFormat mTextFormat;
    std::unique_ptr< QgsNumericFormat > mNumericFormat;

    double mPriority = 0.5;
    QgsLabelPlacementSettings mPlacementSettings;
    QgsLabelThinningSettings mThinningSettings;
    double mZIndex = 0;

    bool mScaleVisibility = false;
    double mMaximumScale = 0;
    double mMinimumScale = 0;
};

#endif // QGSRASTERCONTOURLABELING_H
