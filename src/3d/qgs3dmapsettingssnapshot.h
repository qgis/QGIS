/***************************************************************************
  qgs3dmapsettingssnapshot.h
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

#ifndef QGS3DMAPSETTINGSSNAPSHOT_H
#define QGS3DMAPSETTINGSSNAPSHOT_H

#include "qgis_3d.h"
#include "qgsvector3d.h"
#include "qgsrectangle.h"
#include "qgsrange.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransformcontext.h"

#include <QColor>

class QgsTerrainGenerator;

#define SIP_NO_FILE

/**
 * \ingroup 3d
 * \brief A snapshot of properties from Qgs3DMapSettings, in a thread-safe, cheap-to-copy structure.
 *
 * \note Not available in Python bindings.
 *
 * \since QGIS 3.40
 */
class _3D_EXPORT Qgs3DMapSettingsSnapshot
{

  public:

    Qgs3DMapSettingsSnapshot() = default;

    /**
     * Sets the coordinate reference system used in the 3D scene
     *
     * \see crs()
     */
    void setCrs( const QgsCoordinateReferenceSystem &crs ) { mCrs = crs; }

    /**
     * Returns the coordinate reference system used in the 3D scene.
     *
     * \see setCrs()
     */
    QgsCoordinateReferenceSystem crs() const { return mCrs; }

    /**
     * Returns the coordinate transform context, which stores various
     * information regarding which datum transforms should be used when transforming points
     * from a source to destination coordinate reference system.
     *
     * \see setTransformContext()
     */
    QgsCoordinateTransformContext transformContext() const { return mTransformContext; }

    /**
     * Sets the coordinate transform \a context, which stores various
     * information regarding which datum transforms should be used when transforming points
     * from a source to destination coordinate reference system.
     *
     * \see transformContext()
     */
    void setTransformContext( const QgsCoordinateTransformContext &context ) { mTransformContext = context; }

    /**
     * Returns the 3D scene's 2D extent in the 3D scene's CRS
     *
     * \see crs()
     * \see setExtent()
     */
    QgsRectangle extent() const { return mExtent; }

    /**
     * Sets the 3D scene's 2D \a extent in the 3D scene's CRS, while also setting the scene's origin to the extent's center
     * This needs to be called during initialization, as terrain will only be generated
     * within this extent and layer 3D data will only be loaded within this extent too.
     *
     * \see extent()
     * \see setOrigin()
     * \see setCrs()
     */
    void setExtent( const QgsRectangle &extent ) { mExtent = extent; }

    /**
     * Sets coordinates in map CRS at which our 3D world has origin (0,0,0)
     *
     * We move the 3D world origin to the center of the extent of our terrain: this is done
     * to minimize the impact of numerical errors when operating with 32-bit floats.
     * Unfortunately this is not enough when working with a large area (still results in jitter
     * with scenes spanning hundreds of kilometers and zooming in a lot).
     *
     * Need to look into more advanced techniques like "relative to center" or "relative to eye"
     * to improve the precision.
     *
     * \see origin()
     */
    void setOrigin( const QgsVector3D &origin ) { mOrigin = origin; }

    /**
     * Returns coordinates in map CRS at which 3D scene has origin (0,0,0)
     *
     * \see setOrigin()
     */
    QgsVector3D origin() const { return mOrigin; }

    /**
     * Sets the temporal \a range for the map.
     *
     * \see temporalRange()
    */
    void setTemporalRange( const QgsDateTimeRange &range ) { mTemporalRange = range; }

    /**
     * Returns the temporal range for the map.
     *
     * \see setTemporalRange()
    */
    const QgsDateTimeRange &temporalRange() const { return mTemporalRange; }

    /**
     * Sets color used for selected features
     *
     * \see selectionColor()
     */
    void setSelectionColor( const QColor &color ) { mSelectionColor = color; }

    /**
     * Returns color used for selected features
     *
     * \see setSelectionColor()
     */
    QColor selectionColor() const { return mSelectionColor; }

    /**
     * Sets DPI used for conversion between real world units (e.g. mm) and pixels
     *
     * \see outputDpi()
     */
    void setOutputDpi( const double dpi ) {mDpi = dpi;}

    /**
     * Returns DPI used for conversion between real world units (e.g. mm) and pixels
     * Default value is 96
     *
     * \see setOutputDpi()
     */
    double outputDpi() const { return mDpi; }

    /**
     * Returns the camera lens' field of view.
     *
     * \see setFieldOfView()
     */
    float fieldOfView() const { return mFieldOfView; }

    /**
     * Sets the camera lens' field of view.
     *
     * \see fieldOfView()
     */
    void setFieldOfView( const float fieldOfView ) { mFieldOfView = fieldOfView; }

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
     * \see setTerrainRenderingEnabled()
     */
    bool terrainRenderingEnabled() const { return mTerrainRenderingEnabled; }

    /**
     * Sets whether the 2D terrain surface will be rendered in.
     * \see terrainRenderingEnabled()
     */
    void setTerrainRenderingEnabled( bool terrainRenderingEnabled ) { mTerrainRenderingEnabled = terrainRenderingEnabled; }

    /**
     * Sets the vertical scale (exaggeration) of terrain.
     * (1 = true scale, > 1 = hills get more pronounced)
     *
     * \see terrainVerticalScale()
     */
    void setTerrainVerticalScale( double zScale ) { mTerrainVerticalScale = zScale; }

    /**
     * Returns vertical scale (exaggeration) of terrain
     * \see setTerrainVerticalScale()
     */
    double terrainVerticalScale() const { return mTerrainVerticalScale; }

    /**
     * Sets terrain generator.
     *
     * Ownership is NOT transferred, and belongs to the Qgs3DMapSettings.
     *
     * \see terrainGenerator()
     */
    void setTerrainGenerator( QgsTerrainGenerator *gen ) { mTerrainGenerator = gen; }

    /**
     * Returns the terrain generator.
     *
     * \see setTerrainGenerator()
     */
    QgsTerrainGenerator *terrainGenerator() const { return mTerrainGenerator; }


  private:
    QgsCoordinateReferenceSystem mCrs;   //!< Destination coordinate system of the world
    //! Coordinate transform context
    QgsCoordinateTransformContext mTransformContext;
    //! Offset in map CRS coordinates at which our 3D world has origin (0,0,0)
    QgsVector3D mOrigin;
    QgsRectangle mExtent; //!< 2d extent used to limit the 3d view
    QgsDateTimeRange mTemporalRange;
    QColor mSelectionColor; //!< Color to be used for selected map features
    double mDpi = 96;  //!< Dot per inch value for the screen / painter
    float mFieldOfView = 45.0f; //!< Camera lens field of view value
    bool mTerrainRenderingEnabled = true;
    double mTerrainVerticalScale = 1;   //!< Multiplier of terrain heights to make the terrain shape more pronounced

    // not owned, currently a pointer to the Qgs3DMapSettings terrain generator.
    // TODO -- fix during implementation of https://github.com/qgis/QGIS-Enhancement-Proposals/issues/301
    QgsTerrainGenerator *mTerrainGenerator = nullptr;  //!< Implementation of the terrain generation
};


#endif // QGS3DMAPSETTINGSSNAPSHOT_H
