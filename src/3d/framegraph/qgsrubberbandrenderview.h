/***************************************************************************
  qgsrubberbandrenderview.h
  --------------------------------------
  Date                 : June 2026
  Copyright            : (C) 2026 by Benoit De Mezzo and (C) 2020 by Belgacem Nedjima
  Email                : benoit dot de dot mezzo at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRUBBERBANDRENDERVIEW_H
#define QGSRUBBERBANDRENDERVIEW_H

#include "qgsabstractrenderview.h"

#define SIP_NO_FILE

namespace Qt3DRender
{
  class QLayer;
  class QCamera;
  class QTexture2D;
  class QRenderTargetSelector;
  class QRenderTarget;
} // namespace Qt3DRender

namespace Qt3DCore
{
  class QEntity;
} // namespace Qt3DCore

/**
 * \ingroup qgis_3d
 * \brief Container class that holds different objects related to rubberband rendering
 *
 * \note Not available in Python bindings
 *
 * The rubberband buffer render pass is made to copy the rubberband buffer into
 * an RGB texture that can be captured into a QImage and sent to the CPU for
 * calculating real 3D points from mouse coordinates (for zoom, rotation, drag..)
 *
 * \since QGIS 4.2
 */
class QgsRubberBandRenderView : public QgsAbstractRenderView
{
  public:
    QgsRubberBandRenderView( const QString &viewName, Qt3DRender::QCamera *mainCamera, Qt3DCore::QEntity *rootSceneEntity, Qt3DRender::QRenderTarget *fwdRenderTarget );

    //! Returns entity for all rubber bands (to show them always on top)
    Qt3DCore::QEntity *rubberBandEntity() const;

    //! Returns rubberband tard selector
    Qt3DRender::QRenderTargetSelector *renderTargetSelector() const;

  private:
    Qt3DRender::QLayer *mLayer = nullptr;
    Qt3DRender::QCamera *mMainCamera = nullptr;
    Qt3DRender::QRenderTargetSelector *mRenderTargetSelector = nullptr;
    Qt3DCore::QEntity *mRubberBandsRootEntity = nullptr;

    void constructRenderPass( Qt3DRender::QRenderTarget *fwdRenderTarget ); //#spellok
};

#endif // QGSRUBBERBANDRENDERVIEW_H
