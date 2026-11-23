/***************************************************************************
  qgslinevertexdata_p.h
  --------------------------------------
  Date                 : Apr 2019
  Copyright            : (C) 2019 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLINEVERTEXDATA_P_H
#define QGSLINEVERTEXDATA_P_H

/// @cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//


#include "qgis.h"
#include "qgs3drendercontext.h"

#include <QByteArray>
#include <QVector>
#include <QVector3D>

#define SIP_NO_FILE

namespace Qt3DCore
{
  class QNode;
  class QGeometry;
  class QEntity;
} // namespace Qt3DCore

class QgsLineString;
class QgsMaterial;
class QgsLineMaterial;
class QgsAbstractMaterialSettings;
class QgsAbstractMaterial3DHandler;

/**
 * \ingroup qgis_3d
 * \brief Helper class to build the reusable per-instance quad geometry used to render
 * thick 3D lines with GPU instancing (one quad instanced per line segment).
 *
 * Line strings are stored as independent segments (pointA/pointB pairs). Each segment
 * also carries the point before pointA and after pointB (pointPrev/pointNext) so the
 * vertex shader can pull back the vertex that would otherwise overlap the neighboring
 * segment, which matters when lines are rendered with alpha blending.
 */
struct QgsLineVertexData
{
    QVector<QVector3D> pointsA;    //!< Start point of each line segment (one entry per instance)
    QVector<QVector3D> pointsB;    //!< End point of each line segment (one entry per instance)
    QVector<QVector3D> pointsPrev; //!< Point before pointsA on the line strip
    QVector<QVector3D> pointsNext; //!< Point after pointsB on the line strip
    QVector<QVector3D> vertices;   //!< Flat list of unique vertices added, e.g. for placing one marker per vertex

    //! First/middle/next point of each interior line join
    QVector<QVector3D> joinPointA;
    QVector<QVector3D> joinPointB;
    QVector<QVector3D> joinPointC;

    QByteArray materialDataDefined;
    //! Per-instance data-defined stroke color for joins, kept in sync with joinPointA/B/C
    QByteArray materialDataDefinedJoins;

    // extra info to calculate elevation
    Qgis::AltitudeClamping altClamping = Qgis::AltitudeClamping::Absolute;
    Qgis::AltitudeBinding altBinding = Qgis::AltitudeBinding::Vertex;
    float baseHeight = 0;
    Qgs3DRenderContext renderContext;   // used for altitude clamping
    QgsVector3D origin;                 // all coordinates are relative to this origin (e.g. center of the chunk)
    bool geocentricCoordinates = false; // whether input coordinates are geocentric (i.e. Z can't be interpreted as elevation)

    void init( Qgis::AltitudeClamping clamping, Qgis::AltitudeBinding binding, float height, const Qgs3DRenderContext &renderContext, const QgsVector3D &chunkOrigin );

    void addLineString( const QgsLineString &lineString, float extraHeightOffset = 0, bool closePolygon = false );

    void addVerticalLines( const QgsLineString &lineString, float verticalLength, float extraHeightOffset = 0 );

    QByteArray createPointABuffer() const;
    QByteArray createPointBBuffer() const;
    QByteArray createPointPrevBuffer() const;
    QByteArray createPointNextBuffer() const;

    QByteArray createJoinPointABuffer() const;
    QByteArray createJoinPointBBuffer() const;
    QByteArray createJoinPointCBuffer() const;

    Qt3DCore::QGeometry *createGeometry( Qt3DCore::QNode *parent );
    Qt3DCore::QGeometry *createJoinGeometry( Qt3DCore::QNode *parent );
    Qt3DCore::QEntity *createSegmentEntity( QgsMaterial *material, const QgsAbstractMaterialSettings *materialSettings = nullptr, const QgsAbstractMaterial3DHandler *materialHandler = nullptr );
    Qt3DCore::QEntity *createJoinEntity(
      const QgsLineMaterial *segmentMaterial, const QgsAbstractMaterialSettings *materialSettings = nullptr, const QgsAbstractMaterial3DHandler *materialHandler = nullptr
    );
};

/// @endcond

#endif // QGSLINEVERTEXDATA_P_H
