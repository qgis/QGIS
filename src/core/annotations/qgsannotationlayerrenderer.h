/***************************************************************************
    qgsannotationlayerrenderer.h
    ----------------
    copyright            : (C) 2019 by Sandro Mani
    email                : smani at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSANNOTATIONLAYERRENDERER_H
#define QGSANNOTATIONLAYERRENDERER_H

#define SIP_NO_FILE

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsmaplayerrenderer.h"
#include "qgsannotationitem.h"
#include <tuple>
#include <vector>
#include <memory>

class QgsAnnotationLayer;
class QgsPaintEffect;

/**
 * \ingroup core
 * \brief Implementation of threaded rendering for annotation layers.
 *
 * \note not available in Python bindings
 * \since QGIS 3.16
 */
class CORE_EXPORT QgsAnnotationLayerRenderer : public QgsMapLayerRenderer
{
  public:

    /**
     * Constructor for a QgsAnnotationLayerRenderer, for the specified \a layer.
     */
    QgsAnnotationLayerRenderer( QgsAnnotationLayer *layer, QgsRenderContext &context );
    ~QgsAnnotationLayerRenderer() override;
    QgsFeedback *feedback() const override;
    bool render() override;
    bool forceRasterRender() const override;

  private:
    std::vector < std::pair< QString, std::unique_ptr< QgsAnnotationItem > > > mItems;
    std::unique_ptr< QgsFeedback > mFeedback;
    double mLayerOpacity = 1.0;
    std::unique_ptr< QgsPaintEffect > mPaintEffect;

};

#endif // QGSANNOTATIONLAYERRENDERER_H
