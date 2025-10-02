/***************************************************************************
    qgsraycasthit.h
    ---------------------
    begin                : September 2025
    copyright            : (C) 2025 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRAYCASTHIT_H
#define QGSRAYCASTHIT_H

#include "qgis_3d.h"
#include "qgsfeatureid.h"
#include "qgsvector3d.h"

#include <QVector3D>

class QgsMapLayer;

/**
 * \ingroup qgis_3d
 *
 * \brief Contains details about the ray intersecting entities when ray casting in a 3D map canvas.
 *
 * \since QGIS 4.0
 */
class _3D_EXPORT QgsRayCastHit
{
  public:
    QgsRayCastHit() = default;

    //! Returns the hit's distance from the ray's origin
    double distance() const;

    //! Returns the hit position in 3d map coordinates
    QgsVector3D mapCoordinates() const;

    /**
     * Returns a map with the properties of the intersected entity.
     * For vector layer hits, the FID value is stored in key "fid".
     * For point cloud layer hits, it contains the point attributes keys/values.
     * For tiled scene layer hits, it contains the intersecting node details.
     */
    QVariantMap properties() const;

    //! Sets the hit's \a distance from the ray's origin
    void setDistance( double distance );

    //! Sets the hit \a point position in 3d map coordinates
    void setMapCoordinates( const QgsVector3D &point );

    //! Sets the point cloud point \a attributes, empty map if hit was not on a point cloud point
    void setProperties( const QVariantMap &attributes );

  private:
    double mDistance = -1.;  //!< Distance from ray's origin
    QgsVector3D mPos;        //!< Hit position in 3d map coordinates
    QVariantMap mAttributes; //!< Point cloud point attributes, empty map if no point cloud points hit
};

#endif // QGSRAYCASTHIT_H
