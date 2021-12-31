/***************************************************************************
    qgsgrouplayerrenderer.h
    ----------------
  Date                 : September 2021
  Copyright            : (C) 2021 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGROUPLAYERRENDERER_H
#define QGSGROUPLAYERRENDERER_H

#define SIP_NO_FILE

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsmaplayerrenderer.h"
#include <QPainter>
#include <tuple>
#include <vector>
#include <memory>

class QgsGroupLayer;
class QgsFeedback;
class QgsPaintEffect;
class QgsCoordinateTransform;

/**
 * \ingroup core
 * \brief Implementation of threaded rendering for group layers.
 *
 * \note not available in Python bindings
 * \since QGIS 3.24
 */
class CORE_EXPORT QgsGroupLayerRenderer : public QgsMapLayerRenderer
{
  public:

    /**
     * Constructor for a QgsGroupLayerRenderer, for the specified \a layer.
     */
    QgsGroupLayerRenderer( QgsGroupLayer *layer, QgsRenderContext &context );
    ~QgsGroupLayerRenderer() override;
    QgsFeedback *feedback() const override;
    bool render() override;
    bool forceRasterRender() const override;

  private:
    std::unique_ptr< QgsFeedback > mFeedback;
    bool mForceRasterRender = false;
    std::vector< std::unique_ptr< QgsMapLayerRenderer > > mChildRenderers;
    std::vector< QPainter::CompositionMode > mRendererCompositionModes;
    std::vector< double > mRendererOpacity;
    std::vector< QgsCoordinateTransform > mTransforms;
    double mLayerOpacity = 1.0;
    std::unique_ptr< QgsPaintEffect > mPaintEffect;

};

#endif // QGSGROUPLAYERRENDERER_H
