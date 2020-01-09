#ifndef QGSMESHTERRAINGENERATOR_H
#define QGSMESHTERRAINGENERATOR_H

#include "qgsmaplayerref.h"
#include "qgsmesh3dsymbol.h"
#include "qgsmeshlayer.h"
#include "qgsterraingenerator.h"
#include "qgsterraintileloader_p.h"



class QgsMeshTerrainTileLoader: public QgsTerrainTileLoader
{
  public:
    QgsMeshTerrainTileLoader( QgsTerrainEntity *terrain, QgsChunkNode *node, QgsMeshLayer *meshLayer, const QgsMesh3DSymbol &symbol ):
      QgsTerrainTileLoader( terrain, node ),
      mMeshLayer( meshLayer ),
      mSymbol( symbol )
    {
      loadTexture();
    }

    Qt3DCore::QEntity *createEntity( Qt3DCore::QEntity *parent ) override;

  private:
    QgsMeshLayer *mMeshLayer = nullptr;
    QgsMesh3DSymbol mSymbol;
};


class _3D_EXPORT QgsMeshTerrainGenerator: public QgsTerrainGenerator
{
  public:
    //! Creates mesh terrain generator object
    QgsMeshTerrainGenerator() = default;

    QgsMeshLayer *meshLayer() const;
    void setLayer( QgsMeshLayer *layer );

    QgsMesh3DSymbol symbol() const;
    void setSymbol( const QgsMesh3DSymbol &symbol );

    void updateGenerator() {}

    QgsChunkLoader *createChunkLoader( QgsChunkNode *node ) const override;
    float rootChunkError( const Qgs3DMapSettings &map ) const override {Q_UNUSED( map ); return 0;}
    //void rootChunkHeightRange( float &hMin, float &hMax ) const override;
    void resolveReferences( const QgsProject &project ) override;
    //QgsAABB rootChunkBbox( const Qgs3DMapSettings &map ) const override;
    QgsTerrainGenerator *clone() const override;
    Type type() const override;
    QgsRectangle extent() const override;
    void writeXml( QDomElement &elem ) const override {}
    void readXml( const QDomElement &elem ) override {}



  private:
    //! source layer for heights
    QgsMapLayerRef mLayer;
    Qgs3DMapSettings *mMapSettings;
    QgsMesh3DSymbol mSymbol;

};

#endif // QGSMESHTERRAINGENERATOR_H
