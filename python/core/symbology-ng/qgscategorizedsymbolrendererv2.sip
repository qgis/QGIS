class QgsRendererCategoryV2
{
%TypeHeaderCode
#include <qgscategorizedsymbolrendererv2.h>
%End
  public:

    //! takes ownership of symbol
    QgsRendererCategoryV2( QVariant value, QgsSymbolV2* symbol /Transfer/, QString label, bool render = true );

    //! copy constructor
    QgsRendererCategoryV2( const QgsRendererCategoryV2& cat );

    ~QgsRendererCategoryV2();

    QVariant value() const;
    QgsSymbolV2* symbol() const;
    QString label() const;

    void setValue( const QVariant &value );
    void setSymbol( QgsSymbolV2* s /Transfer/ );
    void setLabel( const QString &label );

    // @note added in 2.5
    bool renderState() const;
    void setRenderState( bool render );

    // debugging
    QString dump() const;

    void toSld( QDomDocument& doc, QDomElement &element, QgsStringMap props ) const;
};

typedef QList<QgsRendererCategoryV2> QgsCategoryList;

class QgsCategorizedSymbolRendererV2 : QgsFeatureRendererV2
{
%TypeHeaderCode
#include <qgscategorizedsymbolrendererv2.h>
%End

  public:

    QgsCategorizedSymbolRendererV2( QString attrName = QString(), QgsCategoryList categories = QgsCategoryList() );

    virtual ~QgsCategorizedSymbolRendererV2();

    virtual QgsSymbolV2* symbolForFeature( QgsFeature& feature, QgsRenderContext& context );

    virtual QgsSymbolV2* originalSymbolForFeature( QgsFeature& feature, QgsRenderContext& context );

    virtual void startRender( QgsRenderContext& context, const QgsFields& fields );

    virtual void stopRender( QgsRenderContext& context );

    virtual QList<QString> usedAttributes();

    virtual QString dump() const;

    virtual QgsFeatureRendererV2* clone() const /Factory/;

    virtual void toSld( QDomDocument& doc, QDomElement &element ) const;

    //! returns bitwise OR-ed capabilities of the renderer
    virtual int capabilities();

    virtual QgsSymbolV2List symbols( QgsRenderContext& context );
    void updateSymbols( QgsSymbolV2 * sym );

    const QgsCategoryList& categories() const;

    //! return index of category with specified value (-1 if not found)
    int categoryIndexForValue( QVariant val );

    //! return index of category with specified label (-1 if not found or not unique)
    //! @note added in 2.5
    int categoryIndexForLabel( QString val );

    bool updateCategoryValue( int catIndex, const QVariant &value );
    bool updateCategorySymbol( int catIndex, QgsSymbolV2* symbol /Transfer/ );
    bool updateCategoryLabel( int catIndex, QString label );

    //! @note added in 2.5
    bool updateCategoryRenderState( int catIndex, bool render );

    void addCategory( const QgsRendererCategoryV2 &category );
    bool deleteCategory( int catIndex );
    void deleteAllCategories();

    //! Moves the category at index position from to index position to.
    void moveCategory( int from, int to );

    void sortByValue( Qt::SortOrder order = Qt::AscendingOrder );
    void sortByLabel( Qt::SortOrder order = Qt::AscendingOrder );

    QString classAttribute() const;
    void setClassAttribute( QString attr );

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

    // Update the color ramp used and all symbols colors.
    //! @note added in 2.5
    void updateColorRamp( QgsVectorColorRampV2* ramp, bool inverted = false );

    void setRotationField( QString fieldOrExpression ) /Deprecated/;
    QString rotationField() const /Deprecated/;

    void setSizeScaleField( QString fieldOrExpression );
    QString sizeScaleField() const;

    void setScaleMethod( QgsSymbolV2::ScaleMethod scaleMethod );
    QgsSymbolV2::ScaleMethod scaleMethod() const;

    //! items of symbology items in legend should be checkable
    //! @note added in 2.5
    virtual bool legendSymbolItemsCheckable() const;

    //! item in symbology was checked
    // @note added in 2.5
    virtual bool legendSymbolItemChecked( QString key );

    //! item in symbology was checked
    // @note added in 2.5
    virtual void checkLegendSymbolItem( QString key, bool state = true );

    //! If supported by the renderer, return classification attribute for the use in legend
    //! @note added in 2.6
    virtual QString legendClassificationAttribute() const;

    //! creates a QgsCategorizedSymbolRendererV2 from an existing renderer.
    //! @note added in 2.5
    //! @returns a new renderer if the conversion was possible, otherwise 0.
    static QgsCategorizedSymbolRendererV2* convertFromRenderer( const QgsFeatureRendererV2 *renderer ) /Factory/;

  protected:
    void rebuildHash();

    QgsSymbolV2* symbolForValue( QVariant value );

  private:
    QgsCategorizedSymbolRendererV2( const QgsCategorizedSymbolRendererV2 & );
    QgsCategorizedSymbolRendererV2 & operator=( const QgsCategorizedSymbolRendererV2 & );
};

