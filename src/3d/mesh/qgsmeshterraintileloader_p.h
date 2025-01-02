/***************************************************************************
                         qgsmeshterraintileloader_p.h
                         -------------------------
    begin                : September 2024
    copyright            : (C) 2024 by Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESHTERRAINTILELOADER_H
#define QGSMESHTERRAINTILELOADER_H

#include "qgsmesh3dsymbol.h"
#include "qgstriangularmesh.h"
#include "qgsterraintileloader.h"

#define SIP_NO_FILE

///@cond PRIVATE

//! Chunk loader for mesh terrain implementation
class QgsMeshTerrainTileLoader : public QgsTerrainTileLoader
{
    Q_OBJECT
  public:
    //! Construct the loader for a node
    QgsMeshTerrainTileLoader( QgsTerrainEntity *terrain, QgsChunkNode *node, const QgsTriangularMesh &triangularMesh, const QgsMesh3DSymbol *symbol );

    //! Create the 3D entity and returns it
    Qt3DCore::QEntity *createEntity( Qt3DCore::QEntity *parent ) override;

  private:
    QgsTriangularMesh mTriangularMesh;
    std::unique_ptr<QgsMesh3DSymbol> mSymbol;
};

///@endcond

#endif
