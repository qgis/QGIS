/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology-ng/qgssinglesymbolrenderer.h                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/


class QgsSingleSymbolRenderer : QgsFeatureRenderer
{

%TypeHeaderCode
#include "qgssinglesymbolrenderer.h"
%End
  public:

    QgsSingleSymbolRenderer( QgsSymbol *symbol /Transfer/ );

    virtual QgsSymbol *symbolForFeature( QgsFeature &feature, QgsRenderContext &context );
    virtual QgsSymbol *originalSymbolForFeature( QgsFeature &feature, QgsRenderContext &context );
    virtual void startRender( QgsRenderContext &context, const QgsFields &fields );
    virtual void stopRender( QgsRenderContext &context );
    virtual QSet<QString> usedAttributes( const QgsRenderContext &context ) const;

    QgsSymbol *symbol() const;
%Docstring
 :rtype: QgsSymbol
%End
    void setSymbol( QgsSymbol *s /Transfer/ );

    virtual QString dump() const;

    virtual QgsSingleSymbolRenderer *clone() const /Factory/;

    virtual void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props = QgsStringMap() ) const;
    static QgsFeatureRenderer *createFromSld( QDomElement &element, QgsWkbTypes::GeometryType geomType );

    virtual QgsFeatureRenderer::Capabilities capabilities();
    virtual QgsSymbolList symbols( QgsRenderContext &context );

    static QgsFeatureRenderer *create( QDomElement &element, const QgsReadWriteContext &context ) /Factory/;
%Docstring
create renderer from XML element
 :rtype: QgsFeatureRenderer
%End
    virtual QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context );
    virtual QgsLegendSymbologyList legendSymbologyItems( QSize iconSize );
    virtual QgsLegendSymbolListV2 legendSymbolItemsV2() const;
    virtual QSet< QString > legendKeysForFeature( QgsFeature &feature, QgsRenderContext &context );
    virtual void setLegendSymbolItem( const QString &key, QgsSymbol *symbol /Transfer/ );

    static QgsSingleSymbolRenderer *convertFromRenderer( const QgsFeatureRenderer *renderer ) /Factory/;
%Docstring
:return: a new renderer if the conversion was possible, otherwise 0.
 :rtype: QgsSingleSymbolRenderer
%End

  protected:

  private:
    QgsSingleSymbolRenderer( const QgsSingleSymbolRenderer & );
    QgsSingleSymbolRenderer &operator=( const QgsSingleSymbolRenderer & );
%Docstring
 :rtype: QgsSingleSymbolRenderer
%End
};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology-ng/qgssinglesymbolrenderer.h                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
