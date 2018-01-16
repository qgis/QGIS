/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgsgraduatedsymbolrenderer.h                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsRendererRange
{

%TypeHeaderCode
#include "qgsgraduatedsymbolrenderer.h"
%End
  public:

    QgsRendererRange();
%Docstring
Constructor for QgsRendererRange.
%End
    QgsRendererRange( double lowerValue, double upperValue, QgsSymbol *symbol /Transfer/, const QString &label, bool render = true );
    QgsRendererRange( const QgsRendererRange &range );


    bool operator<( const QgsRendererRange &other ) const;

    double lowerValue() const;
    double upperValue() const;

    QgsSymbol *symbol() const;
    QString label() const;

    void setSymbol( QgsSymbol *s /Transfer/ );
    void setLabel( const QString &label );
    void setLowerValue( double lowerValue );
    void setUpperValue( double upperValue );

    bool renderState() const;
    void setRenderState( bool render );

    QString dump() const;

    void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props, bool firstRange = false ) const;
%Docstring
Creates a DOM element representing the range in SLD format.

:param doc: DOM document
:param element: destination DOM element
:param props: graduated renderer properties
:param firstRange: set to true if the range is the first range, where the lower value uses a <= test
rather than a < test.
%End

  protected:

    void swap( QgsRendererRange &other );
};

typedef QList<QgsRendererRange> QgsRangeList;


class QgsRendererRangeLabelFormat
{
%Docstring

.. versionadded:: 2.6
%End

%TypeHeaderCode
#include "qgsgraduatedsymbolrenderer.h"
%End
  public:
    QgsRendererRangeLabelFormat();
    QgsRendererRangeLabelFormat( const QString &format, int precision = 4, bool trimTrailingZeroes = false );

    bool operator==( const QgsRendererRangeLabelFormat &other ) const;
    bool operator!=( const QgsRendererRangeLabelFormat &other ) const;

    QString format() const;
    void setFormat( const QString &format );

    int precision() const;
    void setPrecision( int precision );

    bool trimTrailingZeroes() const;
    void setTrimTrailingZeroes( bool trimTrailingZeroes );

    QString labelForRange( double lower, double upper ) const /PyName=labelForLowerUpper/;
%Docstring

.. note::

   labelForLowerUpper in Python bindings
%End
    QString labelForRange( const QgsRendererRange &range ) const;
    QString formatNumber( double value ) const;

    void setFromDomElement( QDomElement &element );
    void saveToDomElement( QDomElement &element );

    static const int MAX_PRECISION;
    static const int MIN_PRECISION;

  protected:
};


class QgsGraduatedSymbolRenderer : QgsFeatureRenderer
{

%TypeHeaderCode
#include "qgsgraduatedsymbolrenderer.h"
%End
  public:

    QgsGraduatedSymbolRenderer( const QString &attrName = QString(), const QgsRangeList &ranges = QgsRangeList() );

    ~QgsGraduatedSymbolRenderer();

    virtual QgsSymbol *symbolForFeature( QgsFeature &feature, QgsRenderContext &context );

    virtual QgsSymbol *originalSymbolForFeature( QgsFeature &feature, QgsRenderContext &context );

    virtual void startRender( QgsRenderContext &context, const QgsFields &fields );

    virtual void stopRender( QgsRenderContext &context );

    virtual QSet<QString> usedAttributes( const QgsRenderContext &context ) const;

    virtual QString dump() const;

    virtual QgsGraduatedSymbolRenderer *clone() const /Factory/;

    virtual void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props = QgsStringMap() ) const;

    virtual QgsFeatureRenderer::Capabilities capabilities();
    virtual QgsSymbolList symbols( QgsRenderContext &context );


    QString classAttribute() const;
    void setClassAttribute( const QString &attr );

    const QgsRangeList &ranges() const;

    bool updateRangeSymbol( int rangeIndex, QgsSymbol *symbol /Transfer/ );
    bool updateRangeLabel( int rangeIndex, const QString &label );
    bool updateRangeUpperValue( int rangeIndex, double value );
    bool updateRangeLowerValue( int rangeIndex, double value );
    bool updateRangeRenderState( int rangeIndex, bool render );
