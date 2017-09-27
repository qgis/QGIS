#ifndef QGSTERRAINTILELOADER_P_H
#define QGSTERRAINTILELOADER_P_H

/// @cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include "qgschunkloader_p.h"

#include <QImage>
#include "qgsrectangle.h"

class QgsTerrainEntity;
class QgsTerrainTileEntity;


/** \ingroup 3d
 * Base class for chunk loaders for terrain tiles.
 * Adds functionality for asynchronous rendering of terrain tile map texture and access to the terrain entity.
 * \since QGIS 3.0
 */
class QgsTerrainTileLoader : public QgsChunkLoader
{
  public:
    //! Constructs loader for a chunk node
    QgsTerrainTileLoader( QgsTerrainEntity *terrain, QgsChunkNode *mNode );

  protected:
    //! Starts asynchronous rendering of map texture
    void loadTexture();
    //! Creates material component for the entity with the rendered map as a texture
    void createTextureComponent( QgsTerrainTileEntity *entity );
    //! Gives access to the terain entity
    QgsTerrainEntity *terrain() { return mTerrain; }

  private slots:
    void onImageReady( int jobId, const QImage &image );

  private:
    QgsTerrainEntity *mTerrain;
    QgsRectangle mExtentMapCrs;
    QString mTileDebugText;
    int mTextureJobId;
    QImage mTextureImage;
};

/// @endcond

#endif // QGSTERRAINTILELOADER_P_H
