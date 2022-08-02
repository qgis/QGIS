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
  class QRenderTargetOutput;
}

namespace Qt3DExtras
{
  class Qt3DWindow;
}

class Qgs3DMapSettings;
class QgsDirectionalLightSettings;

#define SIP_NO_FILE

/**
 * \ingroup 3d
 * \brief Container class that holds different objects related to shadow rendering
 *
 * \note Not available in Python bindings
 *
 * \since QGIS 3.40
 */
class QgsShadowRenderView : public QgsAbstractRenderView
{
    Q_OBJECT
  public:
    QgsShadowRenderView( QObject *parent, const QString &viewName );

    //! Enable or disable via \a enable the renderview sub tree
    virtual void setEnabled( bool enable ) override;

    //! Returns the shadow bias value
    float shadowBias() const { return mBias; }
    //! Sets the shadow bias value
    void setShadowBias( float bias );

    //! Returns the light camera
    Qt3DRender::QCamera *lightCamera() { return mLightCamera; }

    //! Sets shadow rendering to use a directional light
    void setupDirectionalLight( const QgsDirectionalLightSettings &light, float maximumShadowRenderingDistance,
                                const Qt3DRender::QCamera *mainCamera );

  signals:

    void shadowDirectionLightUpdated( const QVector3D &lightPosition, const QVector3D &lightDirection );
    void shadowExtentChanged( float minX, float maxX, float minY, float maxY, float minZ, float maxZ );
    void shadowBiasChanged( float bias );
    void shadowRenderingEnabled( bool isEnabled );

  private:
    float mBias = 0.00001f;

    // Shadow rendering pass branch nodes:
    Qt3DRender::QCameraSelector *mLightCameraSelector = nullptr;
    Qt3DRender::QLayerFilter *mLayerFilter = nullptr;
    Qt3DRender::QClearBuffers *mClearBuffers = nullptr;
    Qt3DRender::QRenderStateSet *mRenderStateSet = nullptr;

    Qt3DRender::QCamera *mLightCamera = nullptr;

    Qt3DRender::QFrameGraphNode *buildRenderPass();

    static void calculateViewExtent( const Qt3DRender::QCamera *camera, float shadowRenderingDistance, float y, float &minX, float &maxX, float &minY, float &maxY, float &minZ, float &maxZ );

};

#endif // QGSSHADOWRENDERVIEW_H
