/***************************************************************************
  qgsonlineterraingenerator.h
  --------------------------------------
  Date                 : March 2019
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

#ifndef QGSONLINETERRAINGENERATOR_H
#define QGSONLINETERRAINGENERATOR_H

#include "qgis_3d.h"

#include "qgsterraingenerator.h"

#include "qgscoordinatetransformcontext.h"

class QgsDemHeightMapGenerator;

/**
 * \ingroup 3d
 * Implementation of terrain generator that uses online resources to download heightmaps.
 * \since QGIS 3.8
 */
class _3D_EXPORT QgsOnlineTerrainGenerator : public QgsTerrainGenerator
{
  public:
    //! Constructor for QgsOnlineTerrainGenerator
    QgsOnlineTerrainGenerator();
    ~QgsOnlineTerrainGenerator() override;

    //! Sets extent of the terrain
    void setExtent( const QgsRectangle &extent );

    //! Sets CRS of the terrain
    void setCrs( const QgsCoordinateReferenceSystem &crs, const QgsCoordinateTransformContext &context );
    //! Returns CRS of the terrain
    QgsCoordinateReferenceSystem crs() const { return mCrs; }

    //! Sets resolution of the generator (how many elevation samples on one side of a terrain tile)
    void setResolution( int resolution ) { mResolution = resolution; updateGenerator(); }
    //! Returns resolution of the generator (how many elevation samples on one side of a terrain tile)
    int resolution() const { return mResolution; }

    //! Sets skirt height (in world units). Skirts at the edges of terrain tiles help hide cracks between adjacent tiles.
    void setSkirtHeight( float skirtHeight ) { mSkirtHeight = skirtHeight; }
    //! Returns skirt height (in world units). Skirts at the edges of terrain tiles help hide cracks between adjacent tiles.
    float skirtHeight() const { return mSkirtHeight; }

    //! Returns height map generator object - takes care of extraction of elevations from the layer)
    QgsDemHeightMapGenerator *heightMapGenerator() { return mHeightMapGenerator.get(); }

    QgsTerrainGenerator *clone() const override SIP_FACTORY;
    Type type() const override;
    QgsRectangle extent() const override;
    float heightAt( double x, double y, const Qgs3DMapSettings &map ) const override;
    void writeXml( QDomElement &elem ) const override;
    void readXml( const QDomElement &elem ) override;
    //void resolveReferences( const QgsProject &project ) override;

    QgsChunkLoader *createChunkLoader( QgsChunkNode *node ) const override SIP_FACTORY;

  private:

    void updateGenerator();

    QgsRectangle mExtent;
    QgsCoordinateReferenceSystem mCrs;
    QgsCoordinateTransformContext mTransformContext;

    //! how many vertices to place on one side of the tile
    int mResolution = 16;
    //! height of the "skirts" at the edges of tiles to hide cracks between adjacent cracks
    float mSkirtHeight = 10.f;

    std::unique_ptr<QgsDemHeightMapGenerator> mHeightMapGenerator;
};

#endif // QGSONLINETERRAINGENERATOR_H
