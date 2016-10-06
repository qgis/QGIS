class QgsRendererRange
{
%TypeHeaderCode
#include <qgsgraduatedsymbolrenderer.h>
%End

  public:
    QgsRendererRange();
    QgsRendererRange( double lowerValue, double upperValue, QgsSymbol* symbol /Transfer/, const QString& label, bool render = true );
    QgsRendererRange( const QgsRendererRange& range );

    ~QgsRendererRange();

    double lowerValue() const;
    double upperValue() const;

    QgsSymbol* symbol() const;
    QString label() const;

    void setSymbol( QgsSymbol* s /Transfer/ );
    void setLabel( const QString& label );
    void setLowerValue( double lowerValue );
    void setUpperValue( double upperValue );

    // @note added in 2.5
    bool renderState() const;
    void setRenderState( bool render );

    // debugging
    QString dump() const;

    /** Creates a DOM element representing the range in SLD format.
     * @param doc DOM document
     * @param element destination DOM element
     * @param props graduated renderer properties
     * @param firstRange set to true if the range is the first range, where the lower value uses a <= test
     * rather than a < test.
     */
    void toSld( QDomDocument& doc, QDomElement &element, QgsStringMap props, bool firstRange = false ) const;

  protected:
    // for cpy+swap idiom
    void swap( QgsRendererRange & other );
};

typedef QList<QgsRendererRange> QgsRangeList;


// @note added in 2.6
class QgsRendererRangeLabelFormat
{
%TypeHeaderCode
#include <qgsgraduatedsymbolrenderer.h>
%End
  public:
    QgsRendererRangeLabelFormat();
    QgsRendererRangeLabelFormat( const QString& format, int precision = 4, bool trimTrailingZeroes = false );

    bool operator==( const QgsRendererRangeLabelFormat & other ) const;
    bool operator!=( const QgsRendererRangeLabelFormat & other ) const;

    QString format() const;
    void setFormat( const QString& format );

    int precision();
    void setPrecision( int precision );

    bool trimTrailingZeroes() const;
    void setTrimTrailingZeroes( bool trimTrailingZeroes );

    //! @note labelForLowerUpper in python bindings
    QString labelForRange( double lower, double upper ) const /PyName=labelForLowerUpper/;
    QString labelForRange( const QgsRendererRange &range ) const;
    QString formatNumber( double value ) const;

    void setFromDomElement( QDomElement &element );
    void saveToDomElement( QDomElement &element );

    static int MaxPrecision;
    static int MinPrecision;

};


class QgsGraduatedSymbolRenderer : QgsFeatureRenderer
{
%TypeHeaderCode
#include <qgsgraduatedsymbolrenderer.h>
%End
  public:

    QgsGraduatedSymbolRenderer( const QString& attrName = QString(), const QgsRangeList& ranges = QgsRangeList() );

    virtual ~QgsGraduatedSymbolRenderer();

    //! @note labelForLowerUpper in python bindings
    virtual QgsSymbol* symbolForFeature( QgsFeature& feature, QgsRenderContext& context );
    //! @note originalSymbolForFeature2 in python bindings
    virtual QgsSymbol* originalSymbolForFeature( QgsFeature& feature, QgsRenderContext& context );

    virtual void startRender( QgsRenderContext& context, const QgsFields& fields );

    virtual void stopRender( QgsRenderContext& context );

    virtual QSet<QString> usedAttributes() const;

    virtual QString dump() const;

    virtual QgsGraduatedSymbolRenderer* clone() const /Factory/;

    virtual void toSld( QDomDocument& doc, QDomElement &element ) const;

    //! returns bitwise OR-ed capabilities of the renderer
    virtual QgsFeatureRenderer::Capabilities capabilities();

    //! @note symbol2 in python bindings
    virtual QgsSymbolList symbols( QgsRenderContext& context );

    QString classAttribute() const;
    void setClassAttribute( const QString& attr );

    const QgsRangeList& ranges();

    bool updateRangeSymbol( int rangeIndex, QgsSymbol* symbol /Transfer/ );
    bool updateRangeLabel( int rangeIndex, const QString& label );
    bool updateRangeUpperValue( int rangeIndex, double value );
    bool updateRangeLowerValue( int rangeIndex, double value );
    //! @note added in 2.5
    bool updateRangeRenderState( int rangeIndex, bool render );

    void addClass( QgsSymbol* symbol );
    //! @note available in python bindings as addClassRange
    void addClass( const QgsRendererRange& range ) /PyName=addClassRange/;
    //! @note available in python bindings as addClassLowerUpper
    void addClass( double lower, double upper ) /PyName=addClassLowerUpper/;

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

    Mode mode() const;
    void setMode( Mode mode );
    //! Recalculate classes for a layer
    //! @param vlayer  The layer being rendered (from which data values are calculated)
    //! @param mode    The calculation mode
    //! @param nclasses The number of classes to calculate (approximate for some modes)
    //! @note Added in 2.6
    void updateClasses( QgsVectorLayer *vlayer, Mode mode, int nclasses );

