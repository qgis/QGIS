/***************************************************************************
  qgsterraingenerator.h
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

#ifndef QGSTERRAINGENERATOR_H
#define QGSTERRAINGENERATOR_H

#include "qgis_3d.h"

#include "qgstilingscheme.h"
#include "qgschunkloader.h"

class QgsAABB;
class Qgs3DMapSettings;
class Qgs3DRenderContext;
class QgsRectangle;
class QgsTerrainEntity;
class QgsCoordinateTransformContext;

class QDomElement;
class QDomDocument;
class QgsProject;

#define SIP_NO_FILE

/**
 * \ingroup 3d
 * \brief Base class for generators of terrain.
 *
 * All terrain generators are tile based
 * to support hierarchical level of detail. Tiling scheme of a generator is defined
 * by the generator itself. Terrain generators are asked to produce new terrain tiles
 * whenever that is deemed necessary by the terrain controller (that caches generated tiles).
 *
 * \note Not available in Python bindings
 *
 */
class _3D_EXPORT QgsTerrainGenerator : public QgsQuadtreeChunkLoaderFactory
{
    Q_OBJECT
  public:
    //! Enumeration of the available terrain generators
    enum Type
    {
      Flat,          //!< The whole terrain is flat area
      Dem,           //!< Terrain is built from raster layer with digital elevation model
      Online,        //!< Terrain is built from downloaded tiles with digital elevation model
      Mesh,          //!< Terrain is built from mesh layer with z value on vertices
      QuantizedMesh, //!< Terrain is built from quantized mesh tiles
    };

    //! Sets terrain entity for the generator (does not transfer ownership)
    virtual void setTerrain( QgsTerrainEntity *t ) { mTerrain = t; }

    //! Makes a copy of the current instance
    virtual QgsTerrainGenerator *clone() const = 0 SIP_FACTORY;

    //! What texture generator implementation is this
    virtual Type type() const = 0;

    //! sets the extent of the terrain in terrain's CRS
    virtual void setExtent( const QgsRectangle &extent ) { Q_UNUSED( extent ) }

    //! extent of the terrain's root chunk in terrain's CRS
    virtual QgsRectangle rootChunkExtent() const = 0;

    //! Returns 3D box (in map coordinates) of the root chunk
    virtual QgsBox3D rootChunkBox3D( const Qgs3DMapSettings &map ) const;

    //! Returns error of the root chunk in world coordinates
    virtual float rootChunkError( const Qgs3DMapSettings &map ) const;

    //! Returns height range of the root chunk in world coordinates
    virtual void rootChunkHeightRange( float &hMin, float &hMax ) const;

    //! Returns height at (x,y) in map's CRS
    virtual float heightAt( double x, double y, const Qgs3DRenderContext &context ) const;

    //! Converts terrain generator type enumeration into a string
    static QString typeToString( Type type );

    //! Returns tiling scheme of the terrain
    const QgsTilingScheme &tilingScheme() const { return mTerrainTilingScheme; }

    /**
     * Sets the CRS associated with the terrain.
     *
     * \see crs()
     */
    virtual void setCrs( const QgsCoordinateReferenceSystem &crs, const QgsCoordinateTransformContext &context );

    //! Returns CRS of the terrain
    virtual QgsCoordinateReferenceSystem crs() const { return mTerrainTilingScheme.crs(); }

    //! Returns whether the terrain generator is valid
    bool isValid() const;

  signals:

    //! Emitted when the terrain changed (for example, raster DEM or mesh have data changed)
    void terrainChanged();

  protected:
    QgsTilingScheme mTerrainTilingScheme; //!< Tiling scheme of the terrain
    QgsTerrainEntity *mTerrain = nullptr;
    QgsRectangle mExtent;

    bool mIsValid = true;
};


#endif // QGSTERRAINGENERATOR_H
