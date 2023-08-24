/***************************************************************************
                         qgstiledscenetexturerenderer.h
                         --------------------
    begin                : August 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTILEDSCENETEXTURERENDERER_H
#define QGSTILEDSCENETEXTURERENDERER_H

#include "qgstiledscenerenderer.h"
#include "qgis_core.h"
#include "qgis_sip.h"

class QgsFillSymbol;

/**
 * \ingroup core
 * \brief Renders tiled scene layers using textures.
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsTiledSceneTextureRenderer : public QgsTiledSceneRenderer
{
  public:

    /**
     * Constructor for QgsTiledSceneTextureRenderer.
     */
    QgsTiledSceneTextureRenderer();
    ~QgsTiledSceneTextureRenderer();

    QString type() const override;
    QgsTiledSceneRenderer *clone() const override;
    QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) const override;
    Qgis::TiledSceneRendererFlags flags() const override;
    void renderTriangle( QgsTiledSceneRenderContext &context, const QPolygonF &triangle ) override;
    void renderLine( QgsTiledSceneRenderContext &context, const QPolygonF &line ) override;
    void startRender( QgsTiledSceneRenderContext &context ) override;
    void stopRender( QgsTiledSceneRenderContext &context ) override;

    /**
     * Creates a textured renderer from an XML \a element.
     */
    static QgsTiledSceneRenderer *create( QDomElement &element, const QgsReadWriteContext &context ) SIP_FACTORY;

    /**
     * Returns a copy of the default fill symbol used to render triangles without textures.
     *
     * \see setFillSymbol()
     */
    static QgsFillSymbol *createDefaultFillSymbol() SIP_FACTORY;

    /**
     * Returns the fill symbol used to render triangles without textures.
     *
     * \see setFillSymbol()
     */
    QgsFillSymbol *fillSymbol() const;

    /**
     * Sets the fill \a symbol used to render triangles without textures.
     *
     * Ownership of \a symbol is transferred.
     *
     * \see fillSymbol()
     */
    void setFillSymbol( QgsFillSymbol *symbol SIP_TRANSFER );

  private:
    std::unique_ptr< QgsFillSymbol> mFillSymbol;
};

#endif // QGSTILEDSCENETEXTURERENDERER_H
