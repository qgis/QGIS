/***************************************************************************
                         qgsmeshlayerinterpolator.h
                         --------------------------
    begin                : April 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESHLAYERINTERPOLATOR_H
#define QGSMESHLAYERINTERPOLATOR_H

class QgsMeshLayer;
class QgsSymbol;

#define SIP_NO_FILE

#include <QSize>

#include "qgis.h"

#include "qgsmaplayerrenderer.h"
#include "qgsrendercontext.h"
#include "qgstriangularmesh.h"
#include "qgsrasterinterface.h"
#include "qgssinglebandpseudocolorrenderer.h"
#include "qgsrastershader.h"

///@cond PRIVATE

/**
 * \ingroup core
 * Interpolate mesh scalar dataset to raster block
 *
 * \note not available in Python bindings
 * \since QGIS 3.2
 */
class QgsMeshLayerInterpolator : public QgsRasterInterface
{
  public:
    //! Ctor
    QgsMeshLayerInterpolator( const QgsTriangularMesh &m,
                              const QVector<double> &datasetValues,
                              const QgsMeshDataBlock &activeFaceFlagValues,
                              bool dataIsOnVertices,
                              const QgsRenderContext &context,
                              const QSize &size );
    ~QgsMeshLayerInterpolator() override;

    QgsRasterInterface *clone() const override;
    Qgis::DataType dataType( int ) const override;
    int bandCount() const override;
    QgsRasterBlock *block( int, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback = nullptr ) override;

  private:
    const QgsTriangularMesh &mTriangularMesh;
    const QVector<double> &mDatasetValues;
    const QgsMeshDataBlock &mActiveFaceFlagValues;
    const QgsRenderContext &mContext;
    bool mDataOnVertices = true;
    QSize mOutputSize;
};

///@endcond

#endif // QGSMESHLAYERINTERPOLATOR_H
