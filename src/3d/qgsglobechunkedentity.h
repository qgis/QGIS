/***************************************************************************
  qgsglobechunkedentity.h
  --------------------------------------
  Date                 : March 2025
  Copyright            : (C) 2025 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGLOBECHUNKEDENTITY_H
#define QGSGLOBECHUNKEDENTITY_H

#include "qgis_3d.h"

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


#include "qgschunkedentity.h"

class QgsMapLayer;
class QgsGlobeMapUpdateJobFactory;


/**
 * 3D chunked entity implementation to generate globe mesh with constant elevation
 *
 * \since QGIS 3.44
 */
class _3D_EXPORT QgsGlobeEntity : public QgsChunkedEntity
{
  public:
    QgsGlobeEntity( Qgs3DMapSettings *mapSettings );
    ~QgsGlobeEntity();

    QVector<QgsRayCastingUtils::RayHit> rayIntersection( const QgsRayCastingUtils::Ray3D &ray, const QgsRayCastingUtils::RayCastContext &context ) const override;

  private slots:
    void invalidateMapImages();
    void onLayersChanged();

  private:
    void connectToLayersRepaintRequest();

  private:
    std::unique_ptr<QgsGlobeMapUpdateJobFactory> mUpdateJobFactory;

    //! layers that are currently being used for map rendering (and thus being watched for renderer updates)
    QList<QgsMapLayer *> mLayers;
};


/// @endcond

#endif // QGSGLOBECHUNKEDENTITY_H
