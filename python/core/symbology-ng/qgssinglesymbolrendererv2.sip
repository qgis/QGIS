class QgsSingleSymbolRendererV2 : QgsFeatureRendererV2
{
%TypeHeaderCode
#include <qgssinglesymbolrendererv2.h>
%End
  public:

    QgsSingleSymbolRendererV2( QgsSymbolV2* symbol /Transfer/ );

    virtual ~QgsSingleSymbolRendererV2();

    virtual QgsSymbolV2* symbolForFeature( QgsFeature& feature, QgsRenderContext& context );

    virtual QgsSymbolV2* originalSymbolForFeature( QgsFeature& feature, QgsRenderContext& context );

    virtual void startRender( QgsRenderContext& context, const QgsFields& fields );

    virtual void stopRender( QgsRenderContext& context );

    virtual QList<QString> usedAttributes();

    QgsSymbolV2* symbol() const;
    void setSymbol( QgsSymbolV2* s /Transfer/ );

    void setRotationField( QString fieldOrExpression ) /Deprecated/;
    QString rotationField() const /Deprecated/;

    void setSizeScaleField( QString fieldOrExpression );
    QString sizeScaleField() const;

    void setScaleMethod( QgsSymbolV2::ScaleMethod scaleMethod );
    QgsSymbolV2::ScaleMethod scaleMethod() const;

    virtual QString dump() const;

    virtual QgsFeatureRendererV2* clone() const /Factory/;

    virtual void toSld( QDomDocument& doc, QDomElement &element ) const;
    static QgsFeatureRendererV2* createFromSld( QDomElement& element, QGis::GeometryType geomType );

    //! returns bitwise OR-ed capabilities of the renderer
    virtual int capabilities();

    virtual QgsSymbolV2List symbols( QgsRenderContext& context );

    //! create renderer from XML element
    static QgsFeatureRendererV2* create( QDomElement& element ) /Factory/;

    //! store renderer info to XML element
    virtual QDomElement save( QDomDocument& doc );

    //! return a list of symbology items for the legend
    virtual QgsLegendSymbologyList legendSymbologyItems( QSize iconSize );

    //! return a list of item text / symbol
    //! @note not available in python bindings
    // virtual QgsLegendSymbolList legendSymbolItems( double scaleDenominator = -1, QString rule = QString() );

    //! Return a list of symbology items for the legend. Better choice than legendSymbolItems().
    //! @note added in 2.6
    virtual QgsLegendSymbolListV2 legendSymbolItemsV2() const;

    //! creates a QgsSingleSymbolRendererV2 from an existing renderer.
    //! @note added in 2.5
    //! @returns a new renderer if the conversion was possible, otherwise 0.
    static QgsSingleSymbolRendererV2* convertFromRenderer( const QgsFeatureRendererV2 *renderer ) /Factory/;

  private:
    QgsSingleSymbolRendererV2( const QgsSingleSymbolRendererV2 & );
    QgsSingleSymbolRendererV2 & operator=( const QgsSingleSymbolRendererV2 & );
};
