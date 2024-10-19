/***************************************************************************
  qgsterraintileentity_p.h
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

#ifndef QGSTERRAINTILEENTITY_P_H
#define QGSTERRAINTILEENTITY_P_H

/// @cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

class QgsTerrainTextureImage;

#include <Qt3DCore/QEntity>

#include "qgschunknode.h"

/**
 * \ingroup 3d
 * \brief Base class for 3D entities representing one tile of terrain.
 * It contains pointer to tile's texture image.
 *
 */
class QgsTerrainTileEntity : public Qt3DCore::QEntity
{
    Q_OBJECT
  public:
    //! Constructs the entity, optionally with a parent that will own it
    QgsTerrainTileEntity( QgsChunkNodeId tileId, Qt3DCore::QNode *parent = nullptr )
      : Qt3DCore::QEntity( parent )
      , mTileId( tileId )
    {
    }

    //! Returns coordinates of the tile
    QgsChunkNodeId tileId() const { return mTileId; }

    /**
     * Assigns texture image. Should be called when the class is being initialized.
     * Texture image is owned by the texture used by the entity.
     */
    void setTextureImage( QgsTerrainTextureImage *textureImage ) { mTextureImage = textureImage; }
    //! Returns assigned texture image
    QgsTerrainTextureImage *textureImage() { return mTextureImage; }

  private:
    QgsChunkNodeId mTileId;
    QgsTerrainTextureImage *mTextureImage = nullptr;
};

/// @endcond

#endif // QGSTERRAINTILEENTITY_P_H
