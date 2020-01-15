#ifndef QGSMESHTERRAINGENERATOR_H
#define QGSMESHTERRAINGENERATOR_H

#include "qgsmaplayerref.h"
#include "qgsmesh3dsymbol.h"
#include "qgsmeshlayer.h"
#include "qgstriangularmesh.h"
#include "qgsterraingenerator.h"
#include "qgsterraintileloader_p.h"

#define SIP_NO_FILE

///@cond PRIVATE

//! Chunk loader for mesh terrain implementation
class QgsMeshTerrainTileLoader: public QgsTerrainTileLoader
{
  public:
    //! Construct the loader for a node
    QgsMeshTerrainTileLoader( QgsTerrainEntity *terrain,
                              QgsChunkNode *node,
                              const QgsTriangularMesh &triangularMesh,
                              const QgsRectangle &extent,
                              const QgsMesh3DSymbol &symbol );

    //! Create the 3D entity and returns it
    Qt3DCore::QEntity *createEntity( Qt3DCore::QEntity *parent ) override;

  private:
    QgsRectangle mExtent;
    QgsTriangularMesh mTriangularMesh;
    QgsMesh3DSymbol mSymbol;
};

///@endcond

/**
 * \ingroup 3d
 * Implementation of terrain generator that uses the Z values of a mesh layer to build a terrain
 * \since QGIS 3.12
 */
class _3D_EXPORT QgsMeshTerrainGenerator: public QgsTerrainGenerator
{
  public:
    //! Creates mesh terrain generator object
    QgsMeshTerrainGenerator() = default;

    //! Returns the mesh layer to be used for terrain generation
    QgsMeshLayer *meshLayer() const;
    //! Sets the mesh layer to be used for terrain generation
    void setLayer( QgsMeshLayer *layer );

    //! Returns the symbol used to render the mesh as terrain
    QgsMesh3DSymbol symbol() const;

    //! Sets the symbol used to render the mesh as terrain
    void setSymbol( const QgsMesh3DSymbol &symbol );

    //! Sets CRS of the terrain
    void setCrs( const QgsCoordinateReferenceSystem &crs, const QgsCoordinateTransformContext &context );

    QgsChunkLoader *createChunkLoader( QgsChunkNode *node ) const override SIP_FACTORY;
    float rootChunkError( const Qgs3DMapSettings &map ) const override;
    void rootChunkHeightRange( float &hMin, float &hMax ) const override;
    void resolveReferences( const QgsProject &project ) override;
    QgsTerrainGenerator *clone() const override SIP_FACTORY;
    Type type() const override;
    QgsRectangle extent() const override;
    void writeXml( QDomElement &elem ) const override;
    void readXml( const QDomElement &elem ) override;

  private:
    QgsMapLayerRef mLayer;
    QgsCoordinateReferenceSystem mCrs;
    QgsCoordinateTransformContext mTransformContext;
    QgsMesh3DSymbol mSymbol;

};

#endif // QGSMESHTERRAINGENERATOR_H
