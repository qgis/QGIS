#ifndef QGSMESHTERRAINGENERATOR_H
#define QGSMESHTERRAINGENERATOR_H

#include "qgsmaplayerref.h"
#include "qgsmesh3dsymbol.h"
#include "qgsmeshlayer.h"
#include "qgstriangularmesh.h"
#include "qgsterraingenerator.h"
#include "qgsterraintileloader_p.h"

#define SIP_NO_FILE

class QgsMeshTerrainTileLoader: public QgsTerrainTileLoader
{
  public:
    QgsMeshTerrainTileLoader( QgsTerrainEntity *terrain,
                              QgsChunkNode *node,
                              const QgsTriangularMesh &triangularMesh,
                              const QgsRectangle &extent,
                              const QgsMesh3DSymbol &symbol );

    Qt3DCore::QEntity *createEntity( Qt3DCore::QEntity *parent ) override;

  private:
    QgsRectangle mExtent;
    QgsTriangularMesh mTriangularMesh;
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

    void setCrs( const QgsCoordinateReferenceSystem &crs, const QgsCoordinateTransformContext &context );

    QgsChunkLoader *createChunkLoader( QgsChunkNode *node ) const override;
    float rootChunkError( const Qgs3DMapSettings &map ) const override {Q_UNUSED( map ); return 0;}
    void rootChunkHeightRange( float &hMin, float &hMax ) const override;
    void resolveReferences( const QgsProject &project ) override;
    QgsTerrainGenerator *clone() const override;
    Type type() const override;
    QgsRectangle extent() const override;
    void writeXml( QDomElement &elem ) const override;
    void readXml( const QDomElement &elem ) override;

  private:
    //! source layer for heights
    QgsMapLayerRef mLayer;
    QgsCoordinateReferenceSystem mCrs;
    QgsCoordinateTransformContext mTransformContext;
    QgsMesh3DSymbol mSymbol;

};

#endif // QGSMESHTERRAINGENERATOR_H
