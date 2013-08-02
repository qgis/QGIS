class QgsSingleSymbolRendererV2 : QgsFeatureRendererV2
{
%TypeHeaderCode
#include <qgssinglesymbolrendererv2.h>
%End
  public:

    QgsSingleSymbolRendererV2( QgsSymbolV2* symbol /Transfer/ );

    virtual ~QgsSingleSymbolRendererV2();

    virtual QgsSymbolV2* symbolForFeature( QgsFeature& feature );

    virtual void startRender( QgsRenderContext& context, const QgsVectorLayer *vlayer );

    virtual void stopRender( QgsRenderContext& context );

    virtual QList<QString> usedAttributes();

    QgsSymbolV2* symbol() const;
    void setSymbol( QgsSymbolV2* s /Transfer/ );

    //! @note added in 1.5
    void setRotationField( QString fieldName );
    //! @note added in 1.5
    QString rotationField() const;

    //! @note added in 1.5
    void setSizeScaleField( QString fieldName );
    //! @note added in 1.5
    QString sizeScaleField() const;

    //! @note added in 2.0
    void setScaleMethod( QgsSymbolV2::ScaleMethod scaleMethod );
    //! @note added in 2.0
    QgsSymbolV2::ScaleMethod scaleMethod() const;

    virtual QString dump() const;

    virtual QgsFeatureRendererV2* clone() /Factory/;

    virtual void toSld( QDomDocument& doc, QDomElement &element ) const;
    static QgsFeatureRendererV2* createFromSld( QDomElement& element, QGis::GeometryType geomType );

    //! returns bitwise OR-ed capabilities of the renderer
    //! \note added in 2.0
    virtual int capabilities();

    virtual QgsSymbolV2List symbols();

    //! create renderer from XML element
    static QgsFeatureRendererV2* create( QDomElement& element ) /Factory/;

    //! store renderer info to XML element
    virtual QDomElement save( QDomDocument& doc );

    //! return a list of symbology items for the legend
    virtual QgsLegendSymbologyList legendSymbologyItems( QSize iconSize );

    //! return a list of item text / symbol
    //! @note: this method was added in version 1.5
    // virtual QgsLegendSymbolList legendSymbolItems();
};
