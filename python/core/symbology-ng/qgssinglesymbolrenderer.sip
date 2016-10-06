class QgsSingleSymbolRenderer : QgsFeatureRenderer
{
%TypeHeaderCode
#include <qgssinglesymbolrenderer.h>
%End
  public:

    QgsSingleSymbolRenderer( QgsSymbol* symbol /Transfer/ );

    virtual ~QgsSingleSymbolRenderer();

    //! @note available in python as symbolForFeature2
    virtual QgsSymbol* symbolForFeature( QgsFeature& feature, QgsRenderContext& context );

    //! @note available in python as originalSymbolForFeature2
    virtual QgsSymbol* originalSymbolForFeature( QgsFeature& feature, QgsRenderContext& context );

    virtual void startRender( QgsRenderContext& context, const QgsFields& fields );

    virtual void stopRender( QgsRenderContext& context );

    virtual QSet<QString> usedAttributes() const;

    QgsSymbol* symbol() const;
    void setSymbol( QgsSymbol* s /Transfer/ );

    virtual QString dump() const;

    virtual QgsSingleSymbolRenderer* clone() const /Factory/;

    virtual void toSld( QDomDocument& doc, QDomElement &element ) const;
    static QgsFeatureRenderer* createFromSld( QDomElement& element, QgsWkbTypes::GeometryType geomType );

    //! returns bitwise OR-ed capabilities of the renderer
    virtual QgsFeatureRenderer::Capabilities capabilities();

    //! @note available in python as symbol2
    virtual QgsSymbolList symbols( QgsRenderContext& context );

    //! create renderer from XML element
    static QgsFeatureRenderer* create( QDomElement& element ) /Factory/;

    //! store renderer info to XML element
    virtual QDomElement save( QDomDocument& doc );

    //! return a list of symbology items for the legend
    virtual QgsLegendSymbologyList legendSymbologyItems( QSize iconSize );

    //! return a list of item text / symbol
    //! @note not available in python bindings
    // virtual QgsLegendSymbolList legendSymbolItems( double scaleDenominator = -1, const QString& rule = QString() );

    //! Return a list of symbology items for the legend. Better choice than legendSymbolItems().
    //! @note added in 2.6
    virtual QgsLegendSymbolListV2 legendSymbolItemsV2() const;

    virtual QSet< QString > legendKeysForFeature( QgsFeature& feature, QgsRenderContext& context );

    virtual void setLegendSymbolItem( const QString& key, QgsSymbol* symbol /Transfer/ );

    //! creates a QgsSingleSymbolRenderer from an existing renderer.
    //! @note added in 2.5
    //! @returns a new renderer if the conversion was possible, otherwise 0.
    static QgsSingleSymbolRenderer* convertFromRenderer( const QgsFeatureRenderer *renderer ) /Factory/;

  private:
    QgsSingleSymbolRenderer( const QgsSingleSymbolRenderer & );
    QgsSingleSymbolRenderer & operator=( const QgsSingleSymbolRenderer & );
};
