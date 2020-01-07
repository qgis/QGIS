/***************************************************************************
                         qgsmesh3drendercontroler.h
                         -------------------------
    begin                : january 2020
    copyright            : (C) 2020 by Vincent Cloarec
    email                : vcloarec at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESH3DRENDERCONTROLER_H
#define QGSMESH3DRENDERCONTROLER_H

#include <QObject>

#include <Qt3DRender>

#include "qgsabstract3dengine.h"

class QgsMesh3dRenderControler: public QObject
{
  public:
    QgsMesh3dRenderControler( QgsAbstract3DEngine *engine3D );
    void setBufferSize( int width, int height );

    void activate();
    void deactivate();

  private:

    void createGrapheNode();
    void createBuffer();

    QgsAbstract3DEngine *mEngine3D = nullptr;
    Qt3DRender::QFrameGraphNode *mRootFrameGraph = nullptr;
    QSize mViewPortSize;
    Qt3DRender::QTexture2D *mMeshColorBuffer;
    Qt3DRender::QTexture2D *mMeshDepthBuffer;
    Qt3DRender::QTexture2D *mMeshLightedColorBuffer;
    Qt3DRender::QTexture2D *mMeshLightedDepthBuffer;
    Qt3DRender::QParameter *mMeshColorTextureParameter = nullptr;
    Qt3DRender::QParameter *mMeshColorBufferWidthParameter = nullptr;
    Qt3DRender::QParameter *mMeshColorBufferHeightParameter = nullptr;
};

#endif // QGSMESH3DRENDERCONTROLER_H
