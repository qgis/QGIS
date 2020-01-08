#ifndef QGSMESHTERRAINGENERATOR_H
#define QGSMESHTERRAINGENERATOR_H

#include "qgsterraingenerator.h"
#include "qgsterraintileloader_p.h"

#include "qgsmeshlayer.h"

class QgsMeshTerrainTileLoader: public QgsTerrainTileLoader
{
  public:
    QgsMeshTerrainTileLoader( QgsTerrainEntity *terrain, QgsChunkNode *node, QgsMeshLayer *meshLayer ):
      QgsTerrainTileLoader( terrain, node ),
      mMeshLayer( meshLayer )
    {
      loadTexture();
    }

    Qt3DCore::QEntity *createEntity( Qt3DCore::QEntity *parent ) override;

  private:
    QgsMeshLayer *mMeshLayer = nullptr;
};


class _3D_EXPORT QgsMeshTerrainGenerator: public QgsTerrainGenerator
{
  public:
    //! Creates mesh terrain generator object
    QgsMeshTerrainGenerator() = default;

    QgsMeshLayer *meshLayer() const;
    void setLayer( QgsMeshLayer *layer );

    QgsChunkLoader *createChunkLoader( QgsChunkNode *node ) const override;

    float rootChunkError( const Qgs3DMapSettings &map ) const override {return 0;}

    QgsTerrainGenerator *clone() const override;
    Type type() const override;
    QgsRectangle extent() const override;
    void writeXml( QDomElement &elem ) const override {}
    void readXml( const QDomElement &elem ) override {}

  private:
    QgsMeshLayer *mMeshLayer;
};

#endif // QGSMESHTERRAINGENERATOR_H
