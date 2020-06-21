/***************************************************************************
  qgsabstract3dengine.h
  --------------------------------------
  Date                 : July 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSABSTRACT3DENGINE_H
#define QGSABSTRACT3DENGINE_H

#include "qgis_3d.h"

#include <QObject>

#define SIP_NO_FILE

class QColor;
class QRect;
class QSurface;

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
 * \ingroup 3d
 * Base class for 3D engine implementation. A 3D engine is responsible for setting up
 * rendering with Qt3D. This means mainly:
 *
 * - creating Qt3D aspect engine and registering rendering aspect
 * - setting up a camera, render settings and frame graph
 *
 * We have two implementations:
 *
 * - QgsWindow3DEngine - used for rendering on display (has a QWindow that can be embedded into QWidget)
 * - QgsOffscreen3DEngine - renders scene to images
 *
 * \note Not available in Python bindings
 * \since QGIS 3.4
 */
class _3D_EXPORT QgsAbstract3DEngine : public QObject
{
    Q_OBJECT
  public:

    //! Sets background color of the scene
    virtual void setClearColor( const QColor &color ) = 0;
    //! Sets whether frustum culling is enabled (this should make rendering faster by not rendering entities outside of camera's view)
    virtual void setFrustumCullingEnabled( bool enabled ) = 0;
    //! Sets root entity of the 3D scene
    virtual void setRootEntity( Qt3DCore::QEntity *root ) = 0;

    //! Returns access to the engine's render settings (the frame graph can be accessed from here)
    virtual Qt3DRender::QRenderSettings *renderSettings() = 0;
    //! Returns pointer to the engine's camera entity
    virtual Qt3DRender::QCamera *camera() = 0;
    //! Returns size of the engine's rendering area in pixels
    virtual QSize size() const = 0;

    /**
     * Starts a request for an image rendered by the engine.
     * The function does not block - when the rendered image is captured, it is returned in imageCaptured() signal.
     * Only one image request can be active at a time.
     */
    virtual void requestCaptureImage() = 0;

    /**
     * Returns the surface of the engine
     *
     * \since QGIS 3.14
     */
    virtual QSurface *surface() const = 0;

  signals:
    //! Emitted after a call to requestCaptureImage() to return the captured image.
    void imageCaptured( const QImage &image );
};


#endif // QGSABSTRACT3DENGINE_H
