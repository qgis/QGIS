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

#include "qgis.h"
#include "qgsrendererv2.h"
#include "qgssymbolv2.h"

/** \ingroup core
 * \class QgsNullSymbolRenderer
 * \brief Null symbol renderer. Renderer which draws no symbols for features by default, but allows for labeling
 * and diagrams for the layer. Selected features will also be drawn with a default symbol.
 * \note Added in version 2.16
 */

class CORE_EXPORT QgsNullSymbolRenderer : public QgsFeatureRendererV2
{
  public:

    QgsNullSymbolRenderer();

    virtual ~QgsNullSymbolRenderer();

    virtual QgsSymbolV2* symbolForFeature( QgsFeature& feature, QgsRenderContext& context ) override;
    virtual QgsSymbolV2* originalSymbolForFeature( QgsFeature& feature, QgsRenderContext& context ) override;

    virtual bool renderFeature( QgsFeature& feature, QgsRenderContext& context, int layer = -1, bool selected = false, bool drawVertexMarker = false ) override;
    virtual void startRender( QgsRenderContext& context, const QgsFields& fields ) override;
    virtual void stopRender( QgsRenderContext& context ) override;
    virtual bool willRenderFeature( QgsFeature& feat, QgsRenderContext& context ) override;

    virtual QList<QString> usedAttributes() override;
    virtual QString dump() const override;
    virtual QgsFeatureRendererV2* clone() const override;
    virtual QgsSymbolV2List symbols( QgsRenderContext& context ) override;

    /** Creates a null renderer from XML element.
     * @param element DOM element
     * @returns new null symbol renderer
     */
    static QgsFeatureRendererV2* create( QDomElement& element );

    virtual QDomElement save( QDomDocument& doc ) override;

    /** Creates a QgsNullSymbolRenderer from an existing renderer.
     * @param renderer renderer to convert from
     * @returns a new renderer if the conversion was possible, otherwise nullptr.
     */
    static QgsNullSymbolRenderer* convertFromRenderer( const QgsFeatureRendererV2 *renderer );

  private:

    //! Symbol to use for rendering selected features
    QScopedPointer<QgsSymbolV2> mSymbol;

};

#endif // QGSNULLSYMBOLRENDERER_H
