/***************************************************************************
  qgs3dwindow.h
  --------------------------------------
  Date                 : May 2023
  Copyright            : (C) 2023 by Jean-Baptiste Peter
  Email                : jbpeter at outlook dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DWINDOW_H
#define QGS3DWINDOW_H

#include <QtGui/QWindow>
#include <Qt3DRender/qrenderapi.h>


namespace Qt3DCore
{
  class QAspectEngine;
  class QAbstractAspect;
  class QEntity;
}

namespace Qt3DRender
{
  class QCamera;
  class QFrameGraphNode;
  class QRenderAspect;
  class QRenderSettings;
}

namespace Qt3DExtras
{
  class QForwardRenderer;
}

namespace Qt3DInput
{
  class QInputAspect;
  class QInputSettings;
}

namespace Qt3DLogic
{
  class QLogicAspect;
}

class Qgs3DWindow : public QWindow
{
    Q_OBJECT
  public:
    /**
     * Constructor for Qgs3DWindow.
     */
    Qgs3DWindow( QScreen *screen = nullptr, Qt3DRender::API = Qt3DRender::API::OpenGL );

    /**
     * Destructor for Qgs3DWindow.
     */
    ~Qgs3DWindow();

    /**
     * Registers the specified aspect.
     */
    void registerAspect( Qt3DCore::QAbstractAspect *aspect );

    /**
     * Registers the specified aspect name.
     */
    void registerAspect( const QString &name );

    /**
     * Sets the specified root entity of the scene.
     */
    void setRootEntity( Qt3DCore::QEntity *root );

    /**
     * Activates the specified activeFrameGraph.
     */
    void setActiveFrameGraph( Qt3DRender::QFrameGraphNode *activeFrameGraph );

    /**
     * Returns the node of the active frame graph.
     */
    Qt3DRender::QFrameGraphNode *activeFrameGraph() const;

    /**
     * Returns the node of the default framegraph
     */
    Qt3DExtras::QForwardRenderer *defaultFrameGraph() const;

    /**
     * Returns the default camera of the 3D Window.
     */
    Qt3DRender::QCamera *camera() const;

    /**
     * Returns the render settings of the 3D Window.
     */
    Qt3DRender::QRenderSettings *renderSettings() const;


  protected:
    /**
     * Manages the display events specified in e.
     */
    void showEvent( QShowEvent *e ) override;

    /**
     * Resets the aspect ratio of the 3D window.
     */
    void resizeEvent( QResizeEvent * ) override;

  private:
    Qt3DCore::QAspectEngine *m_aspectEngine;

    // Aspects
    Qt3DRender::QRenderAspect *m_renderAspect;
    Qt3DInput::QInputAspect *m_inputAspect;
    Qt3DLogic::QLogicAspect *m_logicAspect;

    // Renderer configuration
    Qt3DRender::QRenderSettings *m_renderSettings;
    Qt3DExtras::QForwardRenderer *m_forwardRenderer;
    Qt3DRender::QCamera *m_defaultCamera;

    // Input configuration
    Qt3DInput::QInputSettings *m_inputSettings;

    // Scene
    Qt3DCore::QEntity *m_root;
    Qt3DCore::QEntity *m_userRoot;

    bool m_initialized;
};

#endif //QGS3DWINDOW_H
