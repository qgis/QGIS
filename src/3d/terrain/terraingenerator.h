#ifndef FLATTERRAINTILE_H
#define FLATTERRAINTILE_H

#include "tilingscheme.h"
#include "chunkloader.h"

class AABB;
class Map3D;
class QgsRectangle;
class Terrain;


class QDomElement;
class QDomDocument;
class QgsProject;

/**
 * Base class for generators of terrain. All terrain generators are tile based
 * to support hierarchical level of detail. Tiling scheme of a generator is defined
 * by the generator itself. Terrain generators are asked to produce new terrain tiles
 * whenever that is deemed necessary by the terrain controller (that caches generated tiles).
 */
class TerrainGenerator : public ChunkLoaderFactory
{
  public:

    enum Type
    {
      Flat,
      Dem,
      QuantizedMesh
    };

    virtual ~TerrainGenerator() {}

    void setTerrain( Terrain *t ) { mTerrain = t; }

    //! Makes a copy of the current instance
    virtual TerrainGenerator *clone() const = 0;

    //! What texture generator implementation is this
    virtual Type type() const = 0;

    //! extent of the terrain in terrain's CRS
    virtual QgsRectangle extent() const = 0;

    //! Returns bounding box of the root chunk
    virtual AABB rootChunkBbox( const Map3D &map ) const;

    //! Returns error of the root chunk in world coordinates
    virtual float rootChunkError( const Map3D &map ) const;

    //! Returns height range of the root chunk in world coordinates
    virtual void rootChunkHeightRange( float &hMin, float &hMax ) const;

    //! Returns height at (x,y) in terrain's CRS
    virtual float heightAt( double x, double y, const Map3D &map ) const;

    //! Write terrain generator's configuration to XML
    virtual void writeXml( QDomElement &elem ) const = 0;

    //! Read terrain generator's configuration from XML
    virtual void readXml( const QDomElement &elem ) = 0;

    //! After read of XML, resolve references to any layers that have been read as layer IDs
    virtual void resolveReferences( const QgsProject &project ) { Q_UNUSED( project ); }

    static QString typeToString( Type type );

    QgsCoordinateReferenceSystem crs() const { return terrainTilingScheme.crs; }

    TilingScheme terrainTilingScheme;   //!< Tiling scheme of the terrain

    Terrain *mTerrain = nullptr;
};


#endif // FLATTERRAINTILE_H
