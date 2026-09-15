/***************************************************************************
  qgsannotationlayerchunkloader_p.h
  --------------------------------------
  Date                 : September 2025
  Copyright            : (C) 2025 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSANNOTATIONLAYERCHUNKLOADER_P_H
#define QGSANNOTATIONLAYERCHUNKLOADER_P_H

///@cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include "qgs3drendercontext.h"
#include "qgsabstractfeaturebasedchunkedentity.h"
#include "qgschunkloader.h"
#include "qgstextformat.h"

#include <QImage>

#define SIP_NO_FILE

class QgsAnnotationLayer;
class QgsAnnotationItem;

namespace Qt3DCore
{
  class QTransform;
}

#include <QFutureWatcher>


/**
 * \ingroup qgis_3d
 * \brief This loader is responsible for creation of QgsAnnotationLayerChunkedEntity chunks.
 *
 * \since QGIS 4.0
 */
class QgsAnnotationLayerChunkLoader : public QgsQuadtreeChunkLoader
{
    Q_OBJECT

  public:
    //! Constructs the loader
    QgsAnnotationLayerChunkLoader(
      const Qgs3DRenderContext &context,
      QgsAnnotationLayer *layer,
      int leafLevel,
      Qgis::AltitudeClamping clamping,
      double zOffset,
      bool showCallouts,
      const QColor &calloutLineColor,
      double calloutLineWidth,
      const QgsTextFormat &textFormat,
      double zMin,
      double zMax
    );

    QFuture<QgsChunkLoaderResult> loadChunk( QgsChunkNode *node ) override;

    Qgs3DRenderContext mRenderContext;
    QgsAnnotationLayer *mLayer = nullptr;
    int mLeafLevel = 0;

    // Each worker thread loading a chunk copies this data.
    struct
    {
        Qgis::AltitudeClamping mClamping = Qgis::AltitudeClamping::Relative;
        double mZOffset = 0;
        bool mShowCallouts = false;
        QColor mCalloutLineColor;
        double mCalloutLineWidth = 2;
        QgsTextFormat mTextFormat;
    } mData;
};

/**
 * \ingroup qgis_3d
 * \brief 3D entity used for rendering of annotation layers.
 *
 * Internally it uses QgsAnnotationLayerChunkLoader and
 * QgsAnnotationLayerChunkLoader to do the actual work
 * of loading and creating 3D sub-entities for the layer.
 *
 * \since QGIS 4.0
 */
class QgsAnnotationLayerChunkedEntity : public QgsAbstractFeatureBasedChunkedEntity
{
    Q_OBJECT
  public:
    //! Constructs the entity.
    explicit QgsAnnotationLayerChunkedEntity(
      Qgs3DMapSettings *map,
      QgsAnnotationLayer *layer,
      Qgis::AltitudeClamping clamping,
      double zOffset,
      bool showCallouts,
      const QColor &calloutLineColor,
      double calloutLineWidth,
      const QgsTextFormat &textFormat,
      double zMin,
      double zMax
    );
    ~QgsAnnotationLayerChunkedEntity() override;

    QList<QgsRayCastHit> rayIntersection( const QgsRay3D &ray, const QgsRayCastContext &context ) const override;

  private:
    bool applyTerrainOffset() const override;
};

/// @endcond

#endif // QGSANNOTATIONLAYERCHUNKLOADER_P_H
