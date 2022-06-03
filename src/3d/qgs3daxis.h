/***************************************************************************
  qgs3daxis.h
  --------------------------------------
  Date                 : March 2022
  Copyright            : (C) 2022 by Jean Felder
  Email                : jean dot felder at oslandia dot com
  ***************************************************************************
  *                                                                         *
  *   This program is free software; you can redistribute it and/or modify  *
  *   it under the terms of the GNU General Public License as published by  *
  *   the Free Software Foundation; either version 2 of the License, or     *
  *   (at your option) any later version.                                   *
  *                                                                         *
  ***************************************************************************/

#ifndef QGS3DAXIS_H
#define QGS3DAXIS_H

#include "qgis_3d.h"

#include "qgscoordinatereferencesystem.h"
#include <Qt3DCore/QEntity>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DExtras/QText2DEntity>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QViewport>
#include <Qt3DRender/QPickEvent>
#include <Qt3DRender/QScreenRayCaster>
#include <QVector3D>
#include <QVector2D>

#include <Qt3DRender/QLayer>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QGeometryRenderer>

#include <QtWidgets/QMenu>

#define SIP_NO_FILE

class QgsCameraController;
class Qgs3DMapSettings;
class Qgs3DMapScene;

/**
 * \ingroup 3d
 * \brief Display 3D ortho axis in the main 3D view.
 *
 * Axis are displayed in a dedicated viewport which can be placed all around the main viewport.
 * Axis labels are displayed in a dedicated viewport with a specific camera to act as a billboarding layer.
 * Axis labels can be changed with the mode combo box from the navigation widget.

 * \note Not available in Python bindings
 *
 * \since QGIS 3.26
 */
class _3D_EXPORT Qgs3DAxis : public QObject
{
    Q_OBJECT
  public:

    /**
     * \brief Defaul Qgs3DAxis constructor
     * \param parentWindow qt3d windows
     * @param parent3DScene root entity to set as parent
     * @param mapScene 3d map scene to retrieve terrain and 3d engine data
     * @param camera camera controller used to track camera movements
     * @param map 3D map settings
     */
    Qgs3DAxis( Qt3DExtras::Qt3DWindow *parentWindow,  Qt3DCore::QEntity *parent3DScene,
               Qgs3DMapScene *mapScene, QgsCameraController *camera, Qgs3DMapSettings *map );
    ~Qgs3DAxis() override;

    /**
     * \brief The Mode enum
     */
    enum class Mode
    {
      Off = 1, //!< Hide 3d axis
      Crs = 2, //!< Respect CRS directions
      Cube = 3, //!< Abstract cube mode
    };
    Q_ENUM( Mode )

    /**
     * \brief Returns axis mode
     */
    Qgs3DAxis::Mode mode() { return mMode; }

    /**
     * \brief set axis representation mode
     * \param axisMode new node
     */
    void setMode( Qgs3DAxis::Mode axisMode );

    /**
     * \brief set axis viewport position parameters
     * \param axisViewportSize height/width size in pixel
     * @param axisViewportVertPos start vertical position
     * @param axisViewportHorizPos start horizontal position
     */
    void setAxisViewportPosition( int axisViewportSize, Qt::AnchorPoint axisViewportVertPos, Qt::AnchorPoint axisViewportHorizPos );

    /**
     * \brief Returns axis viewport size
     */
    int axisViewportSize() const { return mAxisViewportSize;}

    /**
     * \brief Returns axis viewport horizontal position
     */
    Qt::AnchorPoint axisViewportHorizontalPosition() const { return mAxisViewportHorizPos;}

    /**
     * \brief Returns axis viewport vertical position
     */
    Qt::AnchorPoint axisViewportVerticalPosition() const { return mAxisViewportVertPos;}

    /**
     * \brief project a 3D position from sourceCamera (in sourceViewport) to a 2D position for destCamera (in destViewport). destCamera and the destViewport act as a billboarding layer. The labels displayed by this process will always face the camera.
     *
     * \param sourcePos 3D label coordinates
     * @param sourceCamera main view camera
     * @param sourceViewport main viewport
     * @param destCamera billboarding camera
     * @param destViewport billboarding viewport
     * @param destSize main qt3d window size
     * @return
     */
    QVector3D from3dTo2dLabelPosition( const QVector3D &sourcePos,
                                       Qt3DRender::QCamera *sourceCamera, Qt3DRender::QViewport *sourceViewport,
                                       Qt3DRender::QCamera *destCamera, Qt3DRender::QViewport *destViewport,
                                       const QSize &destSize );

  private slots:

    void onCameraUpdate( );
    void onAxisViewportSizeUpdate( int val = 0 );

    // axis picking and menu
    void onTouchedByRay( const Qt3DRender::QAbstractRayCaster::Hits &hits );
    void onAxisModeChanged( Qgs3DAxis::Mode mode );
    void onAxisHorizPositionChanged( Qt::AnchorPoint pos );
    void onAxisVertPositionChanged( Qt::AnchorPoint pos );
    void onCameraViewChange( float pitch, float yaw );

