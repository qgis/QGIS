/***************************************************************************
  qgsvectorlayerchunkloader_p.h
  --------------------------------------
  Date                 : September 2026
  Copyright            : (C) 2026 by Dominik Cindric
  Email                : viper dot miniq at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGLOBEUTILS_P_H
#define QGSGLOBEUTILS_P_H

///@cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include "qgis.h"
#include "qgsbox3d.h"
#include "qgschunknode.h"
#include "qgsrectangle.h"

#define SIP_NO_FILE

class QgsCoordinateTransform;

/**
 * \ingroup qgis_3d
 * \brief Utility functions for the quadtree fixed tilling scheme for globe.
 */
class QgsGlobeUtils
{
  public:
    //! Returns the lon/lat extent of the tile with the given \a id
    static QgsRectangle nodeIdToLonLatRect( QgsChunkNodeId id );

    //! Returns the id of the smallest tile that fully contains \a lonLatExtent
    static QgsChunkNodeId tileIdForExtent( const QgsRectangle &lonLatExtent );

    //! Returns the ECEF world-space bounding box of the given tile
    static QgsBox3D nodeIdToBox3D( QgsChunkNodeId id, const QgsCoordinateTransform &crsToLatLon, double radiusX, double radiusY, double radiusZ );

    //! Returns the XY bounding rectangle resulting from transforming the corners of \a box3D through \a transform
    static QgsRectangle box3DTransformedExtent( const QgsBox3D &box3D, const QgsCoordinateTransform &transform, Qgis::TransformDirection direction = Qgis::TransformDirection::Forward );
};

/// @endcond

#endif // QGSGLOBEUTILS_P_H
