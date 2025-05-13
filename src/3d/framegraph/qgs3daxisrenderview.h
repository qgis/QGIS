/***************************************************************************
  qgs3daxisrenderview.h
  --------------------------------------
  Date                 : June 2024
  Copyright            : (C) 2024 by Benoit De Mezzo
  Email                : benoit dot de dot mezzo at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DAXISRENDERVIEW_H
#define QGS3DAXISRENDERVIEW_H

#include "qgis_3d.h"
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
  class QRenderTargetSelector;
} // namespace Qt3DRender

class Qgs3DMapCanvas;
class QgsCameraController;

class QgsFrameGraph;
class Qgs3DMapSettings;
class Qgs3DAxis;

#define SIP_NO_FILE

/**
 * \ingroup qgis_3d
 * \brief 3D axis render view.
 *
 * \note Not available in Python bindings
 * \since QGIS 3.44
 */
class _3D_EXPORT Qgs3DAxisRenderView : public QgsAbstractRenderView
{
  public:
    /**
     * Constructor for Qgs3DAxisRenderView with the specified \a parent object.
     */
    Qgs3DAxisRenderView( const QString &viewName, Qgs3DMapCanvas *canvas,             //
                         QgsCameraController *cameraCtrl, Qgs3DMapSettings *settings, //
                         Qgs3DAxis *axis3D );

    //! Returns the viewport associated to this renderview
    Qt3DRender::QViewport *viewport() const;

    //! Returns the layer to be used by entities to be included in the label renderpass
    Qt3DRender::QLayer *labelLayer() const;

    //! Returns main object layer
    Qt3DRender::QLayer *objectLayer() const;

    //! Returns main object camera (used for axis or cube)
    Qt3DRender::QCamera *objectCamera() const;

    //! Returns camera used for billboarded labels
    Qt3DRender::QCamera *labelCamera() const;

    virtual void updateWindowResize( int width, int height ) override;

    //! Updates viewport horizontal \a position
    void onHorizontalPositionChanged( Qt::AnchorPoint position );

    //! Updates viewport vertical \a position
    void onVerticalPositionChanged( Qt::AnchorPoint position );

    //! Updates viewport \a size. Uses canvas size by default.
    void onViewportSizeUpdate( int width = -1, int height = -1 );

  private:
    Qgs3DMapCanvas *mCanvas;
    Qt3DRender::QCamera *mObjectCamera;
    Qt3DRender::QCamera *mLabelCamera;
    Qt3DRender::QLayer *mObjectLayer = nullptr;
    Qt3DRender::QLayer *mLabelLayer = nullptr;
    Qt3DRender::QViewport *mViewport = nullptr;
    Qgs3DMapSettings *mMapSettings = nullptr;
    Qgs3DAxis *m3DAxis;
};


#endif // QGS3DAXISRENDERVIEW_H
