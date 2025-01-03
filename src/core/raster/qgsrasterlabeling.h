/***************************************************************************
                         qgsrasterlabeling.h
                         ---------------
    begin                : December 2024
    copyright            : (C) 2024 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSRASTERLABELING_H
#define QGSRASTERLABELING_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgslabelingengine.h"
#include "qgstextformat.h"
#include "qgslabelplacementsettings.h"
#include "qgslabelthinningsettings.h"

class QgsRasterLayer;
class QgsNumericFormat;
class QgsRasterPipe;
struct QgsRasterViewPort;
class QgsRasterLayerRendererFeedback;

#ifndef SIP_RUN

/**
 * \ingroup core
 * \brief Implements labeling support for raster layers.
 *
 * \note Not available in Python bindings
 *
 * \since QGIS 3.42
 */
class CORE_EXPORT QgsRasterLayerLabelProvider final : public QgsAbstractLabelProvider
{
  public:

    /**
     * Constructor for QgsRasterLayerLabelProvider.
     */
    explicit QgsRasterLayerLabelProvider( QgsRasterLayer *layer );

    ~QgsRasterLayerLabelProvider() final;
    QList<QgsLabelFeature *> labelFeatures( QgsRenderContext & ) final;
    void drawLabel( QgsRenderContext &context, pal::LabelPosition *label ) const final;
    void startRender( QgsRenderContext &context ) final;

    /**
     * Generates the labels, given a render context and input pipe.
     */
    void generateLabels( QgsRenderContext &context, QgsRasterPipe *pipe, QgsRasterViewPort *rasterViewPort, QgsRasterLayerRendererFeedback *feedback );

    /**
     * Adds a label at the specified point in map coordinates.
     */
    void addLabel( const QgsPoint &mapPoint, const QString &text, QgsRenderContext &context );

    /**
     * Sets the text \a format used for rendering the labels.
     */
    void setTextFormat( const QgsTextFormat &format );

    /**
     * Sets the numeric \a format used for the labels.
     *
     * \see numericFormat()
     */
    void setNumericFormat( std::unique_ptr< QgsNumericFormat > format );

    /**
     * Returns the numeric format to be used for the labels.
     *
     * \see setNumericFormat()
     */
    QgsNumericFormat *numericFormat();

    /**
     * Returns the raster band to use for label values.
     *
     * \see setBand()
     */
    int band() const { return mBandNumber; }

    /**
     * Sest the raster \a band to use for label values.
     *
     * \see band()
     */
    void setBand( int band ) { mBandNumber = band; }

    /**
     * Sets the \a priority of labels.
     *
     * This is a value between 0 to 1, where 0 = highest priority and 1 = lowest priority.
     */
    void setPriority( double priority ) { mPriority = priority; }

    /**
     * Sets the label placement \a settings.
     */
    void setPlacementSettings( const QgsLabelPlacementSettings &settings ) { mPlacementSettings = settings; }

    /**
     * Sets the Z-Index of the labels.
     *
     * Labels with a higher z-index are rendered on top of labels with a lower z-index.
     */
    void setZIndex( double index ) { mZIndex = index; }

    /**
     * Sets the label thinning \a settings.
     */
    void setThinningSettings( const QgsLabelThinningSettings &settings ) { mThinningSettings = settings; }

    /**
     * Sets the resampling \a method to use when the raster labels are being
     * resampled over neighboring pixels.
     *
     * \see setResampleOver()
     */
    void setResampleMethod( Qgis::RasterResamplingMethod method );

    /**
     * Sets the number of neighboring \a pixels to resample over, when labels are
     * showing values resampled over neighboring pixels.
     *
     * \see setResampleMethod()
     */
    void setResampleOver( int pixels );

  private:
    QgsTextFormat mFormat;
    int mBandNumber = 1;
    std::unique_ptr< QgsNumericFormat > mNumericFormat;

