/***************************************************************************
  qgs3dutils.h
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DUTILS_H
#define QGS3DUTILS_H

#include "qgis_sip.h"

class QgsLineString;
class QgsPolygon;

class QgsAbstract3DEngine;
class Qgs3DMapScene;

#include "qgs3dmapsettings.h"
#include "qgs3dtypes.h"
#include "qgsaabb.h"


#ifndef SIP_RUN

/**
 * \ingroup 3d
 * Miscellaneous utility functions used from 3D code.
 * \since QGIS 3.0
 */
class _3D_EXPORT Qgs3DUtils
{
  public:

    /**
     * Captures image of the current 3D scene of a 3D engine. The function waits
     * until the scene is not fully loaded/updated before capturing the image.
     * \since QGIS 3.4
     */
    static QImage captureSceneImage( QgsAbstract3DEngine &engine, Qgs3DMapScene *scene );

    /**
     * Calculates the highest needed zoom level for tiles in quad-tree given width of the base tile (zoom level 0)
     * in map units, resolution of the tile (e.g. tile's texture width) and desired maximum error in map units.
     */
    static int maxZoomLevel( double tile0width, double tileResolution, double maxError );

    //! Converts a value from AltitudeClamping enum to a string
    static QString altClampingToString( Qgs3DTypes::AltitudeClamping altClamp );
    //! Converts a string to a value from AltitudeClamping enum
    static Qgs3DTypes::AltitudeClamping altClampingFromString( const QString &str );

    //! Converts a value from AltitudeBinding enum to a string
    static QString altBindingToString( Qgs3DTypes::AltitudeBinding altBind );
    //! Converts a string to a value from AltitudeBinding enum
    static Qgs3DTypes::AltitudeBinding altBindingFromString( const QString &str );

    //! Converts a value from CullingMode enum to a string
    static QString cullingModeToString( Qgs3DTypes::CullingMode mode );
    //! Converts a string to a value from CullingMode enum
    static Qgs3DTypes::CullingMode cullingModeFromString( const QString &str );

    //! Clamps altitude of a vertex according to the settings, returns Z value
    static float clampAltitude( const QgsPoint &p, Qgs3DTypes::AltitudeClamping altClamp, Qgs3DTypes::AltitudeBinding altBind, float height, const QgsPoint &centroid, const Qgs3DMapSettings &map );
    //! Clamps altitude of vertices of a linestring according to the settings
    static void clampAltitudes( QgsLineString *lineString, Qgs3DTypes::AltitudeClamping altClamp, Qgs3DTypes::AltitudeBinding altBind, const QgsPoint &centroid, float height, const Qgs3DMapSettings &map );
    //! Clamps altitude of vertices of a polygon according to the settings
    static bool clampAltitudes( QgsPolygon *polygon, Qgs3DTypes::AltitudeClamping altClamp, Qgs3DTypes::AltitudeBinding altBind, float height, const Qgs3DMapSettings &map );

    //! Converts a 4x4 transform matrix to a string
    static QString matrix4x4toString( const QMatrix4x4 &m );
    //! Convert a string to a 4x4 transform matrix
    static QMatrix4x4 stringToMatrix4x4( const QString &str );

    /**
     * Calculates (x,y,z) positions of a (multi)point in the Point vector layers
     */
    static QList<QVector3D> positions( const Qgs3DMapSettings &map, QgsVectorLayer *layer, const QgsFeatureRequest &req, Qgs3DTypes::AltitudeClamping altClamp );

    /**
        Returns true if bbox is completely outside the current viewing volume.
        This is used to perform object culling checks.
    */
    static bool isCullable( const QgsAABB &bbox, const QMatrix4x4 &viewProjectionMatrix );

    //! Converts map coordinates to 3D world coordinates (applies offset and turns (x,y,z) into (x,-z,y))
    static QgsVector3D mapToWorldCoordinates( const QgsVector3D &mapCoords, const QgsVector3D &origin );
    //! Converts 3D world coordinates to map coordinates (applies offset and turns (x,y,z) into (x,-z,y))
    static QgsVector3D worldToMapCoordinates( const QgsVector3D &worldCoords, const QgsVector3D &origin );

    //! Transforms a world point from (origin1, crs1) to (origin2, crs2)
    static QgsVector3D transformWorldCoordinates( const QgsVector3D &worldPoint1, const QgsVector3D &origin1, const QgsCoordinateReferenceSystem &crs1, const QgsVector3D &origin2, const QgsCoordinateReferenceSystem &crs2,
        const QgsCoordinateTransformContext &context );
};

#endif

#endif // QGS3DUTILS_H