%Docstring

.. versionadded:: 2.5
%End

    void addClass( QgsSymbol *symbol );
    void addClass( const QgsRendererRange &range ) /PyName=addClassRange/;
%Docstring

.. note::

   available in Python bindings as addClassRange
%End
    void addClass( double lower, double upper ) /PyName=addClassLowerUpper/;
%Docstring

.. note::

   available in Python bindings as addClassLowerUpper
%End

    void addBreak( double breakValue, bool updateSymbols = true );
%Docstring
Add a breakpoint by splitting existing classes so that the specified
value becomes a break between two classes.

:param breakValue: position to insert break
:param updateSymbols: set to true to reapply ramp colors to the new
symbol ranges

.. versionadded:: 2.9
%End

    void deleteClass( int idx );
    void deleteAllClasses();

    void moveClass( int from, int to );
%Docstring
Moves the category at index position from to index position to.
%End

    bool rangesOverlap() const;
%Docstring
Tests whether classes assigned to the renderer have ranges which overlap.

:return: true if ranges overlap

.. versionadded:: 2.10
%End

    bool rangesHaveGaps() const;
%Docstring
Tests whether classes assigned to the renderer have gaps between the ranges.

:return: true if ranges have gaps

.. versionadded:: 2.10
%End

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

    Mode mode() const;
    void setMode( Mode mode );

    void updateClasses( QgsVectorLayer *vlayer, Mode mode, int nclasses );
%Docstring
Recalculate classes for a layer

:param vlayer:  The layer being rendered (from which data values are calculated)
:param mode:    The calculation mode
:param nclasses: The number of classes to calculate (approximate for some modes)

.. versionadded:: 2.6
%End

    const QgsRendererRangeLabelFormat &labelFormat() const;
%Docstring
Return the label format used to generate default classification labels

.. versionadded:: 2.6
%End

    void setLabelFormat( const QgsRendererRangeLabelFormat &labelFormat, bool updateRanges = false );
%Docstring
Set the label format used to generate default classification labels

:param labelFormat: The string appended to classification labels
:param updateRanges: If true then ranges ending with the old unit string are updated to the new.

.. versionadded:: 2.6
%End

    void calculateLabelPrecision( bool updateRanges = true );
%Docstring
Reset the label decimal places to a numberbased on the minimum class interval

:param updateRanges: if true then ranges currently using the default label will be updated

.. versionadded:: 2.6
%End

    static QgsGraduatedSymbolRenderer *createRenderer( QgsVectorLayer *vlayer,
        const QString &attrName,
        int classes,
        Mode mode,
        QgsSymbol *symbol /Transfer/,
        QgsColorRamp *ramp /Transfer/,
        const QgsRendererRangeLabelFormat &legendFormat = QgsRendererRangeLabelFormat() );
%Docstring
Creates a new graduated renderer.

:param vlayer: vector layer
:param attrName: attribute to classify
:param classes: number of classes
:param mode: classification mode
:param symbol: base symbol
:param ramp: color ramp for classes
:param legendFormat:

:return: new QgsGraduatedSymbolRenderer object
%End

    static QgsFeatureRenderer *create( QDomElement &element, const QgsReadWriteContext &context ) /Factory/;
%Docstring
create renderer from XML element
%End

    virtual QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context );

    virtual QgsLegendSymbolList legendSymbolItems() const;

    virtual QSet< QString > legendKeysForFeature( QgsFeature &feature, QgsRenderContext &context );


    QgsSymbol *sourceSymbol();
%Docstring
Returns the renderer's source symbol, which is the base symbol used for the each classes' symbol before applying
the classes' color.

.. seealso:: :py:func:`setSourceSymbol`

.. seealso:: :py:func:`sourceColorRamp`
%End

    void setSourceSymbol( QgsSymbol *sym /Transfer/ );
%Docstring
Sets the source symbol for the renderer, which is the base symbol used for the each classes' symbol before applying
the classes' color.

