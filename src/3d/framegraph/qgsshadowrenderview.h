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

#include "qgs3d.h"
#include "qgsabstractrenderview.h"

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
  class QTexture2DArray;
} // namespace Qt3DRender

namespace Qt3DExtras
{
  class Qt3DWindow;
}

class QgsShadowSettings;
class QgsDirectionalLightSettings;
class QgsLightSource;


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
    QgsShadowRenderView( const QString &viewName, Qt3DCore::QEntity *rootEntity );

    //! Enable or disable via \a enable the renderview sub tree
    void setEnabled( bool enable ) override;

    //! Returns the light camera with the specified \a index.
    Qt3DRender::QCamera *lightCamera( int index );

    //! Returns the shadow depth texture array
    Qt3DRender::QTexture2DArray *mapTextureArray() const;

    //! Returns the layer to be used by entities to be included in this renderview
    Qt3DRender::QLayer *entityCastingShadowsLayer() const;

    //! Update shadow depth texture size
    void setMapSize( int width, int height );

  private:
    static constexpr int mDefaultMapResolution = 2048;
    Qt3DCore::QEntity *mRootEntity = nullptr;

    // Shadow rendering pass branch nodes:
    Qt3DRender::QLayer *mEntityCastingShadowsLayer = nullptr;
    Qt3DRender::QLayerFilter *mLayerFilters[Qgs3D::NUM_SHADOW_CASCADES] = { nullptr };
    Qt3DRender::QCamera *mLightCameras[Qgs3D::NUM_SHADOW_CASCADES] = { nullptr };
    Qt3DRender::QTexture2DArray *mMapTextureArray = nullptr;

    void buildRenderPass();
};

#endif // QGSSHADOWRENDERVIEW_H
