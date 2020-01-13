/***************************************************************************
                         qgsmesh3dmaterial.h
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

#ifndef QGSMESH3DMATERIAL_H
#define QGSMESH3DMATERIAL_H

#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QRenderPassFilter>
#include <Qt3DRender/QTechnique>

#include "qgsmesh3dsymbol.h"
#include "qgscolorrampshader.h"


class QgsMesh3dMaterial : public Qt3DRender::QMaterial
{
  public:
    enum MagnitudeType
    {
      zValue,
      scalarDataSet
    };

    QgsMesh3dMaterial( MagnitudeType magnitudeType, const QgsMesh3DSymbol &symbol );


  private:
    MagnitudeType mMagnitudeType;
    QgsMesh3DSymbol mSymbol;
    Qt3DRender::QBuffer *mColorsRampBuffer = nullptr;
    Qt3DRender::QTechnique *mTechnique;

    void configure();
};

#endif // QGSMESH3DMATERIAL_H
