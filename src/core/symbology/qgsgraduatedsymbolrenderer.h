/***************************************************************************
    qgsgraduatedsymbolrenderer.h
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGRADUATEDSYMBOLRENDERER_H
#define QGSGRADUATEDSYMBOLRENDERER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"
#include "qgsrenderer.h"
#include "qgsrendererrange.h"
#include "qgsclassificationmethod.h"

class QgsVectorLayer;
class QgsColorRamp;
class QgsDataDefinedSizeLegend;
class QgsSymbol;
class QgsExpression;

/**
 * \ingroup core
 * \class QgsGraduatedSymbolRenderer
 */
class CORE_EXPORT QgsGraduatedSymbolRenderer : public QgsFeatureRenderer
{
  public:

    QgsGraduatedSymbolRenderer( const QString &attrName = QString(), const QgsRangeList &ranges = QgsRangeList() );

    ~QgsGraduatedSymbolRenderer() override;

    QgsSymbol *symbolForFeature( const QgsFeature &feature, QgsRenderContext &context ) const override;
    QgsSymbol *originalSymbolForFeature( const QgsFeature &feature, QgsRenderContext &context ) const override;
    void startRender( QgsRenderContext &context, const QgsFields &fields ) override;
    void stopRender( QgsRenderContext &context ) override;
    QSet<QString> usedAttributes( const QgsRenderContext &context ) const override;
    bool filterNeedsGeometry() const override;
    QString dump() const override;
    QgsGraduatedSymbolRenderer *clone() const override SIP_FACTORY;
    void toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props = QVariantMap() ) const override;
    QgsFeatureRenderer::Capabilities capabilities() override { return SymbolLevels | Filter; }
    QgsSymbolList symbols( QgsRenderContext &context ) const override;
    bool accept( QgsStyleEntityVisitorInterface *visitor ) const override;

    /**
     * Returns the attribute name (or expression) used for the classification.
     *
     * \see setClassAttribute()
     */
    QString classAttribute() const { return mAttrName; }

    /**
     * Sets the attribute name (or expression) used for the classification.
     *
     * \see classAttribute()
     */
    void setClassAttribute( const QString &attr ) { mAttrName = attr; }

    /**
     * Returns a list of all ranges used in the classification.
     */
    const QgsRangeList &ranges() const { return mRanges; }

    bool updateRangeSymbol( int rangeIndex, QgsSymbol *symbol SIP_TRANSFER );
    bool updateRangeLabel( int rangeIndex, const QString &label );
    bool updateRangeUpperValue( int rangeIndex, double value );
    bool updateRangeLowerValue( int rangeIndex, double value );
    //! \since QGIS 2.5
    bool updateRangeRenderState( int rangeIndex, bool render );

    void addClass( QgsSymbol *symbol );
    //! \note available in Python bindings as addClassRange
    void addClass( const QgsRendererRange &range ) SIP_PYNAME( addClassRange );
    //! \note available in Python bindings as addClassLowerUpper
    void addClass( double lower, double upper ) SIP_PYNAME( addClassLowerUpper );

    /**
     * Add a breakpoint by splitting existing classes so that the specified
     * value becomes a break between two classes.
     * \param breakValue position to insert break
     * \param updateSymbols set to TRUE to reapply ramp colors to the new
     * symbol ranges
     * \since QGIS 2.9
     */
    void addBreak( double breakValue, bool updateSymbols = true );

    void deleteClass( int idx );
    void deleteAllClasses();

    //! Moves the category at index position from to index position to.
    void moveClass( int from, int to );

    /**
     * Tests whether classes assigned to the renderer have ranges which overlap.
     * \returns TRUE if ranges overlap
     * \since QGIS 2.10
     */
    bool rangesOverlap() const;

    /**
     * Tests whether classes assigned to the renderer have gaps between the ranges.
     * \returns TRUE if ranges have gaps
     * \since QGIS 2.10
     */
    bool rangesHaveGaps() const;

    void sortByValue( Qt::SortOrder order = Qt::AscendingOrder );
    void sortByLabel( Qt::SortOrder order = Qt::AscendingOrder );

    /**
     * Returns the classification method
     * \since QGIS 3.10
     */
    QgsClassificationMethod *classificationMethod() const;

    /**
     * Defines the classification method
     * This will take ownership of the method
     * \since QGIS 3.10
     */
    void setClassificationMethod( QgsClassificationMethod *method SIP_TRANSFER );

    /**
      * Classification mode
      * \deprecated since QGIS 3.10 use QgsClassificationMethod::MethodId instead
      */
    enum Mode
    {
      EqualInterval,
      Quantile,
      Jenks,
      StdDev,
      Pretty,
      Custom
    };
    // TODO QGIS 4: remove
    // this could not be tagged with Q_DECL_DEPRECATED due to Doxygen warning
    // might be fixed in newer Doxygen (does not on 1.8.13, might be ok on 1.8.16)


    //! \deprecated since QGIS 3.10 use classficationMethod instead
    Q_DECL_DEPRECATED Mode mode() const SIP_DEPRECATED { return modeFromMethodId( mClassificationMethod->id() ); }
    //! \deprecated since QGIS 3.10 use classficationMethod instead
    Q_DECL_DEPRECATED void setMode( Mode mode ) SIP_DEPRECATED;

    /**
     * Returns if we want to classify symmetric around a given value
     * \since QGIS 3.4
     * \deprecated since QGIS 3.10 use classficationMethod instead
     */
    Q_DECL_DEPRECATED bool useSymmetricMode() const SIP_DEPRECATED { return mClassificationMethod->symmetricModeEnabled(); }

    /**
     * Set if we want to classify symmetric around a given value
     * \since QGIS 3.4
     * \deprecated since QGIS 3.10 use classficationMethod instead
     */
    Q_DECL_DEPRECATED  void setUseSymmetricMode( bool useSymmetricMode ) SIP_DEPRECATED;

    /**
     * Returns the pivot value for symmetric classification
     * \since QGIS 3.4
     * \deprecated since QGIS 3.10 use classficationMethod instead
     */
    Q_DECL_DEPRECATED double symmetryPoint() const SIP_DEPRECATED { return mClassificationMethod->symmetryPoint(); }

    /**
     * Set the pivot point
     * \since QGIS 3.4
     * \deprecated since QGIS 3.10 use classficationMethod instead
     */
    Q_DECL_DEPRECATED void setSymmetryPoint( double symmetryPoint ) SIP_DEPRECATED;


    /**
     * Returns if we want to have a central class astride the pivot value
     * \since QGIS 3.4
     * \deprecated since QGIS 3.10 use classficationMethod instead
     */
    Q_DECL_DEPRECATED bool astride() const SIP_DEPRECATED { return mClassificationMethod->symmetryAstride(); }

    /**
     * Set if we want a central class astride the pivot value
     * \since QGIS 3.4
     * \deprecated since QGIS 3.10 use classficationMethod instead
     */
    Q_DECL_DEPRECATED void setAstride( bool astride ) SIP_DEPRECATED;

    /**
     * Remove the breaks that are above the existing opposite sign classes to keep colors symmetrically balanced around symmetryPoint
     * Does not put a break on the symmetryPoint. This is done before.
     * \param breaks The breaks of an already-done classification
     * \param symmetryPoint The point around which we want a symmetry
     * \param astride A bool indicating if the symmetry is made astride the symmetryPoint or not ( [-1,1] vs. [-1,0][0,1] )
     * \since QGIS 3.4
     * \deprecated since QGIS 3.10, use QgsClassificationMethod::makeBreaksSymmetric instead
     */
    Q_DECL_DEPRECATED static void makeBreaksSymmetric( QList<double> &breaks SIP_INOUT, double symmetryPoint, bool astride ) SIP_DEPRECATED;

    /**
     * Compute the equal interval classification
     * \param minimum The minimum value of the distribution
     * \param maximum The maximum value of the distribution
     * \param classes The number of classes desired
     * \param useSymmetricMode A bool indicating if we want to have classes and hence colors ramp symmetric around a value
     * \param symmetryPoint The point around which we want a symmetry
     * \param astride A bool indicating if the symmetry is made astride the symmetryPoint or not ( [-1,1] vs. [-1,0][0,1] )
     * \deprecated since QGIS 3.10 use QgsClassificationEqualInterval class instead
     */
    Q_DECL_DEPRECATED static QList<double> calcEqualIntervalBreaks( double minimum, double maximum, int classes, bool useSymmetricMode, double symmetryPoint, bool astride ) SIP_DEPRECATED;

    /**
     * Recalculate classes for a layer
     * \param vlayer  The layer being rendered (from which data values are calculated)
     * \param mode    The calculation mode
     * \param nclasses The number of classes to calculate (approximate for some modes)
     * \param useSymmetricMode A bool indicating if we want to have classes and hence colors ramp symmetric around a value
     * \param symmetryPoint The value around which the classes will be symmetric if useSymmetricMode is checked
     * \param astride A bool indicating if the symmetry is made astride the symmetryPoint or not ( [-1,1] vs. [-1,0][0,1] )
     * \since QGIS 2.6 (three first arguments) and 3.4 (three last arguments)
     * \deprecated since QGIS 3.10
     */
    Q_DECL_DEPRECATED void updateClasses( QgsVectorLayer *vlayer, Mode mode, int nclasses, bool useSymmetricMode = false, double symmetryPoint = 0.0, bool astride = false ) SIP_DEPRECATED;

    /**
     * Recalculate classes for a layer
     * \param vl  The layer being rendered (from which data values are calculated)
     * \param nclasses the number of classes
     */
    void updateClasses( const QgsVectorLayer *vl, int nclasses );

    Q_NOWARN_DEPRECATED_PUSH;

    /**
     * Returns the label format used to generate default classification labels
     * \since QGIS 2.6
     * \deprecated since QGIS 3.10 use classificationMethod() and QgsClassificationMethod::setLabelFormat instead
     */
    Q_DECL_DEPRECATED QgsRendererRangeLabelFormat labelFormat() const SIP_DEPRECATED;

    /**
     * Set the label format used to generate default classification labels
     * \param labelFormat The string appended to classification labels
     * \param updateRanges If TRUE then ranges ending with the old unit string are updated to the new.
     * \since QGIS 2.6
     * \deprecated since QGIS 3.10 use classificationMethod() and QgsClassificationMethod::setLabelFormat instead
     */
    Q_DECL_DEPRECATED void setLabelFormat( const QgsRendererRangeLabelFormat &labelFormat, bool updateRanges = false ) SIP_DEPRECATED;

    Q_NOWARN_DEPRECATED_POP;

    /**
     * Reset the label decimal places to a numberbased on the minimum class interval
     * \param updateRanges if TRUE then ranges currently using the default label will be updated
     * \since QGIS 2.6
     */
    void calculateLabelPrecision( bool updateRanges = true );

    Q_NOWARN_DEPRECATED_PUSH;

    /**
     * Creates a new graduated renderer.
     * \param vlayer vector layer
     * \param attrName attribute to classify
     * \param classes number of classes
     * \param mode classification mode
     * \param symbol base symbol
     * \param ramp color ramp for classes
     * \param legendFormat
     * \param useSymmetricMode A bool indicating if we want to have classes and hence colors ramp symmetric around a value
     * \param symmetryPoint The value around which the classes will be symmetric if useSymmetricMode is checked
     * \param listForCboPrettyBreaks The list of potential pivot values for symmetric mode with prettybreaks mode
     * \param astride A bool indicating if the symmetry is made astride the symmetryPoint or not ( [-1,1] vs. [-1,0][0,1] )
     * \returns new QgsGraduatedSymbolRenderer object
     * \deprecated since QGIS 3.10
     */
    Q_DECL_DEPRECATED static QgsGraduatedSymbolRenderer *createRenderer( QgsVectorLayer *vlayer,
        const QString &attrName,
        int classes,
        Mode mode,
        QgsSymbol *symbol SIP_TRANSFER,
        QgsColorRamp *ramp SIP_TRANSFER,
        const QgsRendererRangeLabelFormat &legendFormat = QgsRendererRangeLabelFormat(),
        bool useSymmetricMode = false,
        double symmetryPoint = 0.0,
        const QStringList &listForCboPrettyBreaks = QStringList(),
        bool astride = false ) SIP_DEPRECATED;
    Q_NOWARN_DEPRECATED_POP;

    //! create renderer from XML element
    static QgsFeatureRenderer *create( QDomElement &element, const QgsReadWriteContext &context ) SIP_FACTORY;

    QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) override;
    QgsLegendSymbolList legendSymbolItems() const override;
    QSet< QString > legendKeysForFeature( const QgsFeature &feature, QgsRenderContext &context ) const override;
    QString legendKeyToExpression( const QString &key, QgsVectorLayer *layer, bool &ok ) const override;

    /**
     * Returns the renderer's source symbol, which is the base symbol used for the each classes' symbol before applying
     * the classes' color.
     * \see setSourceSymbol()
     * \see sourceColorRamp()
     */
    QgsSymbol *sourceSymbol();

    /**
     * Returns the renderer's source symbol, which is the base symbol used for the each classes' symbol before applying
     * the classes' color.
     * \see setSourceSymbol()
     * \see sourceColorRamp()
     * \note Not available in Python bindings.
     */
    const QgsSymbol *sourceSymbol() const SIP_SKIP;

    /**
     * Sets the source symbol for the renderer, which is the base symbol used for the each classes' symbol before applying
     * the classes' color.
     * \param sym source symbol, ownership is transferred to the renderer
     * \see sourceSymbol()
     * \see setSourceColorRamp()
     */
    void setSourceSymbol( QgsSymbol *sym SIP_TRANSFER );

    /**
     * Returns the source color ramp, from which each classes' color is derived.
     * \see setSourceColorRamp()
     * \see sourceSymbol()
     */
    QgsColorRamp *sourceColorRamp();

    /**
     * Returns the source color ramp, from which each classes' color is derived.
     * \see setSourceColorRamp()
     * \see sourceSymbol()
     * \note Not available in Python bindings.
     */
    const QgsColorRamp *sourceColorRamp() const SIP_SKIP;

    /**
     * Sets the source color ramp.
     * \param ramp color ramp. Ownership is transferred to the renderer
     */
    void setSourceColorRamp( QgsColorRamp *ramp SIP_TRANSFER );

    /**
     * Update the color ramp used. Also updates all symbols colors.
     * Doesn't alter current breaks.
     * \param ramp color ramp. Ownership is transferred to the renderer
     */
    void updateColorRamp( QgsColorRamp *ramp SIP_TRANSFER = nullptr );

    /**
     * Update all the symbols but leave breaks and colors. This method also sets the source
     * symbol for the renderer.
     * \param sym source symbol to use for classes. Ownership is not transferred.
     * \see setSourceSymbol()
     */
    void updateSymbols( QgsSymbol *sym );

    /**
     * set varying symbol size for classes
     * \note the classes must already be set so that symbols exist
     * \since QGIS 2.10
     */
    void setSymbolSizes( double minSize, double maxSize );

    /**
     * Returns the min symbol size when graduated by size
     * \since QGIS 2.10
     */
    double minSymbolSize() const;

    /**
     * Returns the max symbol size when graduated by size
     * \since QGIS 2.10
     */
    double maxSymbolSize() const;

    /**
     * Returns the method used for graduation (either size or color).
     *
     * \see setGraduatedMethod()
     * \since QGIS 2.10
     */
    Qgis::GraduatedMethod graduatedMethod() const { return mGraduatedMethod; }

    /**
     * Set the \a method used for graduation (either size or color).
     *
     * \see graduatedMethod()
     * \since QGIS 2.10
     */
    void setGraduatedMethod( Qgis::GraduatedMethod method ) { mGraduatedMethod = method; }

    bool legendSymbolItemsCheckable() const override;
    bool legendSymbolItemChecked( const QString &key ) override;
    void checkLegendSymbolItem( const QString &key, bool state = true ) override;
    void setLegendSymbolItem( const QString &key, QgsSymbol *symbol SIP_TRANSFER ) override;
    QString legendClassificationAttribute() const override { return classAttribute(); }

    /**
     * creates a QgsGraduatedSymbolRenderer from an existing renderer.
     * \returns a new renderer if the conversion was possible, otherwise NULLPTR.
     * \since QGIS 2.6
     */
    static QgsGraduatedSymbolRenderer *convertFromRenderer( const QgsFeatureRenderer *renderer ) SIP_FACTORY;

    /**
     * Configures appearance of legend when renderer is configured to use data-defined size for marker symbols.
     * This allows configuring for which values (symbol sizes) should be shown in the legend, whether to display
     * different symbol sizes collapsed in one legend node or separated across multiple legend nodes etc.
     *
     * When renderer does not use data-defined size or does not use marker symbols, these settings will be ignored.
     * Takes ownership of the passed settings objects. NULLPTR is a valid input that disables data-defined
     * size legend.
     * \since QGIS 3.0
     */
    void setDataDefinedSizeLegend( QgsDataDefinedSizeLegend *settings SIP_TRANSFER );

    /**
     * Returns configuration of appearance of legend when using data-defined size for marker symbols.
     * Will return NULLPTR if the functionality is disabled.
     * \since QGIS 3.0
     */
    QgsDataDefinedSizeLegend *dataDefinedSizeLegend() const;

    /**
     * Updates the labels of the ranges
     * \since QGIS 3.10
     */
    void updateRangeLabels();

    /**
     * Returns the renderer range matching the provided \a value, or NULLPTR if no
     * range matches the value.
     *
     * \since QGIS 3.10.1
     */
    const QgsRendererRange *rangeForValue( double value ) const;

  protected:
    QString mAttrName;
    QgsRangeList mRanges;
    std::unique_ptr<QgsSymbol> mSourceSymbol;
    std::unique_ptr<QgsColorRamp> mSourceColorRamp;

    std::unique_ptr<QgsExpression> mExpression;
    Qgis::GraduatedMethod mGraduatedMethod = Qgis::GraduatedMethod::Color;
    //! attribute index (derived from attribute name in startRender)

    int mAttrNum = -1;
    bool mCounting = false;

    std::unique_ptr<QgsDataDefinedSizeLegend> mDataDefinedSizeLegend;

    /**
     * Gets the symbol which is used to represent \a value.
     */
    QgsSymbol *symbolForValue( double value ) const;

    /**
     * Returns the matching legend key for a value.
     */
    QString legendKeyForValue( double value ) const;

    //! \note not available in Python bindings
    static QString graduatedMethodStr( Qgis::GraduatedMethod method ) SIP_SKIP;

    std::shared_ptr<QgsClassificationMethod> mClassificationMethod;

  private:

    /**
     * Returns calculated value used for classifying a feature.
     */
    QVariant valueForFeature( const QgsFeature &feature, QgsRenderContext &context ) const;

    //! Returns list of legend symbol items from individual ranges
    QgsLegendSymbolList baseLegendSymbolItems() const;

    // TODO QGIS 4: remove
    Q_NOWARN_DEPRECATED_PUSH
    static QString methodIdFromMode( QgsGraduatedSymbolRenderer::Mode mode );
    static QgsGraduatedSymbolRenderer::Mode modeFromMethodId( const QString &methodId );
    Q_NOWARN_DEPRECATED_POP

#ifdef SIP_RUN
    QgsGraduatedSymbolRenderer( const QgsGraduatedSymbolRenderer & );
    QgsGraduatedSymbolRenderer &operator=( const QgsGraduatedSymbolRenderer & );
#endif

    friend class TestQgsGraduatedSymbolRenderer;

};

#endif // QGSGRADUATEDSYMBOLRENDERER_H
