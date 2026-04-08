/***************************************************************************
  qgsabstractfeaturebasedchunkedentity.h
  --------------------------------------
  Date                 : March 2026
  Copyright            : (C) 2026 by Jean Felder
  Email                : jean dot felder at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSABSTRACTFEATUREBASEDCHUNKEDENTITY_H
#define QGSABSTRACTFEATUREBASEDCHUNKEDENTITY_H

#include "qgis_3d.h"

#define SIP_NO_FILE

///@cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include "qgschunkedentity.h"

namespace Qt3DCore
{
  class QTransform;
}

/**
 * Abstract base class for handling common logic of chunked entities in vector layers and annotations.
 *
 * \since QGIS 4.2
 */
class _3D_EXPORT QgsAbstractFeatureBasedChunkedEntity : public QgsChunkedEntity
{
    Q_OBJECT

  public:
    //! Constructs the entity.
    QgsAbstractFeatureBasedChunkedEntity(
      Qgs3DMapSettings *mapSettings, float tau, QgsChunkLoaderFactory *loaderFactory, bool ownsFactory, int primitivesBudget = std::numeric_limits<int>::max(), Qt3DCore::QNode *parent = nullptr
    );

    QList<QgsRayCastHit> rayIntersection( const QgsRay3D &ray, const QgsRayCastContext &context ) const override;

  protected slots:
    void onTerrainElevationOffsetChanged();

  private:
    //! Returns whether terrain offset should be applied based on the layer's altitude clamping mode.
    virtual bool applyTerrainOffset() const = 0;

    //! Performs ray casting against active nodes and returns the nearest hit.
    QList<QgsRayCastHit> rayIntersection( const QList<QgsChunkNode *> &activeNodes, const QMatrix4x4 &transformMatrix, const QgsRay3D &ray, const QgsRayCastContext &context, const QgsVector3D &origin ) const;

  private:
    Qt3DCore::QTransform *mTransform = nullptr;
};

/// @endcond

#endif // QGSABSTRACTFEATUREBASEDCHUNKEDENTITY_H
