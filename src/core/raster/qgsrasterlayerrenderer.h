#ifndef QGSRASTERLAYERRENDERER_H
#define QGSRASTERLAYERRENDERER_H

#include "qgsmaplayerrenderer.h"

class QPainter;

class QgsMapToPixel;
class QgsRasterLayer;
class QgsRasterPipe;
struct QgsRasterViewPort;
class QgsRenderContext;

class QgsRasterLayerRenderer : public QgsMapLayerRenderer
{
  public:
    QgsRasterLayerRenderer( QgsRasterLayer* layer, QgsRenderContext& rendererContext );
    ~QgsRasterLayerRenderer();

    virtual bool render();

  protected:

    QPainter* mPainter;
    const QgsMapToPixel* mMapToPixel;
    QgsRasterViewPort* mRasterViewPort;

    QgsRasterPipe* mPipe;
};

#endif // QGSRASTERLAYERRENDERER_H
