class QgsRendererCategoryV2
{
%TypeHeaderCode
#include <qgscategorizedsymbolrendererv2.h>
%End
  public:

    //! takes ownership of symbol
    QgsRendererCategoryV2( const QVariant& value, QgsSymbolV2* symbol /Transfer/, const QString& label, bool render = true );

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

  protected:

    void swap( QgsRendererCategoryV2 & other );
};

typedef QList<QgsRendererCategoryV2> QgsCategoryList;

class QgsCategorizedSymbolRendererV2 : QgsFeatureRendererV2
{
%TypeHeaderCode
#include <qgscategorizedsymbolrendererv2.h>
%End

  public:

    QgsCategorizedSymbolRendererV2( const QString& attrName = QString(), const QgsCategoryList& categories = QgsCategoryList() );

    virtual ~QgsCategorizedSymbolRendererV2();

    //! @note available in python as symbolForFeature2
    virtual QgsSymbolV2* symbolForFeature( QgsFeature& feature, QgsRenderContext& context ) /PyName=symbolForFeature2/;

    //! @note available in python as originalSymbolForFeature2
    virtual QgsSymbolV2* originalSymbolForFeature( QgsFeature& feature, QgsRenderContext& context ) /PyName=originalSymbolForFeature2/;

    virtual void startRender( QgsRenderContext& context, const QgsFields& fields );

    virtual void stopRender( QgsRenderContext& context );

    virtual QList<QString> usedAttributes();

    virtual QString dump() const;

    virtual QgsCategorizedSymbolRendererV2* clone() const /Factory/;

    virtual void toSld( QDomDocument& doc, QDomElement &element ) const;

    //! returns bitwise OR-ed capabilities of the renderer
    virtual int capabilities();

    virtual QString filter( const QgsFields& fields = QgsFields() );

    //! @note available in python as symbols2
    virtual QgsSymbolV2List symbols( QgsRenderContext& context ) /PyName=symbols2/;

    /** Update all the symbols but leave categories and colors. This method also sets the source
     * symbol for the renderer.
     * @param sym source symbol to use for categories. Ownership is not transferred.
     * @see setSourceSymbol()
     */
    void updateSymbols( QgsSymbolV2 * sym );

    const QgsCategoryList& categories() const;

    //! return index of category with specified value (-1 if not found)
    int categoryIndexForValue( const QVariant& val );

    //! return index of category with specified label (-1 if not found or not unique)
    //! @note added in 2.5
    int categoryIndexForLabel( const QString& val );

    bool updateCategoryValue( int catIndex, const QVariant &value );
    bool updateCategorySymbol( int catIndex, QgsSymbolV2* symbol /Transfer/ );
    bool updateCategoryLabel( int catIndex, const QString& label );

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
    void setClassAttribute( const QString& attr );

    //! create renderer from XML element
    static QgsFeatureRendererV2* create( QDomElement& element ) /Factory/;

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

    /** Returns the renderer's source symbol, which is the base symbol used for the each categories' symbol before applying
     * the categories' color.
     * @see setSourceSymbol()
     * @see sourceColorRamp()
     */
    QgsSymbolV2* sourceSymbol();

    /** Sets the source symbol for the renderer, which is the base symbol used for the each categories' symbol before applying
     * the categories' color.
     * @param sym source symbol, ownership is transferred to the renderer
     * @see sourceSymbol()
     * @see setSourceColorRamp()
     */
    void setSourceSymbol( QgsSymbolV2* sym /Transfer/ );

    /** Returns the source color ramp, from which each categories' color is derived.
     * @see setSourceColorRamp()
     * @see sourceSymbol()
     */
    QgsVectorColorRampV2* sourceColorRamp();

    /** Sets the source color ramp.
      * @param ramp color ramp. Ownership is transferred to the renderer
      * @see sourceColorRamp()
      * @see setSourceSymbol()
      */
    void setSourceColorRamp( QgsVectorColorRampV2* ramp /Transfer/ );

    //! @note added in 2.1
    bool invertedColorRamp();
    void setInvertedColorRamp( bool inverted );

    /** Update the color ramp used and all symbols colors.
      * @param ramp color ramp. Ownership is transferred to the renderer
      * @param inverted set to true to invert ramp colors
      * @note added in 2.5
      */
    void updateColorRamp( QgsVectorColorRampV2* ramp /Transfer/, bool inverted = false );

    void setRotationField( const QString& fieldOrExpression ) /Deprecated/;
    QString rotationField() const /Deprecated/;

    void setSizeScaleField( const QString& fieldOrExpression );
    QString sizeScaleField() const;

    void setScaleMethod( QgsSymbolV2::ScaleMethod scaleMethod );
    QgsSymbolV2::ScaleMethod scaleMethod() const;

    //! items of symbology items in legend should be checkable
    //! @note added in 2.5
    virtual bool legendSymbolItemsCheckable() const;

    //! item in symbology was checked
    // @note added in 2.5
    virtual bool legendSymbolItemChecked( const QString& key );

    virtual void setLegendSymbolItem( const QString& key, QgsSymbolV2* symbol /Transfer/ );

    //! item in symbology was checked
    // @note added in 2.5
    virtual void checkLegendSymbolItem( const QString& key, bool state = true );

    //! If supported by the renderer, return classification attribute for the use in legend
    //! @note added in 2.6
    virtual QString legendClassificationAttribute() const;

    //! creates a QgsCategorizedSymbolRendererV2 from an existing renderer.
    //! @note added in 2.5
    //! @returns a new renderer if the conversion was possible, otherwise 0.
    static QgsCategorizedSymbolRendererV2* convertFromRenderer( const QgsFeatureRendererV2 *renderer ) /Factory/;

  protected:
    void rebuildHash();

    QgsSymbolV2* skipRender();

    QgsSymbolV2* symbolForValue( const QVariant& value );

  private:
    QgsCategorizedSymbolRendererV2( const QgsCategorizedSymbolRendererV2 & );
    QgsCategorizedSymbolRendererV2 & operator=( const QgsCategorizedSymbolRendererV2 & );
};

