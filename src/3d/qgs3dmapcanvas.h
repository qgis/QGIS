/***************************************************************************
  qgs3dmapcanvas.h
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DMAPCANVAS_H
#define QGS3DMAPCANVAS_H

#include "qgis_3d.h"

#include "qgis.h"
#include "qgsrange.h"

#include <QtGui/QWindow>

#ifndef SIP_RUN
namespace Qt3DCore
{
  class QAspectEngine;
  class QAbstractAspect;
  class QEntity;
} // namespace Qt3DCore

namespace Qt3DRender
{
  class QCamera;
  class QFrameGraphNode;
  class QRenderAspect;
  class QRenderSettings;
} // namespace Qt3DRender

namespace Qt3DExtras
{
  class QForwardRenderer;
}

namespace Qt3DInput
{
  class QInputAspect;
  class QInputSettings;
} // namespace Qt3DInput

namespace Qt3DLogic
{
  class QLogicAspect;
}
#endif

class QgsRectangle;
class QgsWindow3DEngine;
class Qgs3DMapTool;
class QgsPointXY;
class QgsCameraController;
class QgsTemporalController;
class Qgs3DMapScene;
class Qgs3DMapSettings;
class QgsFeature;
class QgsMapLayer;
class QgsRubberBand3D;


/**
 * \ingroup 3d
 * \brief Qgs3DMapCanvas is a convenience wrapper to simplify the creation of a 3D window ready to be used with QGIS.
 *
 * \note This is a port of qtwindow3d which does not set the default surface when initialized.
 * \note The default surface must be set before the construction of the QApplication when using shared OpenGL context.
 * \note This is required in order to use QT3d and QtWebEngine at the same time.
 *
 * \since QGIS 3.36
 */
class _3D_EXPORT Qgs3DMapCanvas : public QWindow
{
    Q_OBJECT
  public:
    Qgs3DMapCanvas();
    ~Qgs3DMapCanvas();

    //! Returns access to the 3D scene configuration
    Qgs3DMapSettings *mapSettings() { return mMapSettings; }

    //! Returns access to the 3D scene (root 3D entity)
    Qgs3DMapScene *scene() { return mScene; }

    //! Returns access to the view's camera controller. Returns NULLPTR if the scene has not been initialized yet with setMapSettings()
    QgsCameraController *cameraController();

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
     * Returns the default camera of the 3D Window.
     */
    Qt3DRender::QCamera *camera() const;

    /**
     * Returns the render settings of the 3D Window.
     */
    Qt3DRender::QRenderSettings *renderSettings() const;

    //! Configure map scene being displayed. Takes ownership.
    void setMapSettings( Qgs3DMapSettings *mapSettings );

    //! Resets camera position to the default: looking down at the origin of world coordinates
    void resetView();

    //! Sets camera position to look down at the given point (in map coordinates) in given distance from plane with zero elevation
    void setViewFromTop( const QgsPointXY &center, float distance, float rotation = 0 );

    //! Saves the current scene as an image
    void saveAsImage( const QString &fileName, const QString &fileFormat );

    /**
     * Sets the active map \a tool that will receive events from the 3D canvas. Does not transfer ownership.
     * If the tool is NULLPTR, events will be used for camera manipulation.
     */
    void setMapTool( Qgs3DMapTool *tool );

    /**
     * Returns the active map tool that will receive events from the 3D canvas.
     * If the tool is NULLPTR, events will be used for camera manipulation.
     */
    Qgs3DMapTool *mapTool() const { return mMapTool; }

    /**
     * Returns the 3D engine.
     */
    QgsWindow3DEngine *engine() const { return mEngine; }

    /**
     * Sets the temporal controller
     */
    void setTemporalController( QgsTemporalController *temporalController );

    /**
     * Resets camera view to show the \a extent (top view)
     */
    void setViewFrom2DExtent( const QgsRectangle &extent );

    /**
     * Calculates the 2D extent viewed by the 3D camera as the vertices of the viewed trapezoid
     */
    QVector<QgsPointXY> viewFrustum2DExtent();

    /**
     * Highlights a \a feature from \a layer using a QgsRubberBand3D
     * \note Currently only supports point cloud layers with features generated by QgsIdentifyResultsDialog
     */
    void highlightFeature( const QgsFeature &feature, QgsMapLayer *layer );

    /**
     * Clears all QgsRubberBand3D highlights
     */
    void clearHighlights();

  signals:
    //! Emitted when the 3D map canvas was successfully saved as image
    void savedAsImage( const QString &fileName );

    //! Emitted when the the map setting is changed
    void mapSettingsChanged();

    //! Emitted when the FPS count changes (at most every frame)
    void fpsCountChanged( float fpsCount );

    //! Emitted when the FPS counter is enabled or disabeld
    void fpsCounterEnabledChanged( bool enabled );

    //! Emitted when the viewed 2D extent seen by the 3D camera has changed
    void viewed2DExtentFrom3DChanged( QVector<QgsPointXY> extent );

    //! Emitted when the camera navigation \a speed is changed.
    void cameraNavigationSpeedChanged( double speed );

#endif

  private slots:
    void captureDepthBuffer();
    void updateTemporalRange( const QgsDateTimeRange &timeRange );
    void onNavigationModeChanged( Qgis::NavigationMode mode );
    void updateHighlightSizes();

  protected:
    /**
     * Manages the display events specified in e.
     */
    void showEvent( QShowEvent *e ) override;

    /**
     * Resets the aspect ratio of the 3D window.
     */
    void resizeEvent( QResizeEvent * ) override;

    bool eventFilter( QObject *watched, QEvent *event ) override;

  private:
    Qt3DCore::QAspectEngine *m_aspectEngine;

    // Aspects
    Qt3DRender::QRenderAspect *m_renderAspect;
    Qt3DInput::QInputAspect *m_inputAspect;
    Qt3DLogic::QLogicAspect *m_logicAspect;

    // Renderer configuration
    Qt3DRender::QRenderSettings *m_renderSettings;
    Qt3DRender::QCamera *m_defaultCamera;

    // Input configuration
    Qt3DInput::QInputSettings *m_inputSettings;

    // Scene
    Qt3DCore::QEntity *m_root;
    Qt3DCore::QEntity *m_userRoot;

    bool m_initialized;

    QgsWindow3DEngine *mEngine = nullptr;

    //! Description of the 3D scene
    Qgs3DMapSettings *mMapSettings = nullptr;
    //! Root entity of the 3D scene
    Qgs3DMapScene *mScene = nullptr;

    //! Active map tool that receives events (if NULLPTR then mouse/keyboard events are used for camera manipulation)
    Qgs3DMapTool *mMapTool = nullptr;

    QString mCaptureFileName;
    QString mCaptureFileFormat;

    QgsTemporalController *mTemporalController = nullptr;

    //! This holds and owns the rubber bands for highlighting identified features
    QMap<QgsMapLayer *, QgsRubberBand3D *> mHighlights;
};

#endif //QGS3DMAPCANVAS_H
