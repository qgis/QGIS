/***************************************************************************
    qgsgraduatedsymbolrendererv2.h
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
#ifndef QGSGRADUATEDSYMBOLRENDERERV2_H
#define QGSGRADUATEDSYMBOLRENDERERV2_H

#include "qgssymbolv2.h"
#include "qgsrendererv2.h"
#include "qgsexpression.h"
#include <QScopedPointer>
#include <QRegExp>

class CORE_EXPORT QgsRendererRangeV2
{
  public:
    QgsRendererRangeV2();
    QgsRendererRangeV2( double lowerValue, double upperValue, QgsSymbolV2* symbol, QString label, bool render = true );
    QgsRendererRangeV2( const QgsRendererRangeV2& range );

    // default dtor is ok
    QgsRendererRangeV2& operator=( QgsRendererRangeV2 range );

    bool operator<( const QgsRendererRangeV2 &other ) const;

    double lowerValue() const;
    double upperValue() const;

    QgsSymbolV2* symbol() const;
    QString label() const;

    void setSymbol( QgsSymbolV2* s );
    void setLabel( QString label );
    void setLowerValue( double lowerValue );
    void setUpperValue( double upperValue );

    // @note added in 2.5
    bool renderState() const;
    void setRenderState( bool render );

    // debugging
    QString dump() const;

    void toSld( QDomDocument& doc, QDomElement &element, QgsStringMap props ) const;

  protected:
    double mLowerValue, mUpperValue;
    QScopedPointer<QgsSymbolV2> mSymbol;
    QString mLabel;
    bool mRender;

    // for cpy+swap idiom
    void swap( QgsRendererRangeV2 & other );
};

typedef QList<QgsRendererRangeV2> QgsRangeList;


// @note added in 2.6
class CORE_EXPORT QgsRendererRangeV2LabelFormat
{
  public:
    QgsRendererRangeV2LabelFormat();
    QgsRendererRangeV2LabelFormat( QString format, int precision = 4, bool trimTrailingZeroes = false );

    bool operator==( const QgsRendererRangeV2LabelFormat & other ) const;
    bool operator!=( const QgsRendererRangeV2LabelFormat & other ) const;

    QString format() const { return mFormat; }
    void setFormat( QString format ) { mFormat = format; }

    int precision() const { return mPrecision; }
    void setPrecision( int precision );

    bool trimTrailingZeroes() const { return mTrimTrailingZeroes; }
    void setTrimTrailingZeroes( bool trimTrailingZeroes ) { mTrimTrailingZeroes = trimTrailingZeroes; }

    //! @note labelForLowerUpper in python bindings
    QString labelForRange( double lower, double upper ) const;
    QString labelForRange( const QgsRendererRangeV2 &range ) const;
    QString formatNumber( double value ) const;

    void setFromDomElement( QDomElement &element );
    void saveToDomElement( QDomElement &element );

    static int MaxPrecision;
    static int MinPrecision;

  protected:
    QString mFormat;
    int mPrecision;
    bool mTrimTrailingZeroes;
    // values used to manage number formatting - precision and trailing zeroes
    double mNumberScale;
    QString mNumberSuffix;
    QRegExp mReTrailingZeroes;
    QRegExp mReNegativeZero;
};

class QgsVectorLayer;
class QgsVectorColorRampV2;

Q_NOWARN_DEPRECATED_PUSH
class CORE_EXPORT QgsGraduatedSymbolRendererV2 : public QgsFeatureRendererV2
{
  public:

    QgsGraduatedSymbolRendererV2( QString attrName = QString(), QgsRangeList ranges = QgsRangeList() );
    QgsGraduatedSymbolRendererV2( const QgsGraduatedSymbolRendererV2 & other );

    virtual ~QgsGraduatedSymbolRendererV2();

    virtual QgsSymbolV2* symbolForFeature( QgsFeature& feature, QgsRenderContext &context ) override;

    virtual QgsSymbolV2* originalSymbolForFeature( QgsFeature& feature, QgsRenderContext &context ) override;

    virtual void startRender( QgsRenderContext& context, const QgsFields& fields ) override;

    virtual void stopRender( QgsRenderContext& context ) override;

    virtual QList<QString> usedAttributes() override;

    virtual QString dump() const override;

    virtual QgsFeatureRendererV2* clone() const override;

    virtual void toSld( QDomDocument& doc, QDomElement &element ) const override;

    //! returns bitwise OR-ed capabilities of the renderer
    virtual int capabilities() override { return SymbolLevels | RotationField | Filter; }

    virtual QgsSymbolV2List symbols( QgsRenderContext &context ) override;

    QString classAttribute() const { return mAttrName; }
    void setClassAttribute( QString attr ) { mAttrName = attr; }

    const QgsRangeList& ranges() const { return mRanges; }

    bool updateRangeSymbol( int rangeIndex, QgsSymbolV2* symbol );
    bool updateRangeLabel( int rangeIndex, QString label );
    bool updateRangeUpperValue( int rangeIndex, double value );
    bool updateRangeLowerValue( int rangeIndex, double value );
    //! @note added in 2.5
    bool updateRangeRenderState( int rangeIndex, bool render );

    void addClass( QgsSymbolV2* symbol );
    //! @note available in python bindings as addClassRange
    void addClass( QgsRendererRangeV2 range );
    //! @note available in python bindings as addClassLowerUpper
    void addClass( double lower, double upper );

    /** Add a breakpoint by splitting existing classes so that the specified
     * value becomes a break between two classes.
     * @param breakValue position to insert break
     * @param updateSymbols set to true to reapply ramp colors to the new
     * symbol ranges
     * @note added in QGIS 2.9
     */
    void addBreak( double breakValue, bool updateSymbols = true );

    void deleteClass( int idx );
    void deleteAllClasses();

    //! Moves the category at index position from to index position to.
    void moveClass( int from, int to );

    /** Tests whether classes assigned to the renderer have ranges which overlap.
     * @returns true if ranges overlap
     * @note added in QGIS 2.10
     */
    bool rangesOverlap() const;

    /** Tests whether classes assigned to the renderer have gaps between the ranges.
     * @returns true if ranges have gaps
     * @note added in QGIS 2.10
     */
    bool rangesHaveGaps() const;

    void sortByValue( Qt::SortOrder order = Qt::AscendingOrder );
    void sortByLabel( Qt::SortOrder order = Qt::AscendingOrder );

    enum Mode
    {
      EqualInterval,
      Quantile,
      Jenks,
      StdDev,
      Pretty,
      Custom
    };

    Mode mode() const { return mMode; }
    void setMode( Mode mode ) { mMode = mode; }
    //! Recalculate classes for a layer
    //! @param vlayer  The layer being rendered (from which data values are calculated)
    //! @param mode    The calculation mode
    //! @param nclasses The number of classes to calculate (approximate for some modes)
    //! @note Added in 2.6
    void updateClasses( QgsVectorLayer *vlayer, Mode mode, int nclasses );

    //! Evaluates the data expression and returns the list of values from the layer
    //! @param vlayer  The layer for which to evaluate the expression
    //! @note Added in 2.6
    //! @deprecated use QgsVectorLayer::getDoubleValues instead
    Q_DECL_DEPRECATED QList<double> getDataValues( QgsVectorLayer *vlayer );

    //! Return the label format used to generate default classification labels
    //! @note Added in 2.6
    const QgsRendererRangeV2LabelFormat &labelFormat() const { return mLabelFormat; }
    //! Set the label format used to generate default classification labels
    //! @param labelFormat The string appended to classification labels
    //! @param updateRanges If true then ranges ending with the old unit string are updated to the new.
    //! @note Added in 2.6
    void setLabelFormat( const QgsRendererRangeV2LabelFormat &labelFormat, bool updateRanges = false );

    //! Reset the label decimal places to a numberbased on the minimum class interval
    //! @param updateRanges if true then ranges currently using the default label will be updated
    //! @note Added in 2.6
    void calculateLabelPrecision( bool updateRanges = true );

    static QgsGraduatedSymbolRendererV2* createRenderer(
      QgsVectorLayer* vlayer,
      QString attrName,
      int classes,
      Mode mode,
      QgsSymbolV2* symbol,
      QgsVectorColorRampV2* ramp,
      bool inverted = false,
      QgsRendererRangeV2LabelFormat legendFormat = QgsRendererRangeV2LabelFormat()
    );

    //! create renderer from XML element
    static QgsFeatureRendererV2* create( QDomElement& element );

    //! store renderer info to XML element
    virtual QDomElement save( QDomDocument& doc ) override;

    //! return a list of symbology items for the legend
    virtual QgsLegendSymbologyList legendSymbologyItems( QSize iconSize ) override;

    //! return a list of item text / symbol
    //! @note not available in python bindings
    virtual QgsLegendSymbolList legendSymbolItems( double scaleDenominator = -1, QString rule = QString() ) override;

    //! @note added in 2.10
    QgsLegendSymbolListV2 legendSymbolItemsV2() const override;


    QgsSymbolV2* sourceSymbol();
    void setSourceSymbol( QgsSymbolV2* sym );

    QgsVectorColorRampV2* sourceColorRamp();

    /** Sets the source color ramp.
      * @param ramp color ramp. Ownership is transferred to the renderer
      */
    void setSourceColorRamp( QgsVectorColorRampV2* ramp );

    //! @note added in 2.1
    bool invertedColorRamp() { return mInvertedColorRamp; }
    void setInvertedColorRamp( bool inverted ) { mInvertedColorRamp = inverted; }

    /** Update the color ramp used. Also updates all symbols colors.
      * Doesn't alter current breaks.
      * @param ramp color ramp. Ownership is transferred to the renderer
      * @param inverted set to true to invert ramp colors
      */
    void updateColorRamp( QgsVectorColorRampV2* ramp = 0, bool inverted = false );

    /** Update all the symbols but leave breaks and colors. */
    void updateSymbols( QgsSymbolV2* sym );

    //! set varying symbol size for classes
    //! @note the classes must already be set so that symbols exist
    //! @note added in 2.10
    void setSymbolSizes( double minSize, double maxSize );

    //! return the min symbol size when graduated by size
    //! @note added in 2.10
    double minSymbolSize() const;

    //! return the max symbol size when graduated by size
    //! @note added in 2.10
    double maxSymbolSize() const;

    enum GraduatedMethod {GraduatedColor = 0, GraduatedSize = 1 };

    //! return the method used for graduation (either size or color)
    //! @note added in 2.10
    GraduatedMethod graduatedMethod() const { return mGraduatedMethod; }

    //! set the method used for graduation (either size or color)
    //! @note added in 2.10
    void setGraduatedMethod( GraduatedMethod method ) { mGraduatedMethod = method; }

    Q_DECL_DEPRECATED void setRotationField( QString fieldOrExpression ) override;
    Q_DECL_DEPRECATED QString rotationField() const override;

    void setSizeScaleField( QString fieldOrExpression );
    QString sizeScaleField() const;

    void setScaleMethod( QgsSymbolV2::ScaleMethod scaleMethod );
    QgsSymbolV2::ScaleMethod scaleMethod() const { return mScaleMethod; }

    //! items of symbology items in legend should be checkable
    //! @note added in 2.5
    virtual bool legendSymbolItemsCheckable() const override;

    //! item in symbology was checked
    //! @note added in 2.6
    virtual bool legendSymbolItemChecked( QString key ) override;

    //! item in symbology was checked
    //! @note added in 2.6
    virtual void checkLegendSymbolItem( QString key, bool state = true ) override;

    //! If supported by the renderer, return classification attribute for the use in legend
    //! @note added in 2.6
    virtual QString legendClassificationAttribute() const override { return classAttribute(); }

    //! creates a QgsGraduatedSymbolRendererV2 from an existing renderer.
    //! @note added in 2.6
    //! @returns a new renderer if the conversion was possible, otherwise 0.
    static QgsGraduatedSymbolRendererV2* convertFromRenderer( const QgsFeatureRendererV2 *renderer );

  protected:
    QString mAttrName;
    QgsRangeList mRanges;
    Mode mMode;
    QScopedPointer<QgsSymbolV2> mSourceSymbol;
    QScopedPointer<QgsVectorColorRampV2> mSourceColorRamp;
    bool mInvertedColorRamp;
    QgsRendererRangeV2LabelFormat mLabelFormat;

    QScopedPointer<QgsExpression> mRotation;
    QScopedPointer<QgsExpression> mSizeScale;
    QgsSymbolV2::ScaleMethod mScaleMethod;
    QScopedPointer<QgsExpression> mExpression;
    GraduatedMethod mGraduatedMethod;
    //! attribute index (derived from attribute name in startRender)
    int mAttrNum;
    bool mCounting;

    //! temporary symbols, used for data-defined rotation and scaling
    QHash<QgsSymbolV2*, QgsSymbolV2*> mTempSymbols;

    QgsSymbolV2* symbolForValue( double value );

    static const char * graduatedMethodStr( GraduatedMethod method );

};
Q_NOWARN_DEPRECATED_POP

#endif // QGSGRADUATEDSYMBOLRENDERERV2_H