    QgsLabelPlacementSettings mPlacementSettings;
    QgsLabelThinningSettings mThinningSettings;
    double mZIndex = 0;

    Qgis::RasterResamplingMethod mResampleMethod = Qgis::RasterResamplingMethod::Average;
    int mResampleOver = 1;

    QList<QgsLabelFeature *> mLabels;

};

#endif



/**
 * \ingroup core
 * \brief Abstract base class for labeling settings for raster layers.
 *
 * \since QGIS 3.42
 */
class CORE_EXPORT QgsAbstractRasterLayerLabeling SIP_ABSTRACT
{
  public:

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( sipCpp->type() == "simple" )
      sipType = sipType_QgsRasterLayerSimpleLabeling;
    else
      sipType = 0;
    SIP_END
#endif

    QgsAbstractRasterLayerLabeling() = default;
    virtual ~QgsAbstractRasterLayerLabeling() = default;
#ifndef SIP_RUN
    //! QgsAbstractRasterLayerLabeling cannot be copied, use clone() instead
    QgsAbstractRasterLayerLabeling( const QgsAbstractRasterLayerLabeling &rhs ) = delete;
    //! QgsAbstractRasterLayerLabeling cannot be copied, use clone() instead
    QgsAbstractRasterLayerLabeling &operator=( const QgsAbstractRasterLayerLabeling &rhs ) = delete;
#endif

    /**
     * Creates default labeling for a raster \a layer.
     */
    static QgsAbstractRasterLayerLabeling *defaultLabelingForLayer( QgsRasterLayer *layer ) SIP_FACTORY;

    //! Unique type string of the labeling configuration implementation
    virtual QString type() const = 0;

    //! Returns a new copy of the object
    virtual QgsAbstractRasterLayerLabeling *clone() const = 0 SIP_FACTORY;

    /**
     * Creates a raster label provider corresponding to this object's configuration.
     *
     * \note not available in Python bindings
     */
    virtual std::unique_ptr< QgsRasterLayerLabelProvider > provider( QgsRasterLayer *layer ) const = 0 SIP_SKIP;

    /**
     * Saves the labeling configuration to an XML element.
     *
     * \see createFromElement()
     */
    virtual QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) const = 0;

    /**
     * Returns TRUE if drawing labels requires advanced effects like composition
     * modes, which could prevent it being used as an isolated cached image
     * or exported to a vector format.
     */
    virtual bool requiresAdvancedEffects() const = 0;

    /**
     * Multiply opacity by \a opacityFactor.
     *
     * This method multiplies the opacity of the labeling elements (text, shadow, buffer etc.)
     * by \a opacity effectively changing the opacity of the whole labeling elements.
     */
    virtual void multiplyOpacity( double opacityFactor );

    /**
     * Tests whether the labels should be visible at the specified \a scale.
     * The \a scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * \returns TRUE if the labels are visible at the given scale.
     */
    virtual bool isInScaleRange( double scale ) const;

    // static stuff

    /**
     * Tries to create an instance of an implementation based on the XML data.
     */
    static QgsAbstractRasterLayerLabeling *createFromElement( const QDomElement &element, const QgsReadWriteContext &context ) SIP_FACTORY;

    /**
     * Writes the SE 1.1 TextSymbolizer element based on the current layer labeling settings
     */
    virtual void toSld( QDomNode &parent, const QVariantMap &props ) const;

    /**
     * Accepts the specified symbology \a visitor, causing it to visit all symbols associated
     * with the labeling.
     *
     * Returns TRUE if the visitor should continue visiting other objects, or FALSE if visiting
     * should be canceled.
     */
    virtual bool accept( QgsStyleEntityVisitorInterface *visitor ) const;

  private:

#ifdef SIP_RUN
    QgsAbstractRasterLayerLabeling( const QgsAbstractRasterLayerLabeling &rhs );
#endif

};


/**
 * \ingroup core
 * \brief Basic implementation of the labeling interface for raster layers.
 *
 * \since QGIS 3.42
 */
