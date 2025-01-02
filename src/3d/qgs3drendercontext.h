/***************************************************************************
  qgs3drendercontext.h
  --------------------------------------
  Date                 : August 2024
  Copyright            : (C) 2024 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DRENDERCONTEXT_H
#define QGS3DRENDERCONTEXT_H

#include "qgis_3d.h"
#include "qgsvector3d.h"
#include "qgsrectangle.h"
#include "qgsrange.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransformcontext.h"
#include "qgsexpressioncontext.h"
#include "qgsabstractterrainsettings.h"

#include <QColor>

class QgsTerrainGenerator;
class Qgs3DMapSettings;

#define SIP_NO_FILE

/**
 * \ingroup 3d
 * \brief Rendering context for preparation of 3D entities.
 *
 * Contains a snapshot of properties from Qgs3DMapSettings, in a thread-safe, cheap-to-copy structure.
 *
 * \warning For thread safety, no QObject based properties should be attached to this class.
 *
 * \note Not available in Python bindings.
 *
 * \since QGIS 3.40
 */
class _3D_EXPORT Qgs3DRenderContext
{
  public:
    Qgs3DRenderContext() = default;
    ~Qgs3DRenderContext();
    Qgs3DRenderContext( const Qgs3DRenderContext &other );
    Qgs3DRenderContext &operator=( const Qgs3DRenderContext &other );

    /**
     * Creates an initialized Qgs3DRenderContext instance from given Qgs3DMapSettings.
     */
    static Qgs3DRenderContext fromMapSettings( const Qgs3DMapSettings *mapSettings );

    /**
     * Returns the coordinate reference system used in the 3D scene.
     */
    QgsCoordinateReferenceSystem crs() const { return mCrs; }

    /**
     * Returns the coordinate transform context, which stores various
     * information regarding which datum transforms should be used when transforming points
     * from a source to destination coordinate reference system.
     */
    QgsCoordinateTransformContext transformContext() const { return mTransformContext; }

    /**
     * Returns the 3D scene's 2D extent in the 3D scene's CRS
     *
     * \see crs()
     */
    QgsRectangle extent() const { return mExtent; }

    /**
     * Returns coordinates in map CRS at which 3D scene has origin (0,0,0)
     */
    QgsVector3D origin() const { return mOrigin; }

    /**
     * Returns the temporal range for the map.
    */
    const QgsDateTimeRange &temporalRange() const { return mTemporalRange; }

    /**
     * Returns color used for selected features
     */
    QColor selectionColor() const { return mSelectionColor; }

    /**
     * Returns DPI used for conversion between real world units (e.g. mm) and pixels
     * Default value is 96
     */
    double outputDpi() const { return mDpi; }

    /**
     * Returns the camera lens' field of view.
     */
    float fieldOfView() const { return mFieldOfView; }

    /**
     * Converts map coordinates to 3D world coordinates (applies offset and turns (x,y,z) into (x,-z,y)).
     *
     * \see worldToMapCoordinates()
     */
    QgsVector3D mapToWorldCoordinates( const QgsVector3D &mapCoords ) const;

    /**
     * Converts 3D world coordinates to map coordinates (applies offset and turns (x,y,z) into (x,-z,y)).
     *
     * \see mapToWorldCoordinates()
     */
    QgsVector3D worldToMapCoordinates( const QgsVector3D &worldCoords ) const;

    /**
     * Returns whether the 2D terrain surface will be rendered.
     */
    bool terrainRenderingEnabled() const { return mTerrainRenderingEnabled; }

    /**
     * Returns the terrain settings.
     */
    const QgsAbstractTerrainSettings *terrainSettings() const;

    /**
     * Returns the terrain generator.
     */
    QgsTerrainGenerator *terrainGenerator() const { return mTerrainGenerator; }

    /**
     * Sets the expression context. This context is used for all expression evaluation
     * associated with this render context.
     * \see expressionContext()
     */
    void setExpressionContext( const QgsExpressionContext &context ) { mExpressionContext = context; }

    /**
     * Gets the expression context. This context should be used for all expression evaluation
     * associated with this render context.
     * \see setExpressionContext()
     */
    QgsExpressionContext &expressionContext() { return mExpressionContext; }

    /**
     * Gets the expression context (const version). This context should be used for all expression evaluation
     * associated with this render context.
     * \see setExpressionContext()
     * \note not available in Python bindings
     */
    const QgsExpressionContext &expressionContext() const SIP_SKIP { return mExpressionContext; }

  private:
    QgsCoordinateReferenceSystem mCrs; //!< Destination coordinate system of the world
    //! Coordinate transform context
    QgsCoordinateTransformContext mTransformContext;
    //! Offset in map CRS coordinates at which our 3D world has origin (0,0,0)
    QgsVector3D mOrigin;
    QgsRectangle mExtent; //!< 2d extent used to limit the 3d view
    QgsDateTimeRange mTemporalRange;
    QColor mSelectionColor;     //!< Color to be used for selected map features
    double mDpi = 96;           //!< Dot per inch value for the screen / painter
    float mFieldOfView = 45.0f; //!< Camera lens field of view value
    bool mTerrainRenderingEnabled = true;
    std::unique_ptr<QgsAbstractTerrainSettings> mTerrainSettings;
    //! Expression context
    QgsExpressionContext mExpressionContext;

    // not owned, currently a pointer to the Qgs3DMapSettings terrain generator.
    // TODO -- fix during implementation of https://github.com/qgis/QGIS-Enhancement-Proposals/issues/301
    QgsTerrainGenerator *mTerrainGenerator = nullptr; //!< Implementation of the terrain generation
};


#endif // QGS3DRENDERCONTEXT_H