    void onCameraViewChangeHome( ) { onCameraViewChange( 45.0, 45.0 ); }
    void onCameraViewChangeTop( ) {onCameraViewChange( 0.0, 90.0 );}
    void onCameraViewChangeNorth( ) {onCameraViewChange( 90.0, 180.0 );}
    void onCameraViewChangeEast( ) {onCameraViewChange( 90.0, 90.0 );}
    void onCameraViewChangeSouth( ) {onCameraViewChange( 90.0, 0.0 );}
    void onCameraViewChangeWest( ) {onCameraViewChange( 90.0, -90.0 );}
    void onCameraViewChangeBottom() {onCameraViewChange( 180.0, 0.0 );}
    void populateMenu();

  private:

    void createAxisScene();
    void createAxis( Qt::Axis axis );
    void createCube( );
    void setEnableCube( bool show );
    void setEnableAxis( bool show );
    void updateAxisLabelPosition();

    Qt3DRender::QViewport *constructAxisViewport( Qt3DCore::QEntity *parent3DScene );
    Qt3DRender::QViewport *constructLabelViewport( Qt3DCore::QEntity *parent3DScene, const QRectF &parentViewportSize );

    Qt3DExtras::QText2DEntity *addCubeText( const QString &text, float textHeight, float textWidth, const QFont &f, const QMatrix4x4 &rotation, const QVector3D &translation );

    // axis picking and menu
    void init3DObjectPicking( );
    bool eventFilter( QObject *watched, QEvent *event ) override;

    void hideMenu();
    void displayMenuAt( const QPoint &position );

    Qgs3DMapSettings *mMapSettings = nullptr;
    Qt3DExtras::Qt3DWindow *mParentWindow = nullptr;
    Qgs3DMapScene *mMapScene = nullptr;
    QgsCameraController *mCameraController = nullptr;

    float mCylinderLength = 40.0f;
    int mAxisViewportSize = 4.0 * mCylinderLength;
    Qt::AnchorPoint mAxisViewportVertPos = Qt::AnchorPoint::AnchorTop;
    Qt::AnchorPoint mAxisViewportHorizPos = Qt::AnchorPoint::AnchorRight;
    int mFontSize = 10;

    Qt3DCore::QEntity *mAxisSceneEntity = nullptr;
    Qt3DRender::QLayer *mAxisSceneLayer = nullptr;
    Qt3DRender::QCamera *mAxisCamera = nullptr;
    Qt3DRender::QViewport *mAxisViewport = nullptr;

    Qgs3DAxis::Mode mMode = Mode::Crs;
    Qt3DCore::QEntity *mAxisRoot = nullptr;
    Qt3DCore::QEntity *mCubeRoot = nullptr;
    QList<Qt3DExtras::QText2DEntity *> mCubeLabels;

    Qt3DExtras::QText2DEntity *mTextX = nullptr;
    Qt3DExtras::QText2DEntity *mTextY = nullptr;
    Qt3DExtras::QText2DEntity *mTextZ = nullptr;
    QVector3D mTextCoordX;
    QVector3D mTextCoordY;
    QVector3D mTextCoordZ;
    Qt3DCore::QTransform *mTextTransformX = nullptr;
    Qt3DCore::QTransform *mTextTransformY = nullptr;
    Qt3DCore::QTransform *mTextTransformZ = nullptr;
    QgsCoordinateReferenceSystem mCrs;
    QVector3D mPreviousVector;

    Qt3DRender::QCamera *mTwoDLabelCamera  = nullptr;
    Qt3DCore::QEntity *mTwoDLabelSceneEntity = nullptr;
    Qt3DRender::QViewport *mTwoDLabelViewport = nullptr;

    // axis picking and menu
    Qt3DRender::QScreenRayCaster *mScreenRayCaster = nullptr;
    QPoint mLastClickedPos;
    Qt::MouseButton mLastClickedButton;
    QCursor mPreviousCursor = Qt::ArrowCursor;
    QMenu *mMenu = nullptr;

};

/**
 * \ingroup 3d
 * \brief Geometry renderer for lines, draws a wired mesh
 *
 * \since QGIS 3.26
 */
class Qgs3DWiredMesh : public Qt3DRender::QGeometryRenderer
{
    Q_OBJECT

  public:

    /**
     * \brief Defaul Qgs3DWiredMesh constructor
     */
    Qgs3DWiredMesh( Qt3DCore::QNode *parent = nullptr );
    ~Qgs3DWiredMesh() override;

    /**
     * \brief add or replace mesh vertices coordinates
     */
    void setVertices( const QList<QVector3D> &vertices );

  private:
    Qt3DRender::QGeometry *mGeom = nullptr;
    Qt3DRender::QAttribute *mPositionAttribute = nullptr;
    Qt3DRender::QBuffer *mVertexBuffer = nullptr;
};

#endif // QGS3DAXIS_H
