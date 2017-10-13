/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgssinglesymbolrenderer.h                         *
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
%Docstring
 :rtype: QgsFeatureRenderer
%End

    virtual QgsFeatureRenderer::Capabilities capabilities();
    virtual QgsSymbolList symbols( QgsRenderContext &context );

    static QgsFeatureRenderer *create( QDomElement &element, const QgsReadWriteContext &context ) /Factory/;
%Docstring
create renderer from XML element
 :rtype: QgsFeatureRenderer
%End
    virtual QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context );
    virtual QgsLegendSymbolList legendSymbolItems() const;
    virtual QSet< QString > legendKeysForFeature( QgsFeature &feature, QgsRenderContext &context );
    virtual void setLegendSymbolItem( const QString &key, QgsSymbol *symbol /Transfer/ );

    static QgsSingleSymbolRenderer *convertFromRenderer( const QgsFeatureRenderer *renderer ) /Factory/;
%Docstring
 creates a QgsSingleSymbolRenderer from an existing renderer.
.. versionadded:: 2.5
 :return: a new renderer if the conversion was possible, otherwise 0.
 :rtype: QgsSingleSymbolRenderer
%End

    void setDataDefinedSizeLegend( QgsDataDefinedSizeLegend *settings /Transfer/ );
%Docstring
 Configures appearance of legend when renderer is configured to use data-defined size for marker symbols.
 This allows configuring which values (symbol sizes) should be shown in the legend, whether to display
 different symbol sizes collapsed in one legend node or separated across multiple legend nodes etc.

 When renderer does not use data-defined size or does not use marker symbols, these settings will be ignored.
 Takes ownership of the passed settings objects. Null pointer is a valid input that disables data-defined
 size legend.
.. versionadded:: 3.0
%End

    QgsDataDefinedSizeLegend *dataDefinedSizeLegend() const;
%Docstring
 Returns configuration of appearance of legend when using data-defined size for marker symbols.
 Will return null if the functionality is disabled.
.. versionadded:: 3.0
 :rtype: QgsDataDefinedSizeLegend
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
 * src/core/symbology/qgssinglesymbolrenderer.h                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
