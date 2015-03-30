class QgsRendererRangeV2
{
%TypeHeaderCode
#include <qgsgraduatedsymbolrendererv2.h>
%End

  public:
    QgsRendererRangeV2();
    QgsRendererRangeV2( double lowerValue, double upperValue, QgsSymbolV2* symbol /Transfer/, QString label, bool render = true );
    QgsRendererRangeV2( const QgsRendererRangeV2& range );

    // default dtor is ok
    ~QgsRendererRangeV2();

    double lowerValue() const;
    double upperValue() const;

    QgsSymbolV2* symbol() const;
    QString label() const;

    void setSymbol( QgsSymbolV2* s /Transfer/ );
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
    // for cpy+swap idiom
    void swap( QgsRendererRangeV2 & other );
};

typedef QList<QgsRendererRangeV2> QgsRangeList;


// @note added in 2.6
class QgsRendererRangeV2LabelFormat
{
%TypeHeaderCode
#include <qgsgraduatedsymbolrendererv2.h>
%End
  public:
    QgsRendererRangeV2LabelFormat();
    QgsRendererRangeV2LabelFormat( QString format, int precision = 4, bool trimTrailingZeroes = false );

    bool operator==( const QgsRendererRangeV2LabelFormat & other ) const;
    bool operator!=( const QgsRendererRangeV2LabelFormat & other ) const;

    QString format() const;
    void setFormat( QString format );

    int precision();
    void setPrecision( int precision );

    bool trimTrailingZeroes() const;
    void setTrimTrailingZeroes( bool trimTrailingZeroes );

    //! @note labelForLowerUpper in python bindings
    QString labelForRange( double lower, double upper ) const /PyName=labelForLowerUpper/;
    QString labelForRange( const QgsRendererRangeV2 &range ) const;
    QString formatNumber( double value ) const;

    void setFromDomElement( QDomElement &element );
    void saveToDomElement( QDomElement &element );

    static int MaxPrecision;
    static int MinPrecision;

};


class QgsGraduatedSymbolRendererV2 : QgsFeatureRendererV2
{
%TypeHeaderCode
#include <qgsgraduatedsymbolrendererv2.h>
%End

  public:
    QgsGraduatedSymbolRendererV2( QString attrName = QString(), QgsRangeList ranges = QgsRangeList() );

    virtual ~QgsGraduatedSymbolRendererV2();

    virtual QgsSymbolV2* symbolForFeature( QgsFeature& feature );

    virtual QgsSymbolV2* originalSymbolForFeature( QgsFeature& feature );

    virtual void startRender( QgsRenderContext& context, const QgsFields& fields );

    virtual void stopRender( QgsRenderContext& context );

    virtual QList<QString> usedAttributes();

    virtual QString dump() const;

    virtual QgsFeatureRendererV2* clone() const /Factory/;

    virtual void toSld( QDomDocument& doc, QDomElement &element ) const;

    //! returns bitwise OR-ed capabilities of the renderer
    virtual int capabilities();

    virtual QgsSymbolV2List symbols();

    QString classAttribute() const;
    void setClassAttribute( QString attr );

    const QgsRangeList& ranges();

    bool updateRangeSymbol( int rangeIndex, QgsSymbolV2* symbol /Transfer/ );
    bool updateRangeLabel( int rangeIndex, QString label );
    bool updateRangeUpperValue( int rangeIndex, double value );
    bool updateRangeLowerValue( int rangeIndex, double value );
    //! @note added in 2.5
    bool updateRangeRenderState( int rangeIndex, bool render );

    void addClass( QgsSymbolV2* symbol );
    //! @note available in python bindings as addClassRange
    void addClass( QgsRendererRangeV2 range ) /PyName=addClassRange/;
    //! @note available in python bindings as addClassLowerUpper
    void addClass( double lower, double upper ) /PyName=addClassLowerUpper/;
    void deleteClass( int idx );
    void deleteAllClasses();

    //! Moves the category at index position from to index position to.
    void moveClass( int from, int to );

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
    //! Evaluates the data expression and returns the list of values from the layer
    //! @param vlayer  The layer for which to evaluate the expression
    //! @note Added in 2.6
    QList<double> getDataValues( QgsVectorLayer *vlayer );

    //! Return the label format used to generate default classification labels
    //! @note Added in 2.6
    const QgsRendererRangeV2LabelFormat &labelFormat() const;
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
      QgsSymbolV2* symbol /Transfer/,
      QgsVectorColorRampV2* ramp /Transfer/,
      bool inverted = false,
      QgsRendererRangeV2LabelFormat labelFormat=QgsRendererRangeV2LabelFormat()
    );

    //! create renderer from XML element
    static QgsFeatureRendererV2* create( QDomElement& element ) /Factory/;

    //! store renderer info to XML element
    virtual QDomElement save( QDomDocument& doc );

    //! return a list of symbology items for the legend
    virtual QgsLegendSymbologyList legendSymbologyItems( QSize iconSize );

    //! return a list of item text / symbol
    //! @note not available in python bindings
    // virtual QgsLegendSymbolList legendSymbolItems( double scaleDenominator = -1, QString rule = QString() );

    QgsSymbolV2* sourceSymbol();
    void setSourceSymbol( QgsSymbolV2* sym /Transfer/ );

    QgsVectorColorRampV2* sourceColorRamp();
    void setSourceColorRamp( QgsVectorColorRampV2* ramp /Transfer/ );
    //! @note added in 2.1
    bool invertedColorRamp();
    void setInvertedColorRamp( bool inverted );

    /** Update the color ramp used. Also updates all symbols colors.
      * Doesn't alter current breaks.
      */
    void updateColorRamp( QgsVectorColorRampV2* ramp /Transfer/ = 0, bool inverted = false );

    /** Update all the symbols but leave breaks and colors. */
    void updateSymbols( QgsSymbolV2* sym /Transfer/ );

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

    void setRotationField( QString fieldOrExpression );
    QString rotationField() const;

    void setSizeScaleField( QString fieldOrExpression );
    QString sizeScaleField() const;

    void setScaleMethod( QgsSymbolV2::ScaleMethod scaleMethod );
    QgsSymbolV2::ScaleMethod scaleMethod() const;

    //! items of symbology items in legend should be checkable
    //! @note added in 2.5
    virtual bool legendSymbolItemsCheckable() const;

    //! item in symbology was checked
    //! @note added in 2.6
    virtual bool legendSymbolItemChecked( QString key );

    //! item in symbology was checked
    //! @note added in 2.6
    virtual void checkLegendSymbolItem( QString key, bool state = true );

    //! If supported by the renderer, return classification attribute for the use in legend
    //! @note added in 2.6
    virtual QString legendClassificationAttribute();

    //! creates a QgsGraduatedSymbolRendererV2 from an existing renderer.
    //! @note added in 2.6
    //! @returns a new renderer if the conversion was possible, otherwise 0.
    static QgsGraduatedSymbolRendererV2* convertFromRenderer( const QgsFeatureRendererV2 *renderer ) /Factory/;

  protected:
    QgsSymbolV2* symbolForValue( double value );

  private:
    QgsGraduatedSymbolRendererV2( const QgsGraduatedSymbolRendererV2 & );
    QgsGraduatedSymbolRendererV2 & operator=( const QgsGraduatedSymbolRendererV2 & );
};
