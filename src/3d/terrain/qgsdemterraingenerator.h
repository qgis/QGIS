/***************************************************************************
  qgsdemterraingenerator.h
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

#ifndef QGSDEMTERRAINGENERATOR_H
#define QGSDEMTERRAINGENERATOR_H

#include "qgis_3d.h"

#include "qgsterraingenerator.h"


#include <memory>

class QgsRasterLayer;
class QgsDemHeightMapGenerator;

#include "qgsmaplayerref.h"

#define SIP_NO_FILE

/**
 * \ingroup 3d
 * \brief Implementation of terrain generator that uses a raster layer with DEM to build terrain.
 *
 * \note Not available in Python bindings
 *
 * \since QGIS 3.0
 */
class _3D_EXPORT QgsDemTerrainGenerator : public QgsTerrainGenerator
{
    Q_OBJECT

  public:
    //! Constructor for QgsDemTerrainGenerator
    QgsDemTerrainGenerator() = default;
    ~QgsDemTerrainGenerator() override;

    //! Sets raster layer with elevation model to be used for terrain generation
    void setLayer( QgsRasterLayer *layer );
    //! Returns raster layer with elevation model to be used for terrain generation
    QgsRasterLayer *layer() const;

    //! Sets CRS of the terrain
    void setCrs( const QgsCoordinateReferenceSystem &crs, const QgsCoordinateTransformContext &context );

    //! Sets resolution of the generator (how many elevation samples on one side of a terrain tile)
    void setResolution( int resolution ) { mResolution = resolution; updateGenerator(); }
    //! Returns resolution of the generator (how many elevation samples on one side of a terrain tile)
    int resolution() const { return mResolution; }

    //! Sets skirt height (in world units). Skirts at the edges of terrain tiles help hide cracks between adjacent tiles.
    void setSkirtHeight( float skirtHeight ) { mSkirtHeight = skirtHeight; }
    //! Returns skirt height (in world units). Skirts at the edges of terrain tiles help hide cracks between adjacent tiles.
    float skirtHeight() const { return mSkirtHeight; }

    //! Returns height map generator object - takes care of extraction of elevations from the layer)
    QgsDemHeightMapGenerator *heightMapGenerator() { return mHeightMapGenerator; }

    QgsTerrainGenerator *clone() const override SIP_FACTORY;
    Type type() const override;
    QgsRectangle extent() const override;
    float heightAt( double x, double y, const Qgs3DMapSettings &map ) const override;
    void writeXml( QDomElement &elem ) const override;
    void readXml( const QDomElement &elem ) override;
    void resolveReferences( const QgsProject &project ) override;

    QgsChunkLoader *createChunkLoader( QgsChunkNode *node ) const override SIP_FACTORY;

  private:
    void updateGenerator();

    QgsDemHeightMapGenerator *mHeightMapGenerator = nullptr;

    QgsCoordinateReferenceSystem mCrs;

    QgsCoordinateTransformContext mTransformContext;

    //! source layer for heights
    QgsMapLayerRef mLayer;
    //! how many vertices to place on one side of the tile
    int mResolution = 16;
    //! height of the "skirts" at the edges of tiles to hide cracks between adjacent cracks
    float mSkirtHeight = 10.f;
};


#endif // QGSDEMTERRAINGENERATOR_H
