/***************************************************************************
                         qgstiledscenewireframerenderer.h
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

#ifndef QGSTILEDSCENEWIREFRAMERENDERER_H
#define QGSTILEDSCENEWIREFRAMERENDERER_H

#include "qgstiledscenerenderer.h"
#include "qgis_core.h"
#include "qgis_sip.h"

class QgsFillSymbol;
class QgsLineSymbol;

/**
 * \ingroup core
 * \brief Renders tiled scene layers using the raw primitive wireframes.
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsTiledSceneWireframeRenderer : public QgsTiledSceneRenderer
{
  public:

    /**
     * Constructor for QgsTiledSceneWireframeRenderer.
     */
    QgsTiledSceneWireframeRenderer();
    ~QgsTiledSceneWireframeRenderer();

    QString type() const override;
    QgsTiledSceneRenderer *clone() const override;
    QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) const override;
    void renderTriangle( QgsTiledSceneRenderContext &context, const QPolygonF &triangle ) override;
    void renderLine( QgsTiledSceneRenderContext &context, const QPolygonF &line ) override;
    void startRender( QgsTiledSceneRenderContext &context ) override;
    void stopRender( QgsTiledSceneRenderContext &context ) override;
    Qgis::TiledSceneRendererFlags flags() const override;

    /**
     * Creates a textured renderer from an XML \a element.
     */
    static QgsTiledSceneRenderer *create( QDomElement &element, const QgsReadWriteContext &context ) SIP_FACTORY;

    /**
     * Returns a copy of the default fill symbol used to render triangles in the wireframe.
     *
     * \see setFillSymbol()
     */
    static QgsFillSymbol *createDefaultFillSymbol() SIP_FACTORY;

    /**
     * Returns the fill symbol used to render triangles in the wireframe.
     *
     * \see setFillSymbol()
     */
    QgsFillSymbol *fillSymbol() const;

    /**
     * Sets the fill \a symbol used to render triangles in the wireframe.
     *
     * Ownership of \a symbol is transferred.
     *
     * \see fillSymbol()
     */
    void setFillSymbol( QgsFillSymbol *symbol SIP_TRANSFER );

    /**
     * Returns a copy of the default line symbol used to render lines in the wireframe.
     *
     * \see setLineSymbol()
     */
    static QgsLineSymbol *createDefaultLineSymbol() SIP_FACTORY;

    /**
     * Returns the line symbol used to render lines in the wireframe.
     *
     * \see setLineSymbol()
     */
    QgsLineSymbol *lineSymbol() const;

    /**
     * Sets the line \a symbol used to render lines in the wireframe.
     *
     * Ownership of \a symbol is transferred.
     *
     * \see lineSymbol()
     */
    void setLineSymbol( QgsLineSymbol *symbol SIP_TRANSFER );

    /**
     * Returns TRUE if representative colors from the textures will be used to recolor
     * the symbols used to render the wireframe.
     *
     * \see setUseTextureColors()
     */
    bool useTextureColors() const;

    /**
     * Sets whether representative colors from the textures should be used to recolor
     * the symbols used to render the wireframe.
     *
     * \see useTextureColors()
     */
    void setUseTextureColors( bool enabled );

  private:

    std::unique_ptr< QgsFillSymbol> mFillSymbol;
    std::unique_ptr< QgsLineSymbol> mLineSymbol;
    bool mUseTextureColors = false;

};

#endif // QGSTILEDSCENEWIREFRAMERENDERER_H