:param sym: source symbol, ownership is transferred to the renderer

.. seealso:: :py:func:`sourceSymbol`

.. seealso:: :py:func:`setSourceColorRamp`
%End

    QgsColorRamp *sourceColorRamp();
%Docstring
Returns the source color ramp, from which each classes' color is derived.

.. seealso:: :py:func:`setSourceColorRamp`

.. seealso:: :py:func:`sourceSymbol`
%End

    void setSourceColorRamp( QgsColorRamp *ramp /Transfer/ );
%Docstring
Sets the source color ramp.

:param ramp: color ramp. Ownership is transferred to the renderer
%End

    void updateColorRamp( QgsColorRamp *ramp /Transfer/ = 0 );
%Docstring
Update the color ramp used. Also updates all symbols colors.
Doesn't alter current breaks.

:param ramp: color ramp. Ownership is transferred to the renderer
%End

    void updateSymbols( QgsSymbol *sym );
%Docstring
Update all the symbols but leave breaks and colors. This method also sets the source
symbol for the renderer.

:param sym: source symbol to use for classes. Ownership is not transferred.

.. seealso:: :py:func:`setSourceSymbol`
%End

    void setSymbolSizes( double minSize, double maxSize );
%Docstring
set varying symbol size for classes

.. note::

   the classes must already be set so that symbols exist

.. versionadded:: 2.10
%End

    double minSymbolSize() const;
%Docstring
return the min symbol size when graduated by size

.. versionadded:: 2.10
%End

    double maxSymbolSize() const;
%Docstring
return the max symbol size when graduated by size

.. versionadded:: 2.10
%End

    enum GraduatedMethod
    {
      GraduatedColor,
      GraduatedSize
    };

    GraduatedMethod graduatedMethod() const;
%Docstring
return the method used for graduation (either size or color)

.. versionadded:: 2.10
%End

    void setGraduatedMethod( GraduatedMethod method );
%Docstring
set the method used for graduation (either size or color)

.. versionadded:: 2.10
%End

    virtual bool legendSymbolItemsCheckable() const;

    virtual bool legendSymbolItemChecked( const QString &key );

    virtual void checkLegendSymbolItem( const QString &key, bool state = true );

    virtual void setLegendSymbolItem( const QString &key, QgsSymbol *symbol /Transfer/ );

    virtual QString legendClassificationAttribute() const;

    static QgsGraduatedSymbolRenderer *convertFromRenderer( const QgsFeatureRenderer *renderer ) /Factory/;
%Docstring
creates a QgsGraduatedSymbolRenderer from an existing renderer.

:return: a new renderer if the conversion was possible, otherwise 0.

.. versionadded:: 2.6
%End

    void setDataDefinedSizeLegend( QgsDataDefinedSizeLegend *settings /Transfer/ );
%Docstring
Configures appearance of legend when renderer is configured to use data-defined size for marker symbols.
This allows configuring for which values (symbol sizes) should be shown in the legend, whether to display
different symbol sizes collapsed in one legend node or separated across multiple legend nodes etc.

When renderer does not use data-defined size or does not use marker symbols, these settings will be ignored.
Takes ownership of the passed settings objects. Null pointer is a valid input that disables data-defined
size legend.

.. versionadded:: 3.0
%End

    QgsDataDefinedSizeLegend *dataDefinedSizeLegend() const;
%Docstring
Returns configuration of appearance of legend when using data-defined size for marker symbols.
Will return null if the functionality is disabled.

.. versionadded:: 3.0
%End

  protected:



    QgsSymbol *symbolForValue( double value );
%Docstring
attribute index (derived from attribute name in startRender)
%End

    QString legendKeyForValue( double value ) const;
%Docstring
Returns the matching legend key for a value.
%End


  private:
    QgsGraduatedSymbolRenderer( const QgsGraduatedSymbolRenderer & );
    QgsGraduatedSymbolRenderer &operator=( const QgsGraduatedSymbolRenderer & );
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgsgraduatedsymbolrenderer.h                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
