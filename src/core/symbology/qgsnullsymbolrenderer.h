/***************************************************************************
    qgsnullsymbolrenderer.h
    ---------------------
    begin                : November 2014
    copyright            : (C) 2014 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSNULLSYMBOLRENDERER_H
#define QGSNULLSYMBOLRENDERER_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgsrenderer.h"
#include "qgssymbol.h"

/**
 * \ingroup core
 * \class QgsNullSymbolRenderer
 * \brief Null symbol renderer. Renderer which draws no symbols for features by default, but allows for labeling
 * and diagrams for the layer. Selected features will also be drawn with a default symbol.
 * \since QGIS 2.16
 */

class CORE_EXPORT QgsNullSymbolRenderer : public QgsFeatureRenderer
{
  public:

    QgsNullSymbolRenderer();

    QgsSymbol *symbolForFeature( const QgsFeature &feature, QgsRenderContext &context ) const override;
    QgsSymbol *originalSymbolForFeature( const QgsFeature &feature, QgsRenderContext &context ) const override;

    bool renderFeature( const QgsFeature &feature, QgsRenderContext &context, int layer = -1, bool selected = false, bool drawVertexMarker = false ) override;
    void stopRender( QgsRenderContext &context ) override;
    bool willRenderFeature( const QgsFeature &feature, QgsRenderContext &context ) const override;

    QSet<QString> usedAttributes( const QgsRenderContext &context ) const override;
    QString dump() const override;
    QgsFeatureRenderer *clone() const override SIP_FACTORY;
    QgsSymbolList symbols( QgsRenderContext &context ) const override;

    /**
     * Creates a null renderer from XML element.
     * \param element DOM element
     * \param context reading context
     * \returns new null symbol renderer
     */
    static QgsFeatureRenderer *create( QDomElement &element, const QgsReadWriteContext &context ) SIP_FACTORY;

    QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) override;

    /**
     * Creates a QgsNullSymbolRenderer from an existing renderer.
     * \param renderer renderer to convert from
     * \returns a new renderer if the conversion was possible, otherwise NULLPTR.
     */
    static QgsNullSymbolRenderer *convertFromRenderer( const QgsFeatureRenderer *renderer ) SIP_FACTORY;

  private:

    //! Symbol to use for rendering selected features
    std::unique_ptr<QgsSymbol> mSymbol;

};

#endif // QGSNULLSYMBOLRENDERER_H
