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


#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QRenderPassFilter>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QAbstractTextureImage>

#include "qgsmesh3dsymbol.h"
#include "qgsvector3d.h"

///@cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#define SIP_NO_FILE

class QgsMeshLayer;

/**
 * \ingroup 3d
 * \brief Implementation of material used to render the mesh layer
 * \since QGIS 3.12
 */
class QgsMesh3dMaterial : public Qt3DRender::QMaterial
{
    Q_OBJECT

  public:
    //! enum used to defined which type of value will be used to render the 3D entity
    enum MagnitudeType
    {

      /**
       * Only the z values of the mesh will be used to build the geometry and to render the color
       * Used to render the terrain
       */
      ZValue,

      /**
       * The datasets are used to build the geometry and to render the color
       * For example, can be used to render the geometry of the water surface with color varying with velocity
       */
      ScalarDataSet
    };

    //! Constructor
    QgsMesh3dMaterial( QgsMeshLayer *layer,
                       const QgsDateTimeRange &timeRange,
                       const QgsVector3D &origin,
                       const QgsMesh3DSymbol *symbol,
                       MagnitudeType magnitudeType = ZValue );

  private:
    std::unique_ptr< QgsMesh3DSymbol > mSymbol;
    Qt3DRender::QTechnique *mTechnique;
    MagnitudeType mMagnitudeType = ZValue;
    QgsVector3D mOrigin;

    void configure();
    void configureArrows( QgsMeshLayer *layer, const QgsDateTimeRange &timeRange );
};


class ArrowsGridTexture: public Qt3DRender::QAbstractTextureImage
{
    Q_OBJECT

  public:
    ArrowsGridTexture( const QVector<QgsVector> &vectors, const QSize &size, bool fixedSize, double maxVectorLength );

  protected:
    Qt3DRender::QTextureImageDataGeneratorPtr dataGenerator() const override;

  private:
    const QVector<QgsVector> mVectors;
    const QSize mSize;
    const bool mFixedSize;
    const double mMaxVectorLength;
};


///@endcond

#endif // QGSMESH3DMATERIAL_H
