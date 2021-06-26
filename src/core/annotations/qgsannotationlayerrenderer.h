/***************************************************************************
    qgsannotationlayerrenderer.h
    ----------------
    copyright            : (C) 2019 by Sandro Mani
    email                : smani at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSANNOTATIONLAYERRENDERER_H
#define QGSANNOTATIONLAYERRENDERER_H

#define SIP_NO_FILE

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsmaplayerrenderer.h"
#include "qgsannotationitem.h"

class QgsAnnotationLayer;

/**
 * \ingroup core
 * Implementation of threaded rendering for annotation layers.
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
    QVector< QgsAnnotationItem *> mItems;
    std::unique_ptr< QgsFeedback > mFeedback;
    double mLayerOpacity = 1.0;

};

#endif // QGSANNOTATIONLAYERRENDERER_H
