/***************************************************************************
  qgistindemterraingenerator.h
  --------------------------------------
  Date                 : october 2019
  Copyright            : (C) 2019 by Vincent Cloarec
  Email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTINDEMTERRAINGENERATOR_H
#define QGSTINDEMTERRAINGENERATOR_H

#include <tuple>
#include <limits>

#include "qgsmeshlayer.h"

#include "qgsterraingenerator.h"
#include "qgsterraintileloader_p.h"
#include "qgstindemterraintilegeometry_p.h"

/**
 * \ingroup 3d
 * Chunk loader for TIN DEM terrain tiles.
 * \since QGIS 3.12
 */
class QgsTinDemTerrainTileLoader: public QgsTerrainTileLoader
{
  public:
    //! Constructs loader for the given chunk node
    QgsTinDemTerrainTileLoader( QgsTerrainEntity *terrain, QgsChunkNode *node, QgsMeshLayer *layer );

    Qt3DCore::QEntity *createEntity( Qt3DCore::QEntity *parent ) override;

  private:
    QgsMeshLayer *mLayer = nullptr;
    QgsTriangularMeshTile mMeshTile;
};

/**
 * \ingroup 3d
 * Implementation of terrain generator that uses a mesh layer with Z value to build terrain.
 * \since QGIS 3.12
 */
class _3D_EXPORT QgsTinDemTerrainGenerator: public QgsTerrainGenerator
{
  public:
    //! constructor
    QgsTinDemTerrainGenerator() = default;

    //! Returns the mesh layer associated with
    QgsMeshLayer *layer() const {return mLayer;}
    //! Sets the mesh layer associated with
    void setLayer( QgsMeshLayer *layer );

    QgsChunkLoader *createChunkLoader( QgsChunkNode *node ) const override;
    QgsTerrainGenerator *clone() const override;
    Type type() const override;
    QgsRectangle extent() const override;

    void writeXml( QDomElement &elem ) const override {( void )elem;}
    void readXml( const QDomElement &elem ) override {( void )elem;}

  private:
    void updateTilingScheme();


    QgsMeshLayer *mLayer = nullptr;
};


#endif // QGSTINDEMTERRAINGENERATOR_H