class CORE_EXPORT QgsRasterLayerSimpleLabeling : public QgsAbstractRasterLayerLabeling
{
  public:

    explicit QgsRasterLayerSimpleLabeling();
    ~QgsRasterLayerSimpleLabeling() override;

    QString type() const override;
    QgsRasterLayerSimpleLabeling *clone() const override SIP_FACTORY;
    std::unique_ptr< QgsRasterLayerLabelProvider > provider( QgsRasterLayer *layer ) const override SIP_SKIP;
    QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) const override;
    bool accept( QgsStyleEntityVisitorInterface *visitor ) const override;
    bool requiresAdvancedEffects() const override;
    void multiplyOpacity( double opacityFactor ) override;

    //! Creates a QgsRasterLayerSimpleLabeling from a DOM element with saved configuration
    static QgsRasterLayerSimpleLabeling *create( const QDomElement &element, const QgsReadWriteContext &context ) SIP_FACTORY;

    /**
     * Returns the text format used for rendering the labels.
     *
     * \see setTextFormat()
     */
    QgsTextFormat textFormat() const;

    /**
     * Sets the text \a format used for rendering the labels.
     *
     * \see textFormat()
     */
    void setTextFormat( const QgsTextFormat &format );

    /**
     * Returns the numeric format used for the labels.
     *
     * \see setNumericFormat()
     */
    const QgsNumericFormat *numericFormat() const;

    /**
     * Sets the numeric \a format used for the labels.
     *
     * Ownership of \a format is transferred to the labeling.
     *
     * \see numericFormat()
     */
    void setNumericFormat( QgsNumericFormat *format SIP_TRANSFER );

    /**
     * Returns the raster band to use for label values.
     *
     * \see setBand()
     */
    int band() const { return mBandNumber; }

    /**
     * Sest the raster \a band to use for label values.
     *
     * \see band()
     */
    void setBand( int band ) { mBandNumber = band; }

    /**
     * Returns the priority of labels.
     *
     * This is a value between 0 to 1, where 0 = highest priority and 1 = lowest priority.
     *
     * The default is 0.5.
     *
     * \see setPriority()
     */
    double priority() const { return mPriority; }

    /**
     * Sets the \a priority of labels.
     *
     * This is a value between 0 to 1, where 0 = highest priority and 1 = lowest priority.
     *
     * \see priority()
     */
    void setPriority( double priority ) { mPriority = priority; }

    /**
     * Returns the label placement settings.
     * \see setPlacementSettings()
     * \note Not available in Python bindings
     */
    const QgsLabelPlacementSettings &placementSettings() const { return mPlacementSettings; } SIP_SKIP

    /**
     * Returns the label placement settings.
     * \see setPlacementSettings()
     */
    QgsLabelPlacementSettings &placementSettings() { return mPlacementSettings; }

    /**
     * Sets the label placement \a settings.
     * \see placementSettings()
     */
    void setPlacementSettings( const QgsLabelPlacementSettings &settings ) { mPlacementSettings = settings; }

    /**
     * Returns the label thinning settings.
     * \see setThinningSettings()
     * \note Not available in Python bindings
     */
    const QgsLabelThinningSettings &thinningSettings() const { return mThinningSettings; } SIP_SKIP

    /**
    * Returns the label thinning settings.
    * \see setThinningSettings()
    */
    QgsLabelThinningSettings &thinningSettings() { return mThinningSettings; }

    /**
     * Sets the label thinning \a settings.
     * \see thinningSettings()
     */
    void setThinningSettings( const QgsLabelThinningSettings &settings ) { mThinningSettings = settings; }

    /**
     * Returns the Z-Index of the labels.
     *
     * Labels with a higher z-index are rendered on top of labels with a lower z-index.
     *
     * \see setZIndex()
     */
    double zIndex() const;

    /**
     * Sets the Z-Index of the labels.
     *
     * Labels with a higher z-index are rendered on top of labels with a lower z-index.
     *
     * \see zIndex()
     */
    void setZIndex( double index );

    /**
     * Returns the maximum map scale (i.e. most "zoomed in" scale) at which the labels will be visible.
     *
     * The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * A scale of 0 indicates no maximum scale visibility.
     *
     * This setting is only considered if hasScaleBasedVisibility() is TRUE.
     *
     * \see setMaximumScale()
     * \see minimumScale()
     * \see hasScaleBasedVisibility()
    */
    double maximumScale() const;

    /**
     * Sets the maximum map \a scale (i.e. most "zoomed in" scale) at which the labels will be visible.
     *
     * The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * A scale of 0 indicates no maximum scale visibility.
     *
     * This setting is only considered if hasScaleBasedVisibility() is TRUE.
     *
     * \see maximumScale()
     * \see setMinimumScale()
     * \see setScaleBasedVisibility()
    */
    void setMaximumScale( double scale );

    /**
     * Returns the minimum map scale (i.e. most "zoomed out" scale) at which the labels will be visible.
     *
     * The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * A scale of 0 indicates no minimum scale visibility.
     *
     * This setting is only considered if hasScaleBasedVisibility() is TRUE.
     *
     * \see setMinimumScale()
     * \see maximumScale()
     * \see hasScaleBasedVisibility()
    */
    double minimumScale() const;

    /**
     * Sets the minimum map \a scale (i.e. most "zoomed out" scale) at which the labels will be visible.
     *
     * The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * A scale of 0 indicates no minimum scale visibility.
     *
     * This setting is only considered if hasScaleBasedVisibility() is TRUE.
     *
     * \see minimumScale()
     * \see setMaximumScale()
     * \see hasScaleBasedVisibility()
    */
    void setMinimumScale( double scale );

    /**
     * Sets whether scale based visibility is enabled for the labels.
     * \see setMinimumScale()
     * \see setMaximumScale()
     * \see hasScaleBasedVisibility()
     */
    void setScaleBasedVisibility( bool enabled );

    /**
     * Returns whether scale based visibility is enabled for the labels.

     * \see minimumScale()
     * \see maximumScale()
     * \see setScaleBasedVisibility()
     */
    bool hasScaleBasedVisibility() const;

    bool isInScaleRange( double scale ) const override;

    /**
     * Returns the resampling method used when the raster labels are being
     * resampled over neighboring pixels.
     *
     * \see setResampleMethod()
     * \see resampleOver()
     */
    Qgis::RasterResamplingMethod resampleMethod() const;

    /**
     * Sets the resampling \a method to use when the raster labels are being
     * resampled over neighboring pixels.
     *
     * \see resampleMethod()
     * \see setResampleOver()
     */
    void setResampleMethod( Qgis::RasterResamplingMethod method );

    /**
     * Returns the number of neighboring pixels to resample over, when labels are
     * showing values resampled over neighboring pixels.
     *
     * \see setResampleOver()
     * \see resampleMethod()
     */
    int resampleOver() const;

    /**
     * Sets the number of neighboring \a pixels to resample over, when labels are
     * showing values resampled over neighboring pixels.
     *
     * \see resampleOver()
     * \see setResampleMethod()
     */
    void setResampleOver( int pixels );

  private:
    int mBandNumber = 1;

    QgsTextFormat mTextFormat;

    std::unique_ptr< QgsNumericFormat > mNumericFormat;

    //! Priority of labels. 0 = highest priority, 1 = lowest priority
    double mPriority = 0.5;

    QgsLabelPlacementSettings mPlacementSettings;
    QgsLabelThinningSettings mThinningSettings;

    double mZIndex = 0;

    bool mScaleVisibility = false;
    double mMaximumScale = 0;
    double mMinimumScale = 0;

    Qgis::RasterResamplingMethod mResampleMethod = Qgis::RasterResamplingMethod::Average;
    int mResampleOver = 1;

};



#endif // QGSRASTERLABELING_H
