/***************************************************************************
    quantizedmeshterraingenerator.h
    ---------------------
    begin                : July 2017
    copyright            : (C) 2017 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QUANTIZEDMESHTERRAINGENERATOR_H
#define QUANTIZEDMESHTERRAINGENERATOR_H

#include "qgsterraingenerator.h"


/**
 * \ingroup 3d
 * \brief Terrain generator using downloaded terrain tiles using quantized mesh specification
 * \since QGIS 3.0
 */
class QuantizedMeshTerrainGenerator : public QgsTerrainGenerator
{
    Q_OBJECT

  public:
    QuantizedMeshTerrainGenerator();

    //! Determines base tile from map extent
    void setBaseTileFromExtent( const QgsRectangle &extentInTerrainCrs );

    //! Converts tile coordinates (x,y,z) in our quadtree to tile coordinates of quantized mesh tree
    void quadTreeTileToBaseTile( int x, int y, int z, int &tx, int &ty, int &tz ) const;

    QgsTerrainGenerator::Type type() const override;
    QgsRectangle extent() const override;
    void writeXml( QDomElement &elem ) const override;
    void readXml( const QDomElement &elem ) override;

    QgsChunkLoader *createChunkLoader( QgsChunkNode *node ) const override SIP_FACTORY;

    int terrainBaseX, terrainBaseY, terrainBaseZ;   //!< Coordinates of the base tile
};


#endif // QUANTIZEDMESHTERRAINGENERATOR_H
