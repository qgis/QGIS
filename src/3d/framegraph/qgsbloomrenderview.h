/***************************************************************************
  qgsbloomrenderview.h
  --------------------------------------
  Date                 : May 2026
  Copyright            : (C) 2026 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSBLOOMRENDERVIEW_H
#define QGSBLOOMRENDERVIEW_H

#include "qgsabstractrenderview.h"

#include <QSize>

#define SIP_NO_FILE

class QgsRenderPassQuad;

namespace Qt3DCore
{
  class QEntity;
} //namespace Qt3DCore

namespace Qt3DRender
{
  class QTexture2D;
  class QRenderStateSet;
  class QParameter;
} //namespace Qt3DRender

class QgsBloomDownsampleEntity;
class QgsBloomUpsampleEntity;

/**
 * \ingroup qgis_3d
 * \brief Container class that holds different objects related to bloom rendering.
 *
 * This render view implements a HDR bloom effect, following the CoD: Advanced Warfare
 * approach, from ACM Siggraph 2014.
 *
 * This technique mimics the physical scattering of light inside a camera lens using
 * a dual-filtering mip-chain. It operates in two phases:
 *
 * - Downsampling: Progressively scales down the HDR scene texture across
 *   several mip levels using a specialized 13-tap filter. This filter
 *   prevents temporal aliasing (flickering) of bright, sub-pixel highlights.
 * - Upsampling: Progressively reconstructs the image back up the mip-chain
 *   using a 9-tap tent filter, additively blending each level together.
 *
 * \note Not available in Python bindings
 *
 * \since QGIS 4.2
 */
class QgsBloomRenderView : public QgsAbstractRenderView
{
  public:
    //! Default constructor
    QgsBloomRenderView( const QString &viewName, Qt3DRender::QTexture2D *sourceColorTexture, const QSize &size, Qt3DCore::QEntity *rootSceneEntity );
    ~QgsBloomRenderView() override;

    /**
     * Returns the texture containing the final bloom effect.
     */
    Qt3DRender::QTexture2D *bloomTexture() const;

    /**
     * Sets the bloom filter \a radius.
     *
     * The default radius is 0.005.
     */
    void setFilterRadius( float radius );

    /**
     * Sets the aspect \a ratio of the view.
     */
    void setAspectRatio( float ratio );

    void updateWindowResize( int width, int height ) override;
    void setEnabled( bool enabled ) override;

  private:
    void buildRenderPasses( Qt3DRender::QTexture2D *sourceTexture, Qt3DCore::QEntity *rootEntity );

    // from https://learnopengl.com/Guest-Articles/2022/Phys.-Based-Bloom
    // "You may go as small as you wish, though 5 or 6 is usually good enough" (Jorge Jimenez)
    static const int MIP_PASSES = 5;

    QSize mBaseSize;

    std::vector<Qt3DRender::QTexture2D *> mTextures;

    Qt3DRender::QParameter *mFilterRadiusParameter = nullptr;
    Qt3DRender::QParameter *mAspectRatioParameter = nullptr;
};

#endif // QGSBLOOMRENDERVIEW_H
