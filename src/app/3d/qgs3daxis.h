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

#include <QVector3D>
#include <Qt3DCore/QEntity>
#include <Qt3DRender/QCamera>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DRender/QViewport>
#include <Qt3DExtras/QText2DEntity>
#include "qgs3dmapsettings.h"

#define SIP_NO_FILE

/**
 * Display 3D ortho axis in the main 3D view.
 *
 * Axis are displayed in a dedicated viewport which can be placed all around the main viewport.
 * Axis labels are displayed in a dedicated viewport with a specific camera to act as a billboarding layer.
 * Axis labels can be changed with the mode combo box from the navigation widget.

 * \note Not available in Python bindings
 *
 * \since QGIS 3.25
 */
class Qgs3DAxis : public QObject
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
    enum Axis
    {
      X = 1,
      Y = 2,
      Z = 3
    };

    /**
     * \brief The AxisViewportPosition enum
     */
    enum AxisViewportPosition
    {
      //! top or left
      BEGIN = 1,
      MIDDLE = 2,
      //! bottom or right
      END = 3
    };

    /**
     * \brief The Mode enum
     */
    enum Mode
    {
      //! disabled
      OFF = 1,
      //! CRS specific. TODO: should handle up axis
      SRS = 2,
      //! Compass axis ie. North-East-Up
      NEU = 3
    };

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
    void updateCamera( );
    void updateAxisViewportSize( int val = 0 );
    void updateLabelPosition();

    Qt3DRender::QViewport *constructAxisViewport( Qt3DCore::QEntity *parent3DScene );
    Qt3DRender::QViewport *constructLabelViewport( Qt3DCore::QEntity *parent3DScene, const QRectF &parentViewportSize );

    Qt3DExtras::Qt3DWindow *mParentWindow;
    Qt3DRender::QCamera *mParentCamera;
    float mCylinderLength;
    int mAxisViewportSize;
    AxisViewportPosition mAxisViewportVertPos;
    AxisViewportPosition mAxisViewportHorizPos;
    int mFontSize;

    Qt3DCore::QEntity *mAxisSceneEntity;
    Qt3DRender::QCamera *mAxisCamera;
    Qt3DRender::QViewport *mAxisViewport;

    Qgs3DAxis::Mode mMode;
    Qt3DCore::QEntity *mAxisRoot;

    Qt3DExtras::QText2DEntity *mText_X, *mText_Y, *mText_Z;
    QVector3D mTextCoord_X, mTextCoord_Y, mTextCoord_Z;
    Qt3DCore::QTransform *mTextTransform_X, *mTextTransform_Y, *mTextTransform_Z;
    QgsCoordinateReferenceSystem mCrs;
    QVector3D mPreviousVector;

    Qt3DRender::QCamera *mTwoDLabelCamera;
    Qt3DCore::QEntity *mTwoDLabelSceneEntity;
    Qt3DRender::QViewport *mTwoDLabelViewport;
};

#endif // QGS3DAXIS_H
