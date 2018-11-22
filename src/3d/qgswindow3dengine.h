/***************************************************************************
  qgswindow3dengine.h
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

#ifndef QGSWINDOW3DENGINE_H
#define QGSWINDOW3DENGINE_H

#include "qgsabstract3dengine.h"


namespace Qt3DRender
{
  class QRenderCapture;
}

namespace Qt3DExtras
{
  class Qt3DWindow;
}

class QWindow;


/**
 * \ingroup 3d
 * On-screen 3D engine: it creates OpenGL window (QWindow) and displays rendered 3D scene there.
 * The window can be embedded into a QWidget-based application with QWidget::createWindowContainer().
 *
 * \since QGIS 3.4
 */
class _3D_EXPORT QgsWindow3DEngine : public QgsAbstract3DEngine
{
    Q_OBJECT
  public:
    QgsWindow3DEngine();

    //! Returns the internal 3D window where all the rendered output is displayed
    QWindow *window();

    void requestCaptureImage() override;

    void setClearColor( const QColor &color ) override;
    void setFrustumCullingEnabled( bool enabled ) override;
    void setRootEntity( Qt3DCore::QEntity *root ) override;

    Qt3DRender::QRenderSettings *renderSettings() override;
    Qt3DRender::QCamera *camera() override;
    QSize size() const override;

  private:
    //! 3D window with all the 3D magic inside
    Qt3DExtras::Qt3DWindow *mWindow3D = nullptr;
    //! Frame graph node for render capture
    Qt3DRender::QRenderCapture *mCapture = nullptr;
};


#endif // QGSWINDOW3DENGINE_H
