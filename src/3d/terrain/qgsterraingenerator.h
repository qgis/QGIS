#ifndef QGSTERRAINGENERATOR_H
#define QGSTERRAINGENERATOR_H

#include "qgstilingscheme.h"
#include "qgschunkloader_p.h"

class QgsAABB;
class Qgs3DMapSettings;
class QgsRectangle;
class QgsTerrainEntity;

class QDomElement;
class QDomDocument;
class QgsProject;


/** \ingroup 3d
 * Base class for generators of terrain. All terrain generators are tile based
 * to support hierarchical level of detail. Tiling scheme of a generator is defined
 * by the generator itself. Terrain generators are asked to produce new terrain tiles
 * whenever that is deemed necessary by the terrain controller (that caches generated tiles).
 * \since QGIS 3.0
 */
class QgsTerrainGenerator : public QgsChunkLoaderFactory
{
  public:

    //! Enumeration of the available terrain generators
    enum Type
    {
      Flat,           //!< The whole terrain is flat area
      Dem,            //!< Terrain is built from raster layer with digital elevation model
      QuantizedMesh,  //!< Terrain is built from downloaded tiles in quantized mesh format
    };

    virtual ~QgsTerrainGenerator() {}

    //! Sets terrain entity for the generator
    void setTerrain( QgsTerrainEntity *t ) { mTerrain = t; }

    //! Makes a copy of the current instance
    virtual QgsTerrainGenerator *clone() const = 0;

    //! What texture generator implementation is this
    virtual Type type() const = 0;

    //! extent of the terrain in terrain's CRS
    virtual QgsRectangle extent() const = 0;

    //! Returns bounding box of the root chunk
    virtual QgsAABB rootChunkBbox( const Qgs3DMapSettings &map ) const;

    //! Returns error of the root chunk in world coordinates
    virtual float rootChunkError( const Qgs3DMapSettings &map ) const;

    //! Returns height range of the root chunk in world coordinates
    virtual void rootChunkHeightRange( float &hMin, float &hMax ) const;

    //! Returns height at (x,y) in terrain's CRS
    virtual float heightAt( double x, double y, const Qgs3DMapSettings &map ) const;

    //! Write terrain generator's configuration to XML
    virtual void writeXml( QDomElement &elem ) const = 0;

    //! Read terrain generator's configuration from XML
    virtual void readXml( const QDomElement &elem ) = 0;

    //! After read of XML, resolve references to any layers that have been read as layer IDs
    virtual void resolveReferences( const QgsProject &project ) { Q_UNUSED( project ); }

    //! Converts terrain generator type enumeration into a string
    static QString typeToString( Type type );

    //! Returns tiling scheme of the terrain
    const QgsTilingScheme &tilingScheme() const { return mTerrainTilingScheme; }

    //! Returns CRS of the terrain
    QgsCoordinateReferenceSystem crs() const { return mTerrainTilingScheme.crs(); }

  protected:
    QgsTilingScheme mTerrainTilingScheme;   //!< Tiling scheme of the terrain
    QgsTerrainEntity *mTerrain = nullptr;
};


#endif // QGSTERRAINGENERATOR_H
