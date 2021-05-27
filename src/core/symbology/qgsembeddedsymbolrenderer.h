/***************************************************************************
    qgsembeddedsymbolrenderer.h
    ---------------------
    begin                : March 2021
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSEMBEDDEDSYMBOLRENDERER_H
#define QGSEMBEDDEDSYMBOLRENDERER_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgsrenderer.h"

/**
 * \ingroup core
 * \class QgsEmbeddedSymbolRenderer
 *
 * \brief A vector feature renderer which uses embedded feature symbology to render per-feature
 * symbols.
 *
 * This renderer can be used for vector layers with a data provider which supports the
 * QgsVectorDataProvider::FeatureSymbology capability, where the dataset has embedded information
 * on how each feature should be rendered.
 *
 * \since QGIS 3.20
 */
class CORE_EXPORT QgsEmbeddedSymbolRenderer : public QgsFeatureRenderer
{
  public:

    /**
     * Constructor for QgsEmbeddedSymbolRenderer.
     *
     * The \a defaultSymbol will be used to render any feature without embedded symbology. Ownership
     * of \a defaultSymbol is transferred to the renderer.
     */
    QgsEmbeddedSymbolRenderer( QgsSymbol *defaultSymbol SIP_TRANSFER );
    ~QgsEmbeddedSymbolRenderer() override;

    QgsSymbol *symbolForFeature( const QgsFeature &feature, QgsRenderContext &context ) const override;
    QgsSymbol *originalSymbolForFeature( const QgsFeature &feature, QgsRenderContext &context ) const override;
    void startRender( QgsRenderContext &context, const QgsFields &fields ) override;
    bool renderFeature( const QgsFeature &feature, QgsRenderContext &context, int layer = -1, bool selected = false, bool drawVertexMarker = false ) SIP_THROW( QgsCsException ) override;
    void stopRender( QgsRenderContext &context ) override;
    QSet<QString> usedAttributes( const QgsRenderContext &context ) const override;
    bool usesEmbeddedSymbols() const override;
    QgsEmbeddedSymbolRenderer *clone() const override SIP_FACTORY;
    QgsFeatureRenderer::Capabilities capabilities() override;
    QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) override;
    QgsSymbolList symbols( QgsRenderContext &context ) const override;

    /**
     * Returns the default symbol which will be rendered for any feature which does not have embedded symbology.
     * \see setDefaultSymbol()
     */
    QgsSymbol *defaultSymbol() const;

    /**
     * Sets the default \a symbol which will be rendered for any feature which does not have embedded symbology.
     *
     * Ownership of \a symbol is transferred to the renderer.
     *
     * \see defaultSymbol()
     */
    void setDefaultSymbol( QgsSymbol *symbol SIP_TRANSFER );

    /**
     * Creates a new embedded symbol renderer from an XML \a element, using the supplied read/write \a context.
     *
     * The caller takes ownership of the returned renderer.
     */
    static QgsFeatureRenderer *create( QDomElement &element, const QgsReadWriteContext &context ) SIP_FACTORY;

    /**
     * Creates a QgsEmbeddedSymbolRenderer from an existing \a renderer.
     * \returns a new renderer if the conversion was possible, otherwise NULLPTR.
     */
    static QgsEmbeddedSymbolRenderer *convertFromRenderer( const QgsFeatureRenderer *renderer ) SIP_FACTORY;

  private:
#ifdef SIP_RUN
    QgsEmbeddedSymbolRenderer( const QgsEmbeddedSymbolRenderer & );
    QgsEmbeddedSymbolRenderer &operator=( const QgsEmbeddedSymbolRenderer & );
#endif

    std::unique_ptr<QgsSymbol> mDefaultSymbol;

};


#endif // QGSEMBEDDEDSYMBOLRENDERER_H
