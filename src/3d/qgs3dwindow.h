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

#include "qgis_3d.h"

#include <QtGui/QWindow>

#ifndef SIP_RUN
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
#endif


/**
 * \ingroup 3d
 * \brief qgs3dWindow is a convenience wrapper to simplify the creation of a 3D window ready to be used with QGIS.
 *
 * \note This is a port of qtwindow3d which does not set the default surface when initialized.
 * \note The default surface must be set before the construction of the QApplication when using shared OpenGL context.
 * \note This is required in order to use QT3d and QtWebEngine at the same time.
 *
 * \since QGIS 3.36
 */
class _3D_EXPORT Qgs3DWindow : public QWindow
{
    Q_OBJECT
  public:

    /**
     * Constructor for Qgs3DWindow.
     */
    Qgs3DWindow();

    /**
     * Destructor for Qgs3DWindow.
     */
    ~Qgs3DWindow();

#ifndef SIP_RUN

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
#endif

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
