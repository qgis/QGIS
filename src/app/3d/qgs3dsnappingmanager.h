/***************************************************************************
  qgs3dsnappingmanager.h
  --------------------------------------
  Date                 : November 2025
  Copyright            : (C) 2025 by Benoit De Mezzo
  Email                : benoit dot de dot mezzo at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGS3DSNAPPINGMANAGER_H
#define QGS3DSNAPPINGMANAGER_H

#include <memory>

#include "qgsfeatureid.h"
#include "qgspoint.h"
#include "qgsrubberband3d.h"

#include <Qt3DCore/QEntity>

class Qgs3DMapCanvasWidget;
class Qgs3DMapCanvas;
class QgsMapLayer;
class QgsFeature;

namespace Qt3DCore
{
  class QNode;
} //namespace Qt3DCore

/**
 * \ingroup qgis_3d
 * 3D snapping class.
 *
 * 4 snapping types are available and can be combined.
 * The snapper can be used as a screen to map coordinate converter
 * when handling mouse movements whatever the snapping type.
 *
 * \since QGIS 4.0
 */
class Qgs3DSnappingManager
{
  public:
    /**
     * 3D snapping type
     */
    enum SnappingType3D
    {
      Off = 1 << 0,        //!< Snap anything
      Vertex = 1 << 1,     //!< Snap any face vertex
      AlongEdge = 1 << 2,  //!< Snap nearest point on an edge
      MiddleEdge = 1 << 3, //!< Snap the middle point on an edge
      CenterFace = 1 << 4, //!< Snap the face center
    };

    /**
     * Default constructor
     * \param tolerance snapping tolerance in map unit
     */
    Qgs3DSnappingManager( double tolerance = 0.0 );

    /**
     * Default destructor
     */
    virtual ~Qgs3DSnappingManager() = default;

    /**
     * Makes the snapper ready
     */
    void start( Qgs3DMapCanvas *canvas );

    void reset();

    /**
     * Ends the snapper. Remove the root entity and all highlighted entities if any.
     */
    void finish();

    /**
     * Update snapping type
     * \param type a binary combinaison of snapping types
     */
    void setSnappingType( SnappingType3D type );

    /**
     * \return current snapping type
     */
    SnappingType3D snappingType() const { return mType; }

    /**
     * Update tolerance
     * \param tolerance new tolerance in map unit
     */
    void setTolerance( double tolerance );

    /**
     * \return current tolerance
     */
    double tolerance() const { return mTolerance; }

    /**
     * Try to snap an object in the 3D canvas according to the snapping type.
     *
     * We use ray casting from the \a screenPos to find the nearest triangle in the 3D scene matching the snapping types.
     * If the snapping type is Off we return the best hit found in the 3D map.
     *
     * \param screenPos screen position to start the search from
     * \param success it will be set to true if the operation succeeds, false otherwise.
     * \param highlightSnappedPoint if true, will highlight the snapped point
     * \return the snapped point in map coordinates or invalid point if nothing found
     */
    QgsPoint screenToMap( const QPoint &screenPos, bool *success, bool highlightSnappedPoint = true );

  private:
    /**
     *
     * \param screenPos screen position to start the search from
     * \param success will be set to false when ray casting failed, else true
     * \param snapFound will be set to Off when nothing is found, else the best snapping type
     * \param layerId will contain the layer id of the found entity, else empty
     * \param nearestFid will contain the feature id of the found entity, else empty
     * \return the snapped point in WORLD coordinates or invalid point if nothing found
     */
    QVector3D screenToWorld( const QPoint &screenPos, bool *success, SnappingType3D *snapFound, QString *layerId, QgsFeatureId *nearestFid ) const;

    void updateHighlightedPoint( const QVector3D &highlightedPointInWorld, SnappingType3D snapFound );

  private:
    SnappingType3D mType = SnappingType3D::Off;
    Qgs3DMapCanvas *mCanvas = nullptr;
    double mTolerance = 0.0;

    std::unique_ptr<QgsRubberBand3D> mHighlightedPointBB = nullptr;
    QVector3D mPreviousHighlightedPoint;
};

#endif // QGS3DSNAPPINGMANAGER_H
