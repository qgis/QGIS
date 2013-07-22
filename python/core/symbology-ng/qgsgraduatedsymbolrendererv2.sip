class QgsRendererRangeV2
{
%TypeHeaderCode
#include <qgsgraduatedsymbolrendererv2.h>
%End

  public:
    QgsRendererRangeV2( double lowerValue, double upperValue, QgsSymbolV2* symbol /Transfer/, QString label );
    QgsRendererRangeV2( const QgsRendererRangeV2& range );

    ~QgsRendererRangeV2();

    double lowerValue() const;
    double upperValue() const;

    QgsSymbolV2* symbol() const;
    QString label() const;

    void setSymbol( QgsSymbolV2* s /Transfer/ );
    void setLabel( QString label );
    void setLowerValue( double lowerValue );
    void setUpperValue( double upperValue );

    // debugging
    QString dump() const;

    void toSld( QDomDocument& doc, QDomElement &element, QgsStringMap props ) const;
};

typedef QList<QgsRendererRangeV2> QgsRangeList;

class QgsGraduatedSymbolRendererV2 : QgsFeatureRendererV2
{
%TypeHeaderCode
#include <qgsgraduatedsymbolrendererv2.h>
%End

  public:
    QgsGraduatedSymbolRendererV2( QString attrName = QString(), QgsRangeList ranges = QgsRangeList() );

    virtual ~QgsGraduatedSymbolRendererV2();

    virtual QgsSymbolV2* symbolForFeature( QgsFeature& feature );

    virtual void startRender( QgsRenderContext& context, const QgsVectorLayer *vlayer );

    virtual void stopRender( QgsRenderContext& context );

    virtual QList<QString> usedAttributes();

    virtual QString dump() const;

    virtual QgsFeatureRendererV2* clone() /Factory/;

    virtual void toSld( QDomDocument& doc, QDomElement &element ) const;

    //! returns bitwise OR-ed capabilities of the renderer
    //! \note added in 2.0
    virtual int capabilities();

    virtual QgsSymbolV2List symbols();

    QString classAttribute() const;
    void setClassAttribute( QString attr );

    const QgsRangeList& ranges();

    bool updateRangeSymbol( int rangeIndex, QgsSymbolV2* symbol /Transfer/ );
    bool updateRangeLabel( int rangeIndex, QString label );
    bool updateRangeUpperValue( int rangeIndex, double value );
    bool updateRangeLowerValue( int rangeIndex, double value );

    void addClass( QgsSymbolV2* symbol );
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

    static QgsGraduatedSymbolRendererV2* createRenderer(
      QgsVectorLayer* vlayer,
      QString attrName,
      int classes,
      Mode mode,
      QgsSymbolV2* symbol /Transfer/,
      QgsVectorColorRampV2* ramp /Transfer/ ) /Factory/;

    //! create renderer from XML element
    static QgsFeatureRendererV2* create( QDomElement& element ) /Factory/;

    //! store renderer info to XML element
    virtual QDomElement save( QDomDocument& doc );

    //! return a list of symbology items for the legend
    virtual QgsLegendSymbologyList legendSymbologyItems( QSize iconSize );

    //! return a list of item text / symbol
    //! @note: this method was added in version 1.5
    //! @note not available in python bindings
    // virtual QgsLegendSymbolList legendSymbolItems();

    QgsSymbolV2* sourceSymbol();
    void setSourceSymbol( QgsSymbolV2* sym /Transfer/ );

    QgsVectorColorRampV2* sourceColorRamp();
    void setSourceColorRamp( QgsVectorColorRampV2* ramp /Transfer/ );

    /** Update the color ramp used. Also updates all symbols colors.
      * Doesn't alter current breaks.
      */
    void updateColorRamp( QgsVectorColorRampV2* ramp /Transfer/ );

    /** Update the all symbols but leave breaks and colors. */
    void updateSymbols( QgsSymbolV2* sym /Transfer/ );

    //! @note added in 1.6
    void setRotationField( QString fieldName );
    //! @note added in 1.6
    QString rotationField() const;

    //! @note added in 1.6
    void setSizeScaleField( QString fieldName );
    //! @note added in 1.6
    QString sizeScaleField() const;

    //! @note added in 2.0
    void setScaleMethod( QgsSymbolV2::ScaleMethod scaleMethod );
    //! @note added in 2.0
    QgsSymbolV2::ScaleMethod scaleMethod() const;


  protected:
    QgsSymbolV2* symbolForValue( double value );
};