    //! Return the label format used to generate default classification labels
    //! @note Added in 2.6
    const QgsRendererRangeLabelFormat &labelFormat() const;
    //! Set the label format used to generate default classification labels
    //! @param labelFormat The string appended to classification labels
    //! @param updateRanges If true then ranges ending with the old unit string are updated to the new.
    //! @note Added in 2.6
    void setLabelFormat( const QgsRendererRangeLabelFormat &labelFormat, bool updateRanges = false );

    //! Reset the label decimal places to a numberbased on the minimum class interval
    //! @param updateRanges if true then ranges currently using the default label will be updated
    //! @note Added in 2.6
    void calculateLabelPrecision( bool updateRanges = true );

    static QgsGraduatedSymbolRenderer* createRenderer(
      QgsVectorLayer* vlayer,
      const QString& attrName,
      int classes,
      Mode mode,
      QgsSymbol* symbol /Transfer/,
      QgsColorRamp* ramp /Transfer/,
      bool inverted = false,
      const QgsRendererRangeLabelFormat& legendFormat = QgsRendererRangeLabelFormat()
    );

    //! create renderer from XML element
    static QgsFeatureRenderer* create( QDomElement& element ) /Factory/;

    //! store renderer info to XML element
    virtual QDomElement save( QDomDocument& doc );

    //! return a list of symbology items for the legend
    virtual QgsLegendSymbologyList legendSymbologyItems( QSize iconSize );

    //! return a list of item text / symbol
    //! @note not available in python bindings
    // virtual QgsLegendSymbolList legendSymbolItems( double scaleDenominator = -1, const QString& rule = QString() );

    //! @note added in 2.10
    QgsLegendSymbolListV2 legendSymbolItemsV2() const;

    virtual QSet< QString > legendKeysForFeature( QgsFeature& feature, QgsRenderContext& context );

    /** Returns the renderer's source symbol, which is the base symbol used for the each classes' symbol before applying
     * the classes' color.
     * @see setSourceSymbol()
     * @see sourceColorRamp()
     */
    QgsSymbol* sourceSymbol();

    /** Sets the source symbol for the renderer, which is the base symbol used for the each classes' symbol before applying
     * the classes' color.
     * @param sym source symbol, ownership is transferred to the renderer
     * @see sourceSymbol()
     * @see setSourceColorRamp()
     */
    void setSourceSymbol( QgsSymbol* sym /Transfer/ );

    /** Returns the source color ramp, from which each classes' color is derived.
     * @see setSourceColorRamp()
     * @see sourceSymbol()
     */
    QgsColorRamp* sourceColorRamp();

    /** Sets the source color ramp.
     * @param ramp color ramp. Ownership is transferred to the renderer
     */
    void setSourceColorRamp( QgsColorRamp* ramp /Transfer/ );

    //! @note added in 2.1
    bool invertedColorRamp();
    void setInvertedColorRamp( bool inverted );

    /** Update the color ramp used. Also updates all symbols colors.
     * Doesn't alter current breaks.
     * @param ramp color ramp. Ownership is transferred to the renderer
     * @param inverted set to true to invert ramp colors
     */
    void updateColorRamp( QgsColorRamp* ramp /Transfer/ = 0, bool inverted = false );

    /** Update all the symbols but leave breaks and colors. This method also sets the source
     * symbol for the renderer.
     * @param sym source symbol to use for classes. Ownership is not transferred.
     * @see setSourceSymbol()
     */
    void updateSymbols( QgsSymbol* sym );

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
    GraduatedMethod graduatedMethod() const;

    //! set the method used for graduation (either size or color)
    //! @note added in 2.10
    void setGraduatedMethod( GraduatedMethod method );

    //! items of symbology items in legend should be checkable
    //! @note added in 2.5
    virtual bool legendSymbolItemsCheckable() const;

    //! item in symbology was checked
    //! @note added in 2.6
    virtual bool legendSymbolItemChecked( const QString& key );

    //! item in symbology was checked
    //! @note added in 2.6
    virtual void checkLegendSymbolItem( const QString& key, bool state = true );

    virtual void setLegendSymbolItem( const QString& key, QgsSymbol* symbol /Transfer/ );

    //! If supported by the renderer, return classification attribute for the use in legend
    //! @note added in 2.6
    virtual QString legendClassificationAttribute();

    //! creates a QgsGraduatedSymbolRenderer from an existing renderer.
    //! @note added in 2.6
    //! @returns a new renderer if the conversion was possible, otherwise 0.
    static QgsGraduatedSymbolRenderer* convertFromRenderer( const QgsFeatureRenderer *renderer ) /Factory/;

  protected:
    QgsSymbol* symbolForValue( double value );

    /** Returns the matching legend key for a value.
     */
    QString legendKeyForValue( double value ) const;


  private:
    QgsGraduatedSymbolRenderer( const QgsGraduatedSymbolRenderer & );
    QgsGraduatedSymbolRenderer & operator=( const QgsGraduatedSymbolRenderer & );
};
