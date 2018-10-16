/***************************************************************************
  qgsflatterraingenerator.h
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

#ifndef QGSFLATTERRAINGENERATOR_H
#define QGSFLATTERRAINGENERATOR_H

#include "qgis_3d.h"

#include "qgsterraingenerator.h"
#include "qgsterraintileloader_p.h"
#include "qgsrectangle.h"
#include <Qt3DExtras/QPlaneGeometry>

///@cond PRIVATE

//! Chunk loader for flat terrain implementation
class FlatTerrainChunkLoader : public QgsTerrainTileLoader
{
    Q_OBJECT

  public:
    //! Construct the loader for a node
    FlatTerrainChunkLoader( QgsTerrainEntity *terrain, QgsChunkNode *mNode );

    Qt3DCore::QEntity *createEntity( Qt3DCore::QEntity *parent ) override;

  private:
    Qt3DExtras::QPlaneGeometry *mTileGeometry = nullptr;
};

///@endcond

/**
 * \ingroup 3d
 * Terrain generator that creates a simple square flat area.
 *
 * \since QGIS 3.0
 */
class _3D_EXPORT QgsFlatTerrainGenerator : public QgsTerrainGenerator
{
  public:
    //! Creates flat terrain generator object
    QgsFlatTerrainGenerator() = default;

    QgsChunkLoader *createChunkLoader( QgsChunkNode *node ) const override SIP_FACTORY;

    QgsTerrainGenerator *clone() const override SIP_FACTORY;
    Type type() const override;
    QgsRectangle extent() const override;
    void rootChunkHeightRange( float &hMin, float &hMax ) const override;
    void writeXml( QDomElement &elem ) const override;
    void readXml( const QDomElement &elem ) override;

    //! Sets extent of the terrain
    void setExtent( const QgsRectangle &extent );

    //! Sets CRS of the terrain
    void setCrs( const QgsCoordinateReferenceSystem &crs );
    //! Returns CRS of the terrain
    QgsCoordinateReferenceSystem crs() const { return mCrs; }

  private:

    void updateTilingScheme();

    QgsRectangle mExtent;
    QgsCoordinateReferenceSystem mCrs;
};




#endif // QGSFLATTERRAINGENERATOR_H
