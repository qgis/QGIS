/***************************************************************************
  qgsterraintileloader.h
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

#ifndef QGSTERRAINTILELOADER_H
#define QGSTERRAINTILELOADER_H

/// @cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#define SIP_NO_FILE

#include "qgschunkloader.h"

#include <QImage>
#include "qgsrectangle.h"

class QgsPhongMaterialSettings;
class QgsTerrainEntity;
class QgsTerrainTileEntity;

namespace Qt3DRender
{
  class QTexture2D;
}


/**
 * \ingroup 3d
 * \brief Base class for chunk loaders for terrain tiles.
 *
 * Adds functionality for asynchronous rendering of terrain tile map texture and access to the terrain entity.
 */
class QgsTerrainTileLoader : public QgsChunkLoader
{
    Q_OBJECT

  public:
    //! Constructs loader for a chunk node
    QgsTerrainTileLoader( QgsTerrainEntity *terrain, QgsChunkNode *mNode );

  protected:
    //! Starts asynchronous rendering of map texture
    void loadTexture();

    //! Creates a new texture that is linked to the entity
    Qt3DRender::QTexture2D *createTexture( QgsTerrainTileEntity *entity );
    //! Creates material component for the entity with the rendered map as a texture
    void createTextureComponent( QgsTerrainTileEntity *entity, bool isShadingEnabled, const QgsPhongMaterialSettings &shadingMaterial, bool useTexture );
    //! Gives access to the terain entity
    QgsTerrainEntity *terrain() { return mTerrain; }
    // Emit finished by default when texture is loaded.
    // Subclasses can override this and emit finish later.
    virtual void onTextureLoaded();

  protected slots:
    void onImageReady( int jobId, const QImage &image );

  private:
    QgsTerrainEntity *mTerrain = nullptr;
    QgsRectangle mExtentMapCrs;
    QString mTileDebugText;
    int mTextureJobId = -1;
    QImage mTextureImage;
};

/// @endcond

#endif // QGSTERRAINTILELOADER_H
