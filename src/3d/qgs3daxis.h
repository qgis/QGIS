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

#include "qgs3dmapsettings.h"
#include <Qt3DCore/QEntity>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DExtras/QText2DEntity>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QViewport>
#include <QVector3D>

#include <Qt3DRender/QBuffer>

#define SIP_NO_FILE

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
     * @param camera camera controller used to track camera movements
     * @param map 3D map settings
     */
    Qgs3DAxis( Qt3DExtras::Qt3DWindow *parentWindow,  Qt3DCore::QEntity *parent3DScene, QgsCameraController *camera, const Qgs3DMapSettings *map );

    /**
     * \brief The Axis enum
     */
    enum class Axis
    {
      X = 1,
      Y = 2,
      Z = 3
    };
    Q_ENUM( Axis )

    /**
     * \brief The AxisViewportPosition enum
     */
    enum class AxisViewportPosition
    {
      //! top or left
      Begin = 1,
      Middle = 2,
      //! bottom or right
      End = 3
    };
    Q_ENUM( AxisViewportPosition )

    /**
     * \brief The Mode enum
     */
    enum class Mode
    {
      //! disabled
      Off = 1,
      //! CRS specific.
      Crs = 2,
      //! Cube with label
      Cube = 3
    };
    Q_ENUM( Mode )

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
    void setAxisViewportPosition( int axisViewportSize, AxisViewportPosition axisViewportVertPos, AxisViewportPosition axisViewportHorizPos );

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

  private:
    void createAxisScene();
    void createAxis( const Axis &axis );
    void createCube( );
    void setEnableCube( bool show );
    void setEnableAxis( bool show );
    void updateCamera( );
    void updateAxisViewportSize( int val = 0 );
    void updateAxisLabelPosition();

    Qt3DRender::QViewport *constructAxisViewport( Qt3DCore::QEntity *parent3DScene );
    Qt3DRender::QViewport *constructLabelViewport( Qt3DCore::QEntity *parent3DScene, const QRectF &parentViewportSize );

    Qt3DExtras::QText2DEntity *addCubeText( const QString &text, float textHeight, float textWidth, const QFont &f, const QMatrix4x4 &rotation, const QVector3D &translation );

    Qt3DExtras::Qt3DWindow *mParentWindow;
    Qt3DRender::QCamera *mParentCamera;
    float mCylinderLength = 40.0f;
    int mAxisViewportSize = 4.0 * mCylinderLength;
    AxisViewportPosition mAxisViewportVertPos = AxisViewportPosition::Begin;
    AxisViewportPosition mAxisViewportHorizPos = AxisViewportPosition::End;
    int mFontSize = 10;

    Qt3DCore::QEntity *mAxisSceneEntity;
    Qt3DRender::QCamera *mAxisCamera;
    Qt3DRender::QViewport *mAxisViewport;

    Qgs3DAxis::Mode mMode = Mode::Crs;
    Qt3DCore::QEntity *mAxisRoot = nullptr;
    Qt3DCore::QEntity *mCubeRoot = nullptr;
    QList<Qt3DExtras::QText2DEntity *> mCubeLabels;

    Qt3DExtras::QText2DEntity *mText_X, *mText_Y, *mText_Z;
    QVector3D mTextCoord_X, mTextCoord_Y, mTextCoord_Z;
    Qt3DCore::QTransform *mTextTransform_X = nullptr, *mTextTransform_Y = nullptr, *mTextTransform_Z = nullptr;
    QgsCoordinateReferenceSystem mCrs;
    QVector3D mPreviousVector;

    Qt3DRender::QCamera *mTwoDLabelCamera;
    Qt3DCore::QEntity *mTwoDLabelSceneEntity;
    Qt3DRender::QViewport *mTwoDLabelViewport;
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
