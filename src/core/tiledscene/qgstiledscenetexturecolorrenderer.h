/***************************************************************************
                         qgstiledscenetexturecolorrenderer.h
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

#ifndef QGSTILEDSCENETEXTURECOLORRENDERER_H
#define QGSTILEDSCENETEXTURECOLORRENDERER_H

#include "qgstiledscenerenderer.h"
#include "qgis_core.h"
#include "qgis_sip.h"

/**
 * \ingroup core
 * \brief Renders tiled scene layers using representative colors from textures.
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsTiledSceneTextureColorRenderer : public QgsTiledSceneRenderer
{
  public:

    /**
     * Constructor for QgsTiledSceneTextureColorRenderer.
     */
    QgsTiledSceneTextureColorRenderer();

    QString type() const override;
    QgsTiledSceneRenderer *clone() const override;
    QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) const override;
    Qgis::TiledSceneRendererFlags flags() const override;
    void renderTriangle( QgsTiledSceneRenderContext &context, const QPolygonF &triangle ) override;

    /**
     * Creates a textured renderer from an XML \a element.
     */
    static QgsTiledSceneRenderer *create( QDomElement &element, const QgsReadWriteContext &context ) SIP_FACTORY;

  private:

};

#endif // QGSTILEDSCENETEXTURECOLORRENDERER_H
