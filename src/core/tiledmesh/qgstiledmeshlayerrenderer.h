/***************************************************************************
                         qgstiledmeshlayerrenderer.h
                         --------------------
    begin                : June 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTILEDMESHLAYERRENDERER_H
#define QGSTILEDMESHLAYERRENDERER_H

#include "qgis_core.h"
#include "qgsmaplayerrenderer.h"
#include "qgscoordinatereferencesystem.h"

#include <memory>
#include <QElapsedTimer>

#define SIP_NO_FILE

class QgsTiledMeshLayer;
class QgsFeedback;
class QgsMapClippingRegion;
class QgsAbstractTiledMeshNodeBoundingVolume;


/**
 * \ingroup core
 *
 * \brief Implementation of threaded 2D rendering for tiled mesh layers.
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 * \note Not available in Python bindings
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsTiledMeshLayerRenderer: public QgsMapLayerRenderer
{
  public:

    //! Ctor
    explicit QgsTiledMeshLayerRenderer( QgsTiledMeshLayer *layer, QgsRenderContext &context );
    ~QgsTiledMeshLayerRenderer();

    bool render() override;
    void setLayerRenderingTimeHint( int time ) override;

    QgsFeedback *feedback() const override { return mFeedback.get(); }

  private:
    QList< QgsMapClippingRegion > mClippingRegions;

    int mRenderTimeHint = 0;
    bool mBlockRenderUpdates = false;
    QElapsedTimer mElapsedTimer;

    QgsCoordinateReferenceSystem mMeshCrs;
    std::unique_ptr< QgsAbstractTiledMeshNodeBoundingVolume > mLayerBoundingVolume;

    std::unique_ptr<QgsFeedback> mFeedback = nullptr;
};

#endif // QGSTILEDMESHLAYERRENDERER_H
