#ifndef QGSABSTRACT3DENGINE_H
#define QGSABSTRACT3DENGINE_H

#include "qgis_3d.h"

#include <QObject>

class QColor;
class QRect;

namespace Qt3DCore
{
  class QEntity;
}

namespace Qt3DRender
{
  class QRenderSettings;
  class QCamera;
}

/**
 * Base class for 3D engine implementation. A 3D engine is responsible for setting up
 * rendering with Qt3D. This means mainly:
 * - creating Qt3D aspect engine and registering rendering aspect
 * - setting up a camera, render settings and frame graph
 *
 * We have two implementations:
 * - QgsWindow3DEngine - used for rendering on display (has a QWindow that can be embedded into QWidget)
 * - QgsOffscreen3DEngine - renders scene to images
 *
 * \since QGIS 3.4
 */
class _3D_EXPORT QgsAbstract3DEngine : public QObject
{
    Q_OBJECT
  public:

    virtual void setClearColor( const QColor &color ) = 0;
    virtual void setFrustumCullingEnabled( bool enabled ) = 0;
    virtual void setRootEntity( Qt3DCore::QEntity *root ) = 0;

    virtual Qt3DRender::QRenderSettings *renderSettings() = 0;
    virtual Qt3DRender::QCamera *camera() = 0;
    virtual QSize size() const = 0;

  signals:
    void imageCaptured( const QImage &image );
};


#endif // QGSABSTRACT3DENGINE_H
