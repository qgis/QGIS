/***************************************************************************
  qgsdepthrenderview.h
  --------------------------------------
  Date                 : June 2024
  Copyright            : (C) 2024 by Benoit De Mezzo and (C) 2020 by Belgacem Nedjima
  Email                : benoit dot de dot mezzo at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDEPTHRENDERVIEW_H
#define QGSDEPTHRENDERVIEW_H

#include "qgsabstractrenderview.h"

namespace Qt3DRender
{
  class QCamera;
  class QCameraSelector;
  class QLayer;
  class QRenderCapture;
  class QRenderTarget;
  class QTexture2D;
} //namespace Qt3DRender

namespace Qt3DCore
{
  class QEntity;
} //namespace Qt3DCore

#define SIP_NO_FILE

/**
 * \ingroup qgis_3d
 * \brief Container class that holds different objects related to depth rendering
 *
 * \note Not available in Python bindings
 *
 * The depth buffer render pass is made to copy the depth buffer into
 * an RGB texture that can be captured into a QImage and sent to the CPU for
 * calculating real 3D points from mouse coordinates (for zoom, rotation, drag..)
 *
 * \since QGIS 3.44
 */
class QgsDepthRenderView : public QgsAbstractRenderView
{
  public:
    //! Constructor
    QgsDepthRenderView( const QString &viewName, QSize size, Qt3DRender::QTexture2D *forwardDepthTexture, Qt3DCore::QEntity *rootSceneEntity );

    //! Returns the render capture object used to take an image of the depth buffer of the scene
    Qt3DRender::QRenderCapture *renderCapture() { return mDepthRenderCapture; }

    virtual void updateWindowResize( int width, int height ) override;

  private:
    Qt3DRender::QRenderCapture *mDepthRenderCapture = nullptr;
    Qt3DRender::QLayer *mLayer = nullptr;
    Qt3DRender::QTexture2D *mColorTexture = nullptr;

    Qt3DRender::QRenderTarget *buildTextures( QSize size );
    void buildRenderPass( QSize size, Qt3DRender::QTexture2D *forwardDepthTexture, Qt3DCore::QEntity *rootSceneEntity );
};

#endif // QGSDEPTHRENDERVIEW_H
