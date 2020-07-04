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
#include <QFile>

class QgsTessellatedPolygonGeometry;
class QgsTerrainTileEntity;
class QgsTerrainEntity;
class Qgs3DMapSettings;
class QgsFlatTerrainGenerator;
class QgsDemTerrainGenerator;
class QgsChunkNode;
class QgsExportObject;

class Qgs3DSceneExporter : public Qt3DCore::QEntity
{
    Q_OBJECT
  public:
    Qgs3DSceneExporter( Qt3DCore::QNode *parent = nullptr );

    void parseEntity( Qt3DCore::QEntity *entity );
    void parseEntity( QgsTerrainEntity *terrain );
    void saveToFile( const QString &filePath );

    void setSmoothEdges( bool smoothEdges ) { mSmoothEdges = smoothEdges; }
    bool smoothEdges() { return mSmoothEdges; }

    void setTerrainResolution( int resolution ) { mTerrainResolution = resolution; }
    int terrainResolution() { return mTerrainResolution; }

  private:
    void processAttribute( Qt3DRender::QAttribute *attribute );
    void process( QgsTessellatedPolygonGeometry *geom );

    QgsTerrainTileEntity *getFlatTerrainEntity( QgsTerrainEntity *terrain, QgsChunkNode *node );
    QgsTerrainTileEntity *getDemTerrainEntity( QgsTerrainEntity *terrain, QgsChunkNode *node );

    void parseDemTile( QgsTerrainTileEntity *tileEntity );
    void parseFlatTile( QgsTerrainTileEntity *tileEntity );

    QgsTerrainTileEntity *createDEMTileEntity( QgsTerrainEntity *terrain, QgsChunkNode *node );
  private:
    QVector<QgsExportObject *> mObjects;

    bool mSmoothEdges;
    int mTerrainResolution;
};

#endif // QGS3DSCENEEXPORTER_H
