#ifndef QUANTIZEDMESHTERRAINGENERATOR_H
#define QUANTIZEDMESHTERRAINGENERATOR_H

#include "qgsterraingenerator.h"


/**
 * \ingroup 3d
 * Terrain generator using downloaded terrain tiles using quantized mesh specification
 * \since QGIS 3.0
 */
class QuantizedMeshTerrainGenerator : public QgsTerrainGenerator
{
  public:
    QuantizedMeshTerrainGenerator();

    //! Determines base tile from map extent
    void setBaseTileFromExtent( const QgsRectangle &extentInTerrainCrs );

    //! Converts tile coordinates (x,y,z) in our quadtree to tile coordinates of quantized mesh tree
    void quadTreeTileToBaseTile( int x, int y, int z, int &tx, int &ty, int &tz ) const;

    QgsTerrainGenerator::Type type() const override;
    QgsRectangle extent() const override;
    virtual void writeXml( QDomElement &elem ) const override;
    virtual void readXml( const QDomElement &elem ) override;

    virtual QgsChunkLoader *createChunkLoader( QgsChunkNode *node ) const override SIP_FACTORY;

    int terrainBaseX, terrainBaseY, terrainBaseZ;   //!< Coordinates of the base tile
};


#endif // QUANTIZEDMESHTERRAINGENERATOR_H
