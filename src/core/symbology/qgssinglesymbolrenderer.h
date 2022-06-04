/***************************************************************************
    qgssinglesymbolrenderer.h
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSSINGLESYMBOLRENDERER_H
#define QGSSINGLESYMBOLRENDERER_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgsrenderer.h"

class QgsDataDefinedSizeLegend;
class QgsSymbol;

/**
 * \ingroup core
 * \class QgsSingleSymbolRenderer
 */
class CORE_EXPORT QgsSingleSymbolRenderer : public QgsFeatureRenderer
{
  public:

    /**
     * Constructor for QgsSingleSymbolRenderer.
     *
     * The same \a symbol will be used to render every feature. Ownership
     * of \a symbol is transferred to the renderer.
     */
    QgsSingleSymbolRenderer( QgsSymbol *symbol SIP_TRANSFER );
    ~QgsSingleSymbolRenderer() override;

    QgsSymbol *symbolForFeature( const QgsFeature &feature, QgsRenderContext &context ) const override;
    QgsSymbol *originalSymbolForFeature( const QgsFeature &feature, QgsRenderContext &context ) const override;
    void startRender( QgsRenderContext &context, const QgsFields &fields ) override;
    void stopRender( QgsRenderContext &context ) override;
    QSet<QString> usedAttributes( const QgsRenderContext &context ) const override;
    bool accept( QgsStyleEntityVisitorInterface *visitor ) const override;

    /**
     * Returns the symbol which will be rendered for every feature.
     * \see setSymbol()
     */
    QgsSymbol *symbol() const;

    /**
     * Sets the symbol which will be rendered for every feature. Ownership
     * of the symbol is transferred to the renderer.
     * \see symbol()
     */
    void setSymbol( QgsSymbol *s SIP_TRANSFER );

    QString dump() const override;

    QgsSingleSymbolRenderer *clone() const override SIP_FACTORY;

    void toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props = QVariantMap() ) const override;

    /**
     * Creates a new single symbol renderer from an SLD \a element.
     *
     * The geometry type for features to be rendered must be specified via the \a geomType argument.
     *
     * The caller takes ownership of the returned renderer.
     */
    static QgsFeatureRenderer *createFromSld( QDomElement &element, QgsWkbTypes::GeometryType geomType ) SIP_FACTORY;

    QgsFeatureRenderer::Capabilities capabilities() override { return SymbolLevels; }
    QgsSymbolList symbols( QgsRenderContext &context ) const override;

    /**
     * Creates a new single symbol renderer from an XML \a element, using the supplied read/write \a context.
     *
     * The caller takes ownership of the returned renderer.
     */
    static QgsFeatureRenderer *create( QDomElement &element, const QgsReadWriteContext &context ) SIP_FACTORY;

    QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) override;
    QgsLegendSymbolList legendSymbolItems() const override;
    QSet< QString > legendKeysForFeature( const QgsFeature &feature, QgsRenderContext &context ) const override;
    QString legendKeyToExpression( const QString &key, QgsVectorLayer *layer, bool &ok ) const override;
    void setLegendSymbolItem( const QString &key, QgsSymbol *symbol SIP_TRANSFER ) override;

    /**
     * Creates a new single symbol renderer from an existing \a renderer.
     * \returns a new renderer if the conversion was possible, otherwise NULLPTR.
     * \since QGIS 2.5
     */
    static QgsSingleSymbolRenderer *convertFromRenderer( const QgsFeatureRenderer *renderer ) SIP_FACTORY;

    /**
     * Configures appearance of legend when renderer is configured to use data-defined size for marker symbols.
     * This allows configuring which values (symbol sizes) should be shown in the legend, whether to display
     * different symbol sizes collapsed in one legend node or separated across multiple legend nodes etc.
     *
     * When renderer does not use data-defined size or does not use marker symbols, these settings will be ignored.
     * Takes ownership of the passed settings objects. NULLPTR is a valid input that disables data-defined
     * size legend.
     * \since QGIS 3.0
     */
    void setDataDefinedSizeLegend( QgsDataDefinedSizeLegend *settings SIP_TRANSFER );

    /**
     * Returns configuration of appearance of legend when using data-defined size for marker symbols.
     * Will return NULLPTR if the functionality is disabled.
     * \since QGIS 3.0
     */
    QgsDataDefinedSizeLegend *dataDefinedSizeLegend() const;

  protected:
    std::unique_ptr<QgsSymbol> mSymbol;
    std::unique_ptr<QgsDataDefinedSizeLegend> mDataDefinedSizeLegend;

  private:
#ifdef SIP_RUN
    QgsSingleSymbolRenderer( const QgsSingleSymbolRenderer & );
    QgsSingleSymbolRenderer &operator=( const QgsSingleSymbolRenderer & );
#endif

};


#endif // QGSSINGLESYMBOLRENDERER_H
