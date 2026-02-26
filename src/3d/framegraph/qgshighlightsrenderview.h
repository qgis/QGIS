/***************************************************************************
    qgshighlightsrenderview.h
    ---------------------
    begin                : December 2025
    copyright            : (C) 2025 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSHIGHLIGHTSRENDERVIEW_H
#define QGSHIGHLIGHTSRENDERVIEW_H

#include "qgsabstractrenderview.h"

#define SIP_NO_FILE

namespace Qt3DRender
{
  class QCamera;
  class QCameraSelector;
  class QLayer;
  class QLayerFilter;
  class QRenderTarget;
  class QRenderTargetSelector;
  class QViewport;
} // namespace Qt3DRender


/**
 * \ingroup qgis_3d
 * \brief Container class that holds different objects related to highlighting identified features
 * \note Not available in Python bindings
 *
 * \since QGIS 4.0
 */
class QgsHighlightsRenderView : public QgsAbstractRenderView
{
  public:
    //! Constructor
    QgsHighlightsRenderView( const QString &viewName, Qt3DRender::QRenderTarget *target, Qt3DRender::QCamera *camera );

    void updateWindowResize( int width, int height ) override;

    //! Returns a layer that should be attached to entities meant to be rendered by QgsHighlightsRenderView
    Qt3DRender::QLayer *highlightsLayer() { return mHighlightsLayer; }

    //! Returns the width of the generated silhouette effect in pixels
    static int silhouetteWidth() { return 3; }

  private:
    /**
     * Builds the two passes needed for highlighting:
     * one for semi transparent highlights while writing to the stencil buffer,
     * one for the silhouettes while reading the stencil buffer and rendering outside of it
     */
    void buildRenderPasses();

    void updateViewportSizes( int width, int height );

    Qt3DRender::QRenderTarget *mRenderTarget = nullptr;
    Qt3DRender::QCamera *mMainCamera = nullptr;
    //! Four viewports displaced by pixel offset, for rendering the silhouette
    Qt3DRender::QViewport *mViewportUp = nullptr;
    Qt3DRender::QViewport *mViewportDown = nullptr;
    Qt3DRender::QViewport *mViewportLeft = nullptr;
    Qt3DRender::QViewport *mViewportRight = nullptr;

    Qt3DRender::QLayer *mHighlightsLayer = nullptr;
};

#endif // QGSHIGHLIGHTSRENDERVIEW_H
