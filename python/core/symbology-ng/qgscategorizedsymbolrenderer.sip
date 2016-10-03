class QgsRendererCategory
{
%TypeHeaderCode
#include <qgscategorizedsymbolrenderer.h>
%End
  public:

    //! takes ownership of symbol
    QgsRendererCategory( const QVariant& value, QgsSymbol* symbol /Transfer/, const QString& label, bool render = true );

    //! copy constructor
    QgsRendererCategory( const QgsRendererCategory& cat );

    ~QgsRendererCategory();

    QVariant value() const;
    QgsSymbol* symbol() const;
    QString label() const;

    void setValue( const QVariant &value );
    void setSymbol( QgsSymbol* s /Transfer/ );
    void setLabel( const QString &label );

    // @note added in 2.5
    bool renderState() const;
    void setRenderState( bool render );

    // debugging
    QString dump() const;

    void toSld( QDomDocument& doc, QDomElement &element, QgsStringMap props ) const;

  protected:

    void swap( QgsRendererCategory & other );
};

typedef QList<QgsRendererCategory> QgsCategoryList;

class QgsCategorizedSymbolRenderer : QgsFeatureRenderer
{
%TypeHeaderCode
#include <qgscategorizedsymbolrenderer.h>
%End

  public:

    QgsCategorizedSymbolRenderer( const QString& attrName = QString(), const QgsCategoryList& categories = QgsCategoryList() );

    virtual ~QgsCategorizedSymbolRenderer();

    //! @note available in python as symbolForFeature2
    virtual QgsSymbol* symbolForFeature( QgsFeature& feature, QgsRenderContext& context );

    //! @note available in python as originalSymbolForFeature2
    virtual QgsSymbol* originalSymbolForFeature( QgsFeature& feature, QgsRenderContext& context );

    virtual void startRender( QgsRenderContext& context, const QgsFields& fields );

    virtual void stopRender( QgsRenderContext& context );

    virtual QSet<QString> usedAttributes() const;

    virtual QString dump() const;

    virtual QgsCategorizedSymbolRenderer* clone() const /Factory/;

    virtual void toSld( QDomDocument& doc, QDomElement &element ) const;

    //! returns bitwise OR-ed capabilities of the renderer
    virtual QgsFeatureRenderer::Capabilities capabilities();

    virtual QString filter( const QgsFields& fields = QgsFields() );

    //! @note available in python as symbols2
    virtual QgsSymbolList symbols( QgsRenderContext& context );

    /** Update all the symbols but leave categories and colors. This method also sets the source
     * symbol for the renderer.
     * @param sym source symbol to use for categories. Ownership is not transferred.
     * @see setSourceSymbol()
     */
    void updateSymbols( QgsSymbol * sym );

    const QgsCategoryList& categories() const;

    //! return index of category with specified value (-1 if not found)
    int categoryIndexForValue( const QVariant& val );

    //! return index of category with specified label (-1 if not found or not unique)
    //! @note added in 2.5
    int categoryIndexForLabel( const QString& val );

    bool updateCategoryValue( int catIndex, const QVariant &value );
    bool updateCategorySymbol( int catIndex, QgsSymbol* symbol /Transfer/ );
    bool updateCategoryLabel( int catIndex, const QString& label );

    //! @note added in 2.5
    bool updateCategoryRenderState( int catIndex, bool render );

    void addCategory( const QgsRendererCategory &category );
    bool deleteCategory( int catIndex );
    void deleteAllCategories();

    //! Moves the category at index position from to index position to.
    void moveCategory( int from, int to );

    void sortByValue( Qt::SortOrder order = Qt::AscendingOrder );
    void sortByLabel( Qt::SortOrder order = Qt::AscendingOrder );

    QString classAttribute() const;
    void setClassAttribute( const QString& attr );

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

    /** Returns the renderer's source symbol, which is the base symbol used for the each categories' symbol before applying
     * the categories' color.
     * @see setSourceSymbol()
     * @see sourceColorRamp()
     */
    QgsSymbol* sourceSymbol();

    /** Sets the source symbol for the renderer, which is the base symbol used for the each categories' symbol before applying
     * the categories' color.
     * @param sym source symbol, ownership is transferred to the renderer
     * @see sourceSymbol()
     * @see setSourceColorRamp()
     */
    void setSourceSymbol( QgsSymbol* sym /Transfer/ );

    /** Returns the source color ramp, from which each categories' color is derived.
     * @see setSourceColorRamp()
     * @see sourceSymbol()
     */
    QgsColorRamp* sourceColorRamp();

    /** Sets the source color ramp.
      * @param ramp color ramp. Ownership is transferred to the renderer
      * @see sourceColorRamp()
      * @see setSourceSymbol()
      */
    void setSourceColorRamp( QgsColorRamp* ramp /Transfer/ );

    //! @note added in 2.1
    bool invertedColorRamp();
    void setInvertedColorRamp( bool inverted );

    /** Update the color ramp used and all symbols colors.
      * @param ramp color ramp. Ownership is transferred to the renderer
      * @param inverted set to true to invert ramp colors
      * @note added in 2.5
      */
    void updateColorRamp( QgsColorRamp* ramp /Transfer/, bool inverted = false );

    //! items of symbology items in legend should be checkable
    //! @note added in 2.5
    virtual bool legendSymbolItemsCheckable() const;

    //! item in symbology was checked
    // @note added in 2.5
    virtual bool legendSymbolItemChecked( const QString& key );

    virtual void setLegendSymbolItem( const QString& key, QgsSymbol* symbol /Transfer/ );

    //! item in symbology was checked
    // @note added in 2.5
    virtual void checkLegendSymbolItem( const QString& key, bool state = true );

    //! If supported by the renderer, return classification attribute for the use in legend
    //! @note added in 2.6
    virtual QString legendClassificationAttribute() const;

    //! creates a QgsCategorizedSymbolRenderer from an existing renderer.
    //! @note added in 2.5
    //! @returns a new renderer if the conversion was possible, otherwise 0.
    static QgsCategorizedSymbolRenderer* convertFromRenderer( const QgsFeatureRenderer *renderer ) /Factory/;

  protected:
    void rebuildHash();

    QgsSymbol* skipRender();

    QgsSymbol* symbolForValue( const QVariant& value );

  private:
    QgsCategorizedSymbolRenderer( const QgsCategorizedSymbolRenderer & );
    QgsCategorizedSymbolRenderer & operator=( const QgsCategorizedSymbolRenderer & );
};

