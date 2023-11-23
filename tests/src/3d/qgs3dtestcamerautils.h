/***************************************************************************
  qgs3dtestcamerautils.h
  --------------------------------------
  Date                 : November 2023
  Copyright            : (C) 2023 by Benoit De Mezzo
  Email                : benoit dot de dot mezzo at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscameracontroller.h"

/**
 * \ingroup UnitTests
 * Helper class to access QgsCameraController properties
 */
class QgsCameraController4Test : public QgsCameraController
{
    Q_OBJECT
  public:
    QgsCameraController4Test( Qgs3DMapScene *parent = nullptr )
      : QgsCameraController( parent )
    { }

    // wraps protected methods
    void superOnWheel( Qt3DInput::QWheelEvent *wheel ) { onWheel( wheel ); }
    void superOnMousePressed( Qt3DInput::QMouseEvent *mouse ) { onMousePressed( mouse ); }
    double superSampleDepthBuffer( const QImage &buffer, int px, int py ) { return sampleDepthBuffer( buffer, px, py ); }

    // wraps protected member vars
    QVector3D zoomPoint() { return mZoomPoint; }
    double cumulatedWheelY() { return mCumulatedWheelY; }
    Qt3DRender::QCamera *cameraBefore() { return mCameraBefore.get(); }
    QgsCameraPose *cameraPose() { return &mCameraPose; }


};
