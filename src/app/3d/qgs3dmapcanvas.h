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

#include <QWidget>
#include <Qt3DRender/QRenderCapture>
#include <QSplitter>

#include "qgsrange.h"
#include "qgscameracontroller.h"

namespace Qt3DExtras
{
  class Qt3DWindow;
}

namespace Qt3DLogic
{
  class QFrameAction;
}

class Qgs3DMapSettings;
class Qgs3DMapScene;
class Qgs3DMapTool;
class QgsWindow3DEngine;
class QgsPointXY;
class Qgs3DNavigationWidget;
class QgsTemporalController;


class Qgs3DMapCanvas : public QWidget
{
    Q_OBJECT
  public:
    Qgs3DMapCanvas( QWidget *parent = nullptr );
    ~Qgs3DMapCanvas() override;

    //! Configure map scene being displayed. Takes ownership.
    void setMap( Qgs3DMapSettings *map );

    //! Returns access to the 3D scene configuration
    Qgs3DMapSettings *map() { return mMap; }

    //! Returns access to the 3D scene (root 3D entity)
    Qgs3DMapScene *scene() { return mScene; }

    //! Returns access to the view's camera controller. Returns NULLPTR if the scene has not been initialized yet with setMap()
    QgsCameraController *cameraController();

    //! Resets camera position to the default: looking down at the origin of world coordinates
    void resetView( bool resetExtent = false );

    //! Sets camera position to look down at the given point (in map coordinates) in given distance from plane with zero elevation
    void setViewFromTop( const QgsPointXY &center, float distance, float rotation = 0 );

    //! Saves the current scene as an image
    void saveAsImage( QString fileName, QString fileFormat );

    /**
     * Sets the active map tool that will receive events from the 3D canvas. Does not transfer ownership.
     * If the tool is NULLPTR, events will be used for camera manipulation.
     */
    void setMapTool( Qgs3DMapTool *tool );

    /**
     * Returns the active map tool that will receive events from the 3D canvas.
     * If the tool is NULLPTR, events will be used for camera manipulation.
     */
    Qgs3DMapTool *mapTool() const { return mMapTool; }

    /**
     * Sets the visibility of on-screen navigation widget.
     */
    void setOnScreenNavigationVisibility( bool visibility );

    /**
     * Sets the temporal controller
     */
    void setTemporalController( QgsTemporalController *temporalController );

    /**
     * Returns the size of the 3D canvas window
     *
     * \since QGIS 3.18
     */
    QSize windowSize() const;

  signals:
    //! Emitted when the 3D map canvas was successfully saved as image
    void savedAsImage( const QString &fileName );

    //! Emitted when the the map setting is changed
    void mapSettingsChanged();

    //! Emitted when the FPS count changes (at most every frame)
    void fpsCountChanged( float fpsCount );
    //! Emitted when the FPS counter is enabled or disabeld
    void fpsCounterEnabledChanged( bool enabled );

    /**
     * Emitted when the camera navigation \a speed is changed.
     *
     * \since QGIS 3.18
     */
    void cameraNavigationSpeedChanged( double speed );
  public slots:
    void captureDepthBuffer();

  private slots:
    void updateTemporalRange( const QgsDateTimeRange &timeRange );
    void onNavigationModeHotKeyPressed( QgsCameraController::NavigationMode mode );

  protected:
    void resizeEvent( QResizeEvent *ev ) override;
    bool eventFilter( QObject *watched, QEvent *event ) override;

  private:
    QgsWindow3DEngine *mEngine = nullptr;

    //! Container QWidget that encapsulates mWindow3D so we can use it embedded in ordinary widgets app
    QWidget *mContainer = nullptr;
    //! Description of the 3D scene
    Qgs3DMapSettings *mMap = nullptr;
    //! Root entity of the 3D scene
    Qgs3DMapScene *mScene = nullptr;

    //! Active map tool that receives events (if NULLPTR then mouse/keyboard events are used for camera manipulation)
    Qgs3DMapTool *mMapTool = nullptr;

    QString mCaptureFileName;
    QString mCaptureFileFormat;

    //! On-Screen Navigation widget.
    Qgs3DNavigationWidget *mNavigationWidget = nullptr;

    QgsTemporalController *mTemporalController = nullptr;

    QSplitter *mSplitter = nullptr;
};

#endif // QGS3DMAPCANVAS_H
