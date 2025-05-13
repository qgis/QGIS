/***************************************************************************
  qgsshadowrenderview.h
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

#ifndef QGSSHADOWRENDERVIEW_H
#define QGSSHADOWRENDERVIEW_H

#include "qgsabstractrenderview.h"

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
  class QFrameGraphNode;
  class QLayer;
  class QViewport;
  class QSubtreeEnabler;
  class QTexture2D;
  class QCameraSelector;
  class QLayerFilter;
  class QRenderTargetSelector;
  class QClearBuffers;
  class QRenderStateSet;
  class QRenderTarget;
  class QRenderTargetOutput;
} // namespace Qt3DRender

namespace Qt3DExtras
{
  class Qt3DWindow;
}

class QgsShadowSettings;
class QgsDirectionalLightSettings;
class QgsLightSource;

#define SIP_NO_FILE

/**
 * \ingroup qgis_3d
 * \brief Container class that holds different objects related to shadow rendering
 *
 * \note Not available in Python bindings
 *
 * \since QGIS 3.44
 */
class QgsShadowRenderView : public QgsAbstractRenderView
{
  public:
    //! Default constructor
    QgsShadowRenderView( const QString &viewName );

    //! Enable or disable via \a enable the renderview sub tree
    virtual void setEnabled( bool enable ) override;

    //! Returns the light camera
    Qt3DRender::QCamera *lightCamera() { return mLightCamera; }

    //! Returns shadow depth texture
    Qt3DRender::QTexture2D *mapTexture() const;

    //! Returns the layer to be used by entities to be included in this renderview
    Qt3DRender::QLayer *entityCastingShadowsLayer() const;

    //! Update shadow depth texture size
    void setMapSize( int width, int height );

  private:
    static constexpr int mDefaultMapResolution = 2048;

    // Shadow rendering pass branch nodes:
    Qt3DRender::QLayer *mEntityCastingShadowsLayer = nullptr;
    Qt3DRender::QLayerFilter *mLayerFilter = nullptr;
    Qt3DRender::QCamera *mLightCamera = nullptr;
    Qt3DRender::QTexture2D *mMapTexture = nullptr;

    Qt3DRender::QRenderTarget *buildTextures();
    void buildRenderPass();
};

#endif // QGSSHADOWRENDERVIEW_H
