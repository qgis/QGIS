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
#include "qgsshadowrenderingframegraph.h"
#include "qgspostprocessingentity.h"
#include "qgspreviewquad.h"

namespace Qt3DRender
{
  class QRenderCapture;
}

namespace Qt3DExtras
{
  class Qt3DWindow;
  class QForwardRenderer;
}

class QWindow;


#define SIP_NO_FILE

/**
 * \ingroup 3d
 * \brief On-screen 3D engine: it creates OpenGL window (QWindow) and displays rendered 3D scene there.
 * The window can be embedded into a QWidget-based application with QWidget::createWindowContainer().
 *
 * \note Not available in Python bindings
 *
 * \since QGIS 3.4
 */
class _3D_EXPORT QgsWindow3DEngine : public QgsAbstract3DEngine
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsWindow3DEngine with the specified \a parent object.
     */
    QgsWindow3DEngine( QObject *parent = nullptr );

    //! Returns the internal 3D window where all the rendered output is displayed
    QWindow *window();

    //! Returns the root entity
    Qt3DCore::QEntity *root() const;

    //! Sets whether shadow rendering is enabled
    void setShadowRenderingEnabled( bool enabled );
    //! Returns whether shadow rendering is enabled
    bool shadowRenderingEnabled() { return mShadowRenderingEnabled; }

    void setClearColor( const QColor &color ) override;
    void setFrustumCullingEnabled( bool enabled ) override;
    void setRootEntity( Qt3DCore::QEntity *root ) override;

    Qt3DRender::QRenderSettings *renderSettings() override;
    Qt3DRender::QCamera *camera() override;
    QSize size() const override;
    QSurface *surface() const override;

    void setSize( QSize s ) override;
  private:
    //! 3D window with all the 3D magic inside
    Qt3DExtras::Qt3DWindow *mWindow3D = nullptr;
    //! Frame graph node for render capture
    bool mShadowRenderingEnabled = false;
    Qt3DCore::QEntity *mRoot = nullptr;
    Qt3DCore::QEntity *mSceneRoot = nullptr;

    QgsPreviewQuad *mPreviewQuad = nullptr;
    QSize mSize = QSize( 1024, 768 );
};

#endif // QGSWINDOW3DENGINE_H
