/***************************************************************************
                         qgsmeshlayerrenderer.h
                         ----------------------
    begin                : April 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESHLAYERRENDERER_H
#define QGSMESHLAYERRENDERER_H

class QgsMeshLayer;
class QgsSymbol;

#define SIP_NO_FILE

#include <memory>

#include "qgis.h"

#include "qgsmaplayerrenderer.h"
#include "qgsrendercontext.h"
#include "qgstriangularmesh.h"

/**
 * \ingroup core
 * Implementation of threaded rendering for mesh layers.
 *
 * \since QGIS 3.2
 * \note not available in Python bindings
 */
class QgsMeshLayerRenderer : public QgsMapLayerRenderer
{
  public:
    //! Ctor
    QgsMeshLayerRenderer( QgsMeshLayer *layer, QgsRenderContext &context );

    ~QgsMeshLayerRenderer() override = default;
    bool render() override;

  private:
    void renderMesh( const std::unique_ptr<QgsSymbol> &symbol, const QVector<QgsMeshFace> &faces );


  protected:
    // copy from mesh layer
    QgsMesh mNativeMesh;

    // copy from mesh layer
    QgsTriangularMesh mTriangularMesh;

    // copy from mesh layer
    std::unique_ptr<QgsSymbol> mNativeMeshSymbol = nullptr;

    // copy from mesh layer
    std::unique_ptr<QgsSymbol> mTriangularMeshSymbol = nullptr;

    // rendering context
    QgsRenderContext &mContext;
};


#endif // QGSMESHLAYERRENDERER_H
