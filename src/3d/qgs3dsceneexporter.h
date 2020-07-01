/***************************************************************************
  qgs3dsceneexporter.h
  --------------------------------------
  Date                 : June 2020
  Copyright            : (C) 2020 by Belgacem Nedjima
  Email                : gb underscore nedjima at esi dot dz
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DSCENEEXPORTER_H
#define QGS3DSCENEEXPORTER_H

#include <Qt3DCore/QEntity>
#include <Qt3DRender/QAttribute>
#include <Qt3DExtras/QPlaneGeometry>
#include <QMap>

class QgsTessellatedPolygonGeometry;
class QgsTerrainTileEntity;
class QgsTerrainEntity;
class Qgs3DMapSettings;
class QgsFlatTerrainGenerator;
class QgsDemTerrainGenerator;
class QgsChunkNode;

class Qgs3DSceneExporter
{

  public:
    Qgs3DSceneExporter( );

    void parseEntity( Qt3DCore::QEntity *entity );
    void parseEntity( QgsTerrainEntity *terrain );
    void saveToFile( const QString &filePath );
  private:
    void processAttribute( Qt3DRender::QAttribute *attribute );
    void process( QgsTessellatedPolygonGeometry *geom );
    void process( Qt3DExtras::QPlaneGeometry *plane );

    void generateFlatTerrain( QgsTerrainEntity *terrain, QgsChunkNode *node );
    void generateDemTerrain( QgsTerrainEntity *terrain, QgsChunkNode *node );

    void parseDemTile( QgsTerrainTileEntity *tileEntity );
    void parseFlatTile( QgsTerrainTileEntity *tileEntity );

    QgsTerrainTileEntity *createDEMTileEntity( QgsTerrainEntity *terrain, QgsChunkNode *node );
  private:
    QVector<float> mVertxPosition;
    QVector<int> mIndexes;
    QMap<int, QString> mComments;
};

#endif // QGS3DSCENEEXPORTER_H
